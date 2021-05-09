/** \file
 * \brief Ole control.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPOLE_H 
#define __IUPOLE_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DOXYGEN_SHOULD_IGNORE_THIS
  /** @cond DOXYGEN_SHOULD_IGNORE_THIS */
#ifndef IUPOLE_API
#ifdef IUPOLE_BUILD_LIBRARY
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#define IUPOLE_API EMSCRIPTEN_KEEPALIVE
#elif WIN32
#define IUPOLE_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#define IUPOLE_API __attribute__ ((visibility("default")))
#else
#define IUPOLE_API
#endif
#else
#define IUPOLE_API
#endif /* IUP_BUILD_LIBRARY */
#endif /* IUPOLE_API */
  /** @endcond DOXYGEN_SHOULD_IGNORE_THIS */
#endif /* DOXYGEN_SHOULD_IGNORE_THIS */


IUPOLE_API Ihandle *IupOleControl(const char* progid);

IUPOLE_API int IupOleControlOpen(void);


#ifdef __cplusplus
}
#endif

#endif
