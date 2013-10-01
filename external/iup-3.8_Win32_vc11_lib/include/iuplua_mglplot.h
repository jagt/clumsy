/** \file
 * \brief IupMglPlot Binding for Lua.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPLUA_MGLPLOT_H 
#define __IUPLUA_MGLPLOT_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef LUA_NOOBJECT  /* Lua 3 */
int iup_mglplotlua_open (void);
#endif

#ifdef LUA_TNONE  /* Lua 5 */
int iup_mglplotlua_open (lua_State * L);
#endif

#ifdef __cplusplus
}
#endif

#endif
