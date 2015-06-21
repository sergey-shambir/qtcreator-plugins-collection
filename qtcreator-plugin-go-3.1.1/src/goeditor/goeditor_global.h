#ifndef GOEDITOR_GLOBAL_H
#define GOEDITOR_GLOBAL_H

#include <QtGlobal>

#if defined(GOEDITOR_LIBRARY)
#  define GOEDITOR_EXPORT Q_DECL_EXPORT
#else
#  define GOEDITOR_EXPORT Q_DECL_IMPORT
#endif

#endif // GOEDITOR_GLOBAL_H

