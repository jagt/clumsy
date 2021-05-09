/** \file
 * \brief Web control.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPWEB_H 
#define __IUPWEB_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DOXYGEN_SHOULD_IGNORE_THIS
/** @cond DOXYGEN_SHOULD_IGNORE_THIS */
#ifndef IUPWEB_API
#ifdef IUPWEB_BUILD_LIBRARY
  #ifdef __EMSCRIPTEN__
    #include <emscripten.h>
    #define IUPWEB_API EMSCRIPTEN_KEEPALIVE
  #elif WIN32
    #define IUPWEB_API __declspec(dllexport)
  #elif defined(__GNUC__) && __GNUC__ >= 4
    #define IUPWEB_API __attribute__ ((visibility("default")))
  #else
    #define IUPWEB_API
  #endif
#else
  #define IUPWEB_API
#endif /* IUP_BUILD_LIBRARY */
#endif /* IUPWEB_API */
/** @endcond DOXYGEN_SHOULD_IGNORE_THIS */
#endif /* DOXYGEN_SHOULD_IGNORE_THIS */


IUPWEB_API int IupWebBrowserOpen(void);

IUPWEB_API Ihandle *IupWebBrowser(void);


#ifdef __cplusplus
}
#endif

#endif
