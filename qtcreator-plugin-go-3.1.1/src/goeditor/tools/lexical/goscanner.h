#ifndef GOEDITOR_GOSIMPLELEXER_H
#define GOEDITOR_GOSIMPLELEXER_H

#include "../../goeditor_global.h"
#include <QSet>
#include <pythoneditor/tools/lexical/sourcecodestream.h>

namespace GoEditor {
namespace Internal {

enum Format {
    Format_Number = 0,
    Format_Rune, // Rune is character constant, surrounded by ''.
    Format_String,
    Format_Keyword,
    Format_Comment,
    Format_Identifier,
    Format_Whitespace,
    Format_PredeclaratedType,
    Format_PredeclaratedConst,
    Format_PredeclaratedFunction,
    Format_Operator,

    Format_FormatsAmount,
    Format_EndOfBlock
};

class FormatToken
{
public:
    FormatToken() {}

    FormatToken(Format format, size_t position, size_t length)
        : m_format(format)
        , m_position(position)
        , m_length(length)
    {}

    inline Format format() const { return m_format; }
    inline int begin() const { return m_position; }
    inline int end() const { return m_position + m_length; }
    inline int length() const { return m_length; }

private:
    Format m_format;
    int m_position;
    int m_length;
};

class GoScanner
{
    Q_DISABLE_COPY(GoScanner)
public:
    enum State {
        State_Default = 0,
        State_MultiLineComment,
        State_MultiLineString
    };

    GoScanner(const QChar *text, const int length);
    ~GoScanner();

    void setState(int state);
    int state() const;
    FormatToken read();
    QString value(const FormatToken& tk) const;

private:
    FormatToken onDefaultState();

    FormatToken readStringLiteral(QChar quoteChar);
    FormatToken readMultiLineComment();
    FormatToken readMultiLineString();
    FormatToken readIdentifier();
    FormatToken readNumber();
    FormatToken readFloatNumber();
    FormatToken readComment();
    FormatToken readWhiteSpace();
    FormatToken readOperator();

    void clearState();
    void saveState(State state, QChar savedData);
    void parseState(State &state, QChar &savedData) const;

    PythonEditor::Internal::SourceCodeStream m_src;
    int m_state;
    const QSet<QString> m_keywords;
    const QSet<QString> m_types;
    const QSet<QString> m_consts;
    const QSet<QString> m_funcs;
};

} // namespace GoEditor
} // namespace Internal

#endif // GOEDITOR_GOSIMPLELEXER_H
