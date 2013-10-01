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

#ifdef LUA_NOOBJECT  /* Lua 3 */
int iuplua_open (void);
int iupkey_open (void);

/* utilities */
Ihandle* iuplua_checkihandle(int pos);
void iuplua_pushihandle(Ihandle *n);
int iuplua_dofile (char *filename);
#endif

#ifdef LUA_TNONE  /* Lua 5 */
int iuplua_open(lua_State *L);
int iupkey_open(lua_State *L);  /* does nothing, kept for backward compatibility */
int iuplua_close(lua_State * L);

/* utilities */
Ihandle* iuplua_checkihandle(lua_State *L, int pos);
void iuplua_pushihandle(lua_State *L, Ihandle *n);
int iuplua_dofile(lua_State *L, const char *filename);
int iuplua_dostring(lua_State *L, const char *string, const char *chunk_name);

#endif

#ifdef __cplusplus
}
#endif

#endif
