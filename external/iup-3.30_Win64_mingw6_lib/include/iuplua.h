/** \file
 * \brief IUP Binding for Lua.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPLUA_H 
#define __IUPLUA_H

#ifdef __cplusplus
extern "C" {
#endif


#ifndef DOXYGEN_SHOULD_IGNORE_THIS
/** @cond DOXYGEN_SHOULD_IGNORE_THIS */
#ifndef IUPLUA_API
#ifdef IUPLUA_BUILD_LIBRARY
  #ifdef __EMSCRIPTEN__
    #include <emscripten.h>
    #define IUPLUA_API EMSCRIPTEN_KEEPALIVE
  #elif WIN32
    #define IUPLUA_API __declspec(dllexport)
  #elif defined(__GNUC__) && __GNUC__ >= 4
    #define IUPLUA_API __attribute__ ((visibility("default")))
  #else
    #define IUPLUA_API
  #endif
#else
  #define IUPLUA_API
#endif /* IUPLUA_BUILD_LIBRARY */
#endif /* IUPLUA_API */
/** @endcond DOXYGEN_SHOULD_IGNORE_THIS */
#endif /* DOXYGEN_SHOULD_IGNORE_THIS */


IUPLUA_API int iuplua_open(lua_State *L);
IUPLUA_API int iupkey_open(lua_State *L);  /* does nothing, kept for backward compatibility */
IUPLUA_API int iuplua_close(lua_State * L);

/* utilities */
IUPLUA_API int iuplua_isihandle(lua_State *L, int pos);
IUPLUA_API Ihandle* iuplua_checkihandle(lua_State *L, int pos);
IUPLUA_API void iuplua_pushihandle(lua_State *L, Ihandle *n);
IUPLUA_API int iuplua_dofile(lua_State *L, const char *filename);
IUPLUA_API int iuplua_dostring(lua_State *L, const char *string, const char *chunk_name);
IUPLUA_API int iuplua_dobuffer(lua_State *L, const char *buffer, int len, const char *chunk_name);
IUPLUA_API void iuplua_show_error_message(const char *pname, const char* msg);

#ifdef __cplusplus
}
#endif

#endif
