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

#ifndef XCODEPROJECTMANAGER_PBPROJECTMODEL_H
#define XCODEPROJECTMANAGER_PBPROJECTMODEL_H

#include "pbclasses.h"

namespace XCodeProjectManager {

class PBProjectModel
{
public:
    typedef QSharedPointer<PBProjectModel> Ptr;
    typedef QWeakPointer<PBProjectModel> WeakPtr;

    PBProjectModel(const PBValue &dictionary);
    ~PBProjectModel();

    /// @name Direct access
    /// @{
    const PBValue &objects() const;
    const PBValue &classes() const;
    /// @}

    /// @name Utils
    /// @{
    const PBValue &rootGroup();
    /// @}

private:
    PBValue m_root;
    PBValue m_objects;
    PBValue m_classes;
    PBValue m_rootGroup;
};

} // namespace XCodeProjectManager

#endif // XCODEPROJECTMANAGER_PBPROJECTMODEL_H
