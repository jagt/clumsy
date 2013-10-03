/** \file
 * \brief Binding of iupolecontrol to Lua.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPLUAOLE_H 
#define __IUPLUAOLE_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef LUA_NOOBJECT  /* Lua 3 */
int iupolelua_open(void);
#endif

#ifdef LUA_TNONE  /* Lua 5 */
int iupolelua_open (lua_State * L);
#endif

#ifdef __cplusplus
}
#endif

#endif
