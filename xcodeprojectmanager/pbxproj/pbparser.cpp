/*
 * Copyright (c) 2013 Sergey Shambir
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "pbparser.h"
#include "pblexer.h"

#include <QFile>
#include <QVariant>

#ifdef DEBUG_PBPROJ
#include <QDebug>
#endif

#ifndef __PRETTY_FUNCTION__
#define __PRETTY_FUNCTION__ __FUNCTION__
#endif

/**
 * @class XCodeProjectManager::OpenStepReader
 */

namespace {
using namespace XCodeProjectManager;

void skipUnexpected(Lexer &lexer, const QByteArray &functionName);
void testToken(Lexer &lexer, Token::Kind required, const QByteArray &functionName);
PBKey parseKey(Lexer &lexer);
PBValue parseValue(Lexer &lexer);
PBValue parseArray(Lexer &lexer);
PBValue parseDict(Lexer &lexer);
PBValue parsePBProj(const QByteArray &sources);
QByteArray readPBProjectFile(const QString &path);
PBValue parsePBProj(const QByteArray &source);
QByteArray readPBProjectFile(const QString &path);
bool acceptsPBProjectRootDictionary(const PBValue &dictionary);
bool hasMD5KeysOnly(const PBDictionary *dictionary);

void skipUnexpected(Lexer &lexer, const QByteArray &functionName)
{
#ifdef DEBUG_PBPROJ
    qDebug() << "In" << functionName << ":";
    qDebug() << "Unexpected token, kind =" << int(lexer.tokenKind())
             << ", text =" << lexer.token().text
             << ", pos = (" << lexer.line() << ", " << lexer.column() << ")";
#else
    Q_UNUSED(functionName);
#endif
    lexer.readNext();
}

void testToken(Lexer &lexer, Token::Kind required,
                      const QByteArray &functionName)
{
#ifdef DEBUG_PBPROJ
    if (lexer.tokenKind() == required)
        return;
    qDebug() << "In" << functionName << ":";
    qDebug() << "Token requirement not satisfied =" << int(lexer.tokenKind())
             << ", text =" << lexer.token().text
             << ", pos = (" << lexer.line() << ", " << lexer.column() << ")";
#else
    Q_UNUSED(lexer);
    Q_UNUSED(required);
    Q_UNUSED(functionName);
#endif
}

/// pre: key token consumed
/// post: equal consumed
PBKey parseKey(Lexer &lexer)
{
    PBKey key;
    if (lexer.token().maybeMD5) {
        key.maybeMD5 = true;
        key.key = lexer.token().text;
        lexer.readNext();
        if (lexer.tokenKind() == Token::Comment) {
            key.comment = lexer.token().text;
            lexer.readNext();
        }
    } else if (lexer.tokenKind() == Token::Name
               || lexer.tokenKind() == Token::String) {
        key.key = lexer.token().text;
        lexer.readNext();
    }

    while (lexer.tokenKind() != Token::Equal && lexer.tokenKind() != Token::EndOfFile)
        skipUnexpected(lexer, __PRETTY_FUNCTION__);
    return key;
}

/// pre: first token of value consumed
/// post: token after value consumed
PBValue parseValue(Lexer &lexer)
{
    while (!lexer.tokenKind() == Token::EndOfFile) {
        if (lexer.tokenKind() == Token::LeftBrace) {
            return parseDict(lexer);
        }
        if (lexer.tokenKind() == Token::LeftParen) {
            return parseArray(lexer);
        }
        if (lexer.token().maybeMD5) {
            QString text = lexer.token().text;
            lexer.readNext();
            if (lexer.tokenKind() == Token::Comment) {
                PBKey *key = new PBKey;
                key->key = text;
                key->maybeMD5 = true;
                key->comment = lexer.token().text;
                lexer.readNext();
                return PBValue(key);
            }
            PBString *string = new PBString();
            string->text = text;
            return PBValue(string);
        }
        if (lexer.tokenKind() == Token::Name
                   || lexer.tokenKind() == Token::String) {
            PBString *string = new PBString();
            string->text = lexer.token().text;
            lexer.readNext();
            return PBValue(string);
        }
        skipUnexpected(lexer, __PRETTY_FUNCTION__);
    }

    return PBValue();
}

/// pre: '(' consumed
/// post: ')' consumed
PBValue parseArray(Lexer &lexer)
{
    testToken(lexer, Token::LeftParen, __PRETTY_FUNCTION__);
    lexer.readNext();
    PBArray *ret = new PBArray;

    while (lexer.tokenKind() != Token::EndOfFile && lexer.tokenKind() != Token::RightParen) {
        switch (lexer.tokenKind()) {
        case Token::Comment:
            lexer.readNext();
            continue;
        case Token::Equal:
        case Token::RightBrace:
        case Token::Semicolon:
        case Token::Comma:
            skipUnexpected(lexer, __PRETTY_FUNCTION__);
            continue;
        default:
            break;
        }

        ret->append(parseValue(lexer));
        while (lexer.tokenKind() != Token::Comma && lexer.tokenKind() != Token::RightParen && lexer.tokenKind() != Token::EndOfFile)
            lexer.readNext();// TODO: skipUnexpected(lexer, __PRETTY_FUNCTION__);
        lexer.readNext();
    }
    lexer.readNext();
    return PBValue(ret);
}

/// pre: '{' consumed
/// post: '}' consumed
PBValue parseDict(Lexer &lexer)
{
    testToken(lexer, Token::LeftBrace, __PRETTY_FUNCTION__);
    lexer.readNext();

    PBDictionary *ret = new PBDictionary;
    while (lexer.tokenKind() != Token::EndOfFile && lexer.tokenKind() != Token::RightBrace) {
        switch (lexer.tokenKind()) {
        case Token::Comment:
            lexer.readNext();
            continue;
        case Token::Equal:
        case Token::RightParen:
        case Token::LeftParen:
        case Token::LeftBrace:
        case Token::Semicolon:
        case Token::Comma:
            skipUnexpected(lexer, __PRETTY_FUNCTION__);
            continue;
        default:
            break;
        }

        PBKey key = parseKey(lexer);
        lexer.readNext();
        (*ret)[key] = parseValue(lexer);
        while (lexer.tokenKind() != Token::Semicolon && lexer.tokenKind() != Token::EndOfFile)
            lexer.readNext(); // TODO: skipUnexpected(lexer, __PRETTY_FUNCTION__);
        lexer.readNext(); // consume ';'
    }
    lexer.readNext();
    return PBValue(ret);
}

PBValue parsePBProj(const QByteArray &source)
{
    Lexer lexer(source);
    while (lexer.readNext())
        if (lexer.tokenKind() == Token::LeftBrace) {
            return parseDict(lexer);
        } else if (lexer.tokenKind() != Token::Comment) {
            break;
        }
    return PBValue();
}

QByteArray readPBProjectFile(const QString &path)
{
    QFile file(path);
    if (!file.exists())
        return QByteArray();
    file.open(QFile::ReadOnly | QFile::Text);
    return file.readAll();
}

#ifdef DEBUG_PBPROJ
inline static bool semaFailed(const QString &error)
{
    qDebug() << "Semantic error: " << error;
    return false;
}
#else
#define semaFailed(x) false
#endif

/// @return false if semantic check failed for dictionary
bool acceptsPBProjectRootDictionary(const PBValue &dictionary)
{
    PBDictionary *dict = dictionary.asDict();
    if (!dict)
        return false;

    bool ok = dict->value(QLatin1String("archiveVersion")).text() == QLatin1String("1")
            && dict->value(QLatin1String("objectVersion")).text() == QLatin1String("46")
            && dict->value(QLatin1String("classes")).asDict()
            && dict->value(QLatin1String("objects")).asDict()
            && dict->value(QLatin1String("rootObject")).asKey();
    if (!ok) {
        return semaFailed(QLatin1String("Dict is not valid root object: ") + dict->allKeys().join(QLatin1String(", ")));
    }

    if (!hasMD5KeysOnly(dict->value(QLatin1String("classes")).asDict()))
        return false;
    if (!hasMD5KeysOnly(dict->value(QLatin1String("objects")).asDict()))
        return false;

    return true;
}

bool hasMD5KeysOnly(const PBDictionary *d)
{
    for (PBDictionary::const_iterator i = d->begin(), end = d->end(); i != end; ++i) {
        if (!i.key().maybeMD5)
            return semaFailed(QLatin1String("key is not md5: ") + i.key().key);
        if (PBDictionary *nested = i.value().asDict()) {
            if (nested->value(QLatin1String("isa")).isNull()) {
                return semaFailed(QLatin1String("value have no `isa' key: ") + i.value().repr());
            } else {
                PBValue isa = nested->value(QLatin1String("isa"));
                QStringList accepted = QStringList()
                        << QLatin1String("PBXGroup")
                        << QLatin1String("PBXBuildFile")
                        << QLatin1String("PBXFileReference")
                        << QLatin1String("PBXSourcesBuildPhase")
                        << QLatin1String("PBXFrameworksBuildPhase")
                        << QLatin1String("PBXHeadersBuildPhase")
                        << QLatin1String("PBXResourcesBuildPhase")
                        << QLatin1String("PBXNativeTarget")
                        << QLatin1String("XCConfigurationList")
                        << QLatin1String("XCBuildConfiguration")
                        << QLatin1String("PBXVariantGroup")
                        << QLatin1String("PBXShellScriptBuildPhase")
                        << QLatin1String("PBXCopyFilesBuildPhase")
                        << QLatin1String("PBXProject");
                if (!accepted.contains(isa.repr()))
                    return semaFailed(QLatin1String("invalid `isa' value: ") + isa.repr());
            }
        } else {
            return semaFailed(QLatin1String("value is not dictionary: ") + i.value().repr());
        }
    }
    return true;
}

} // anonymous namespace

namespace XCodeProjectManager {

PBProjectModel::Ptr loadXCodeProjectModel(const QString &path)
{
    PBValue root = parsePBProj(readPBProjectFile(path));;
    if (acceptsPBProjectRootDictionary(root))
        return QSharedPointer<PBProjectModel>(new PBProjectModel(root));
    return QSharedPointer<PBProjectModel>();
}

} // namespace XCodeProjectManager
