/** \file
 * \brief Contains all function pointer typedefs.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPCBS_H 
#define __IUPCBS_H

typedef int (*IFidle)(void);  /* idle */

typedef void (*IFii)(int, int); /* globalkeypress_cb */
typedef void (*IFiis)(int, int, char*);  /* globalmotion_cb */
typedef void (*IFiiiis)(int, int, int, int, char*);  /* globalbutton_cb */
typedef void (*IFfiis)(float,int,int,char*);  /* globalwheel_cb */

typedef int (*IFn)(Ihandle*);  /* default definition, same as Icallback (ncols_cb, nlines_cb)*/
typedef int (*IFni)(Ihandle*, int);   /* k_any, show_cb, toggle_action, spin_cb, branchopen_cb, branchclose_cb, executeleaf_cb, showrename_cb, rightclick_cb, extended_cb, height_cb, width_cb */
typedef int (*IFnii)(Ihandle*, int, int);  /* resize_cb, caret_cb, matrix_mousemove_cb, enteritem_cb, leaveitem_cb, scrolltop_cb, dropcheck_cb, selection_cb, select_cb, switch_cb, scrolling_cb, vspan_cb, hspan_cb */
typedef int (*IFniii)(Ihandle*, int, int, int); /* trayclick_cb, edition_cb */
typedef int (*IFniiii)(Ihandle*, int, int, int, int); /* dragdrop_cb */
typedef int (*IFniiiiiiC)(Ihandle*, int, int, int, int, int, int, void*);  /* draw_cb */
typedef int (*IFniiiiii)(Ihandle*, int, int, int, int, int, int);  /* OLD draw_cb */

typedef int (*IFnff)(Ihandle*, float, float);    /* canvas_action */
typedef int (*IFniff)(Ihandle*,int,float,float);  /* scroll_cb */
typedef int (*IFnfiis)(Ihandle*,float,int,int,char*);  /* wheel_cb */

typedef int (*IFnsCi)(Ihandle*, char*, void*, int);  /* dragdata_cb */
typedef int (*IFnsCiii)(Ihandle*, char*, void*, int, int, int);  /* dropdata_cb */
typedef int (*IFnsiii)(Ihandle*, char*, int, int, int);  /* dropfiles_cb */

typedef int (*IFnnii)(Ihandle*, Ihandle*, int, int); /* drop_cb */
typedef int (*IFnnn)(Ihandle*, Ihandle*, Ihandle*); /* tabchange_cb */
typedef int (*IFnss)(Ihandle*, char *, char *);  /* file_cb */
typedef int (*IFns)(Ihandle*, char *);  /* multiselect_cb */
typedef int (*IFnsi)(Ihandle*, char *, int);  /* copydata_cb */
typedef int (*IFnis)(Ihandle*, int, char *);  /* text_action, multiline_action, edit_cb, rename_cb */
typedef int (*IFnsii)(Ihandle*, char*, int, int);  /* list_action */
typedef int (*IFniis)(Ihandle*, int, int, char*);  /* motion_cb, click_cb, value_edit_cb */
typedef int (*IFniiis)(Ihandle*, int, int, int, char*);  /* touch_cb, dblclick_cb */
typedef int (*IFniiiis)(Ihandle*, int, int, int, int, char*);  /* button_cb, matrix_action, mousemotion_cb */
typedef int (*IFniiiiiis)(Ihandle*, int, int, int, int, int, int, char*);  /* mouseclick_cb */

typedef int (*IFnIi)(Ihandle*, int*, int); /* multiselection_cb, multiunselection_cb */
typedef int (*IFnd)(Ihandle*, double);  /* mousemove_cb, button_press_cb, button_release_cb */
typedef int (*IFniiIII)(Ihandle*, int, int, int*, int*, int*); /* fgcolor_cb, bgcolor_cb */
typedef int (*IFniinsii)(Ihandle*, int, int, Ihandle*, char*, int, int); /* dropselect_cb */
typedef int (*IFnccc)(Ihandle*, unsigned char, unsigned char, unsigned char); /* drag_cb, change_cb */
typedef int (*IFniIIII)(Ihandle*, int, int*, int*, int*, int*); /* multitouch_cb */

typedef char* (*sIFnii)(Ihandle*, int, int);  /* value_cb */
typedef char* (*sIFni)(Ihandle*, int);  /* cell_cb */

#endif
