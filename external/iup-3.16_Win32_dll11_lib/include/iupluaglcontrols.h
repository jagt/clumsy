/** \file
 * \brief iupglcontrols Binding for Lua.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPLUAGLCONTROLS_H 
#define __IUPLUAGLCONTROLS_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef LUA_TNONE  /* Lua 5 */
int iupglcontrolslua_open (lua_State * L);
#endif

#ifdef __cplusplus
}
#endif

#endif
