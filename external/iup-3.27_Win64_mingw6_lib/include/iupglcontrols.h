/** \file
 * \brief GL Controls.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPGLCONTROLS_H 
#define __IUPGLCONTROLS_H

#ifdef __cplusplus
extern "C" {
#endif


int  IupGLControlsOpen(void);

Ihandle* IupGLCanvasBoxv(Ihandle** children);
Ihandle* IupGLCanvasBox(Ihandle* child, ...);

Ihandle* IupGLSubCanvas(void);

Ihandle* IupGLLabel(const char* title);
Ihandle* IupGLSeparator(void);
Ihandle* IupGLButton(const char* title);
Ihandle* IupGLToggle(const char* title);
Ihandle* IupGLLink(const char *url, const char * title);
Ihandle* IupGLProgressBar(void);
Ihandle* IupGLVal(void);
Ihandle* IupGLFrame(Ihandle* child);
Ihandle* IupGLExpander(Ihandle* child);
Ihandle* IupGLScrollBox(Ihandle* child);
Ihandle* IupGLSizeBox(Ihandle* child);
Ihandle* IupGLText(void);


/* Utilities */
void IupGLDrawImage(Ihandle* ih, const char* name, int x, int y, int active);
void IupGLDrawText(Ihandle* ih, const char* str, int len, int x, int y);
void IupGLDrawGetTextSize(Ihandle* ih, const char* str, int *w, int *h);
void IupGLDrawGetImageInfo(const char* name, int *w, int *h, int *bpp);


#ifdef __cplusplus
}
#endif

#endif
