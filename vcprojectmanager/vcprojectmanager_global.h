#ifndef VCPROJECTMANAGER_GLOBAL_H
#define VCPROJECTMANAGER_GLOBAL_H

#include <QtGlobal>

#if defined(VCPROJECTMANAGER_LIBRARY)
#  define VCPROJECTMANAGERSHARED_EXPORT Q_DECL_EXPORT
#else
#  define VCPROJECTMANAGERSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // VCPROJECTMANAGER_GLOBAL_H
