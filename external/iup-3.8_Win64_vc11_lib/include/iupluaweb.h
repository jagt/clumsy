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

#ifdef LUA_NOOBJECT  /* Lua 3 */
int iupweblua_open(void);
#endif

#ifdef LUA_TNONE  /* Lua 5 */
int iupweblua_open (lua_State * L);
#endif

#ifdef __cplusplus
}
#endif

#endif
