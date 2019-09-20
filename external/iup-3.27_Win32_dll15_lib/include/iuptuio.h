/** \file
 * \brief IupTuioClient control
 *
 * See Copyright Notice in "iup.h"
 */

#ifndef __IUPTUIO_H
#define __IUPTUIO_H

#if	defined(__cplusplus)
extern "C" {
#endif

int IupTuioOpen(void);
Ihandle* IupTuioClient(int port);

#if defined(__cplusplus)
}
#endif

#endif
