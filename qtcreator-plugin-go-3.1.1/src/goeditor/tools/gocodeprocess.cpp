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

#include "gocodeprocess.h"
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

namespace GoEditor {

static QLatin1String WARN_UNKNOWN_CLASS("Unknown completion class '%1'.");
static QLatin1String ERROR_JSON_PARSING_FAILED("Cannot parse JSON response. JSON error: %2");
static QLatin1String PANIC_COMPLETION_CLASS("PANIC");
static QLatin1String GOCODE_COMMAND("gocode");

GocodeProcess::GocodeProcess(const QString &filePath)
    : GoToolProcess(filePath)
{
    initStringMaps();
}

GocodeProcess::GocodeProcess(const QString &filePath, const QByteArray &contents)
    : GoToolProcess(filePath, contents)
{
    initStringMaps();
}

QList<CodeCompletion> GocodeProcess::codeCompleteAt(quint64 offset)
{
    setToolCommand(GOCODE_COMMAND);
    QStringList arguments;
    arguments << QLatin1String("-f=json");
    if (!isFileInMemory())
        arguments << QString::fromLatin1("--in=%1").arg(filePath());
    arguments << QLatin1String("autocomplete");
    arguments << QFileInfo(filePath()).fileName();
    arguments << QString::number(offset);

    QList<CodeCompletion> ret;
    QByteArray response;
    if (runTool(arguments, response)) {
        parseJson(response);
        ret.swap(m_completions);
    }
    return ret;
}

void GocodeProcess::initStringMaps()
{
    m_kindsMap[QLatin1String("func")] = CodeCompletion::Func;
    m_kindsMap[QLatin1String("package")] = CodeCompletion::Package;
    m_kindsMap[QLatin1String("var")] = CodeCompletion::Variable;
    m_kindsMap[QLatin1String("type")] = CodeCompletion::Type;
    m_kindsMap[QLatin1String("const")] = CodeCompletion::Const;
}

void GocodeProcess::parseJson(const QByteArray &response)
{
    m_completions.clear();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(response, &error);
    if (error.error != QJsonParseError::NoError) {
        reportError(QString(ERROR_JSON_PARSING_FAILED).arg(error.errorString()));
        return;
    }
    QJsonArray items = doc.array();
    if (items.size() != 2) {
        return;
    }
    QJsonArray results = items.at(1).toArray();
    foreach (QJsonValue resultValue, results) {
        QJsonObject result = resultValue.toObject();
        QString kind = result.value(QLatin1String("class")).toString();
        CodeCompletion completion;
        completion.kind = kindFromString(kind);
        completion.text = result.value(QLatin1String("name")).toString();
        completion.hint = result.value(QLatin1String("type")).toString();
        if (completion.hint.startsWith(kind)) {
            completion.hint = kind + QLatin1String(" ") + completion.text
                    + completion.hint.mid(kind.size());
        }

        if (!completion.text.isEmpty())
            m_completions.append(completion);
    }
}

CodeCompletion::Kind GocodeProcess::kindFromString(const QString &string) const
{
    auto it = m_kindsMap.find(string);
    if (it == m_kindsMap.end()) {
        if (string == PANIC_COMPLETION_CLASS) {
            reportError(QString(WARN_UNKNOWN_CLASS).arg(string));
        }
        return CodeCompletion::Other;
    }
    return it.value();
}

} // namespace GoEditor
