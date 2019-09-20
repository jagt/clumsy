/** \file
 * \brief Utilities using IM
 *
 * See Copyright Notice in "iup.h"
 */

#ifndef __IUPIM_H
#define __IUPIM_H

#if	defined(__cplusplus)
extern "C" {
#endif


Ihandle* IupLoadImage(const char* file_name);
int IupSaveImage(Ihandle* ih, const char* file_name, const char* format);

Ihandle* IupLoadAnimation(const char* file_name);
Ihandle* IupLoadAnimationFrames(const char** file_name_list, int file_count);

#ifdef __IM_IMAGE_H
imImage* IupGetNativeHandleImage(void* handle);
void* IupGetImageNativeHandle(const imImage* image);

Ihandle* IupImageFromImImage(const imImage* image);
imImage* IupImageToImImage(Ihandle* iup_image);
#endif


#if defined(__cplusplus)
}
#endif

#endif
