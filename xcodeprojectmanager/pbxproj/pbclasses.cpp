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

#include "pbclasses.h"

namespace XCodeProjectManager {

PBKey *PBKey::asKey()
{
    return this;
}

bool PBKey::operator ==(const PBKey &o) const
{
    return key == o.key;
}

bool PBKey::operator !=(const PBKey &o) const
{
    return key != o.key;
}

bool PBKey::operator <(const PBKey &o) const
{
    return key < o.key;
}

bool PBKey::operator >(const PBKey &o) const
{
    return key > o.key;
}

QString PBKey::repr(int indent) const
{
    Q_UNUSED(indent)
    if (comment.isEmpty())
        return key;
    return QString::fromLatin1("%1 /*%2*/").arg(key).arg(comment);
}

PBValue::PBValue()
{
}

PBValue::PBValue(PBObject *ptr)
    : d(ptr)
{
}

int PBValue::count() const
{
    if (d)
        return d->count();
    return 0;
}

PBValue PBValue::valueForKey(const PBKey &key) const
{
    if (d)
        if (PBDictionary *dict = d->asDict())
            return dict->value(key);
    return PBValue();
}

PBValue PBValue::valueForKey(const QString &key) const
{
    if (d)
        if (PBDictionary *dict = d->asDict())
            return dict->value(key);
    return PBValue();
}

PBValue PBValue::valueAtIndex(int index) const
{
    if (d)
        if (PBArray *array = d->asArray())
            return (*array)[index];
    return PBValue();
}

QString PBValue::text() const
{
    if (d)
        if (PBString *s = d->asString())
            return s->text;
    return QString();
}

QString PBValue::key() const
{
    if (d)
        if (PBKey *s = d->asKey())
            return s->key;
    return QString();
}

PBArray *PBValue::asArray() const
{
    if (d)
        return d->asArray();
    return 0;
}

PBString *PBValue::asString() const
{
    if (d)
        return d->asString();
    return 0;
}

PBDictionary *PBValue::asDict() const
{
    if (d)
        return d->asDict();
    return 0;
}

PBKey *PBValue::asKey() const
{
    if (d)
        return d->asKey();
    return 0;
}

bool PBValue::isNull() const
{
    return d.isNull();
}

QString PBValue::repr(int indent) const
{
    if (d)
        return d->repr(indent);
    return QLatin1String("<null>");
}

PBObject::PBObject()
{
}

PBObject::~PBObject()
{
}

PBArray *PBObject::asArray()
{
    return 0;
}

PBString *PBObject::asString()
{
    return 0;
}

PBDictionary *PBObject::asDict()
{
    return 0;
}

PBKey *PBObject::asKey()
{
    return 0;
}

int PBObject::count() const
{
    return 0;
}

PBString *PBString::asString()
{
    return this;
}

QString PBString::repr(int indent) const
{
    Q_UNUSED(indent)
    return text;
}

PBDictionary *PBDictionary::asDict()
{
    return this;
}

PBValue &PBDictionary::operator [](const PBKey &key)
{
    return m_data[key];
}

PBValue &PBDictionary::operator [](const QString &key)
{
    PBKey pbKey;
    pbKey.key = key;
    return m_data[pbKey];
}

PBValue PBDictionary::value(const PBKey &key)
{
    return m_data.value(key);
}

PBValue PBDictionary::value(const QString &key)
{
    PBKey pbKey;
    pbKey.key = key;
    return m_data.value(pbKey);
}

PBDictionary::iterator PBDictionary::begin()
{
    return m_data.begin();
}

PBDictionary::const_iterator PBDictionary::begin() const
{
    return m_data.begin();
}

PBDictionary::iterator PBDictionary::end()
{
    return m_data.end();
}

PBDictionary::const_iterator PBDictionary::end() const
{
    return m_data.end();
}

QString PBDictionary::repr(int indent) const
{
    QString ret = QLatin1String("{");
    for (const_iterator it = begin(), to = end(); it != to; ++it)
    {
        ret += QLatin1String("\n");
        for (int i = 0; i < indent + 2; ++i)
            ret += QLatin1Char(' ');
        ret += it.key().repr(indent + 2);
        ret += QLatin1String(" = ");
        ret += it.value().repr(indent + 2);
        ret += QLatin1String(";");
    }
    ret += QLatin1String("\n");
    for (int i = 0; i < indent; ++i)
        ret += QLatin1Char(' ');
    ret += QLatin1String("}");
    return ret;
}

QStringList PBDictionary::allKeys() const
{
    QStringList ret;
    for (const_iterator it = begin(), to = end(); it != to; ++it)
    {
        ret << it.key().repr();
    }
    return ret;
}

int PBDictionary::count() const
{
    return m_data.size();
}

PBArray *PBArray::asArray()
{
    return this;
}

PBValue &PBArray::operator [](int index)
{
    return m_data[index];
}

const PBValue &PBArray::operator [](int index) const
{
    return m_data[index];
}

QString PBArray::repr(int indent) const
{
    QString ret = QLatin1String("(");
    foreach (const PBValue &value, m_data)
    {
        ret += QLatin1String("\n");
        for (int i = 0; i < indent + 2; ++i)
            ret += QLatin1Char(' ');
        ret += value.repr(indent + 2);
        ret += QLatin1String(",");
    }
    ret += QLatin1String("\n");
    for (int i = 0; i < indent; ++i)
        ret += QLatin1Char(' ');
    ret += QLatin1String(")");
    return ret;
}

int PBArray::count() const
{
    return m_data.size();
}

void PBArray::append(const PBValue &value)
{
    m_data.append(value);
}

} // namespace XCodeProjectManager
