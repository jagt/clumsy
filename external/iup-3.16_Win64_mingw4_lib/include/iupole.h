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


Ihandle *IupOleControl(const char* progid);

int IupOleControlOpen(void);


#ifdef __cplusplus
}
#endif

#endif
