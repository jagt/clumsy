/** \file
* \brief IUP API with explicit variable argument parameters.
*
* See Copyright Notice in "iup.h"
*/

#ifndef __IUP_VARG_H 
#define __IUP_VARG_H

#include <stdarg.h>
#include "iup.h"

#ifdef __cplusplus
extern "C" {
#endif

void      IupLogV(const char* type, const char* format, va_list arglist);

Ihandle*  IupSetAttV(const char* handle_name, Ihandle* ih, const char* name, va_list arglist);

void      IupSetStrfV(Ihandle* ih, const char* name, const char* format, va_list arglist);
void      IupSetStrfIdV(Ihandle* ih, const char* name, int id, const char* format, va_list arglist);
void      IupSetStrfId2V(Ihandle* ih, const char* name, int lin, int col, const char* format, va_list arglist);

Ihandle*  IupSetCallbacksV(Ihandle* ih, const char *name, Icallback func, va_list arglist);

Ihandle*  IupCreateV(const char *classname, void* first, va_list arglist);
Ihandle*  IupVboxV(Ihandle* child, va_list arglist);
Ihandle*  IupZboxV(Ihandle* child, va_list arglist);
Ihandle*  IupHboxV(Ihandle* child,va_list arglist);
Ihandle*  IupNormalizerV(Ihandle* ih_first, va_list arglist);
Ihandle*  IupCboxV(Ihandle* child, va_list arglist);
Ihandle*  IupGridBoxV(Ihandle* child, va_list arglist);
Ihandle*  IupMultiBoxV(Ihandle* child, va_list arglist);
Ihandle*  IupMenuV(Ihandle* child, va_list arglist);
Ihandle*  IupTabsV(Ihandle* child, va_list arglist);
Ihandle*  IupFlatTabsV(Ihandle* child, va_list arglist);

void      IupMessageV(const char *title, const char *format, va_list arglist);
Ihandle*  IupParamBoxV(Ihandle* param, va_list arglist);
int       IupGetParamV(const char* title, Iparamcb action, void* user_data, const char* format, va_list arglist);

Ihandle*  IupGLCanvasBoxV(Ihandle* child, va_list arglist);


#endif
