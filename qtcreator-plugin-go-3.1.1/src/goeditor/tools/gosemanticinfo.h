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

#pragma once
#include <QVector>
#include <QString>
#include <QSharedPointer>

class QTextDocument;

namespace TextEditor { class BaseTextEditorWidget; }

namespace GoEditor {

struct GoPosition
{
    int line = 1;       // 1-based
    int column = 1;     // 1-based

    quint64 encodePosition() const;
    void decodePosition(quint64 pos);
};

struct GoFoldArea
{
    int lineFrom = 1;   // 1-based
    int lineTo = 1;     // 1-based
};

struct GoOutlineItem : public GoPosition
{
    enum Kind {
        Func,
        Struct
    };

    QString title;
    Kind kind = Func;
};

struct GoError : public GoPosition
{
    int length = 0;
    QString message;
};

struct GoHighlightRange : public GoPosition
{
    enum Format {
        Field,
        Func,
        Label,
        Type,
        Var,
        Const,
        Package,
        Other
    };

    Format format = Other;
    int length = 0;
};

class GoSemanticInfo
{
public:
    void applyCodeFolding(QTextDocument &document);
    void displayDiagnostic(TextEditor::BaseTextEditorWidget *editor);
    void sort();

    QVector<GoFoldArea> foldAreas() const;
    void setFoldAreas(const QVector<GoFoldArea> &foldAreas);
    void addFoldArea(const GoFoldArea &area);

    QVector<GoOutlineItem> outlineItems() const;
    void setOutlineItems(const QVector<GoOutlineItem> &outlineItems);
    void addOutlineItem(const GoOutlineItem &item);

    QVector<GoError> errors() const;
    void setErrors(const QVector<GoError> &errors);
    void addError(const GoError &error);

    QVector<GoHighlightRange> ranges() const;
    void setRanges(const QVector<GoHighlightRange> &ranges);
    void addRange(const GoHighlightRange &range);

private:
    QVector<GoFoldArea> m_foldAreas;
    QVector<GoOutlineItem> m_outlineItems;
    QVector<GoError> m_errors;
    QVector<GoHighlightRange> m_ranges;
};

typedef QSharedPointer<GoSemanticInfo> GoSemanticInfoPtr;

} // namespace GoEditor
