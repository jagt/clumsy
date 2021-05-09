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

IUP_API void      IupLogV(const char* type, const char* format, va_list arglist);

IUP_API Ihandle*  IupSetAttV(const char* handle_name, Ihandle* ih, const char* name, va_list arglist);

IUP_API void      IupSetStrfV(Ihandle* ih, const char* name, const char* format, va_list arglist);
IUP_API void      IupSetStrfIdV(Ihandle* ih, const char* name, int id, const char* format, va_list arglist);
IUP_API void      IupSetStrfId2V(Ihandle* ih, const char* name, int lin, int col, const char* format, va_list arglist);

IUP_API Ihandle*  IupSetCallbacksV(Ihandle* ih, const char *name, Icallback func, va_list arglist);

IUP_API Ihandle*  IupCreateV(const char *classname, void* first, va_list arglist);
IUP_API Ihandle*  IupVboxV(Ihandle* child, va_list arglist);
IUP_API Ihandle*  IupZboxV(Ihandle* child, va_list arglist);
IUP_API Ihandle*  IupHboxV(Ihandle* child,va_list arglist);
IUP_API Ihandle*  IupNormalizerV(Ihandle* ih_first, va_list arglist);
IUP_API Ihandle*  IupCboxV(Ihandle* child, va_list arglist);
IUP_API Ihandle*  IupGridBoxV(Ihandle* child, va_list arglist);
IUP_API Ihandle*  IupMultiBoxV(Ihandle* child, va_list arglist);
IUP_API Ihandle*  IupMenuV(Ihandle* child,va_list arglist);
IUP_API Ihandle*  IupTabsV(Ihandle* child, va_list arglist);
IUP_API Ihandle*  IupFlatTabsV(Ihandle* child, va_list arglist);

IUP_API void      IupMessageV(const char *title, const char *format, va_list arglist);
IUP_API Ihandle*  IupParamBoxV(Ihandle* param, va_list arglist);
IUP_API int       IupGetParamV(const char* title, Iparamcb action, void* user_data, const char* format, va_list arglist);

/* must include iupglcontrols before this file to enable this declaration */
#ifdef __IUPGLCONTROLS_H  
#ifndef IUP_GLCONTROLS_API
#define IUP_GLCONTROLS_API
#endif
IUP_GLCONTROLS_API Ihandle*  IupGLCanvasBoxV(Ihandle* child, va_list arglist);
#endif


#endif
