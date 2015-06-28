#ifndef GOEDITOR_GOSEMKIPROCESS_H
#define GOEDITOR_GOSEMKIPROCESS_H

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

#endif // GOEDITOR_GOSEMKIPROCESS_H
