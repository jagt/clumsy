/** \file
 * \brief New FileDlg (Windows Only).
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPFILEDLG_H 
#define __IUPFILEDLG_H

#ifdef __cplusplus
extern "C" {
#endif

/* the only exported function, 
   once called it will replace regular IupFileDlg */

int IupNewFileDlgOpen(void);


#ifdef __cplusplus
}
#endif

#endif
