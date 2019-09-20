/** \file
 * \brief Scintilla control.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUP_SCINTILLA_H 
#define __IUP_SCINTILLA_H

#ifdef __cplusplus
extern "C" {
#endif


void IupScintillaOpen(void);

Ihandle *IupScintilla(void);
Ihandle *IupScintillaDlg(void);

#ifdef SCINTILLA_H
sptr_t IupScintillaSendMessage(Ihandle* ih, unsigned int iMessage, uptr_t wParam, sptr_t lParam);
#endif


#ifdef __cplusplus
}
#endif

#endif
