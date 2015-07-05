/*******************************************************************************
The MIT License (MIT)

Copyright (c) 2015 Sergey Shambir

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*******************************************************************************/

#include "gosemkiprocess.h"
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

namespace GoEditor {

static QLatin1String GOSEMKI_COMMAND("gosemki");
static QLatin1String ERROR_JSON_PARSING_FAILED("Cannot parse JSON response. JSON error: %2");
static QLatin1String WARN_UNKNOWN_OUTLINE_KIND("Unknown outline kind '%1'.");
static QLatin1String WARN_UNKNOWN_FORMAT("Unknown highlight range format '%1'.");

GosemkiProcess::GosemkiProcess(const QString &filePath)
    : GoToolProcess(filePath)
{
    initStringMaps();
}

GosemkiProcess::GosemkiProcess(const QString &filePath, const QByteArray &contents)
    : GoToolProcess(filePath, contents)
{
    initStringMaps();
}

QSharedPointer<GoSemanticInfo> GosemkiProcess::collectSemanticInfo()
{
    setToolCommand(GOSEMKI_COMMAND);
    QStringList arguments;
    if (!isFileInMemory())
        arguments << QString::fromLatin1("--in=%1").arg(filePath());
    arguments << QLatin1String("highlight");
    if (isFileInMemory())
        arguments << QFileInfo(filePath()).fileName();

    QSharedPointer<GoSemanticInfo> ret;
    QByteArray response;
    if (runTool(arguments, response)) {
        parseJson(response);
        m_sema->sort();
        ret.swap(m_sema);
    }
    return ret;
}

void GosemkiProcess::initStringMaps()
{
    m_outlineKindsMap[QLatin1String("fun")] = GoOutlineItem::Func;
    m_outlineKindsMap[QLatin1String("typ")] = GoOutlineItem::Struct;
    m_formatsMap[QLatin1String("fun")] = GoHighlightRange::Func;
    m_formatsMap[QLatin1String("lbl")] = GoHighlightRange::Label;
    m_formatsMap[QLatin1String("typ")] = GoHighlightRange::Type;
    m_formatsMap[QLatin1String("var")] = GoHighlightRange::Var;
    m_formatsMap[QLatin1String("con")] = GoHighlightRange::Const;
    m_formatsMap[QLatin1String("pkg")] = GoHighlightRange::Package;
    m_formatsMap[QLatin1String("fld")] = GoHighlightRange::Field;
}

void GosemkiProcess::parseJson(const QByteArray &response)
{
    const QString KEY_LINE(QLatin1String("lin"));
    const QString KEY_COLUMN(QLatin1String("col"));
    const QString KEY_LENGTH(QLatin1String("len"));
    const QString KEY_KIND(QLatin1String("knd"));
    const QString KEY_MESSAGE(QLatin1String("msg"));
    const QString KEY_FROM(QLatin1String("from"));
    const QString KEY_TO(QLatin1String("to"));
    const QString KEY_TITLE_STRING(QLatin1String("str"));
    const QString KEY_RANGES(QLatin1String("Ranges"));
    const QString KEY_OUTLINE(QLatin1String("Outline"));
    const QString KEY_ERRORS(QLatin1String("Errors"));
    const QString KEY_FOLDS(QLatin1String("Folds"));
    m_sema.reset(new GoSemanticInfo);

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(response, &error);
    if (error.error != QJsonParseError::NoError) {
        reportError(QString(ERROR_JSON_PARSING_FAILED).arg(error.errorString()));
        return;
    }
    QJsonObject root = doc.object();
    foreach (QJsonValue resultValue, root.value(KEY_RANGES).toArray()) {
        QJsonObject result = resultValue.toObject();
        GoHighlightRange range;
        range.format = formatFromString(result.value(KEY_KIND).toString());
        range.line = result.value(KEY_LINE).toInt();
        range.column = result.value(KEY_COLUMN).toInt();
        range.length = result.value(KEY_LENGTH).toInt();
        m_sema->addRange(range);
    }
    foreach (QJsonValue value, root.value(KEY_OUTLINE).toArray()) {
        QJsonObject result = value.toObject();
        GoOutlineItem item;
        item.kind = outlineKindFromString(result.value(KEY_KIND).toString());
        item.line = result.value(KEY_LINE).toInt();
        item.column = result.value(KEY_COLUMN).toInt();
        item.title = result.value(KEY_TITLE_STRING).toString();
        m_sema->addOutlineItem(item);
    }
    foreach (QJsonValue resultValue, root.value(KEY_ERRORS).toArray()) {
        QJsonObject result = resultValue.toObject();
        GoError err;
        err.message = result.value(KEY_MESSAGE).toString();
        err.line = result.value(KEY_LINE).toInt();
        err.column = result.value(KEY_COLUMN).toInt();
        err.length = result.value(KEY_LENGTH).toInt();
        m_sema->addError(err);
    }
    foreach (QJsonValue value, root.value(KEY_FOLDS).toArray()) {
        QJsonObject result = value.toObject();
        GoFoldArea area;
        area.lineFrom = result.value(KEY_FROM).toInt();
        area.lineTo = result.value(KEY_TO).toInt();
        m_sema->addFoldArea(area);
    }
}

GoHighlightRange::Format GosemkiProcess::formatFromString(const QString &string) const
{
    auto it = m_formatsMap.find(string);
    if (it == m_formatsMap.end()) {
        reportError(QString(WARN_UNKNOWN_FORMAT).arg(string));
        return GoHighlightRange::Other;
    }
    return it.value();
}

GoOutlineItem::Kind GosemkiProcess::outlineKindFromString(const QString &string) const
{
    auto it = m_outlineKindsMap.find(string);
    if (it == m_outlineKindsMap.end()) {
        reportError(QString(WARN_UNKNOWN_OUTLINE_KIND).arg(string));
        return GoOutlineItem::Func;
    }
    return it.value();
}

} // namespace GoEditor
