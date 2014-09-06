// -*- C++ -*-
#ifndef _Export_h_
#define _Export_h_

#if __GNUC__
#   if FREEORION_BUILD_COMMON
#      define FO_COMMON_API __attribute__((__visibility__("default")))
#   else
#      define FO_COMMON_API
#   endif
#elif _MSC_VER
#   if FREEORION_BUILD_COMMON
#      define FO_COMMON_API __declspec(dllexport)
#   else
#      define FO_COMMON_API __declspec(dllimport)
#   endif
#else
#   define FO_COMMON_API
#endif


#endif // _Export_h_
