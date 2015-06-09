#ifndef GOEDITOR_GLOBAL_H
#define GOEDITOR_GLOBAL_H

#include <QtGlobal>

#if defined(GOEDITOR_LIBRARY)
#  define GOEDITORSHARED_EXPORT Q_DECL_EXPORT
#else
#  define GOEDITORSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // GOEDITOR_GLOBAL_H

