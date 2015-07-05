#include "gooutlinemodel.h"
#include "gosemanticinfo.h"
#include "goeditordocument.h"

namespace GoEditor {

GoOutlineModel::GoOutlineModel(GoEditorDocument *document)
    : QStandardItemModel(document)
    , m_funcIcon(QLatin1String(":/goeditor/images/func.png"))
    , m_typeIcon(QLatin1String(":/goeditor/images/type.png"))
{
}

void GoOutlineModel::update(GoSemanticInfo &semanticInfo)
{
    beginResetModel();
    clear();
    QScopedPointer<QStandardItem> modelItem;
    for (GoOutlineItem const& item : semanticInfo.outlineItems()) {
        modelItem.reset(new QStandardItem(item.title));
        modelItem->setData(item.encodePosition(), GoLocationRole);
        modelItem->setData(item.kind, GoTypeRole);
        appendRow(modelItem.take());
    }
    endResetModel();
}

QVariant GoOutlineModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DecorationRole) {
        int type = data(index, GoTypeRole).toInt();
        switch (type) {
        case GoOutlineItem::Func:
            return m_funcIcon;
        case GoOutlineItem::Struct:
            return m_typeIcon;
        default:
            return QVariant();
        }
    }
    return QStandardItemModel::data(index, role);
}

} // namespace GoEditor
