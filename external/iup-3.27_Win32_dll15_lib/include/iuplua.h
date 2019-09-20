/** \file
 * \brief IUP Binding for Lua.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPLUA_H 
#define __IUPLUA_H

#ifdef __cplusplus
extern "C" {
#endif

int iuplua_open(lua_State *L);
int iupkey_open(lua_State *L);  /* does nothing, kept for backward compatibility */
int iuplua_close(lua_State * L);

/* utilities */
int iuplua_isihandle(lua_State *L, int pos);
Ihandle* iuplua_checkihandle(lua_State *L, int pos);
void iuplua_pushihandle(lua_State *L, Ihandle *n);
int iuplua_dofile(lua_State *L, const char *filename);
int iuplua_dostring(lua_State *L, const char *string, const char *chunk_name);
int iuplua_dobuffer(lua_State *L, const char *buffer, int len, const char *chunk_name);
void iuplua_show_error_message(const char *pname, const char* msg);

#ifdef __cplusplus
}
#endif

#endif
