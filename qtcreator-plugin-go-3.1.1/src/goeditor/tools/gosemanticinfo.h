#ifndef GOEDITOR_GOSEMANTICINFO_H
#define GOEDITOR_GOSEMANTICINFO_H

#include <QVector>
#include <QString>
#include <QSharedPointer>

class QTextDocument;

namespace TextEditor { class BaseTextEditorWidget; }

namespace GoEditor {

struct GoFoldArea
{
public:
    int lineFrom = 1;   // 1-based
    int lineTo = 1;     // 1-based
};

struct GoOutlineItem
{
public:
    enum Kind {
        Func,
        Struct
    };

    int line = 1;       // 1-based
    int column = 1;     // 1-based
    QString title;
    Kind kind = Func;
};

struct GoError
{
public:
    int line = 1;       // 1-based
    int column = 1;     // 1-based
    int length = 0;
    QString message;
};

struct GoHighlightRange {
    enum Format {
        Error,
        Field,
        Func,
        Label,
        Type,
        Var,
        Const,
        Package,
        Other
    };

    Format format;
    int line;
    int column;
    int length;
};

class GoSemanticInfo
{
public:
    void applyCodeFolding(QTextDocument *document);
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

#endif // GOEDITOR_GOSEMANTICINFO_H
