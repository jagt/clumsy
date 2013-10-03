/** \file
 * \brief Plot component for Iup.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUP_MGLPLOT_H 
#define __IUP_MGLPLOT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize IupMglPlot widget class */
void IupMglPlotOpen(void);

/* Create an IupMglPlot widget instance */
Ihandle* IupMglPlot(void);

/***********************************************/
/*           Additional API                    */
void IupMglPlotBegin(Ihandle *ih, int dim);
void IupMglPlotAdd1D(Ihandle *ih, const char* name, float y);
void IupMglPlotAdd2D(Ihandle *ih, float x, float y);
void IupMglPlotAdd3D(Ihandle *ih, float x, float y, float z);
int  IupMglPlotEnd(Ihandle *ih);

int IupMglPlotNewDataSet(Ihandle *ih, int dim);

void IupMglPlotInsert1D(Ihandle* ih, int ds_index, int sample_index, const char** names, const float* y, int count);
void IupMglPlotInsert2D(Ihandle* ih, int ds_index, int sample_index, const float* x, const float* y, int count);
void IupMglPlotInsert3D(Ihandle* ih, int ds_index, int sample_index, const float* x, const float* y, const float* z, int count);

void IupMglPlotSet1D(Ihandle* ih, int ds_index, const char** names, const float* y, int count);
void IupMglPlotSet2D(Ihandle* ih, int ds_index, const float* x, const float* y, int count);
void IupMglPlotSet3D(Ihandle* ih, int ds_index, const float* x, const float* y, const float* z, int count);
void IupMglPlotSetFormula(Ihandle* ih, int ds_index, const char* formulaX, const char* formulaY, const char* formulaZ, int count);

void IupMglPlotSetData(Ihandle* ih, int ds_index, const float* data, int count_x, int count_y, int count_z);
void IupMglPlotLoadData(Ihandle* ih, int ds_index, const char* filename, int count_x, int count_y, int count_z);
void IupMglPlotSetFromFormula(Ihandle* ih, int ds_index, const char* formula, int count_x, int count_y, int count_z);

void IupMglPlotTransform(Ihandle* ih, float x, float y, float z, int *ix, int *iy);
void IupMglPlotTransformXYZ(Ihandle* ih, int ix, int iy, float *x, float *y, float *z);

void IupMglPlotDrawMark(Ihandle* ih, float x, float y, float z);
void IupMglPlotDrawLine(Ihandle* ih, float x1, float y1, float z1, float x2, float y2, float z2);
void IupMglPlotDrawText(Ihandle* ih, const char* text, float x, float y, float z);

void IupMglPlotPaintTo(Ihandle *ih, const char* format, int w, int h, float dpi, void *data);
/***********************************************/


#ifdef __cplusplus
}
#endif

#endif
