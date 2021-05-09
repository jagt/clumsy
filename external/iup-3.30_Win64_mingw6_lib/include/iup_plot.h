/** \file
 * \brief Plot component for Iup.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUP_PLOT_H 
#define __IUP_PLOT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize IupPlot widget class */
void IupPlotOpen(void);

/* Create an IupPlot widget instance */
Ihandle* IupPlot(void);

/***********************************************/
/*           Additional API                    */

void IupPlotBegin(Ihandle *ih, int strXdata);
void IupPlotAdd(Ihandle *ih, double x, double y);
void IupPlotAddStr(Ihandle *ih, const char* x, double y);
void IupPlotAddSegment(Ihandle *ih, double x, double y);
int  IupPlotEnd(Ihandle *ih);

int  IupPlotLoadData(Ihandle* ih, const char* filename, int strXdata);

/* available only when linking with "iupluaplot" */
int IupPlotSetFormula(Ihandle* ih, int sample_count, const char* formula, const char* init);

void IupPlotInsert(Ihandle *ih, int ds_index, int sample_index, double x, double y);
void IupPlotInsertStr(Ihandle *ih, int ds_index, int sample_index, const char* x, double y);
void IupPlotInsertSegment(Ihandle *ih, int ds_index, int sample_index, double x, double y);

void IupPlotInsertStrSamples(Ihandle* ih, int ds_index, int sample_index, const char** x, double* y, int count);
void IupPlotInsertSamples(Ihandle* ih, int ds_index, int sample_index, double *x, double *y, int count);

void IupPlotAddSamples(Ihandle* ih, int ds_index, double *x, double *y, int count);
void IupPlotAddStrSamples(Ihandle* ih, int ds_index, const char** x, double* y, int count);

void IupPlotGetSample(Ihandle* ih, int ds_index, int sample_index, double *x, double *y);
void IupPlotGetSampleStr(Ihandle* ih, int ds_index, int sample_index, const char* *x, double *y);
int  IupPlotGetSampleSelection(Ihandle* ih, int ds_index, int sample_index);
double IupPlotGetSampleExtra(Ihandle* ih, int ds_index, int sample_index);
void IupPlotSetSample(Ihandle* ih, int ds_index, int sample_index, double x, double y);
void IupPlotSetSampleStr(Ihandle* ih, int ds_index, int sample_index, const char* x, double y);
void IupPlotSetSampleSelection(Ihandle* ih, int ds_index, int sample_index, int selected);
void IupPlotSetSampleExtra(Ihandle* ih, int ds_index, int sample_index, double extra);

void IupPlotTransform(Ihandle* ih, double x, double y, double *cnv_x, double *cnv_y);
void IupPlotTransformTo(Ihandle* ih, double cnv_x, double cnv_y, double *x, double *y);

int  IupPlotFindSample(Ihandle* ih, double cnv_x, double cnv_y, int *ds_index, int *sample_index);
int  IupPlotFindSegment(Ihandle* ih, double cnv_x, double cnv_y, int *ds_index, int *sample_index1, int *sample_index2);

struct _cdCanvas;

void IupPlotPaintTo(Ihandle *ih, struct _cdCanvas* cnv);

/***********************************************/


#ifdef __cplusplus
}
#endif

#endif
