/** \file
 * \brief OpenGL canvas for Iup.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPGL_H 
#define __IUPGL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Attributes 
** To set the appropriate visual (pixel format) the following
** attributes may be specified. Their values should be set
** before the canvas is mapped to the scrren.
** After mapping, changing their values has no effect.
*/
#ifndef IUP_BUFFER    /* IUP_SINGLE (defaut) or IUP_DOUBLE */
#define IUP_BUFFER    "BUFFER"
#endif
#ifndef IUP_STEREO    /* IUP_NO (defaut) or IUP_YES */
#define IUP_STEREO     "STEREO"
#endif
#ifndef IUP_BUFFER_SIZE    /* Number of bits if index mode */
#define IUP_BUFFER_SIZE    "BUFFER_SIZE"
#endif
#ifndef IUP_RED_SIZE    /* Number of red bits */
#define IUP_RED_SIZE    "RED_SIZE"
#endif
#ifndef IUP_GREEN_SIZE    /* Number of green bits */
#define IUP_GREEN_SIZE    "GREEN_SIZE"
#endif
#ifndef IUP_BLUE_SIZE    /* Number of blue bits */
#define IUP_BLUE_SIZE    "BLUE_SIZE"
#endif
#ifndef IUP_ALPHA_SIZE    /* Number of alpha bits */
#define IUP_ALPHA_SIZE    "ALPHA_SIZE"
#endif
#ifndef IUP_DEPTH_SIZE    /* Number of bits in depth buffer */
#define IUP_DEPTH_SIZE    "DEPTH_SIZE"
#endif
#ifndef IUP_STENCIL_SIZE  /* Number of bits in stencil buffer */
#define IUP_STENCIL_SIZE  "STENCIL_SIZE"
#endif
#ifndef IUP_ACCUM_RED_SIZE  /* Number of red bits in accum. buffer */
#define IUP_ACCUM_RED_SIZE  "ACCUM_RED_SIZE"
#endif
#ifndef IUP_ACCUM_GREEN_SIZE  /* Number of green bits in accum. buffer */
#define IUP_ACCUM_GREEN_SIZE  "ACCUM_GREEN_SIZE"
#endif
#ifndef IUP_ACCUM_BLUE_SIZE  /* Number of blue bits in accum. buffer */
#define IUP_ACCUM_BLUE_SIZE  "ACCUM_BLUE_SIZE"
#endif
#ifndef IUP_ACCUM_ALPHA_SIZE  /* Number of alpha bits in accum. buffer */
#define IUP_ACCUM_ALPHA_SIZE  "ACCUM_ALPHA_SIZE"
#endif


/* Attribute values */
#ifndef IUP_DOUBLE
#define IUP_DOUBLE  "DOUBLE"
#endif
#ifndef IUP_SINGLE
#define IUP_SINGLE  "SINGLE"
#endif
#ifndef IUP_INDEX
#define IUP_INDEX  "INDEX"
#endif
#ifndef IUP_RGBA
#define IUP_RGBA  "RGBA"
#endif
#ifndef IUP_YES
#define IUP_YES    "YES"
#endif
#ifndef IUP_NO
#define IUP_NO    "NO"
#endif

void IupGLCanvasOpen(void);

Ihandle *IupGLCanvas(const char *action);
Ihandle* IupGLBackgroundBox(Ihandle* child);

void IupGLMakeCurrent(Ihandle* ih);
int IupGLIsCurrent(Ihandle* ih);
void IupGLSwapBuffers(Ihandle* ih);
void IupGLPalette(Ihandle* ih, int index, float r, float g, float b);
void IupGLUseFont(Ihandle* ih, int first, int count, int list_base);
void IupGLWait(int gl);

#ifdef __cplusplus
}
#endif

#endif
