#ifndef GOLANG_GLOBAL_H
#define GOLANG_GLOBAL_H

#include <QtGlobal>

#if defined(GO_LIBRARY)
#  define GOSHARED_EXPORT Q_DECL_EXPORT
#else
#  define GOSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // GOLANG_GLOBAL_H

