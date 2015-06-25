#include "goscanner.h"
#include "goeditorplugin.h"

namespace GoEditor {
namespace Internal {

/**
 * @class GoEditor::Internal::GoScanner
 * @brief Splits block of code to text formatting tokens.
 *
 * Go lang has multi-line comments.
 * Go lang does not append second line to first when '\' placed on end of first line.
 */

GoScanner::GoScanner(const QChar *text, const int length)
    : m_src(text, length)
    , m_state(0)
    , m_keywords(GoEditorPlugin::goKeywords())
    , m_types(GoEditorPlugin::goPredeclaratedTypes())
    , m_consts(GoEditorPlugin::goPredeclaratedConsts())
    , m_funcs(GoEditorPlugin::goPredeclaratedFuncs())
{
}

GoScanner::~GoScanner()
{
}

void GoScanner::setState(int state)
{
    m_state = state;
}

int GoScanner::state() const
{
    return m_state;
}

FormatToken GoScanner::read()
{
    m_src.setAnchor();
    if (m_src.isEnd())
        return FormatToken(Format_EndOfBlock, m_src.anchor(), 0);

    State state;
    QChar saved;
    parseState(state, saved);
    switch (state) {
    case State_MultiLineComment:
        return readMultiLineComment();
    case State_MultiLineString:
        return readMultiLineString();
    default:
        return onDefaultState();
    }
}

QString GoScanner::value(const FormatToken &tk) const
{
    return m_src.value(tk.begin(), tk.length());
}

FormatToken GoScanner::onDefaultState()
{
    QChar first = m_src.peek();
    m_src.move();

    if (first == QLatin1Char('\\') && m_src.peek() == QLatin1Char('\n')) {
        m_src.move();
        return FormatToken(Format_Whitespace, m_src.anchor(), 2);
    }

    if (first == QLatin1Char('.') && m_src.peek().isDigit())
        return readFloatNumber();

    if (first == QLatin1Char('\'') || first == QLatin1Char('\"'))
        return readStringLiteral(first);

    if (first == QLatin1Char('`')) {
        m_src.move();
        saveState(State_MultiLineString, QChar());
        return readMultiLineString();
    }

    if (first.isLetter() || (first == QLatin1Char('_')))
        return readIdentifier();

    if (first.isDigit())
        return readNumber();

    if (first == QLatin1Char('/') && m_src.peek() == QLatin1Char('/')) {
        return readComment();
    }

    if (first == QLatin1Char('/') && m_src.peek() == QLatin1Char('*')) {
        m_src.move();
        saveState(State_MultiLineComment, QChar());
        return readMultiLineComment();
    }

    if (first.isSpace())
        return readWhiteSpace();

    return readOperator();
}

/**
  reads single-line string literal, surrounded by ' or " quotes
  */
FormatToken GoScanner::readStringLiteral(QChar quoteChar)
{
    QChar ch = m_src.peek();

    while (ch != quoteChar && !ch.isNull()) {
        m_src.move();
        if (ch == QLatin1Char('\\'))
            m_src.move();
        ch = m_src.peek();
    }
    if (ch == quoteChar)
        clearState();
    m_src.move();
    return FormatToken(Format_String, m_src.anchor(), m_src.length());
}

/**
  reads C-style multi-line comment.
  */
FormatToken GoScanner::readMultiLineComment()
{
    for (;;) {
        QChar ch = m_src.peek();
        if (ch.isNull())
            break;
        if (ch == QLatin1Char('*') && (m_src.peek(1) == QLatin1Char('/'))) {
            clearState();
            m_src.move(); // TODO: is necessary?
            m_src.move();
            m_src.move();
            break;
        }
        m_src.move();
    }

    return FormatToken(Format_String, m_src.anchor(), m_src.length());
}

FormatToken GoScanner::readMultiLineString()
{
    for (;;) {
        QChar ch = m_src.peek();
        if (ch.isNull()) {
            break;
        }
        if (ch == QLatin1Char('`')) {
            clearState();
            m_src.move(); // TODO: is necessary?
            m_src.move();
            break;
        }
        m_src.move();
    }

    return FormatToken(Format_String, m_src.anchor(), m_src.length());
}

/**
  reads identifier and classifies it
  */
FormatToken GoScanner::readIdentifier()
{
    QChar ch = m_src.peek();
    while (ch.isLetterOrNumber() || (ch == QLatin1Char('_'))) {
        m_src.move();
        ch = m_src.peek();
    }
    QString value = m_src.value();

    Format tkFormat = Format_Identifier;
    if (m_keywords.contains(value))
        tkFormat = Format_Keyword;
    else if (m_consts.contains(value))
        tkFormat = Format_PredeclaratedConst;
    else if (m_types.contains(value))
        tkFormat = Format_PredeclaratedType;
    else if (m_funcs.contains(value))
        tkFormat = Format_PredeclaratedFunction;

    return FormatToken(tkFormat, m_src.anchor(), m_src.length());
}

inline static bool isHexDigit(QChar ch)
{
    return (ch.isDigit()
            || (ch >= QLatin1Char('a') && ch <= QLatin1Char('f'))
            || (ch >= QLatin1Char('A') && ch <= QLatin1Char('F')));
}

inline static bool isOctalDigit(QChar ch)
{
    return (ch.isDigit() && ch != QLatin1Char('8') && ch != QLatin1Char('9'));
}

inline static bool isBinaryDigit(QChar ch)
{
    return (ch == QLatin1Char('0') || ch == QLatin1Char('1'));
}

inline static bool isValidIntegerSuffix(QChar ch)
{
    return (ch == QLatin1Char('l') || ch == QLatin1Char('L'));
}

inline static bool isValidComplexSuffix(QChar ch)
{
    return (ch == QLatin1Char('j') || ch == QLatin1Char('J'));
}

FormatToken GoScanner::readNumber()
{
    if (!m_src.isEnd()) {
        QChar ch = m_src.peek();
        if (ch.toLower() == QLatin1Char('b')) {
            m_src.move();
            while (isBinaryDigit(m_src.peek()))
                m_src.move();
        } else if (ch.toLower() == QLatin1Char('o')) {
            m_src.move();
            while (isOctalDigit(m_src.peek()))
                m_src.move();
        } else if (ch.toLower() == QLatin1Char('x')) {
            m_src.move();
            while (isHexDigit(m_src.peek()))
                m_src.move();
        } else { // either integer or float number
            return readFloatNumber();
        }
        if (isValidIntegerSuffix(m_src.peek()))
            m_src.move();
    }
    return FormatToken(Format_Number, m_src.anchor(), m_src.length());
}

FormatToken GoScanner::readFloatNumber()
{
    enum
    {
        State_INTEGER,
        State_FRACTION,
        State_EXPONENT
    } state;
    state = (m_src.peek(-1) == QLatin1Char('.')) ? State_FRACTION : State_INTEGER;

    for (;;) {
        QChar ch = m_src.peek();
        if (ch.isNull())
            break;

        if (state == State_INTEGER) {
            if (ch == QLatin1Char('.'))
                state = State_FRACTION;
            else if (!ch.isDigit())
                break;
        } else if (state == State_FRACTION) {
            if (ch == QLatin1Char('e') || ch == QLatin1Char('E')) {
                QChar next = m_src.peek(1);
                QChar next2 = m_src.peek(2);
                bool isExp = next.isDigit()
                        || (((next == QLatin1Char('-')) || (next == QLatin1Char('+'))) && next2.isDigit());
                if (isExp) {
                    m_src.move();
                    state = State_EXPONENT;
                } else {
                    break;
                }
            } else if (!ch.isDigit()) {
                break;
            }
        } else if (!ch.isDigit()) {
            break;
        }
        m_src.move();
    }

    QChar ch = m_src.peek();
    if ((state == State_INTEGER && (ch == QLatin1Char('l') || ch == QLatin1Char('L')))
            || (ch == QLatin1Char('j') || ch == QLatin1Char('J')))
        m_src.move();

    return FormatToken(Format_Number, m_src.anchor(), m_src.length());
}

/**
  reads single-line python comment, started with "#"
  */
FormatToken GoScanner::readComment()
{
    QChar ch = m_src.peek();
    while (ch != QLatin1Char('\n') && !ch.isNull()) {
        m_src.move();
        ch = m_src.peek();
    }
    return FormatToken(Format_Comment, m_src.anchor(), m_src.length());
}

/**
  reads whitespace
  */
FormatToken GoScanner::readWhiteSpace()
{
    while (m_src.peek().isSpace())
        m_src.move();
    return FormatToken(Format_Whitespace, m_src.anchor(), m_src.length());
}

/**
  reads punctuation symbols, excluding some special
  */
FormatToken GoScanner::readOperator()
{
    const QString EXCLUDED_CHARS = QLatin1String("\'\"_#");
    QChar ch = m_src.peek();
    while (ch.isPunct() && !EXCLUDED_CHARS.contains(ch)) {
        m_src.move();
        ch = m_src.peek();
    }
    return FormatToken(Format_Operator, m_src.anchor(), m_src.length());
}

void GoScanner::clearState()
{
    m_state = 0;
}

void GoScanner::saveState(State state, QChar savedData)
{
    m_state = (state << 16) | static_cast<int>(savedData.unicode());
}

void GoScanner::parseState(State &state, QChar &savedData) const
{
    state = static_cast<State>(m_state >> 16);
    savedData = static_cast<ushort>(m_state);
}

} // namespace GoEditor
} // namespace Internal
