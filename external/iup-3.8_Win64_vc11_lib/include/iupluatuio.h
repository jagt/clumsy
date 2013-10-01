/** \file
 * \brief Binding of iuptuio to Lua.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPLUATUIO_H 
#define __IUPLUATUIO_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef LUA_TNONE  /* Lua 5 */
int iuptuiolua_open (lua_State * L);
#endif

#ifdef __cplusplus
}
#endif

#endif
