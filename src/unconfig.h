/**
 * unconfig.h: Undefine symbols defined by autotools multiple times.
 *
 * This file exists because the headers of The Sleuth Kit nearly inevitably include TSK's config.h.
 */
#ifndef UNCONFIG_H
#define UNCONFIG_H

#ifdef PACKAGE
#undef PACKAGE
#endif

#ifdef PACKAGE_BUGREPORT
#undef PACKAGE_BUGREPORT
#endif

#ifdef PACKAGE_NAME
#undef PACKAGE_NAME
#endif

#ifdef PACKAGE_STRING
#undef PACKAGE_STRING
#endif

#ifdef PACKAGE_TARNAME
#undef PACKAGE_TARNAME
#endif

#ifdef PACKAGE_VERSION
#undef PACKAGE_VERSION
#endif

#ifdef VERSION
#undef VERSION
#endif

#endif //UNCONFIG_H
