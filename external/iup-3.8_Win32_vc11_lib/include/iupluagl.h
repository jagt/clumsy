/** \file
 * \brief Binding of iupglcanvas to Lua.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPLUAGL_H 
#define __IUPLUAGL_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef LUA_NOOBJECT  /* Lua 3 */
int iupgllua_open(void);
#endif

#ifdef LUA_TNONE  /* Lua 5 */
int iupgllua_open (lua_State * L);
#endif

#ifdef __cplusplus
}
#endif

#endif
