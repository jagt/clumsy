/** \file
 * \brief iup_pplot Binding for Lua.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPLUA_PPLOT_H 
#define __IUPLUA_PPLOT_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef LUA_NOOBJECT  /* Lua 3 */
int iup_pplotlua_open (void);
#endif

#ifdef LUA_TNONE  /* Lua 5 */
int iup_pplotlua_open (lua_State * L);
#endif

#ifdef __cplusplus
}
#endif

#endif
