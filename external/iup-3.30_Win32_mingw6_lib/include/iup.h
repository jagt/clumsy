/** \file
 * \brief User API
 * IUP - A Portable User Interface Toolkit
 * Tecgraf: Computer Graphics Technology Group, PUC-Rio, Brazil
 * http://www.tecgraf.puc-rio.br/iup  mailto:iup@tecgraf.puc-rio.br
 *
 * See Copyright Notice at the end of this file
 */
 
#ifndef __IUP_H 
#define __IUP_H

#include "iupkey.h"
#include "iupdef.h"
#include "iup_export.h"

#ifdef __cplusplus
extern "C" {
#endif


#define IUP_NAME "IUP - Portable User Interface"
#define IUP_DESCRIPTION "Multi-platform Toolkit for Building Graphical User Interfaces"
#define IUP_COPYRIGHT "Copyright (C) 1994-2020 Tecgraf/PUC-Rio"
#define IUP_VERSION "3.30"         /* bug fixes are reported only by IupVersion functions */
#define IUP_VERSION_NUMBER 330000
#define IUP_VERSION_DATE "2020/07/30"  /* does not include bug fix releases */

typedef struct Ihandle_ Ihandle;
typedef int (*Icallback)(Ihandle*);


/************************************************************************/
/*                        Main API                                      */
/************************************************************************/

IUP_API int       IupOpen          (int *argc, char ***argv);
IUP_API void      IupClose         (void);
IUP_API int       IupIsOpened      (void);

IUPIMGLIB_API void IupImageLibOpen(void);

IUP_API int       IupMainLoop      (void);
IUP_API int       IupLoopStep      (void);
IUP_API int       IupLoopStepWait  (void);
IUP_API int       IupMainLoopLevel (void);
IUP_API void      IupFlush         (void);
IUP_API void      IupExitLoop      (void);
IUP_API void      IupPostMessage   (Ihandle* ih, const char* s, int i, double d, void* p);

IUP_API int       IupRecordInput(const char* filename, int mode);
IUP_API int       IupPlayInput(const char* filename);

IUP_API void      IupUpdate        (Ihandle* ih);
IUP_API void      IupUpdateChildren(Ihandle* ih);
IUP_API void      IupRedraw        (Ihandle* ih, int children);
IUP_API void      IupRefresh       (Ihandle* ih);
IUP_API void      IupRefreshChildren(Ihandle* ih);

IUP_API int       IupExecute(const char *filename, const char* parameters);
IUP_API int       IupExecuteWait(const char *filename, const char* parameters);
IUP_API int       IupHelp(const char* url);
IUP_API void      IupLog(const char* type, const char* format, ...);

IUP_API char*     IupLoad          (const char *filename);
IUP_API char*     IupLoadBuffer    (const char *buffer);

IUP_API char*     IupVersion       (void);
IUP_API char*     IupVersionDate   (void);
IUP_API int       IupVersionNumber (void);
IUP_API void      IupVersionShow   (void);

IUP_API void      IupSetLanguage   (const char *lng);
IUP_API char*     IupGetLanguage   (void);
IUP_API void      IupSetLanguageString(const char* name, const char* str);
IUP_API void      IupStoreLanguageString(const char* name, const char* str);
IUP_API char*     IupGetLanguageString(const char* name);
IUP_API void      IupSetLanguagePack(Ihandle* ih);

IUP_API void      IupDestroy      (Ihandle* ih);
IUP_API void      IupDetach       (Ihandle* child);
IUP_API Ihandle*  IupAppend       (Ihandle* ih, Ihandle* child);
IUP_API Ihandle*  IupInsert       (Ihandle* ih, Ihandle* ref_child, Ihandle* child);
IUP_API Ihandle*  IupGetChild     (Ihandle* ih, int pos);
IUP_API int       IupGetChildPos  (Ihandle* ih, Ihandle* child);
IUP_API int       IupGetChildCount(Ihandle* ih);
IUP_API Ihandle*  IupGetNextChild (Ihandle* ih, Ihandle* child);
IUP_API Ihandle*  IupGetBrother   (Ihandle* ih);
IUP_API Ihandle*  IupGetParent    (Ihandle* ih);
IUP_API Ihandle*  IupGetDialog    (Ihandle* ih);
IUP_API Ihandle*  IupGetDialogChild(Ihandle* ih, const char* name);
IUP_API int       IupReparent     (Ihandle* ih, Ihandle* new_parent, Ihandle* ref_child);

IUP_API int       IupPopup         (Ihandle* ih, int x, int y);
IUP_API int       IupShow          (Ihandle* ih);
IUP_API int       IupShowXY        (Ihandle* ih, int x, int y);
IUP_API int       IupHide          (Ihandle* ih);
IUP_API int       IupMap           (Ihandle* ih);
IUP_API void      IupUnmap         (Ihandle* ih);

IUP_API void      IupResetAttribute(Ihandle* ih, const char* name);
IUP_API int       IupGetAllAttributes(Ihandle* ih, char** names, int n);
IUP_API void      IupCopyAttributes(Ihandle* src_ih, Ihandle* dst_ih);
IUP_API Ihandle*  IupSetAtt(const char* handle_name, Ihandle* ih, const char* name, ...);
IUP_API Ihandle*  IupSetAttributes (Ihandle* ih, const char *str);
IUP_API char*     IupGetAttributes (Ihandle* ih);

IUP_API void      IupSetAttribute   (Ihandle* ih, const char* name, const char* value);
IUP_API void      IupSetStrAttribute(Ihandle* ih, const char* name, const char* value);
IUP_API void      IupSetStrf        (Ihandle* ih, const char* name, const char* format, ...);
IUP_API void      IupSetInt         (Ihandle* ih, const char* name, int value);
IUP_API void      IupSetFloat       (Ihandle* ih, const char* name, float value);
IUP_API void      IupSetDouble      (Ihandle* ih, const char* name, double value);
IUP_API void      IupSetRGB         (Ihandle* ih, const char* name, unsigned char r, unsigned char g, unsigned char b);
IUP_API void      IupSetRGBA        (Ihandle* ih, const char* name, unsigned char r, unsigned char g, unsigned char b, unsigned char a);

IUP_API char*     IupGetAttribute(Ihandle* ih, const char* name);
IUP_API int       IupGetInt      (Ihandle* ih, const char* name);
IUP_API int       IupGetInt2     (Ihandle* ih, const char* name);
IUP_API int       IupGetIntInt   (Ihandle* ih, const char* name, int *i1, int *i2);
IUP_API float     IupGetFloat    (Ihandle* ih, const char* name);
IUP_API double    IupGetDouble(Ihandle* ih, const char* name);
IUP_API void      IupGetRGB      (Ihandle* ih, const char* name, unsigned char *r, unsigned char *g, unsigned char *b);
IUP_API void      IupGetRGBA     (Ihandle* ih, const char* name, unsigned char *r, unsigned char *g, unsigned char *b, unsigned char *a);

IUP_API void  IupSetAttributeId(Ihandle* ih, const char* name, int id, const char *value);
IUP_API void  IupSetStrAttributeId(Ihandle* ih, const char* name, int id, const char *value);
IUP_API void  IupSetStrfId(Ihandle* ih, const char* name, int id, const char* format, ...);
IUP_API void  IupSetIntId(Ihandle* ih, const char* name, int id, int value);
IUP_API void  IupSetFloatId(Ihandle* ih, const char* name, int id, float value);
IUP_API void  IupSetDoubleId(Ihandle* ih, const char* name, int id, double value);
IUP_API void  IupSetRGBId(Ihandle* ih, const char* name, int id, unsigned char r, unsigned char g, unsigned char b);

IUP_API char*  IupGetAttributeId(Ihandle* ih, const char* name, int id);
IUP_API int    IupGetIntId(Ihandle* ih, const char* name, int id);
IUP_API float  IupGetFloatId(Ihandle* ih, const char* name, int id);
IUP_API double IupGetDoubleId(Ihandle* ih, const char* name, int id);
IUP_API void   IupGetRGBId(Ihandle* ih, const char* name, int id, unsigned char *r, unsigned char *g, unsigned char *b);

IUP_API void  IupSetAttributeId2(Ihandle* ih, const char* name, int lin, int col, const char* value);
IUP_API void  IupSetStrAttributeId2(Ihandle* ih, const char* name, int lin, int col, const char* value);
IUP_API void  IupSetStrfId2(Ihandle* ih, const char* name, int lin, int col, const char* format, ...);
IUP_API void  IupSetIntId2(Ihandle* ih, const char* name, int lin, int col, int value);
IUP_API void  IupSetFloatId2(Ihandle* ih, const char* name, int lin, int col, float value);
IUP_API void  IupSetDoubleId2(Ihandle* ih, const char* name, int lin, int col, double value);
IUP_API void  IupSetRGBId2(Ihandle* ih, const char* name, int lin, int col, unsigned char r, unsigned char g, unsigned char b);

IUP_API char*  IupGetAttributeId2(Ihandle* ih, const char* name, int lin, int col);
IUP_API int    IupGetIntId2(Ihandle* ih, const char* name, int lin, int col);
IUP_API float  IupGetFloatId2(Ihandle* ih, const char* name, int lin, int col);
IUP_API double IupGetDoubleId2(Ihandle* ih, const char* name, int lin, int col);
IUP_API void   IupGetRGBId2(Ihandle* ih, const char* name, int lin, int col, unsigned char *r, unsigned char *g, unsigned char *b);

IUP_API void      IupSetGlobal  (const char* name, const char* value);
IUP_API void      IupSetStrGlobal(const char* name, const char* value);
IUP_API char*     IupGetGlobal  (const char* name);

IUP_API Ihandle*  IupSetFocus     (Ihandle* ih);
IUP_API Ihandle*  IupGetFocus     (void);
IUP_API Ihandle*  IupPreviousField(Ihandle* ih);
IUP_API Ihandle*  IupNextField    (Ihandle* ih);

IUP_API Icallback IupGetCallback (Ihandle* ih, const char *name);
IUP_API Icallback IupSetCallback (Ihandle* ih, const char *name, Icallback func);
IUP_API Ihandle*  IupSetCallbacks(Ihandle* ih, const char *name, Icallback func, ...);

IUP_API Icallback IupGetFunction(const char *name);
IUP_API Icallback IupSetFunction(const char *name, Icallback func);

IUP_API Ihandle*  IupGetHandle    (const char *name);
IUP_API Ihandle*  IupSetHandle    (const char *name, Ihandle* ih);
IUP_API int       IupGetAllNames  (char** names, int n);
IUP_API int       IupGetAllDialogs(char** names, int n);
IUP_API char*     IupGetName      (Ihandle* ih);

IUP_API void      IupSetAttributeHandle(Ihandle* ih, const char* name, Ihandle* ih_named);
IUP_API Ihandle*  IupGetAttributeHandle(Ihandle* ih, const char* name);
IUP_API void      IupSetAttributeHandleId(Ihandle* ih, const char* name, int id, Ihandle* ih_named);
IUP_API Ihandle*  IupGetAttributeHandleId(Ihandle* ih, const char* name, int id);
IUP_API void      IupSetAttributeHandleId2(Ihandle* ih, const char* name, int lin, int col, Ihandle* ih_named);
IUP_API Ihandle*  IupGetAttributeHandleId2(Ihandle* ih, const char* name, int lin, int col);

IUP_API char*     IupGetClassName(Ihandle* ih);
IUP_API char*     IupGetClassType(Ihandle* ih);
IUP_API int       IupGetAllClasses(char** names, int n);
IUP_API int       IupGetClassAttributes(const char* classname, char** names, int n);
IUP_API int       IupGetClassCallbacks(const char* classname, char** names, int n);
IUP_API void      IupSaveClassAttributes(Ihandle* ih);
IUP_API void      IupCopyClassAttributes(Ihandle* src_ih, Ihandle* dst_ih);
IUP_API void      IupSetClassDefaultAttribute(const char* classname, const char *name, const char* value);
IUP_API int       IupClassMatch(Ihandle* ih, const char* classname);

IUP_API Ihandle*  IupCreate (const char *classname);
IUP_API Ihandle*  IupCreatev(const char *classname, void* *params);
IUP_API Ihandle*  IupCreatep(const char *classname, void* first, ...);

/************************************************************************/
/*                        Elements                                      */
/************************************************************************/

IUP_API Ihandle*  IupFill (void);
IUP_API Ihandle*  IupSpace(void);

IUP_API Ihandle*  IupRadio      (Ihandle* child);
IUP_API Ihandle*  IupVbox       (Ihandle* child, ...);
IUP_API Ihandle*  IupVboxv      (Ihandle* *children);
IUP_API Ihandle*  IupZbox       (Ihandle* child, ...);
IUP_API Ihandle*  IupZboxv      (Ihandle* *children);
IUP_API Ihandle*  IupHbox       (Ihandle* child, ...);
IUP_API Ihandle*  IupHboxv      (Ihandle* *children);

IUP_API Ihandle*  IupNormalizer (Ihandle* ih_first, ...);
IUP_API Ihandle*  IupNormalizerv(Ihandle* *ih_list);

IUP_API Ihandle*  IupCbox       (Ihandle* child, ...);
IUP_API Ihandle*  IupCboxv      (Ihandle* *children);
IUP_API Ihandle*  IupSbox       (Ihandle* child);
IUP_API Ihandle*  IupSplit      (Ihandle* child1, Ihandle* child2);
IUP_API Ihandle*  IupScrollBox  (Ihandle* child);
IUP_API Ihandle*  IupFlatScrollBox(Ihandle* child);
IUP_API Ihandle*  IupGridBox    (Ihandle* child, ...);
IUP_API Ihandle*  IupGridBoxv   (Ihandle* *children);
IUP_API Ihandle*  IupMultiBox   (Ihandle* child, ...);
IUP_API Ihandle*  IupMultiBoxv  (Ihandle **children);
IUP_API Ihandle*  IupExpander(Ihandle* child);
IUP_API Ihandle*  IupDetachBox  (Ihandle* child);
IUP_API Ihandle*  IupBackgroundBox(Ihandle* child);

IUP_API Ihandle*  IupFrame      (Ihandle* child);
IUP_API Ihandle*  IupFlatFrame  (Ihandle* child);

IUP_API Ihandle*  IupImage      (int width, int height, const unsigned char* pixels);
IUP_API Ihandle*  IupImageRGB   (int width, int height, const unsigned char* pixels);
IUP_API Ihandle*  IupImageRGBA  (int width, int height, const unsigned char* pixels);

IUP_API Ihandle*  IupItem       (const char* title, const char* action);
IUP_API Ihandle*  IupSubmenu    (const char* title, Ihandle* child);
IUP_API Ihandle*  IupSeparator  (void);
IUP_API Ihandle*  IupMenu       (Ihandle* child, ...);
IUP_API Ihandle*  IupMenuv      (Ihandle* *children);

IUP_API Ihandle*  IupButton     (const char* title, const char* action);
IUP_API Ihandle*  IupFlatButton (const char* title);
IUP_API Ihandle*  IupFlatToggle (const char* title);
IUP_API Ihandle*  IupDropButton (Ihandle* dropchild);
IUP_API Ihandle*  IupFlatLabel  (const char* title);
IUP_API Ihandle*  IupFlatSeparator(void);
IUP_API Ihandle*  IupCanvas     (const char* action);
IUP_API Ihandle*  IupDialog     (Ihandle* child);
IUP_API Ihandle*  IupUser       (void);
IUP_API Ihandle*  IupThread     (void);
IUP_API Ihandle*  IupLabel      (const char* title);
IUP_API Ihandle*  IupList       (const char* action);
IUP_API Ihandle*  IupFlatList   (void);
IUP_API Ihandle*  IupText       (const char* action);
IUP_API Ihandle*  IupMultiLine  (const char* action);
IUP_API Ihandle*  IupToggle     (const char* title, const char* action);
IUP_API Ihandle*  IupTimer      (void);
IUP_API Ihandle*  IupClipboard  (void);
IUP_API Ihandle*  IupProgressBar(void);
IUP_API Ihandle*  IupVal        (const char *type);
IUP_API Ihandle*  IupFlatVal    (const char *type);
IUP_API Ihandle*  IupFlatTree   (void);
IUP_API Ihandle*  IupTabs       (Ihandle* child, ...);
IUP_API Ihandle*  IupTabsv      (Ihandle* *children);
IUP_API Ihandle*  IupFlatTabs   (Ihandle* first, ...);
IUP_API Ihandle*  IupFlatTabsv  (Ihandle* *children);
IUP_API Ihandle*  IupTree       (void);
IUP_API Ihandle*  IupLink       (const char* url, const char* title);
IUP_API Ihandle*  IupAnimatedLabel(Ihandle* animation);
IUP_API Ihandle*  IupDatePick   (void);
IUP_API Ihandle*  IupCalendar   (void);
IUP_API Ihandle*  IupColorbar   (void);
IUP_API Ihandle*  IupGauge      (void);
IUP_API Ihandle*  IupDial       (const char* type);
IUP_API Ihandle*  IupColorBrowser(void);

/* Old controls, use SPIN attribute of IupText */
IUP_API Ihandle*  IupSpin       (void);
IUP_API Ihandle*  IupSpinbox    (Ihandle* child);


/************************************************************************/
/*                      Utilities                                       */
/************************************************************************/

/* String compare utility */
IUP_API int IupStringCompare(const char* str1, const char* str2, int casesensitive, int lexicographic);

/* IupImage utilities */
IUP_API int IupSaveImageAsText(Ihandle* ih, const char* filename, const char* format, const char* name);
IUP_API Ihandle* IupImageGetHandle(const char* name);

/* IupText and IupScintilla utilities */
IUP_API void  IupTextConvertLinColToPos(Ihandle* ih, int lin, int col, int *pos);
IUP_API void  IupTextConvertPosToLinCol(Ihandle* ih, int pos, int *lin, int *col);

/* IupText, IupList, IupTree, IupMatrix and IupScintilla utility */
IUP_API int   IupConvertXYToPos(Ihandle* ih, int x, int y);

/* OLD names, kept for backward compatibility, will never be removed. */
IUP_API void IupStoreGlobal(const char* name, const char* value);
IUP_API void IupStoreAttribute(Ihandle* ih, const char* name, const char* value);
IUP_API void IupSetfAttribute(Ihandle* ih, const char* name, const char* format, ...);
IUP_API void IupStoreAttributeId(Ihandle* ih, const char* name, int id, const char *value);
IUP_API void IupSetfAttributeId(Ihandle* ih, const char* name, int id, const char* f, ...);
IUP_API void IupStoreAttributeId2(Ihandle* ih, const char* name, int lin, int col, const char* value);
IUP_API void IupSetfAttributeId2(Ihandle* ih, const char* name, int lin, int col, const char* format, ...);

/* IupTree and IupFlatTree utilities (work for both) */
IUP_API int   IupTreeSetUserId(Ihandle* ih, int id, void* userid);
IUP_API void* IupTreeGetUserId(Ihandle* ih, int id);
IUP_API int   IupTreeGetId(Ihandle* ih, void *userid);
IUP_API void  IupTreeSetAttributeHandle(Ihandle* ih, const char* name, int id, Ihandle* ih_named); /* deprecated, use IupSetAttributeHandleId */


/************************************************************************/
/*                      Pre-defined dialogs                           */
/************************************************************************/

IUP_API Ihandle* IupFileDlg(void);
IUP_API Ihandle* IupMessageDlg(void);
IUP_API Ihandle* IupColorDlg(void);
IUP_API Ihandle* IupFontDlg(void);
IUP_API Ihandle* IupProgressDlg(void);

IUP_API int  IupGetFile(char *arq);
IUP_API void IupMessage(const char *title, const char *msg);
IUP_API void IupMessagef(const char *title, const char *format, ...);
IUP_API void IupMessageError(Ihandle* parent, const char* message);
IUP_API int IupMessageAlarm(Ihandle* parent, const char* title, const char *message, const char *buttons);
IUP_API int  IupAlarm(const char *title, const char *msg, const char *b1, const char *b2, const char *b3);
IUP_API int  IupScanf(const char *format, ...);
IUP_API int  IupListDialog(int type, const char *title, int size, const char** list,
                   int op, int max_col, int max_lin, int* marks);
IUP_API int  IupGetText(const char* title, char* text, int maxsize);
IUP_API int  IupGetColor(int x, int y, unsigned char* r, unsigned char* g, unsigned char* b);

typedef int (*Iparamcb)(Ihandle* dialog, int param_index, void* user_data);
IUP_API int IupGetParam(const char* title, Iparamcb action, void* user_data, const char* format,...);
IUP_API int IupGetParamv(const char* title, Iparamcb action, void* user_data, const char* format, int param_count, int param_extra, void** param_data);
IUP_API Ihandle* IupParam(const char* format);
IUP_API Ihandle*  IupParamBox(Ihandle* param, ...);
IUP_API Ihandle*  IupParamBoxv(Ihandle* *param_array);

IUP_API Ihandle* IupLayoutDialog(Ihandle* dialog);
IUP_API Ihandle* IupElementPropertiesDialog(Ihandle* parent, Ihandle* elem);
IUP_API Ihandle* IupGlobalsDialog(void);
IUP_API Ihandle* IupClassInfoDialog(Ihandle* parent);


#ifdef __cplusplus
}
#endif

/************************************************************************/
/*                   Common Flags and Return Values                     */
/************************************************************************/
#define IUP_ERROR     1
#define IUP_NOERROR   0
#define IUP_OPENED   -1
#define IUP_INVALID  -1
#define IUP_INVALID_ID -10


/************************************************************************/
/*                   Callback Return Values                             */
/************************************************************************/
#define IUP_IGNORE    -1
#define IUP_DEFAULT   -2
#define IUP_CLOSE     -3
#define IUP_CONTINUE  -4

/************************************************************************/
/*           IupPopup and IupShowXY Parameter Values                    */
/************************************************************************/
#define IUP_CENTER        0xFFFF  /* 65535 */
#define IUP_LEFT          0xFFFE  /* 65534 */
#define IUP_RIGHT         0xFFFD  /* 65533 */
#define IUP_MOUSEPOS      0xFFFC  /* 65532 */
#define IUP_CURRENT       0xFFFB  /* 65531 */
#define IUP_CENTERPARENT  0xFFFA  /* 65530 */
#define IUP_LEFTPARENT    0xFFF9  /* 65529 */
#define IUP_RIGHTPARENT   0xFFF8  /* 65528 */
#define IUP_TOP           IUP_LEFT
#define IUP_BOTTOM        IUP_RIGHT
#define IUP_TOPPARENT     IUP_LEFTPARENT
#define IUP_BOTTOMPARENT  IUP_RIGHTPARENT

/************************************************************************/
/*               SHOW_CB Callback Values                                */
/************************************************************************/
enum{IUP_SHOW, IUP_RESTORE, IUP_MINIMIZE, IUP_MAXIMIZE, IUP_HIDE};

/************************************************************************/
/*               SCROLL_CB Callback Values                              */
/************************************************************************/
enum{IUP_SBUP,   IUP_SBDN,    IUP_SBPGUP,   IUP_SBPGDN,    IUP_SBPOSV, IUP_SBDRAGV, 
     IUP_SBLEFT, IUP_SBRIGHT, IUP_SBPGLEFT, IUP_SBPGRIGHT, IUP_SBPOSH, IUP_SBDRAGH};

/************************************************************************/
/*               Mouse Button Values and Macros                         */
/************************************************************************/
#define IUP_BUTTON1   '1'
#define IUP_BUTTON2   '2'
#define IUP_BUTTON3   '3'
#define IUP_BUTTON4   '4'
#define IUP_BUTTON5   '5'

#define iup_isshift(_s)    (_s[0]=='S')
#define iup_iscontrol(_s)  (_s[1]=='C')
#define iup_isbutton1(_s)  (_s[2]=='1')
#define iup_isbutton2(_s)  (_s[3]=='2')
#define iup_isbutton3(_s)  (_s[4]=='3')
#define iup_isdouble(_s)   (_s[5]=='D')
#define iup_isalt(_s)      (_s[6]=='A')
#define iup_issys(_s)      (_s[7]=='Y')
#define iup_isbutton4(_s)  (_s[8]=='4')
#define iup_isbutton5(_s)  (_s[9]=='5')

/* Old definitions for backward compatibility */
#define isshift     iup_isshift
#define iscontrol   iup_iscontrol
#define isbutton1   iup_isbutton1
#define isbutton2   iup_isbutton2
#define isbutton3   iup_isbutton3
#define isdouble    iup_isdouble
#define isalt       iup_isalt
#define issys       iup_issys
#define isbutton4   iup_isbutton4
#define isbutton5   iup_isbutton5


/************************************************************************/
/*                      Pre-Defined Masks                               */
/************************************************************************/
#define IUP_MASK_FLOAT       "[+/-]?(/d+/.?/d*|/./d+)"
#define IUP_MASK_UFLOAT            "(/d+/.?/d*|/./d+)"
#define IUP_MASK_EFLOAT      "[+/-]?(/d+/.?/d*|/./d+)([eE][+/-]?/d+)?"
#define IUP_MASK_UEFLOAT           "(/d+/.?/d*|/./d+)([eE][+/-]?/d+)?"
#define IUP_MASK_FLOATCOMMA  "[+/-]?(/d+/,?/d*|/,/d+)"
#define IUP_MASK_UFLOATCOMMA       "(/d+/,?/d*|/,/d+)"
#define IUP_MASK_INT          "[+/-]?/d+"
#define IUP_MASK_UINT               "/d+"

/* Old definitions for backward compatibility */
#define IUPMASK_FLOAT     IUP_MASK_FLOAT
#define IUPMASK_UFLOAT    IUP_MASK_UFLOAT
#define IUPMASK_EFLOAT    IUP_MASK_EFLOAT
#define IUPMASK_INT       IUP_MASK_INT
#define IUPMASK_UINT      IUP_MASK_UINT


/************************************************************************/
/*                   IupGetParam Callback situations                    */
/************************************************************************/
#define IUP_GETPARAM_BUTTON1 -1
#define IUP_GETPARAM_INIT    -2
#define IUP_GETPARAM_BUTTON2 -3
#define IUP_GETPARAM_BUTTON3 -4
#define IUP_GETPARAM_CLOSE   -5
#define IUP_GETPARAM_MAP     -6
#define IUP_GETPARAM_OK     IUP_GETPARAM_BUTTON1
#define IUP_GETPARAM_CANCEL IUP_GETPARAM_BUTTON2
#define IUP_GETPARAM_HELP   IUP_GETPARAM_BUTTON3

/************************************************************************/
/*                   Used by IupColorbar                                */
/************************************************************************/
#define IUP_PRIMARY -1
#define IUP_SECONDARY -2

/************************************************************************/
/*                   Record Input Modes                                 */
/************************************************************************/
enum {IUP_RECBINARY, IUP_RECTEXT};


/************************************************************************/
/*              Replacement for the WinMain in Windows,                 */
/*        this allows the application to start from "main".             */
/*        Used only for Watcom.                                         */
/************************************************************************/
#if defined (__WATCOMC__)
#ifdef __cplusplus
extern "C" {
int IupMain (int argc, char** argv); /* In C++ we have to declare the prototype */
}
#endif
#define main IupMain /* this is the trick for Watcom and MetroWerks */
#endif

/******************************************************************************
* Copyright (C) 1994-2019 Tecgraf/PUC-Rio.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************/

#endif
