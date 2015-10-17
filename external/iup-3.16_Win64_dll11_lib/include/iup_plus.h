/** \file
 * \brief Name space for C++ high level API
 *
 * See Copyright Notice in iup.h
 */

#ifndef __IUP_PLUS_H
#define __IUP_PLUS_H


#include <cd_plus.h>

#include "iup.h"
#include "iupkey.h"
#include "iup_class_cbs.hpp"
#include "iupcontrols.h"
#include "iupmatrixex.h"
#include "iupgl.h"
#include "iupglcontrols.h"
#include "iupim.h"
#include "iup_config.h"
#include "iup_mglplot.h"
#include "iup_plot.h"
#include "iupole.h"
#include "iupweb.h"
#include "iup_scintilla.h"
#include "iuptuio.h"
#include <string>


#if 0 //TODO

#ifdef _STRING_
std::string
#endif
void cdCanvasSetfAttribute(cdCanvas* canvas, const char* name, const char* format, ...);
void IupSetStrf(Ihandle* ih, const char* name, const char* format, ...);
void IupSetStrfId(Ihandle *ih, const char* name, int id, const char* format, ...);
void IupSetStrfId2(Ihandle* ih, const char* name, int lin, int col, const char* format, ...);
void IupMessagef(const char *title, const char *format, ...);

int IupScanf(const char *format, ...);
int IupGetParam(const char* title, Iparamcb action, void* user_data, const char* format, ...);
int IupGetParamv(const char* title, Iparamcb action, void* user_data, const char* format, int param_count, int param_extra, void** param_data);

Icallback IupGetCallback(Ihandle* ih, const char *name);
Icallback IupSetCallback(Ihandle* ih, const char *name, Icallback func);

void IupTreeSetAttributeHandle(Ihandle* ih, const char* name, int id, Ihandle* ih_named);

#endif

/** \brief Name space for C++ high level API
 *
 * \par
 * Defines wrapper classes for all C structures.
 *
 * See \ref iup_plus.h
 */
namespace iup
{
  inline char* Version() { return IupVersion(); }
  inline char* VersionDate() { return IupVersionDate(); }
  inline int VersionNumber() { return IupVersionNumber(); }

  inline int Open(int &argc, char **&argv) { return IupOpen(&argc, &argv); }
  inline void Close() { IupClose(); }
  inline void ImageLibOpen() { IupImageLibOpen(); }

  inline int MainLoop() { return IupMainLoop(); }
  inline int LoopStep() { return IupLoopStep(); }
  inline int LoopStepWait() { return IupLoopStepWait(); }
  inline int MainLoopLevel() { return IupMainLoopLevel(); }
  inline void Flush() { IupFlush(); }
  inline void ExitLoop() { IupExitLoop(); }

  inline int RecordInput(const char* filename, int mode) { return IupRecordInput(filename, mode); }
  inline int  PlayInput(const char* filename) { return IupPlayInput(filename); }

  inline int Help(const char* url) { return IupHelp(url); }
  inline const char* Load(const char *filename) { return IupLoad(filename); }
  inline const char* LoadBuffer(const char *buffer) { return IupLoadBuffer(buffer); }

  inline void SetLanguage(const char *lng) { IupSetLanguage(lng); }
  inline const char* GetLanguage() { return IupGetLanguage(); }
  inline void SetLanguageString(const char* name, const char* str) { IupSetLanguageString(name, str); }
  inline void StoreLanguageString(const char* name, const char* str) { IupStoreLanguageString(name, str); }
  inline const char* GetLanguageString(const char* name) { return IupGetLanguageString(name); }

  inline int GetAllClasses(char** names, int n) { return IupGetAllClasses(names, n); }
  inline int GetClassAttributes(const char* classname, char** names, int n) { return IupGetClassAttributes(classname, names, n); }
  inline int GetClassCallbacks(const char* classname, char** names, int n) { return IupGetClassCallbacks(classname, names, n); }
  inline void SetClassDefaultAttribute(const char* classname, const char *name, const char* value) { IupSetClassDefaultAttribute(classname, name, value); }

  inline void SetGlobal(const char* name, const char* value) { IupSetGlobal(name, value); }
  inline void SetStringGlobal(const char* name, const char* value) { IupSetStrGlobal(name, value); }
  inline char* GetGlobal(const char* name) { return IupGetGlobal(name); }

  inline int GetFile(char *arq) { return IupGetFile(arq); }
  inline void Message(const char *title, const char *msg) { IupMessage(title, msg); }
  inline int Alarm(const char *title, const char *msg, const char *b1, const char *b2, const char *b3) { return IupAlarm(title, msg, b1, b2, b3); }
  inline int ListDialog(int type, const char *title, int size, const char** list, int op, int max_col, int max_lin, int* marks) { return IupListDialog(type, title, size, list, op, max_col, max_lin, marks); }
  inline int GetText(const char* title, char* text) { return IupGetText(title, text); }
  inline int GetColor(int x, int y, unsigned char &r, unsigned char &g, unsigned char &b) { return IupGetColor(x, y, &r, &g, &b); }

  inline int GetAllNames(char** names, int n) { return IupGetAllNames(names, n); }
  inline int GetAllDialogs(char** names, int n) { return IupGetAllDialogs(names, n); }


  class Handle
  {
  protected:
    Ihandle* ih;

    /* forbidden */
    Handle() { ih = 0; };

  public:
    Handle(Ihandle* ref_ih) { ih = ref_ih; }
    // There is no destructor because all iup::Handle are just a reference to the Ihandle*,
    // since several IUP elements are automatically destroyed when the dialog is destroyed
    // So, to force an element to be destructed call the Destroy method

    Ihandle* GetHandle() const { return ih; }

    void SetAttribute(const char* name, const char* value) { IupSetAttribute(ih, name, value); }
    char* GetAttribute(const char* name) { return IupGetAttribute(ih, name); }
    void SetUserData(const char* name, void* data) { IupSetAttribute(ih, name, (char*)data); }
    void* GetUserData(const char* name) { return (void*)IupGetAttribute(ih, name); }
    void SetString(const char* name, const char* value) { IupSetStrAttribute(ih, name, value); }
    const char* GetString(const char* name) { return IupGetAttribute(ih, name); }
    void SetInteger(const char* name, int value) { IupSetInt(ih, name, value); }
    int GetInteger(const char* name) { return IupGetInt(ih, name); }
    void GetIntegerInteger(const char* name, int &i1, int &i2) { IupGetIntInt(ih, name, &i1, &i2); }
    void SetNumber(const char* name, double value) { IupSetDouble(ih, name, value); }
    double GetNumber(const char* name) { return IupGetDouble(ih, name); }
    void SetRGB(const char* name, unsigned char r, unsigned char g, unsigned char b) { IupSetRGB(ih, name, r, g, b); }
    void GetRGB(const char* name, unsigned char &r, unsigned char &g, unsigned char &b) { IupGetRGB(ih, name, &r, &g, &b); }

    void SetAttributeId(const char* name, int id, const char* value) { IupSetAttributeId(ih, name, id, value); }
    char* GetAttributeId(const char* name, int id) { return IupGetAttributeId(ih, name, id); }
    void SetUserDataId(const char* name, int id, void* data) { IupSetAttributeId(ih, name, id, (char*)data); }
    void* GetUserDataId(const char* name, int id) { return (void*)IupGetAttributeId(ih, name, id); }
    void SetStringId(const char* name, int id, const char* value) { IupSetStrAttributeId(ih, name, id, value); }
    const char* GetStringId(const char* name, int id) { return IupGetAttributeId(ih, name, id); }
    void SetIntegerId(const char* name, int id, int value) { IupSetIntId(ih, name, id, value); }
    int GetIntegerId(const char* name, int id) { return IupGetIntId(ih, name, id); }
    void SetNumberId(const char* name, int id, double value) { IupSetDoubleId(ih, name, id, value); }
    double GetNumberId(const char* name, int id) { return IupGetDoubleId(ih, name, id); }
    void SetRGBId(const char* name, int id, unsigned char r, unsigned char g, unsigned char b) { IupSetRGBId(ih, name, id, r, g, b); }
    void GetRGBId(const char* name, int id, unsigned char &r, unsigned char &g, unsigned char &b) { IupGetRGBId(ih, name, id, &r, &g, &b); }

    void SetAttributeId2(const char* name, int lin, int col, const char* value) { IupSetAttributeId2(ih, name, lin, col, value); }
    char* GetAttributeId2(const char* name, int lin, int col) { return IupGetAttributeId2(ih, name, lin, col); }
    void SetUserDataId2(const char* name, int lin, int col, void* data) { IupSetAttributeId2(ih, name, lin, col, (char*)data); }
    void* GetUserDataId2(const char* name, int lin, int col) { return (void*)IupGetAttributeId2(ih, name, lin, col); }
    void SetStringId2(const char* name, int lin, int col, const char* value) { IupSetStrAttributeId2(ih, name, lin, col, value); }
    const char* GetStringId2(const char* name, int lin, int col) { return IupGetAttributeId2(ih, name, lin, col); }
    void SetIntegerId2(const char* name, int lin, int col, int value) { IupSetIntId2(ih, name, lin, col, value); }
    int GetIntegerId2(const char* name, int lin, int col) { return IupGetIntId2(ih, name, lin, col); }
    void SetNumberId2(const char* name, int lin, int col, double value) { IupSetDoubleId2(ih, name, lin, col, value); }
    double GetNumberId2(const char* name, int lin, int col) { return IupGetDoubleId2(ih, name, lin, col); }
    void SetRGBId2(const char* name, int lin, int col, unsigned char r, unsigned char g, unsigned char b) { IupSetRGBId2(ih, name, lin, col, r, g, b); }
    void GetRGBId2(const char* name, int lin, int col, unsigned char &r, unsigned char &g, unsigned char &b) { IupGetRGBId2(ih, name, lin, col, &r, &g, &b); }

    void ResetAttribute(const char* name) { IupResetAttribute(ih, name); }
    int GetAllAttributes(char** names, int n) { return IupGetAllAttributes(ih, names, n); }
    void SetAttributeHandle(const char* name, const  Handle& handle) { IupSetAttributeHandle(ih, name, handle.GetHandle()); }
    Handle GetAttributeHandle(const char* name) { return IupGetAttributeHandle(ih, name); }

    void Destroy() { IupDestroy(ih); }

    int Map() { return IupMap(ih); }
    void Unmap() { IupUnmap(ih); }

    char* GetName() { return IupGetName(ih); }

    char* GetClassName() { return IupGetClassName(ih); }
    char* GetClassType() { return IupGetClassType(ih); }
    void SaveClassAttributes() { IupSaveClassAttributes(ih); }
    void CopyClassAttributesTo(const Handle& dst) { IupCopyClassAttributes(ih, dst.ih); }
    int ClassMatch(const char* classname) { return IupClassMatch(ih, classname); }

  };

  inline Handle GetHandle(const char *name) { return Handle(IupGetHandle(name)); }
  inline Handle SetHandle(const char *name, const Handle& handle) { return Handle(IupSetHandle(name, handle.GetHandle())); }
  inline void SetLanguagePack(const Handle& handle) { IupSetLanguagePack(handle.GetHandle()); }
  inline Handle Paramf(const char* format) { return Handle(IupParamf(format)); }
  inline Handle ParamBox(const Handle& parent, const Handle* params_handle, int count) {
    Ihandle** params = new Ihandle* [count];
    for (int i = 0; i < count; i++)
      params[i] = params_handle[i].GetHandle();
    Handle handle = Handle(IupParamBox(parent.GetHandle(), params, count));
    delete [] params;
    return handle;
  }

  class Dialog;
  class Container;

  class Control : public Handle
  {
  public:
    Control(Ihandle* _ih) : Handle(_ih) {}

    void Update() { IupUpdate(ih); }
    void Redraw() { IupRedraw(ih, 0); }
    void Refresh() { IupRefresh(ih); }

    void Detach(const Control& child) { IupDetach(child.ih); }

    Control GetBrother() { return Control(IupGetBrother(ih)); }
    Container GetParent();
    Dialog GetDialog();
    Control GetDialogChild(const char* name) { return Control(IupGetDialogChild(ih, name)); }
    int Reparent(const Container& new_parent, const Control& ref_child);

    Control SetFocus() { return Control(IupSetFocus(ih)); }
    Control PreviousField() { return Control(IupPreviousField(ih)); }
    Control NextField() { return Control(IupNextField(ih)); }
    
    void ConvertLinColToPos(int lin, int col, int &pos) { IupTextConvertLinColToPos(ih, lin, col, &pos); }
    void ConvertPosToLinCol(int pos, int &lin, int &col) { IupTextConvertPosToLinCol(ih, pos, &lin, &col); }
    int ConvertXYToPos(int x, int y) { return IupConvertXYToPos(ih, x, y); }
  };

  inline Control GetFocus() { return Control(IupGetFocus()); }

  class Container : public Control
  {
  public:
    Container(Ihandle* _ih) : Control(_ih) {}
    Container(Ihandle* _ih, const Control* child_array, int count) : Control(_ih) {
      for (int i = 0; i < count; i++)
        IupAppend(ih, child_array[i].GetHandle());
    }
    Container(Ihandle* _ih, const Control* child0, const Control* child1 = 0, const Control* child2 = 0, const Control* child3 = 0, const Control* child4 = 0, const Control* child5 = 0, const Control* child6 = 0, const Control* child7 = 0, const Control* child8 = 0, const Control* child9 = 0)
      : Control(_ih) {
      if (child0) IupAppend(ih, child0->GetHandle());
      if (child1) IupAppend(ih, child1->GetHandle());
      if (child2) IupAppend(ih, child2->GetHandle());
      if (child3) IupAppend(ih, child3->GetHandle());
      if (child4) IupAppend(ih, child4->GetHandle());
      if (child5) IupAppend(ih, child5->GetHandle());
      if (child6) IupAppend(ih, child6->GetHandle());
      if (child7) IupAppend(ih, child7->GetHandle());
      if (child8) IupAppend(ih, child8->GetHandle());
      if (child9) IupAppend(ih, child9->GetHandle());
    }

    Control Append(const Control& child) { return IupAppend(ih, child.GetHandle()); }
    Control Insert(const Control& ref_child, const  Control& child) { return IupInsert(ih, ref_child.GetHandle(), child.GetHandle()); }
    Control GetChild(int pos) { return IupGetChild(ih, pos); }
    int GetChildPos(const  Control& child) { return IupGetChildPos(ih, child.GetHandle()); }
    int GetChildCount() { return IupGetChildCount(ih); }

    Control GetFirstChild() { return Control(IupGetNextChild(ih, 0)); }
    Control GetNextChild(const Control& ref_child) { return Control(IupGetNextChild(ih, ref_child.GetHandle())); }

    void UpdateChildren() { IupUpdateChildren(ih); }
    void RedrawChildren() { IupRedraw(ih, 1); }
    void RefreshChildren() { IupRefreshChildren(ih); }
  };
  class Dialog : public Container
  {
  public:
    Dialog(Ihandle* _ih) : Container(_ih) {}
    Dialog(const Control& child) : Container(IupDialog(child.GetHandle())) { }

    int Popup(int x, int y) { return IupPopup(ih, x, y); }
    int Show() { return IupShow(ih); }
    int ShowXY(int x, int y) { return IupShowXY(ih, x, y); }
    int Hide() { return IupHide(ih); }
  };

  inline Dialog Control::GetDialog() { return Dialog(IupGetDialog(ih)); }
  inline Dialog LayoutDialog(const Dialog& dialog) { return Dialog(IupLayoutDialog(dialog.GetHandle())); }
  inline Dialog ElementPropertiesDialog(const Control& control) { return Dialog(IupElementPropertiesDialog(control.GetHandle())); }
  inline Container Control::GetParent() { return Container(IupGetParent(ih)); }
  inline int Control::Reparent(const Container& new_parent, const Control& ref_child) { return IupReparent(ih, new_parent.GetHandle(), ref_child.GetHandle()); }

  class Menu : public Container
  {
  public:
    Menu() : Container(IupMenu(0)) {}
    Menu(const Control& child) : Container(IupMenu(child.GetHandle(), 0)) {}
    Menu(const Control* child0, const Control* child1 = 0, const Control* child2 = 0, const Control* child3 = 0, const Control* child4 = 0, const Control* child5 = 0, const Control* child6 = 0, const Control* child7 = 0, const Control* child8 = 0, const Control* child9 = 0)
      : Container(IupMenu(0), child0, child1, child2, child3, child4, child5, child6, child7, child8, child9) {}
    Menu(const Control* child_array, int count) : Container(IupMenu(0), child_array, count) {}

    int Popup(int x, int y) { return IupPopup(ih, x, y); }
  };

  class Image : public Handle
  {
  public:
    Image(const char* filename) : Handle(IupLoadImage(filename)) {}
    Image(const im::Image& image) : Handle(IupImageFromImImage(image.GetHandle())) {}

    int Save(const char* filename, const char* im_format) { return IupSaveImage(ih, filename, im_format); }
    int SaveAsText(const char* filename, const char* iup_format, const char* name) { return IupSaveImageAsText(ih, filename, iup_format, name); }
  };
  class Clipboard : public Handle
  {
  public:
    Clipboard() : Handle(IupClipboard()) {}

    void SetImage(const im::Image& image) { SetUserData("NATIVEIMAGE", IupGetImageNativeHandle(image.GetHandle())); }

    im::Image GetImage(void) { return im::Image(IupGetNativeHandleImage(GetUserData("NATIVEIMAGE"))); }
  };
  class User : public Handle
  {
  public:
    User() : Handle(IupUser()) {}
  };
  class Timer : public Handle
  {
  public:
    Timer() : Handle(IupTimer()) {}
  };
  class MenuSeparator : public Control
  {
  public:
    MenuSeparator() : Control(IupSeparator()) {}
  };
  class MenuItem : public Control
  {
  public:
    MenuItem(const char* title = 0) : Control(IupItem(title, 0)) {}
  };
  class Canvas : public Control
  {
  public:
    Canvas() : Control(IupCanvas(0)) {}
  };
  class Link : public Control
  {
  public:
    Link(const char* url = 0, const char* title = 0) : Control(IupLink(url, title)) {}
  };
  class Label : public Control
  {
  public:
    Label(const char* title = 0) : Control(IupLabel(title)) {}
  };
  class Button : public Control
  {
  public:
    Button(const char* title = 0) : Control(IupButton(title, 0)) {}
  };
  class FlatButton : public Control
  {
  public:
    FlatButton(const char* title = 0) : Control(IupFlatButton(title)) {}
  };
  class Toggle : public Control
  {
  public:
    Toggle(const char* title = 0) : Control(IupToggle(title, 0)) {}
  };
  class Fill: public Control
  {
  public:
    Fill() : Control(IupFill()) {}
  };
  class Spin: public Control
  {
  public:
    Spin() : Control(IupSpin()) {}
  };
  class Tree: public Control
  {
  public:
    Tree() : Control(IupTree()) {}
    
    int SetUserId(int id, void* userid) { return IupTreeSetUserId(ih, id, userid); }
    void* GetUserId(int id) { return IupTreeGetUserId(ih, id); }
    int GetId(void *userid) { return IupTreeGetId(ih, userid); }
  };
  class Val : public Control
  {
  public:
    Val(const char* orientation = 0) : Control(IupVal(orientation)) {}
  };
  class ProgressBar: public Control
  {
  public:
    ProgressBar() : Control(IupProgressBar()) {}
  };
  class List: public Control
  {
  public:
    List() : Control(IupList(0)) {}
  };
  class Text: public Control
  {
  public:
    Text() : Control(IupText(0)) {}
  };
  class Split : public Container
  {
  public:
    Split() : Container(IupSplit(0, 0)) {}
    Split(const Control& child) : Container(IupSplit(child.GetHandle(), 0)) {}
    Split(const Control& child1, const Control& child2) : Container(IupSplit(child1.GetHandle(), child2.GetHandle())) {}
  };
  class Submenu : public Container
  {
  public:
    Submenu(const char* title = 0) : Container(IupSubmenu(title, 0)) {}
    Submenu(const char* title, const Control& child) : Container(IupSubmenu(title, child.GetHandle())) {}
  };
  class Radio : public Container
  {
  public:
    Radio() : Container(IupRadio(0)) {}
    Radio(const Control& child) : Container(IupRadio(child.GetHandle())) {}
  };
  class Sbox : public Container
  {
  public:
    Sbox() : Container(IupSbox(0)) {}
    Sbox(const Control& child) : Container(IupSbox(child.GetHandle())) {}
  };
  class ScrollBox : public Container
  {
  public:
    ScrollBox() : Container(IupScrollBox(0)) {}
    ScrollBox(const Control& child) : Container(IupScrollBox(child.GetHandle())) {}
  };
  class Expander : public Container
  {
  public:
    Expander() : Container(IupExpander(0)) {}
    Expander(const Control& child) : Container(IupExpander(child.GetHandle())) {}
  };
  class DetachBox : public Container
  {
  public:
    DetachBox() : Container(IupDetachBox(0)) {}
    DetachBox(const Control& child) : Container(IupDetachBox(child.GetHandle())) {}
  };
  class BackgroundBox : public Container
  {
  public:
    BackgroundBox() : Container(IupBackgroundBox(0)) {}
    BackgroundBox(const Control& child) : Container(IupBackgroundBox(child.GetHandle())) {}
  };
  class Frame : public Container
  {
  public:
    Frame() : Container(IupFrame(0)) {}
    Frame(const Control& child) : Container(IupFrame(child.GetHandle())) {}
  };
  class Spinbox : public Container
  {
  public:
    Spinbox() : Container(IupSpinbox(0)) {}
    Spinbox(const Control& child) : Container(IupSpinbox(child.GetHandle())) {}
  };
  class Vbox : public Container
  {
  public:
    Vbox() : Container(IupVbox(0)) {}
    Vbox(const Control& child) : Container(IupVbox(child.GetHandle(), 0)) {}
    Vbox(const Control* child0, const Control* child1 = 0, const Control* child2 = 0, const Control* child3 = 0, const Control* child4 = 0, const Control* child5 = 0, const Control* child6 = 0, const Control* child7 = 0, const Control* child8 = 0, const Control* child9 = 0)
      : Container(IupVbox(0), child0, child1, child2, child3, child4, child5, child6, child7, child8, child9) {}
    Vbox(const Control* child_array, int count) : Container(IupVbox(0), child_array, count) {}
  };
  class Hbox : public Container
  {
  public:
    Hbox() : Container(IupHbox(0)) {}
    Hbox(const Control& child) : Container(IupHbox(child.GetHandle(), 0)) {}
    Hbox(const Control* child0, const Control* child1 = 0, const Control* child2 = 0, const Control* child3 = 0, const Control* child4 = 0, const Control* child5 = 0, const Control* child6 = 0, const Control* child7 = 0, const Control* child8 = 0, const Control* child9 = 0)
      : Container(IupHbox(0), child0, child1, child2, child3, child4, child5, child6, child7, child8, child9) {}
    Hbox(const Control* child_array, int count) : Container(IupHbox(0), child_array, count) {}
  };
  class Zbox : public Container
  {
  public:
    Zbox() : Container(IupZbox(0)) {}
    Zbox(const Control& child) : Container(IupZbox(child.GetHandle(), 0)) {}
    Zbox(const Control* child0, const Control* child1 = 0, const Control* child2 = 0, const Control* child3 = 0, const Control* child4 = 0, const Control* child5 = 0, const Control* child6 = 0, const Control* child7 = 0, const Control* child8 = 0, const Control* child9 = 0)
      : Container(IupZbox(0), child0, child1, child2, child3, child4, child5, child6, child7, child8, child9) {}
    Zbox(const Control* child_array, int count) : Container(IupZbox(0), child_array, count) {}
  };
  class Cbox : public Container
  {
  public:
    Cbox() : Container(IupCbox(0)) {}
    Cbox(const Control& child) : Container(IupCbox(child.GetHandle(), 0)) {}
    Cbox(const Control* child0, const Control* child1 = 0, const Control* child2 = 0, const Control* child3 = 0, const Control* child4 = 0, const Control* child5 = 0, const Control* child6 = 0, const Control* child7 = 0, const Control* child8 = 0, const Control* child9 = 0)
      : Container(IupCbox(0), child0, child1, child2, child3, child4, child5, child6, child7, child8, child9) {}
    Cbox(const Control* child_array, int count) : Container(IupCbox(0), child_array, count) {}
  };
  class Tabs : public Container
  {
  public:
    Tabs() : Container(IupTabs(0)) {}
    Tabs(const Control& child) : Container(IupTabs(child.GetHandle(), 0)) {}
    Tabs(const Control* child0, const Control* child1 = 0, const Control* child2 = 0, const Control* child3 = 0, const Control* child4 = 0, const Control* child5 = 0, const Control* child6 = 0, const Control* child7 = 0, const Control* child8 = 0, const Control* child9 = 0)
      : Container(IupTabs(0), child0, child1, child2, child3, child4, child5, child6, child7, child8, child9) {}
    Tabs(const Control* child_array, int count) : Container(IupTabs(0), child_array, count) {}
  };
  class GridBox : public Container
  {
  public:
    GridBox() : Container(IupGridBox(0)) {}
    GridBox(const Control& child) : Container(IupGridBox(child.GetHandle(), 0)) {}
    GridBox(const Control* child0, const Control* child1 = 0, const Control* child2 = 0, const Control* child3 = 0, const Control* child4 = 0, const Control* child5 = 0, const Control* child6 = 0, const Control* child7 = 0, const Control* child8 = 0, const Control* child9 = 0)
      : Container(IupGridBox(0), child0, child1, child2, child3, child4, child5, child6, child7, child8, child9) {}
    GridBox(const Control* child_array, int count) : Container(IupGridBox(0), child_array, count) {}
  };
  class Normalizer : public Container
  {
  public:
    Normalizer() : Container(IupNormalizer(0)) {}
    Normalizer(const Control& child) : Container(IupNormalizer(child.GetHandle(), 0)) {}
    Normalizer(const Control* child0, const Control* child1 = 0, const Control* child2 = 0, const Control* child3 = 0, const Control* child4 = 0, const Control* child5 = 0, const Control* child6 = 0, const Control* child7 = 0, const Control* child8 = 0, const Control* child9 = 0)
      : Container(IupNormalizer(0), child0, child1, child2, child3, child4, child5, child6, child7, child8, child9) {}
    Normalizer(const Control* child_array, int count) : Container(IupNormalizer(0), child_array, count) {}
  };


  class FileDialog : public Dialog
  {
  public:
    FileDialog() : Dialog(IupFileDlg()) {}
  };
  class MessageDialog : public Dialog
  {
  public:
    MessageDialog() : Dialog(IupMessageDlg()) {}
  };
  class ColorDialog : public Dialog
  {
  public:
    ColorDialog() : Dialog(IupColorDlg()) {}
  };
  class FontDialog : public Dialog
  {
  public:
    FontDialog() : Dialog(IupFontDlg()) {}
  };
  class ProgressDialog : public Dialog
  {
  public:
    ProgressDialog() : Dialog(IupProgressDlg()) {}
  };
  class GLCanvas : public Control
  {
  public:
    GLCanvas() : Control(IupGLCanvas(0)) {}

    static void Open() { IupGLCanvasOpen(); }

    void MakeCurrent() { IupGLMakeCurrent(ih); }
    int IsCurrent() { return IupGLIsCurrent(ih); }
    void SwapBuffers() { IupGLSwapBuffers(ih); }
    void Palette(int index, float r, float g, float b) { IupGLPalette(ih, index, r, g, b); }
    void UseFont(int first, int count, int list_base) { IupGLUseFont(ih, first, count, list_base); }

    static void Wait(int gl) { IupGLWait(gl); }
  };

  class Controls
  {
  public:
    static void Open() { IupControlsOpen(); }
  };
  class Dial : public Control
  {
  public:
    Dial(const char* orientation = 0) : Control(IupDial(orientation)) {}
  };
  class Gauge : public Control
  {
  public:
    Gauge() : Control(IupGauge()) {}
  };
  class ColorBrowser : public Control
  {
  public:
    ColorBrowser() : Control(IupColorBrowser()) {}
  };
  class Cells : public Control
  {
  public:
    Cells() : Control(IupCells()) {}
  };
  class Colorbar : public Control
  {
  public:
    Colorbar() : Control(IupColorbar()) {}
  };
  class Matrix : public Control
  {
  public:
    Matrix() : Control(IupMatrix(0)) {}

    void SetFormula(int col, const char* formula, const char* init = 0) { IupMatrixSetFormula(ih, col, formula, init); }
    void SetDynamic(Ihandle* ih, const char* init = 0) { IupMatrixSetDynamic(ih, init); }
  };
  class MatrixList : public Control
  {
  public:
    MatrixList() : Control(IupMatrixList()) {}
  };
  class MatrixEx : public Control
  {
  public:
    MatrixEx() : Control(IupMatrixEx()) {}

    static void Open() { IupMatrixExOpen(); }
  };
  class GLControls
  {
  public:
    static void Open() { IupGLControlsOpen(); }
  };
  class GLSubCanvas : public Control
  {
  public:
    GLSubCanvas() : Control(IupGLSubCanvas()) {}
  };
  class GLSeparator : public Control
  {
  public:
    GLSeparator() : Control(IupGLSeparator()) {}
  };
  class GLProgressBar : public Control
  {
  public:
    GLProgressBar() : Control(IupGLProgressBar()) {}
  };
  class GLVal : public Control
  {
  public:
    GLVal() : Control(IupGLVal()) {}
  };
  class GLLabel : public Control
  {
  public:
    GLLabel(const char* title = 0) : Control(IupGLLabel(title)) {}
  };
  class GLButton : public Control
  {
  public:
    GLButton(const char* title = 0) : Control(IupGLButton(title)) {}
  };
  class GLToggle : public Control
  {
  public:
    GLToggle(const char* title = 0) : Control(IupGLToggle(title)) {}
  };
  class GLLink : public Control
  {
  public:
    GLLink(const char *url = 0, const char* title = 0) : Control(IupGLLink(url, title)) {}
  };
  class GLFrame : public Container
  {
  public:
    GLFrame(const Control& child) : Container(IupGLFrame(child.GetHandle())) {}
    GLFrame() : Container(IupGLFrame(0)) {}
  };
  class GLExpander : public Container
  {
  public:
    GLExpander(const Control& child) : Container(IupGLExpander(child.GetHandle())) {}
    GLExpander() : Container(IupGLExpander(0)) {}
  };
  class GLScrollBox : public Container
  {
  public:
    GLScrollBox(const Control& child) : Container(IupGLScrollBox(child.GetHandle())) {}
    GLScrollBox() : Container(IupGLScrollBox(0)) {}
  };
  class GLSizeBox : public Container
  {
  public:
    GLSizeBox(const Control& child) : Container(IupGLSizeBox(child.GetHandle())) {}
    GLSizeBox() : Container(IupGLSizeBox(0)) {}
  };
  class GLCanvasBox : public Container
  {
  public:
    GLCanvasBox() : Container(IupGLCanvasBox(0)) {}
    GLCanvasBox(const Control& child) : Container(IupGLCanvasBox(child.GetHandle(), 0)) {}
    GLCanvasBox(const Control* child0, const Control* child1 = 0, const Control* child2 = 0, const Control* child3 = 0, const Control* child4 = 0, const Control* child5 = 0, const Control* child6 = 0, const Control* child7 = 0, const Control* child8 = 0, const Control* child9 = 0) 
      : Container(IupGLCanvasBox(0), child0, child1, child2, child3, child4, child5, child6, child7, child8, child9) {}
    GLCanvasBox(const Control* child_array, int count) : Container(IupGLCanvasBox(0), child_array, count) {}
  };
  class Plot : public Control
  {
  public:
    Plot() : Control(IupPlot()) {}

    static void Open() { IupPlotOpen(); }

    void Begin(int strXdata) { IupPlotBegin(ih, strXdata); }
    void Add(double x, double y) { IupPlotAdd(ih, x, y); }
    void AddStr(const char* x, double y) { IupPlotAddStr(ih, x, y); }
    void AddSegment(double x, double y) { IupPlotAddSegment(ih, x, y); }
    int End(Ihandle *ih) { return IupPlotEnd(ih); }

    int LoadData(const char* filename, int strXdata) { return IupPlotLoadData(ih, filename, strXdata); }

    int SetFormula(int sample_count, const char* formula, const char* init) { return IupPlotSetFormula(ih, sample_count, formula, init); }

    void Insert(int ds_index, int sample_index, double x, double y) { IupPlotInsert(ih, ds_index, sample_index, x, y); }
    void InsertStr(int ds_index, int sample_index, const char* x, double y) { IupPlotInsertStr(ih, ds_index, sample_index, x, y); }
    void InsertSegment(int ds_index, int sample_index, double x, double y) { IupPlotInsertSegment(ih, ds_index, sample_index, x, y); }

    void InsertStrSamples(int ds_index, int sample_index, const char** x, double* y, int count) { IupPlotInsertStrSamples(ih, ds_index, sample_index, x, y, count); }
    void InsertSamples(int ds_index, int sample_index, double *x, double *y, int count) { IupPlotInsertSamples(ih, ds_index, sample_index, x, y, count); }

    void AddSamples(int ds_index, double *x, double *y, int count) { IupPlotAddSamples(ih, ds_index, x, y, count); }
    void AddStrSamples(int ds_index, const char** x, double* y, int count) { IupPlotAddStrSamples(ih, ds_index, x, y, count); }

    void GetSample(int ds_index, int sample_index, double &x, double &y) { IupPlotGetSample(ih, ds_index, sample_index, &x, &y); }
    void GetSampleStr(int ds_index, int sample_index, const char* &x, double &y) { IupPlotGetSampleStr(ih, ds_index, sample_index, &x, &y); }
    int GetSampleSelection(int ds_index, int sample_index) { return IupPlotGetSampleSelection(ih, ds_index, sample_index); }
    void SetSample(int ds_index, int sample_index, double x, double y) { IupPlotSetSample(ih, ds_index, sample_index, x, y); }
    void SetSampleStr(int ds_index, int sample_index, const char* x, double y) { IupPlotSetSampleStr(ih, ds_index, sample_index, x, y); }
    void SetSampleSelection(int ds_index, int sample_index, int selected) { IupPlotSetSampleSelection(ih, ds_index, sample_index, selected); }

    void Transform(double x, double y, double &cnv_x, double &cnv_y) { IupPlotTransform(ih, x, y, &cnv_x, &cnv_y); }
    void TransformTo(double cnv_x, double cnv_y, double &x, double &y) { IupPlotTransformTo(ih, cnv_x, cnv_y, &x, &y); }

    int FindSample(double cnv_x, double cnv_y, int &ds_index, int &sample_index) { return IupPlotFindSample(ih, cnv_x, cnv_y, &ds_index, &sample_index); }

    void PaintTo(cd::Canvas& cd_canvas) { IupPlotPaintTo(ih, cd_canvas.GetHandle()); }
  };
  class MglPlot : public Control
  {
  public:
    MglPlot() : Control(IupMglPlot()) {}

    static void Open() { IupMglPlotOpen(); }

    void Begin(int dim) { IupMglPlotBegin(ih, dim); }
    void Add1D(const char* name, double y) { IupMglPlotAdd1D(ih, name, y); }
    void Add2D(double x, double y) { IupMglPlotAdd2D(ih, x, y); }
    void Add3D(double x, double y, double z) { IupMglPlotAdd3D(ih, x, y, z); }
    int End() { return IupMglPlotEnd(ih); }

    int NewDataSet(int dim) { return IupMglPlotNewDataSet(ih, dim); }

    void Insert1D(int ds_index, int sample_index, const char** names, const double* y, int count) { IupMglPlotInsert1D(ih, ds_index, sample_index, names, y, count); }
    void Insert2D(int ds_index, int sample_index, const double* x, const double* y, int count) { IupMglPlotInsert2D(ih, ds_index, sample_index, x, y, count); }
    void Insert3D(int ds_index, int sample_index, const double* x, const double* y, const double* z, int count) { IupMglPlotInsert3D(ih, ds_index, sample_index, x, y, z, count); }

    void Set1D(int ds_index, const char** names, const double* y, int count) { IupMglPlotSet1D(ih, ds_index, names, y, count); }
    void Set2D(int ds_index, const double* x, const double* y, int count) { IupMglPlotSet2D(ih, ds_index, x, y, count); }
    void Set3D(int ds_index, const double* x, const double* y, const double* z, int count) { IupMglPlotSet3D(ih, ds_index, x, y, z, count); }
    void SetFormula(int ds_index, const char* formulaX, const char* formulaY, const char* formulaZ, int count) { IupMglPlotSetFormula(ih, ds_index, formulaX, formulaY, formulaZ, count); }

    void SetData(int ds_index, const double* data, int count_x, int count_y, int count_z) { IupMglPlotSetData(ih, ds_index, data, count_x, count_y, count_z); }
    void LoadData(int ds_index, const char* filename, int count_x, int count_y, int count_z) { IupMglPlotLoadData(ih, ds_index, filename, count_x, count_y, count_z); }
    void SetFromFormula(int ds_index, const char* formula, int count_x, int count_y, int count_z) { IupMglPlotSetFromFormula(ih, ds_index, formula, count_x, count_y, count_z); }

    void Transform(double x, double y, double z, int &ix, int &iy) { IupMglPlotTransform(ih, x, y, z, &ix, &iy); }
    void TransformTo(int ix, int iy, double &x, double &y, double &z) { IupMglPlotTransformTo(ih, ix, iy, &x, &y, &z); }

    void DrawMark(double x, double y, double z) { IupMglPlotDrawMark(ih, x, y, z); }
    void DrawLine(double x1, double y1, double z1, double x2, double y2, double z2) { IupMglPlotDrawLine(ih, x1, y1, z1, x2, y2, z2); }
    void DrawText(const char* text, double x, double y, double z) { IupMglPlotDrawText(ih, text, x, y, z); }

    void PaintTo(const char* format, int w, int h, double dpi, unsigned char* data) { IupMglPlotPaintTo(ih, format, w, h, dpi, (void*)data); }
    void PaintTo(const char* format, int w, int h, double dpi, const char* filename) { IupMglPlotPaintTo(ih, format, w, h, dpi, (void*)filename); }

  };
  class MglLabel : public Control
  {
  public:
    MglLabel(const char* title) : Control(IupMglLabel(title)) {}
  };
  class OleControl : public Control
  {
  public:
    OleControl(const char* progid) : Control(IupOleControl(progid)) {}

    static void Open() { IupOleControlOpen(); }
  };
  class WebBrowser : public Control
  {
  public:
    WebBrowser() : Control(IupWebBrowser()) {}

    static void Open() { IupWebBrowserOpen(); }
  };
  class Scintilla : public Control
  {
  public:
    Scintilla(): Control(IupScintilla()) {}

    static void Open() { IupScintillaOpen(); }
  };
  class TuioClient : public Handle
  {
  public:
    TuioClient(int port) : Handle(IupTuioClient(port)) {}

    static void Open() { IupTuioOpen(); }
  };

  class Config: public Handle
  {
  public:
    Config(): Handle(IupConfig()) { }

    int LoadConfig() { return IupConfigLoad(ih); }
    int SaveConfig() { return IupConfigSave(ih); }

    void SetVariableStrId(const char* group, const char* key, int id, const char* value) { IupConfigSetVariableStrId(ih, group, key, id, value); }
    void SetVariableIntId(const char* group, const char* key, int id, int value) { IupConfigSetVariableIntId(ih, group, key, id, value); }
    void SetVariableDoubleId(const char* group, const char* key, int id, double value) { IupConfigSetVariableDoubleId(ih, group, key, id, value); }
    void SetVariableStr(const char* group, const char* key, const char* value) { IupConfigSetVariableStr(ih, group, key, value); }
    void SetVariableInt(const char* group, const char* key, int value) { IupConfigSetVariableInt(ih, group, key, value); }
    void SetVariableDouble(const char* group, const char* key, double value) { IupConfigSetVariableDouble(ih, group, key, value); }

    char* GetVariableStr(const char* group, const char* key) { return (char*)IupConfigGetVariableStr(ih, group, key); }
    int GetVariableInt(const char* group, const char* key) { return IupConfigGetVariableInt(ih, group, key); }
    double GetVariableDouble(const char* group, const char* key) { return IupConfigGetVariableDouble(ih, group, key); }
    char* GetVariableStrId(const char* group, const char* key, int id) { return (char*)IupConfigGetVariableStrId(ih, group, key, id); }
    int GetVariableIntId(const char* group, const char* key, int id) { return IupConfigGetVariableIntId(ih, group, key, id); }
    double GetVariableDoubleId(const char* group, const char* key, int id) { return IupConfigGetVariableDoubleId(ih, group, key, id); }

    char* GetVariableStrDef(const char* group, const char* key, const char* def) { return (char*)IupConfigGetVariableStrDef(ih, group, key, def); }
    int GetVariableIntDef(const char* group, const char* key, int def) { return IupConfigGetVariableIntDef(ih, group, key, def); }
    double GetVariableDoubleDef(const char* group, const char* key, double def) { return IupConfigGetVariableDoubleDef(ih, group, key, def); }
    char* GetVariableStrIdDef(const char* group, const char* key, int id, const char* def) { return (char*)IupConfigGetVariableStrIdDef(ih, group, key, id, def); }
    int GetVariableIntIdDef(const char* group, const char* key, int id, int def) { return IupConfigGetVariableIntIdDef(ih, group, key, id, def); }
    double GetVariableDoubleIdDef(const char* group, const char* key, int id, double def) { return IupConfigGetVariableDoubleIdDef(ih, group, key, id, def); }

    void RecentInit(Ihandle* menu, Icallback recent_cb, int max_recent) { IupConfigRecentInit(ih, menu, recent_cb, max_recent); }
    void RecentUpdate(const char* filename) { IupConfigRecentUpdate(ih, filename); }

    void DialogShow(Ihandle* dialog, const char* name) { IupConfigDialogShow(ih, dialog, name); }
    void DialogClosed(Ihandle* dialog, const char* name) { IupConfigDialogClosed(ih, dialog, name); }
  };
}

namespace cd
{
  class CanvasIup : public Canvas
  {
  public:
    CanvasIup(iup::Canvas& iup_canvas)
      : Canvas() { canvas = cdCreateCanvas(CD_IUP, iup_canvas.GetHandle()); }
  };
  class CanvasIupDoubleBuffer : public Canvas
  {
  public:
    CanvasIupDoubleBuffer(iup::Canvas& iup_canvas)
      : Canvas() { canvas = cdCreateCanvas(CD_IUPDBUFFER, iup_canvas.GetHandle()); }
  };
  class CanvasIupDoubleBufferRGB : public Canvas
  {
  public:
    CanvasIupDoubleBufferRGB(iup::Canvas& iup_canvas)
      : Canvas() { canvas = cdCreateCanvas(CD_IUPDBUFFERRGB, iup_canvas.GetHandle()); }
  };
}

#endif
