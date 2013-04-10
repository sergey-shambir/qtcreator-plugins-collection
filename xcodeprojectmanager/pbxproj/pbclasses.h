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

#ifndef XCODEPROJECTMANAGER_ANNOTATEDKEY_H
#define XCODEPROJECTMANAGER_ANNOTATEDKEY_H

#include <QString>
#include <QMap>
#include <QSharedPointer>
#include <QStringList>

namespace XCodeProjectManager {

class PBKey;
class PBArray;
class PBString;
class PBDictionary;
class PBObject;

class PBValue
{
public:
    explicit PBValue();
    explicit PBValue(PBObject *ptr);

    int count() const;
    PBValue valueForKey(const PBKey &key) const;
    PBValue valueForKey(const QString &key) const;
    PBValue valueAtIndex(int index) const;
    QString text() const;
    QString key() const;

    PBArray *asArray() const;
    PBString *asString() const;
    PBDictionary *asDict() const;
    PBKey *asKey() const;

    bool isNull() const;

    QString repr(int indent = 0) const;

private:
    QSharedPointer<PBObject> d;
};

class PBObject
{
//    PBObject(const PBObject &);
//    PBObject &operator =(const PBObject &);
public:
    PBObject();
    virtual ~PBObject();

    virtual PBArray *asArray();
    virtual PBString *asString();
    virtual PBDictionary *asDict();
    virtual PBKey *asKey();
    virtual QString repr(int indent = 0) const = 0;
    virtual int count() const;
};

/// @brief Key for PBDictionary
class PBKey : public PBObject
{
public:
    virtual PBKey *asKey();

    PBKey() : maybeMD5(false) {}

    QString key;
    QString comment;
    bool maybeMD5;

    bool operator ==(const PBKey &o) const;
    bool operator !=(const PBKey &o) const;
    bool operator <(const PBKey &o) const;
    bool operator >(const PBKey &o) const;

    QString repr(int indent = 0) const;
};

class PBString : public PBObject
{
public:
    PBString *asString();
    QString text;

    QString repr(int indent = 0) const;
};

class PBDictionary : public PBObject
{
public:
    typedef QMap<PBKey, PBValue>::iterator iterator;
    typedef QMap<PBKey, PBValue>::const_iterator const_iterator;

    PBDictionary *asDict();

    /// can modify dictionary
    PBValue &operator [](const PBKey &key);
    PBValue &operator [](const QString &key);

    /// cannot modify dictionary
    PBValue value(const PBKey &key);
    PBValue value(const QString &key);

    iterator begin();
    const_iterator begin() const;
    iterator end();
    const_iterator end() const;

    QString repr(int indent = 0) const;
    virtual int count() const;
    QStringList allKeys() const;

private:
    QMap<PBKey, PBValue> m_data;
};

class PBArray : public PBObject
{
public:
    PBArray *asArray();
    PBValue &operator [](int index);
    const PBValue &operator [](int index) const;

    QString repr(int indent = 0) const;
    virtual int count() const;

    void append(const PBValue &value);

private:
    QList<PBValue> m_data;
};

} // namespace XCodeProjectManager

#endif // XCODEPROJECTMANAGER_ANNOTATEDKEY_H
