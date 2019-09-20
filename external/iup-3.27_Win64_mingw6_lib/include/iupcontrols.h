/** \file
 * \brief initializes dial, gauge, colorbrowser, colorbar controls.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPCONTROLS_H 
#define __IUPCONTROLS_H

#ifdef __cplusplus
extern "C" {
#endif


int  IupControlsOpen(void);

Ihandle* IupCells(void);
Ihandle* IupMatrix(const char *action);
Ihandle* IupMatrixList(void);
Ihandle* IupMatrixEx(void);

/* available only when linking with "iupluamatrix" */
void IupMatrixSetFormula(Ihandle* ih, int col, const char* formula, const char* init);
void IupMatrixSetDynamic(Ihandle* ih, const char* init);


#ifdef __cplusplus
}
#endif

#endif
