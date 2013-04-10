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

#include "pbprojectmodel.h"
#include "pbenums.h"

namespace XCodeProjectManager {

PBProjectModel::PBProjectModel(const PBValue &dictionary)
    : m_root(dictionary)
    , m_objects(dictionary.valueForKey(QLatin1String("objects")))
    , m_classes(dictionary.valueForKey(QLatin1String("classes")))
{
    PBByClassIterator projectIt(m_objects, PBXProject);
    if (projectIt.toNext()) {
        QString key = projectIt.object().valueForKey(QLatin1String("mainGroup")).repr();
        m_rootGroup = m_objects.valueForKey(key);
    }
}

PBProjectModel::~PBProjectModel()
{
}

const PBValue &PBProjectModel::objects() const
{
    return m_objects;
}

const PBValue &PBProjectModel::classes() const
{
    return m_classes;
}

const PBValue &PBProjectModel::rootGroup()
{
    return m_rootGroup;
}

} // namespace XCodeProjectManager
