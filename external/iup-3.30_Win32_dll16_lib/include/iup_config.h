/** \file
 * \brief Configuration file Utilities
 *
 * See Copyright Notice in "iup.h"
 */

#ifndef IUP_CONFIG_H
#define IUP_CONFIG_H

#if	defined(__cplusplus)
extern "C" {
#endif


IUP_API Ihandle* IupConfig(void);

IUP_API int IupConfigLoad(Ihandle* ih);
IUP_API int IupConfigSave(Ihandle* ih);

/****************************************************************/

IUP_API void IupConfigSetVariableStr(Ihandle* ih, const char* group, const char* key, const char* value);
IUP_API void IupConfigSetVariableStrId(Ihandle* ih, const char* group, const char* key, int id, const char* value);
IUP_API void IupConfigSetVariableInt(Ihandle* ih, const char* group, const char* key, int value);
IUP_API void IupConfigSetVariableIntId(Ihandle* ih, const char* group, const char* key, int id, int value);
IUP_API void IupConfigSetVariableDouble(Ihandle* ih, const char* group, const char* key, double value);
IUP_API void IupConfigSetVariableDoubleId(Ihandle* ih, const char* group, const char* key, int id, double value);

IUP_API const char* IupConfigGetVariableStr(Ihandle* ih, const char* group, const char* key);
IUP_API const char* IupConfigGetVariableStrId(Ihandle* ih, const char* group, const char* key, int id);
IUP_API int    IupConfigGetVariableInt(Ihandle* ih, const char* group, const char* key);
IUP_API int    IupConfigGetVariableIntId(Ihandle* ih, const char* group, const char* key, int id);
IUP_API double IupConfigGetVariableDouble(Ihandle* ih, const char* group, const char* key);
IUP_API double IupConfigGetVariableDoubleId(Ihandle* ih, const char* group, const char* key, int id);

IUP_API const char* IupConfigGetVariableStrDef(Ihandle* ih, const char* group, const char* key, const char* def);
IUP_API const char* IupConfigGetVariableStrIdDef(Ihandle* ih, const char* group, const char* key, int id, const char* def);
IUP_API int    IupConfigGetVariableIntDef(Ihandle* ih, const char* group, const char* key, int def);
IUP_API int    IupConfigGetVariableIntIdDef(Ihandle* ih, const char* group, const char* key, int id, int def);
IUP_API double IupConfigGetVariableDoubleDef(Ihandle* ih, const char* group, const char* key, double def);
IUP_API double IupConfigGetVariableDoubleIdDef(Ihandle* ih, const char* group, const char* key, int id, double def);

IUP_API void IupConfigCopy(Ihandle* ih1, Ihandle* ih2, const char* exclude_prefix);

/****************************************************************/

IUP_API void IupConfigSetListVariable(Ihandle* ih, const char *group, const char* key, const char* value, int add);

IUP_API void IupConfigRecentInit(Ihandle* ih, Ihandle* menu, Icallback recent_cb, int max_recent);
IUP_API void IupConfigRecentUpdate(Ihandle* ih, const char* filename);

IUP_API void IupConfigDialogShow(Ihandle* ih, Ihandle* dialog, const char* name);
IUP_API void IupConfigDialogClosed(Ihandle* ih, Ihandle* dialog, const char* name);


#if defined(__cplusplus)
}
#endif

#endif
