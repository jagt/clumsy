/** \file
 * \brief Binding of iupwebbrowser to Lua.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPLUAWEB_H 
#define __IUPLUAWEB_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DOXYGEN_SHOULD_IGNORE_THIS
/** @cond DOXYGEN_SHOULD_IGNORE_THIS */
#ifndef IUPLUAWEB_API
#ifdef IUPLUAWEB_BUILD_LIBRARY
  #ifdef __EMSCRIPTEN__
    #include <emscripten.h>
    #define IUPLUAWEB_API EMSCRIPTEN_KEEPALIVE
  #elif WIN32
    #define IUPLUAWEB_API __declspec(dllexport)
  #elif defined(__GNUC__) && __GNUC__ >= 4
    #define IUPLUAWEB_API __attribute__ ((visibility("default")))
  #else
    #define IUPLUAWEB_API
  #endif
#else
  #define IUPLUAWEB_API
#endif /* IUPLUAWEB_BUILD_LIBRARY */
#endif /* IUPLUAWEB_API */
/** @endcond DOXYGEN_SHOULD_IGNORE_THIS */
#endif /* DOXYGEN_SHOULD_IGNORE_THIS */


IUPLUAWEB_API int iupweblua_open (lua_State * L);

#ifdef __cplusplus
}
#endif

#endif
