#ifndef GOEDITOR_GOFMTPROCESS_H
#define GOEDITOR_GOFMTPROCESS_H

#include "gotoolprocess.h"

namespace GoEditor {

class GofmtProcess : public GoToolProcess
{
public:
    explicit GofmtProcess(const QString &filePath);
    explicit GofmtProcess(const QString &filePath, const QByteArray &contents);

    bool runFormatting();
};

} // namespace GoEditor

#endif // GOEDITOR_GOFMTPROCESS_H
