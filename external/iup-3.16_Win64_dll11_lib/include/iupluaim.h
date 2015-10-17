/** \file
 * \brief Bindig of iupim functions to Lua.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPLUAIM_H
#define __IUPLUAIM_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef LUA_NOOBJECT  /* Lua 3 */
void iupimlua_open(void);
#endif

#ifdef LUA_TNONE  /* Lua 5 */
int iupimlua_open(lua_State * L);
#endif

#ifdef __cplusplus
}
#endif

#endif
