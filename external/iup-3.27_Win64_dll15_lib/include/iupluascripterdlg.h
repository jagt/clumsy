/** \file
 * \brief IupLuaScripterDlg dialog and Lua binding
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPLUASCRIPTERDLG_H 
#define __IUPLUASCRIPTERDLG_H

#ifdef __cplusplus
extern "C" {
#endif

void IupLuaScripterDlgOpen(lua_State * L);

Ihandle* IupLuaScripterDlg(void);

/* Lua binding */
int iupluascripterdlglua_open(lua_State * L);

#ifdef __cplusplus
}
#endif

#endif
