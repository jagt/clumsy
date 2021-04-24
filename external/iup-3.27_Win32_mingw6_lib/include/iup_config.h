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

Ihandle* IupConfig(void);

int IupConfigLoad(Ihandle* ih);
int IupConfigSave(Ihandle* ih);

/****************************************************************/

void IupConfigSetVariableStr(Ihandle* ih, const char* group, const char* key, const char* value);
void IupConfigSetVariableStrId(Ihandle* ih, const char* group, const char* key, int id, const char* value);
void IupConfigSetVariableInt(Ihandle* ih, const char* group, const char* key, int value);
void IupConfigSetVariableIntId(Ihandle* ih, const char* group, const char* key, int id, int value);
void IupConfigSetVariableDouble(Ihandle* ih, const char* group, const char* key, double value);
void IupConfigSetVariableDoubleId(Ihandle* ih, const char* group, const char* key, int id, double value);

const char* IupConfigGetVariableStr(Ihandle* ih, const char* group, const char* key);
const char* IupConfigGetVariableStrId(Ihandle* ih, const char* group, const char* key, int id);
int    IupConfigGetVariableInt(Ihandle* ih, const char* group, const char* key);
int    IupConfigGetVariableIntId(Ihandle* ih, const char* group, const char* key, int id);
double IupConfigGetVariableDouble(Ihandle* ih, const char* group, const char* key);
double IupConfigGetVariableDoubleId(Ihandle* ih, const char* group, const char* key, int id);

const char* IupConfigGetVariableStrDef(Ihandle* ih, const char* group, const char* key, const char* def);
const char* IupConfigGetVariableStrIdDef(Ihandle* ih, const char* group, const char* key, int id, const char* def);
int    IupConfigGetVariableIntDef(Ihandle* ih, const char* group, const char* key, int def);
int    IupConfigGetVariableIntIdDef(Ihandle* ih, const char* group, const char* key, int id, int def);
double IupConfigGetVariableDoubleDef(Ihandle* ih, const char* group, const char* key, double def);
double IupConfigGetVariableDoubleIdDef(Ihandle* ih, const char* group, const char* key, int id, double def);

void IupConfigCopy(Ihandle* ih1, Ihandle* ih2, const char* exclude_prefix);

/****************************************************************/

void IupConfigSetListVariable(Ihandle* ih, const char *group, const char* key, const char* value, int add);

void IupConfigRecentInit(Ihandle* ih, Ihandle* menu, Icallback recent_cb, int max_recent);
void IupConfigRecentUpdate(Ihandle* ih, const char* filename);

void IupConfigDialogShow(Ihandle* ih, Ihandle* dialog, const char* name);
void IupConfigDialogClosed(Ihandle* ih, Ihandle* dialog, const char* name);


#if defined(__cplusplus)
}
#endif

#endif
