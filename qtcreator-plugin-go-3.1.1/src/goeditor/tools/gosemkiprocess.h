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
#include "gotoolprocess.h"
#include "gosemanticinfo.h"
#include <QMap>

namespace GoEditor {

class GosemkiProcess : public GoToolProcess
{
public:
    explicit GosemkiProcess(const QString &filePath);
    explicit GosemkiProcess(const QString &filePath, const QByteArray &contents);

    QSharedPointer<GoSemanticInfo> collectSemanticInfo();

private:
    void initStringMaps();
    void parseJson(const QByteArray &response);
    GoHighlightRange::Format formatFromString(const QString &string) const;
    GoOutlineItem::Kind outlineKindFromString(const QString &string) const;

    QSharedPointer<GoSemanticInfo> m_sema;
    QMap<QString, GoOutlineItem::Kind> m_outlineKindsMap;
    QMap<QString, GoHighlightRange::Format> m_formatsMap;
};

} // namespace GoEditor
