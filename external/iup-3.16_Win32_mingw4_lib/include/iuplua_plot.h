/** \file
 * \brief iup_plot Binding for Lua.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPLUA_PLOT_H 
#define __IUPLUA_PLOT_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef LUA_TNONE  /* Lua 5 */
int iup_plotlua_open (lua_State * L);
#endif

#ifdef __cplusplus
}
#endif

#endif
