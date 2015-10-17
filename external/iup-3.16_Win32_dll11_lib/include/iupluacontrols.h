/** \file
 * \brief iupcontrols Binding for Lua.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPLUACONTROLS_H 
#define __IUPLUACONTROLS_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef LUA_NOOBJECT  /* Lua 3 */
int iupcontrolslua_open (void);
#endif

#ifdef LUA_TNONE  /* Lua 5 */
int iupcontrolslua_open (lua_State * L);
int iupcontrolslua_close(lua_State * L);
#endif

#ifdef __cplusplus
}
#endif

#endif
