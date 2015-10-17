/** \file
 * \brief IupScintilla Binding for Lua.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPLUA_SCINTILLA_H 
#define __IUPLUA_SCINTILLA_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef LUA_NOOBJECT  /* Lua 3 */
int iup_scintillalua_open (void);
#endif

#ifdef LUA_TNONE  /* Lua 5 */
int iup_scintillalua_open (lua_State * L);
#endif

#ifdef __cplusplus
}
#endif

#endif
