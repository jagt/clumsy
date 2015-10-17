/** \file
 * \brief iupmatrixex Binding for Lua.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPLUAMATRIXEX_H 
#define __IUPLUAMATRIXEX_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef LUA_TNONE  /* Lua 5 */
int iupmatrixexlua_open (lua_State * L);
#endif

#ifdef __cplusplus
}
#endif

#endif
