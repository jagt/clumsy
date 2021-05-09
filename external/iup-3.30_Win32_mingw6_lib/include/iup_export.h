#ifndef __IUP_EXPORT_H
#define __IUP_EXPORT_H

#ifndef DOXYGEN_SHOULD_IGNORE_THIS
/** @cond DOXYGEN_SHOULD_IGNORE_THIS */

/* Mark the official functions */
#ifndef IUP_API
#ifdef IUP_BUILD_LIBRARY
  #ifdef __EMSCRIPTEN__
    #include <emscripten.h>
    #define IUP_API EMSCRIPTEN_KEEPALIVE
  #elif WIN32
    #define IUP_API __declspec(dllexport)
  #elif defined(__GNUC__) && __GNUC__ >= 4
    #define IUP_API __attribute__ ((visibility("default")))
  #else
    #define IUP_API
  #endif
#else
  #define IUP_API
#endif /* IUP_BUILD_LIBRARY */
#endif /* IUP_API */

/* Mark the internal SDK functions (some not official but need to be exported) */
#ifndef IUP_SDK_API
#ifdef IUP_BUILD_LIBRARY
  #ifdef __EMSCRIPTEN__
    #include <emscripten.h>
    #define IUP_SDK_API EMSCRIPTEN_KEEPALIVE
  #elif WIN32
    #define IUP_SDK_API __declspec(dllexport)
  #elif defined(__GNUC__) && __GNUC__ >= 4
    #define IUP_SDK_API __attribute__ ((visibility("default")))
  #else
    #define IUP_SDK_API
  #endif
#else
  #define IUP_SDK_API
#endif /* IUP_BUILD_LIBRARY */
#endif /* IUP_SDK_API */

/* Mark the driver functions that need to be exported */
#ifndef IUP_DRV_API
#ifdef IUP_BUILD_LIBRARY
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#define IUP_DRV_API EMSCRIPTEN_KEEPALIVE
#elif WIN32
#define IUP_DRV_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#define IUP_DRV_API __attribute__ ((visibility("default")))
#else
#define IUP_DRV_API
#endif
#else
#define IUP_DRV_API
#endif /* IUP_BUILD_LIBRARY */
#endif /* IUP_DRV_API */

/* Mark the IupImageLib function, it does not have a header of its own */
#ifndef IUPIMGLIB_API
#ifdef IUPIMGLIB_BUILD_LIBRARY
  #ifdef __EMSCRIPTEN__
    #include <emscripten.h>
    #define IUPIMGLIB_API EMSCRIPTEN_KEEPALIVE
  #elif WIN32
    #define IUPIMGLIB_API __declspec(dllexport)
  #elif defined(__GNUC__) && __GNUC__ >= 4
    #define IUPIMGLIB_API __attribute__ ((visibility("default")))
  #else
    #define IUPIMGLIB_API
  #endif
#else
  #define IUPIMGLIB_API
#endif /* IUPIMGLIB_BUILD_LIBRARY */
#endif /* IUPIMGLIB_API */

/** @endcond DOXYGEN_SHOULD_IGNORE_THIS */
#endif /* DOXYGEN_SHOULD_IGNORE_THIS */


#endif /* __IUP_EXPORT_H */
