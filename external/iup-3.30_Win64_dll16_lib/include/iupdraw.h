/** \file
 * \brief Canvas Draw API
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPDRAW_H 
#define __IUPDRAW_H

#ifdef __cplusplus
extern "C" {
#endif

/* all functions can be used only in IUP canvas and inside the ACTION callback */

IUP_API void IupDrawBegin(Ihandle* ih);
IUP_API void IupDrawEnd(Ihandle* ih);

/* all functions can be called only between calls to Begin and End */

IUP_API void IupDrawSetClipRect(Ihandle* ih, int x1, int y1, int x2, int y2);
IUP_API void IupDrawGetClipRect(Ihandle* ih, int *x1, int *y1, int *x2, int *y2);
IUP_API void IupDrawResetClip(Ihandle* ih);

/* color controlled by the attribute DRAWCOLOR */
/* line style or fill controlled by the attribute DRAWSTYLE */

IUP_API void IupDrawParentBackground(Ihandle* ih);
IUP_API void IupDrawLine(Ihandle* ih, int x1, int y1, int x2, int y2);
IUP_API void IupDrawRectangle(Ihandle* ih, int x1, int y1, int x2, int y2);
IUP_API void IupDrawArc(Ihandle* ih, int x1, int y1, int x2, int y2, double a1, double a2);
IUP_API void IupDrawPolygon(Ihandle* ih, int* points, int count);
IUP_API void IupDrawText(Ihandle* ih, const char* text, int len, int x, int y, int w, int h);
IUP_API void IupDrawImage(Ihandle* ih, const char* name, int x, int y, int w, int h);
IUP_API void IupDrawSelectRect(Ihandle* ih, int x1, int y1, int x2, int y2);
IUP_API void IupDrawFocusRect(Ihandle* ih, int x1, int y1, int x2, int y2);

IUP_API void IupDrawGetSize(Ihandle* ih, int *w, int *h);
IUP_API void IupDrawGetTextSize(Ihandle* ih, const char* text, int len, int *w, int *h);
IUP_API void IupDrawGetImageInfo(const char* name, int *w, int *h, int *bpp);


#ifdef __cplusplus
}
#endif

#endif
