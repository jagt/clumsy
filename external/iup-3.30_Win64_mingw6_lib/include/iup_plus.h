/** \file
 * \brief Name space for C++ high level API
 *
 * See Copyright Notice in iup.h
 */

#ifndef __IUP_PLUS_H
#define __IUP_PLUS_H


#include "iup.h"
#include "iupkey.h"
#include "iupdraw.h"
#include "iup_class_cbs.hpp"
#include "iupcontrols.h"
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



/** \brief Name space for C++ high level API
 *
 * \par
 * Defines wrapper classes for all C structures.
 *
 * See \ref iup_plus.h
 */
namespace Iup
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
  inline int PlayInput(const char* filename) { return IupPlayInput(filename); }

  inline int Help(const char* url) { return IupHelp(url); }
  inline void Log(const char* type, const char* str) { IupLog(type, "%s", str); }
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

  inline int GetFile(char* filename) { return IupGetFile(filename); }
  inline void Message(const char *title, const char *msg) { IupMessage(title, msg); }
  inline int Alarm(const char *title, const char *msg, const char *b1, const char *b2, const char *b3) { return IupAlarm(title, msg, b1, b2, b3); }
  inline int ListDialog(int type, const char *title, int size, const char** list, int op, int max_col, int max_lin, int* marks) { return IupListDialog(type, title, size, list, op, max_col, max_lin, marks); }
  inline int GetText(const char* title, char* text, int maxsize = 10240) { return IupGetText(title, text, maxsize); }
  inline int GetColor(int x, int y, unsigned char &r, unsigned char &g, unsigned char &b) { return IupGetColor(x, y, &r, &g, &b); }
  inline int GetParamv(const char* title, Iparamcb action, void* user_data, const char* format, int param_count, int param_extra, void** param_data)
    { return IupGetParamv(title, action, user_data, format, param_count, param_extra, param_data); }

  inline int GetAllNames(char** names, int n) { return IupGetAllNames(names, n); }
  inline int GetAllDialogs(char** names, int n) { return IupGetAllDialogs(names, n); }


  class Element
  {
  protected:
    Ihandle* ih;

    /* forbidden */
    Element() { ih = 0; };

  public:
    Element(Ihandle* ref_ih)  { ih = ref_ih; }
    Element(const Element& elem) : Element(elem.ih) {}

    virtual ~Element() 
    {
      // The destructor does not destroy the element because all Iup::Element are just a reference to the Ihandle*,
      // since several IUP elements are automatically destroyed when the dialog is destroyed.
      // So to force an element to be destroyed explicitly call the Destroy method.
    }

    Ihandle* GetHandle() const { return ih; }

    bool Failed() const {
      return ih == 0;
    }

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
    void SetRGBA(const char* name, unsigned char r, unsigned char g, unsigned char b, unsigned char a) { IupSetRGBA(ih, name, r, g, b, a); }
    void GetRGBA(const char* name, unsigned char &r, unsigned char &g, unsigned char &b, unsigned char &a) { IupGetRGBA(ih, name, &r, &g, &b, &a); }

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

    Element SetAttributes(const char* str) { return IupSetAttributes(ih, str); }
    void ResetAttribute(const char* name) { IupResetAttribute(ih, name); }
    int GetAllAttributes(char** names, int n) { return IupGetAllAttributes(ih, names, n); }
    void CopyAttributes(Ihandle* dst_ih) { IupCopyAttributes(ih, dst_ih); }
    void SetAttributeHandle(const char* name, const  Element& elem) { IupSetAttributeHandle(ih, name, elem.GetHandle()); }
    Element GetAttributeHandle(const char* name) { return IupGetAttributeHandle(ih, name); }
    void SetAttributeHandleId(const char* name, int id, const Element& elem) { IupSetAttributeHandleId(ih, name, id, elem.GetHandle()); }
    Element GetAttributeHandleId(const char* name, int id) { return IupGetAttributeHandleId(ih, name, id); }
    void SetAttributeHandleId2(const char* name, int lin, int col, const Element& elem) { IupSetAttributeHandleId2(ih, name, lin, col, elem.GetHandle()); }
    Element GetAttributeHandleId2(const char* name, int lin, int col) { return IupGetAttributeHandleId2(ih, name, lin, col); }

    Icallback GetCallback(const char *name) { return IupGetCallback(ih, name); }
    Icallback SetCallback(const char *name, Icallback func) { return IupSetCallback(ih, name, func); }

    void Destroy() { IupDestroy(ih); }

    int Map() { return IupMap(ih); }
    void Unmap() { IupUnmap(ih); }

    char* GetName() { return IupGetName(ih); }

    char* GetClassName() { return IupGetClassName(ih); }
    char* GetClassType() { return IupGetClassType(ih); }
    void SaveClassAttributes() { IupSaveClassAttributes(ih); }
    void CopyClassAttributesTo(const Element& dst) { IupCopyClassAttributes(ih, dst.ih); }
    int ClassMatch(const char* classname) { return IupClassMatch(ih, classname); }

  };

  inline Icallback GetFunction(const char *name) { return IupGetFunction(name); }
  inline Icallback SetFunction(const char *name, Icallback func) { return IupSetFunction(name, func); }
  inline Element GetHandle(const char *name) { return Element(IupGetHandle(name)); }
  inline Element SetHandle(const char *name, const Element& elem) { return Element(IupSetHandle(name, elem.GetHandle())); }
  inline void SetLanguagePack(const Element& elem) { IupSetLanguagePack(elem.GetHandle()); }

  class Dialog;
  class Container;

  class Control : public Element
  {
  public:
    Control(Ihandle* _ih) : Element(_ih) {}
    Control(const Control& control) : Element(control.ih) {}
    Control(const Element& elem) : Element(elem.GetHandle()) {}

    Control SetAttributes(const char* str) { IupSetAttributes(ih, str); return *this; }

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
    Container(const Container& container) : Control(container.ih) {}
    Container(const Control& control) : Control(control.GetHandle()) {}
    Container(const Element& elem) : Control(elem.GetHandle()) {}

    Container(Ihandle* _ih, const Control* child_array, int count) : Control(_ih) {
      for (int i = 0; i < count; i++)
        IupAppend(ih, child_array[i].GetHandle());
    }
    Container(Ihandle* _ih, Control child0, Control child1 = (Ihandle*)0, Control child2 = (Ihandle*)0, Control child3 = (Ihandle*)0, Control child4 = (Ihandle*)0, Control child5 = (Ihandle*)0, Control child6 = (Ihandle*)0, Control child7 = (Ihandle*)0, Control child8 = (Ihandle*)0, Control child9 = (Ihandle*)0)
      : Control(_ih) {
      if (!child0.Failed()) IupAppend(ih, child0.GetHandle());
      if (!child1.Failed()) IupAppend(ih, child1.GetHandle());
      if (!child2.Failed()) IupAppend(ih, child2.GetHandle());
      if (!child3.Failed()) IupAppend(ih, child3.GetHandle());
      if (!child4.Failed()) IupAppend(ih, child4.GetHandle());
      if (!child5.Failed()) IupAppend(ih, child5.GetHandle());
      if (!child6.Failed()) IupAppend(ih, child6.GetHandle());
      if (!child7.Failed()) IupAppend(ih, child7.GetHandle());
      if (!child8.Failed()) IupAppend(ih, child8.GetHandle());
      if (!child9.Failed()) IupAppend(ih, child9.GetHandle());
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
    Dialog(const Dialog& dialog) : Container(dialog.GetHandle()) {}
    Dialog(const Element& elem) : Container(elem.GetHandle()) {}
    Dialog(Control child) : Container(IupDialog(child.GetHandle())) { }
    Dialog(Container child) : Container(IupDialog(child.GetHandle())) { }
    Dialog(Ihandle* _ih) : Container(_ih) {}

    int Popup(int x, int y) { return IupPopup(ih, x, y); }
    int Show() { return IupShow(ih); }
    int ShowXY(int x, int y) { return IupShowXY(ih, x, y); }
    int Hide() { return IupHide(ih); }
  };

  inline Dialog Control::GetDialog() { return Dialog(IupGetDialog(ih)); }
  inline Dialog LayoutDialog(const Dialog& dialog) { return Dialog(IupLayoutDialog(dialog.GetHandle())); }
  inline Dialog GlobalsDialog() { return Dialog(IupGlobalsDialog()); }
  inline Dialog ElementPropertiesDialog(const Dialog& parent, const Control& control) { return Dialog(IupElementPropertiesDialog(parent.GetHandle(), control.GetHandle())); }
  inline Dialog ElementPropertiesDialog(const Control& control) { return Dialog(IupElementPropertiesDialog(0, control.GetHandle())); }
  inline Dialog ClassInfoDialog(const Dialog& parent) { return Dialog(IupClassInfoDialog(parent.GetHandle())); }
  inline Container Control::GetParent() { return Container(IupGetParent(ih)); }
  inline int Control::Reparent(const Container& new_parent, const Control& ref_child) { return IupReparent(ih, new_parent.GetHandle(), ref_child.GetHandle()); }

  void MessageError(const Dialog& parent, const char* message)
    { IupMessageError(parent.GetHandle(), message); }
  int MessageAlarm(const Dialog& parent, const char* title, const char* message, const char* buttons)
    { return IupMessageAlarm(parent.GetHandle(), title, message, buttons); }

  class Menu : public Container
  {
  public:
    Menu() : Container(IupMenu(0)) {}
    Menu(Control child0, Control child1 = (Ihandle*)0, Control child2 = (Ihandle*)0, Control child3 = (Ihandle*)0, Control child4 = (Ihandle*)0, Control child5 = (Ihandle*)0, Control child6 = (Ihandle*)0, Control child7 = (Ihandle*)0, Control child8 = (Ihandle*)0, Control child9 = (Ihandle*)0)
      : Container(IupMenu(0), child0, child1, child2, child3, child4, child5, child6, child7, child8, child9) {}
    Menu(const Control *child_array, int count) : Container(IupMenu(0), child_array, count) {}
    Menu(const Menu& menu) : Container(menu.GetHandle()) {}
    Menu(Ihandle* _ih) : Container(_ih) {}

    int Popup(int x, int y) { return IupPopup(ih, x, y); }
  };

  class Image : public Element
  {
  public:
    Image(Ihandle* _ih) : Element(_ih) {}
    Image(const Element& elem) : Element(elem.GetHandle()) {}
    Image(const char* name) : Element(IupImageGetHandle(name)) {}

    int SaveAsText(const char* filename, const char* iup_format, const char* name) { return IupSaveImageAsText(ih, filename, iup_format, name); }

#ifdef __IM_PLUS_H
    Image(const im::Image& image) : Element(IupImageFromImImage(image.GetHandle())) {}
    Image Load(const char* filename) { return Image(IupLoadImage(filename)); }
    int Save(const char* filename, const char* im_format) { return IupSaveImage(ih, filename, im_format); }
    im::Image ToImImage() { return im::Image(IupImageToImImage(GetHandle())); }
#endif
  };
  class Clipboard : public Element
  {
  public:
    Clipboard() : Element(IupClipboard()) {}
    Clipboard(Ihandle* _ih) : Element(_ih) {}
    Clipboard(const Element& elem) : Element(elem.GetHandle()) {}

#ifdef __IM_PLUS_H
    void SetImage(const im::Image& image) { SetUserData("NATIVEIMAGE", IupGetImageNativeHandle(image.GetHandle())); }
    im::Image GetImage(void) { return im::Image(IupGetNativeHandleImage(GetUserData("NATIVEIMAGE"))); }
#endif
  };
  class User : public Element
  {
  public:
    User() : Element(IupUser()) {}
    User(Ihandle* _ih) : Element(_ih) {}
    User(const Element& elem) : Element(elem.GetHandle()) {}
  };
  class Thread : public Element
  {
  public:
    Thread() : Element(IupThread()) {}
    Thread(Ihandle* _ih) : Element(_ih) {}
    Thread(const Element& elem) : Element(elem.GetHandle()) {}
  };
  class Param : public Element
  {
  public:
    Param(const char* format) : Element(IupParam(format)) {}
    Param(Ihandle* _ih) : Element(_ih) {}
    Param(const Element& elem) : Element(elem.GetHandle()) {}
  };
  class Timer : public Element
  {
  public:
    Timer() : Element(IupTimer()) {}
    Timer(Ihandle* _ih) : Element(_ih) {}
    Timer(const Element& elem) : Element(elem.GetHandle()) {}
  };
  class Separator : public Control
  {
  public:
    Separator() : Control(IupSeparator()) {}
    Separator(Ihandle* _ih) : Control(_ih) {}
    Separator(const Element& elem) : Control(elem.GetHandle()) {}
  };
  class Item : public Control
  {
  public:
    Item(const char* title = 0) : Control(IupItem(title, 0)) {}
    Item(Ihandle* _ih) : Control(_ih) {}
    Item(const Element& elem) : Control(elem.GetHandle()) {}
  };
  class Canvas : public Control
  {
  public:
    Canvas() : Control(IupCanvas(0)) {}
    Canvas(Ihandle* _ih) : Control(_ih) {}
    Canvas(const Element& elem) : Control(elem.GetHandle()) {}

    void DrawBegin() { IupDrawBegin(ih); }
    void DrawEnd() { IupDrawEnd(ih); }
    void DrawSetClipRect(int x1, int y1, int x2, int y2) { IupDrawSetClipRect(ih, x1, y1, x2, y2); }
    void DrawGetClipRect(int *x1, int *y1, int *x2, int *y2) { IupDrawGetClipRect(ih, x1, y1, x2, y2); }
    void DrawResetClip() { IupDrawResetClip(ih); }
    void DrawParentBackground() { IupDrawParentBackground(ih); }
    void DrawLine(int x1, int y1, int x2, int y2) { IupDrawLine(ih, x1, y1, x2, y2); }
    void DrawRectangle(int x1, int y1, int x2, int y2) { IupDrawRectangle(ih, x1, y1, x2, y2); }
    void DrawArc(int x1, int y1, int x2, int y2, double a1, double a2) { IupDrawArc(ih, x1, y1, x2, y2, a1, a2); }
    void DrawPolygon(int* points, int count) { IupDrawPolygon(ih, points, count); }
    void DrawText(const char* text, int len, int x, int y, int w, int h) { IupDrawText(ih, text, len, x, y, w, h); }
    void DrawImage(const char* name, int x, int y, int w, int h) { IupDrawImage(ih, name, x, y, w, h); }
    void DrawSelectRect(int x1, int y1, int x2, int y2) { IupDrawSelectRect(ih, x1, y1, x2, y2); }
    void DrawFocusRect(int x1, int y1, int x2, int y2) { IupDrawFocusRect(ih, x1, y1, x2, y2); }
    void DrawGetSize(int &w, int &h) { IupDrawGetSize(ih, &w, &h); }
    void DrawGetTextSize(const char* str, int len, int &w, int &h) { IupDrawGetTextSize(ih, str, len, &w, &h); }
    void DrawGetImageInfo(const char* name, int &w, int &h, int &bpp) { IupDrawGetImageInfo(name, &w, &h, &bpp); }
  };
  class Link : public Control
  {
  public:
    Link(const char* url = 0, const char* title = 0) : Control(IupLink(url, title)) {}
    Link(Ihandle* _ih) : Control(_ih) {}
    Link(const Element& elem) : Control(elem.GetHandle()) {}
  };
  class Label : public Control
  {
  public:
    Label(const char* title = 0) : Control(IupLabel(title)) {}
    Label(Ihandle* _ih) : Control(_ih) {}
    Label(const Element& elem) : Control(elem.GetHandle()) {}
  };

  class Button : public Control
  {
  public:
    Button(const char* title = 0) : Control(IupButton(title, 0)) {}
    Button(Ihandle* _ih) : Control(_ih) {}
    Button(const Element& elem) : Control(elem.GetHandle()) {}
  };
  class FlatButton : public Control
  {
  public:
    FlatButton(const char* title = 0) : Control(IupFlatButton(title)) {}
    FlatButton(Ihandle* _ih) : Control(_ih) {}
    FlatButton(const Element& elem) : Control(elem.GetHandle()) {}
  };
  class FlatToggle : public Control
  {
  public:
    FlatToggle(const char* title = 0) : Control(IupFlatToggle(title)) {}
    FlatToggle(Ihandle* _ih) : Control(_ih) {}
    FlatToggle(const Element& elem) : Control(elem.GetHandle()) {}
  };
  class FlatSeparator : public Control
  {
  public:
    FlatSeparator() : Control(IupFlatSeparator()) {}
    FlatSeparator(Ihandle* _ih) : Control(_ih) {}
    FlatSeparator(const Element& elem) : Control(elem.GetHandle()) {}
  };
  class Space : public Control
  {
  public:
    Space() : Control(IupSpace()) {}
    Space(Ihandle* _ih) : Control(_ih) {}
    Space(const Element& elem) : Control(elem.GetHandle()) {}
  };
  class DropButton : public Control
  {
  public:
    DropButton() : Control(IupDropButton(0)) {}
    DropButton(Control child) : Control(IupDropButton(child.GetHandle())) {}
    DropButton(Ihandle* _ih) : Control(_ih) {}
    DropButton(const Element& elem) : Control(elem.GetHandle()) {}
  };
  class FlatLabel : public Control
  {
  public:
    FlatLabel(const char* title = 0) : Control(IupFlatLabel(title)) {}
    FlatLabel(Ihandle* _ih) : Control(_ih) {}
    FlatLabel(const Element& elem) : Control(elem.GetHandle()) {}
  };
  class AnimatedLabel : public Control
  {
  public:
    AnimatedLabel(Element animation = (Ihandle*)0) : Control(IupAnimatedLabel(animation.GetHandle())) {}
    AnimatedLabel(Ihandle* _ih) : Control(_ih) {}
    AnimatedLabel(const Element& elem) : Control(elem.GetHandle()) {}
  };
  class Toggle : public Control
  {
  public:
    Toggle(const char* title = 0) : Control(IupToggle(title, 0)) {}
    Toggle(Ihandle* _ih) : Control(_ih) {}
    Toggle(const Element& elem) : Control(elem.GetHandle()) {}
  };
  class Fill: public Control
  {
  public:
    Fill() : Control(IupFill()) {}
    Fill(Ihandle* _ih) : Control(_ih) {}
    Fill(const Element& elem) : Control(elem.GetHandle()) {}
  };
  class Spin: public Control
  {
  public:
    Spin() : Control(IupSpin()) {}
    Spin(Ihandle* _ih) : Control(_ih) {}
    Spin(const Element& elem) : Control(elem.GetHandle()) {}
  };
  class Tree: public Control
  {
  public:
    Tree() : Control(IupTree()) {}
    Tree(Ihandle* _ih) : Control(_ih) {}
    Tree(const Element& elem) : Control(elem.GetHandle()) {}

    int SetUserId(int id, void* userid) { return IupTreeSetUserId(ih, id, userid); }
    void* GetUserId(int id) { return IupTreeGetUserId(ih, id); }
    int GetId(void *userid) { return IupTreeGetId(ih, userid); }
  };
  class Val : public Control
  {
  public:
    Val(const char* orientation = 0) : Control(IupVal(orientation)) {}
    Val(Ihandle* _ih) : Control(_ih) {}
    Val(const Element& elem) : Control(elem.GetHandle()) {}
  };
  class FlatVal : public Control
  {
  public:
    FlatVal(const char* orientation = 0) : Control(IupFlatVal(orientation)) {}
    FlatVal(Ihandle* _ih) : Control(_ih) {}
    FlatVal(const Element& elem) : Control(elem.GetHandle()) {}
  };
  class FlatTree : public Control
  {
  public:
    FlatTree() : Control(IupFlatTree()) {}
    FlatTree(Ihandle* _ih) : Control(_ih) {}
    FlatTree(const Element& elem) : Control(elem.GetHandle()) {}
  };
  class ProgressBar : public Control
  {
  public:
    ProgressBar() : Control(IupProgressBar()) {}
    ProgressBar(Ihandle* _ih) : Control(_ih) {}
    ProgressBar(const Element& elem) : Control(elem.GetHandle()) {}
  };
  class List: public Control
  {
  public:
    List() : Control(IupList(0)) {}
    List(Ihandle* _ih) : Control(_ih) {}
    List(const Element& elem) : Control(elem.GetHandle()) {}
  };
  class FlatList : public Control
  {
  public:
    FlatList() : Control(IupFlatList()) {}
    FlatList(Ihandle* _ih) : Control(_ih) {}
    FlatList(const Element& elem) : Control(elem.GetHandle()) {}
  };
  class Text : public Control
  {
  public:
    Text() : Control(IupText(0)) {}
    Text(Ihandle* _ih) : Control(_ih) {}
    Text(const Element& elem) : Control(elem.GetHandle()) {}
  };

  class Split : public Container
  {
  public:
    Split() : Container(IupSplit(0, 0)) {}
    Split(Control child) : Container(IupSplit(child.GetHandle(), 0)) {}
    Split(Control child1, Control child2) : Container(IupSplit(child1.GetHandle(), child2.GetHandle())) {}
    Split(const Split& split) : Container(split.GetHandle()) {}
    Split(Ihandle* _ih) : Container(_ih) {}
  };
  class Submenu : public Container
  {
  public:
    Submenu(const char* title = 0) : Container(IupSubmenu(title, 0)) {}
    Submenu(const char* title, Control child) : Container(IupSubmenu(title, child.GetHandle())) {}
    Submenu(const Submenu& container) : Container(container.GetHandle()) {}
    Submenu(Ihandle* _ih) : Container(_ih) {}
  };
  class Radio : public Container
  {
  public:
    Radio() : Container(IupRadio(0)) {}
    Radio(Control child) : Container(IupRadio(child.GetHandle())) {}
    Radio(const Radio& container) : Container(container.GetHandle()) {}
    Radio(Ihandle* _ih) : Container(_ih) {}
  };
  class Sbox : public Container
  {
  public:
    Sbox() : Container(IupSbox(0)) {}
    Sbox(Control child) : Container(IupSbox(child.GetHandle())) {}
    Sbox(const Sbox& container) : Container(container.GetHandle()) {}
    Sbox(Ihandle* _ih) : Container(_ih) {}
  };
  class ScrollBox : public Container
  {
  public:
    ScrollBox() : Container(IupScrollBox(0)) {}
    ScrollBox(Control child) : Container(IupScrollBox(child.GetHandle())) {}
    ScrollBox(const ScrollBox& container) : Container(container.GetHandle()) {}
    ScrollBox(Ihandle* _ih) : Container(_ih) {}
  };
  class FlatScrollBox : public Container
  {
  public:
    FlatScrollBox() : Container(IupFlatScrollBox(0)) {}
    FlatScrollBox(Control child) : Container(IupFlatScrollBox(child.GetHandle())) {}
    FlatScrollBox(const FlatScrollBox& container) : Container(container.GetHandle()) {}
    FlatScrollBox(Ihandle* _ih) : Container(_ih) {}
  };
  class Expander : public Container
  {
  public:
    Expander(const Expander& container) : Container(container.GetHandle()) {}
    Expander() : Container(IupExpander(0)) {}
    Expander(Control child) : Container(IupExpander(child.GetHandle())) {}
    Expander(Ihandle* _ih) : Container(_ih) {}
  };
  class DetachBox : public Container
  {
  public:
    DetachBox(const DetachBox& container) : Container(container.GetHandle()) {}
    DetachBox() : Container(IupDetachBox(0)) {}
    DetachBox(Control child) : Container(IupDetachBox(child.GetHandle())) {}
    DetachBox(Ihandle* _ih) : Container(_ih) {}
  };
  class BackgroundBox : public Container
  {
  public:
    BackgroundBox() : Container(IupBackgroundBox(0)) {}
    BackgroundBox(Control child) : Container(IupBackgroundBox(child.GetHandle())) {}
    BackgroundBox(const BackgroundBox& container) : Container(container.GetHandle()) {}
    BackgroundBox(Ihandle* _ih) : Container(_ih) {}

    void DrawBegin() { IupDrawBegin(ih); }
    void DrawEnd() { IupDrawEnd(ih); }
    void DrawSetClipRect(int x1, int y1, int x2, int y2) { IupDrawSetClipRect(ih, x1, y1, x2, y2); }
    void DrawGetClipRect(int *x1, int *y1, int *x2, int *y2) { IupDrawGetClipRect(ih, x1, y1, x2, y2); }
    void DrawResetClip() { IupDrawResetClip(ih); }
    void DrawParentBackground() { IupDrawParentBackground(ih); }
    void DrawLine(int x1, int y1, int x2, int y2) { IupDrawLine(ih, x1, y1, x2, y2); }
    void DrawRectangle(int x1, int y1, int x2, int y2) { IupDrawRectangle(ih, x1, y1, x2, y2); }
    void DrawArc(int x1, int y1, int x2, int y2, double a1, double a2) { IupDrawArc(ih, x1, y1, x2, y2, a1, a2); }
    void DrawPolygon(int* points, int count) { IupDrawPolygon(ih, points, count); }
    void DrawText(const char* text, int len, int x, int y, int w, int h) { IupDrawText(ih, text, len, x, y, w, h); }
    void DrawImage(const char* name, int x, int y, int w, int h) { IupDrawImage(ih, name, x, y, w, h); }
    void DrawSelectRect(int x1, int y1, int x2, int y2) { IupDrawSelectRect(ih, x1, y1, x2, y2); }
    void DrawFocusRect(int x1, int y1, int x2, int y2) { IupDrawFocusRect(ih, x1, y1, x2, y2); }
    void DrawGetSize(int &w, int &h) { IupDrawGetSize(ih, &w, &h); }
    void DrawGetTextSize(const char* str, int len, int &w, int &h) { IupDrawGetTextSize(ih, str, len, &w, &h); }
    void DrawGetImageInfo(const char* name, int &w, int &h, int &bpp) { IupDrawGetImageInfo(name, &w, &h, &bpp); }
  };

  class Frame : public Container
  {
  public:
    Frame() : Container(IupFrame(0)) {}
    Frame(Control child) : Container(IupFrame(child.GetHandle())) {}
    Frame(const Frame& container) : Container(container.GetHandle()) {}
    Frame(Ihandle* _ih) : Container(_ih) {}
  };
  class FlatFrame : public Container
  {
  public:
    FlatFrame() : Container(IupFlatFrame(0)) {}
    FlatFrame(Control child) : Container(IupFlatFrame(child.GetHandle())) {}
    FlatFrame(const FlatFrame& container) : Container(container.GetHandle()) {}
    FlatFrame(Ihandle* _ih) : Container(_ih) {}
  };
  class Spinbox : public Container
  {
  public:
    Spinbox() : Container(IupSpinbox(0)) {}
    Spinbox(Control child) : Container(IupSpinbox(child.GetHandle())) {}
    Spinbox(const Spinbox& container) : Container(container.GetHandle()) {}
    Spinbox(Ihandle* _ih) : Container(_ih) {}
  };

  class Vbox : public Container
  {
    Vbox(const Vbox& box) : Container(box.GetHandle()) {}  /* to avoid hierarchy construction problems */
  public:
    Vbox() : Container(IupVbox(0)) {}
    Vbox(Control child0, Control child1 = (Ihandle*)0, Control child2 = (Ihandle*)0, Control child3 = (Ihandle*)0, Control child4 = (Ihandle*)0, Control child5 = (Ihandle*)0, Control child6 = (Ihandle*)0, Control child7 = (Ihandle*)0, Control child8 = (Ihandle*)0, Control child9 = (Ihandle*)0)
      : Container(IupVbox(0), child0, child1, child2, child3, child4, child5, child6, child7, child8, child9) {}
    Vbox(const Control *child_array, int count) : Container(IupVbox(0), child_array, count) {}
    Vbox(Ihandle* _ih) : Container(_ih) {}
  };
  class Hbox : public Container
  {
    Hbox(const Hbox& box) : Container(box.GetHandle()) {}  /* to avoid hierarchy construction problems */
  public:
    Hbox() : Container(IupHbox(0)) {}
    Hbox(Control child0, Control child1 = (Ihandle*)0, Control child2 = (Ihandle*)0, Control child3 = (Ihandle*)0, Control child4 = (Ihandle*)0, Control child5 = (Ihandle*)0, Control child6 = (Ihandle*)0, Control child7 = (Ihandle*)0, Control child8 = (Ihandle*)0, Control child9 = (Ihandle*)0)
      : Container(IupHbox(0), child0, child1, child2, child3, child4, child5, child6, child7, child8, child9) {}
    Hbox(const Control *child_array, int count) : Container(IupHbox(0), child_array, count) {}
    Hbox(Ihandle* _ih) : Container(_ih) {}
  };
  class Zbox : public Container
  {
  public:
    Zbox() : Container(IupZbox(0)) {}
    Zbox(Control child0, Control child1 = (Ihandle*)0, Control child2 = (Ihandle*)0, Control child3 = (Ihandle*)0, Control child4 = (Ihandle*)0, Control child5 = (Ihandle*)0, Control child6 = (Ihandle*)0, Control child7 = (Ihandle*)0, Control child8 = (Ihandle*)0, Control child9 = (Ihandle*)0)
      : Container(IupZbox(0), child0, child1, child2, child3, child4, child5, child6, child7, child8, child9) {}
    Zbox(const Control *child_array, int count) : Container(IupZbox(0), child_array, count) {}
    Zbox(const Zbox& box) : Container(box.GetHandle()) {}
    Zbox(Ihandle* _ih) : Container(_ih) {}
  };
  class Cbox : public Container
  {
  public:
    Cbox() : Container(IupCbox(0)) {}
    Cbox(Control child0, Control child1 = (Ihandle*)0, Control child2 = (Ihandle*)0, Control child3 = (Ihandle*)0, Control child4 = (Ihandle*)0, Control child5 = (Ihandle*)0, Control child6 = (Ihandle*)0, Control child7 = (Ihandle*)0, Control child8 = (Ihandle*)0, Control child9 = (Ihandle*)0)
      : Container(IupCbox(0), child0, child1, child2, child3, child4, child5, child6, child7, child8, child9) {}
    Cbox(const Control *child_array, int count) : Container(IupCbox(0), child_array, count) {}
    Cbox(const Cbox& box) : Container(box.GetHandle()) {}
    Cbox(Ihandle* _ih) : Container(_ih) {}
  };
  class Tabs : public Container
  {
  public:
    Tabs() : Container(IupTabs(0)) {}
    Tabs(Control child0, Control child1 = (Ihandle*)0, Control child2 = (Ihandle*)0, Control child3 = (Ihandle*)0, Control child4 = (Ihandle*)0, Control child5 = (Ihandle*)0, Control child6 = (Ihandle*)0, Control child7 = (Ihandle*)0, Control child8 = (Ihandle*)0, Control child9 = (Ihandle*)0)
      : Container(IupTabs(0), child0, child1, child2, child3, child4, child5, child6, child7, child8, child9) {}
    Tabs(const Control *child_array, int count) : Container(IupTabs(0), child_array, count) {}
    Tabs(const Tabs& tabs) : Container(tabs.GetHandle()) {}
    Tabs(Ihandle* _ih) : Container(_ih) {}
  };
  class FlatTabs : public Container
  {
  public:
    FlatTabs() : Container(IupFlatTabs(0)) {}
    FlatTabs(Control child0, Control child1 = (Ihandle*)0, Control child2 = (Ihandle*)0, Control child3 = (Ihandle*)0, Control child4 = (Ihandle*)0, Control child5 = (Ihandle*)0, Control child6 = (Ihandle*)0, Control child7 = (Ihandle*)0, Control child8 = (Ihandle*)0, Control child9 = (Ihandle*)0)
      : Container(IupFlatTabs(0), child0, child1, child2, child3, child4, child5, child6, child7, child8, child9) {}
    FlatTabs(const Control *child_array, int count) : Container(IupFlatTabs(0), child_array, count) {}
    FlatTabs(const FlatTabs& tabs) : Container(tabs.GetHandle()) {}
    FlatTabs(Ihandle* _ih) : Container(_ih) {}
  };
  class GridBox : public Container
  {
  public:
    GridBox() : Container(IupGridBox(0)) {}
    GridBox(Control child0, Control child1 = (Ihandle*)0, Control child2 = (Ihandle*)0, Control child3 = (Ihandle*)0, Control child4 = (Ihandle*)0, Control child5 = (Ihandle*)0, Control child6 = (Ihandle*)0, Control child7 = (Ihandle*)0, Control child8 = (Ihandle*)0, Control child9 = (Ihandle*)0)
      : Container(IupGridBox(0), child0, child1, child2, child3, child4, child5, child6, child7, child8, child9) {}
    GridBox(const Control *child_array, int count) : Container(IupGridBox(0), child_array, count) {}
    GridBox(const GridBox& box) : Container(box.GetHandle()) {}
    GridBox(Ihandle* _ih) : Container(_ih) {}
  };
  class MultiBox : public Container
  {
  public:
    MultiBox() : Container(IupMultiBox(0)) {}
    MultiBox(Control child0, Control child1 = (Ihandle*)0, Control child2 = (Ihandle*)0, Control child3 = (Ihandle*)0, Control child4 = (Ihandle*)0, Control child5 = (Ihandle*)0, Control child6 = (Ihandle*)0, Control child7 = (Ihandle*)0, Control child8 = (Ihandle*)0, Control child9 = (Ihandle*)0)
      : Container(IupMultiBox(0), child0, child1, child2, child3, child4, child5, child6, child7, child8, child9) {}
    MultiBox(const Control *child_array, int count) : Container(IupMultiBox(0), child_array, count) {}
    MultiBox(const MultiBox& box) : Container(box.GetHandle()) {}
    MultiBox(Ihandle* _ih) : Container(_ih) {}
  };
  class ParamBox : public Container
  {
  public:
    ParamBox() : Container(IupParamBox(0)) {}
    ParamBox(Control child0, Control child1 = (Ihandle*)0, Control child2 = (Ihandle*)0, Control child3 = (Ihandle*)0, Control child4 = (Ihandle*)0, Control child5 = (Ihandle*)0, Control child6 = (Ihandle*)0, Control child7 = (Ihandle*)0, Control child8 = (Ihandle*)0, Control child9 = (Ihandle*)0)
      : Container(IupParamBox(child0.GetHandle(), child1.GetHandle(), child2.GetHandle(), child3.GetHandle(), child4.GetHandle(), child5.GetHandle(), child6.GetHandle(), child7.GetHandle(), child8.GetHandle(), child9.GetHandle(), 0)) {}
    ParamBox(const Control *child_array, int count) : Container(IupParamBox(0), child_array, count) {}
    ParamBox(const ParamBox& box) : Container(box.GetHandle()) {}
    ParamBox(Ihandle* _ih) : Container(_ih) {}
  };
  class Normalizer : public Container
  {
  public:
    Normalizer() : Container(IupNormalizer(0)) {}
    Normalizer(Control child0, Control child1 = (Ihandle*)0, Control child2 = (Ihandle*)0, Control child3 = (Ihandle*)0, Control child4 = (Ihandle*)0, Control child5 = (Ihandle*)0, Control child6 = (Ihandle*)0, Control child7 = (Ihandle*)0, Control child8 = (Ihandle*)0, Control child9 = (Ihandle*)0)
      : Container(IupNormalizer(0), child0, child1, child2, child3, child4, child5, child6, child7, child8, child9) {}
    Normalizer(const Control *child_array, int count) : Container(IupNormalizer(0), child_array, count) {}
    Normalizer(const Normalizer& elem) : Container(elem.GetHandle()) {}
    Normalizer(Ihandle* _ih) : Container(_ih) {}
  };


  class FileDlg : public Dialog
  {
  public:
    FileDlg() : Dialog(IupFileDlg()) {}
  };
  class MessageDlg : public Dialog
  {
  public:
    MessageDlg() : Dialog(IupMessageDlg()) {}
  };
  class ColorDlg : public Dialog
  {
  public:
    ColorDlg() : Dialog(IupColorDlg()) {}
  };
  class FontDlg : public Dialog
  {
  public:
    FontDlg() : Dialog(IupFontDlg()) {}
  };
  class ProgressDlg : public Dialog
  {
  public:
    ProgressDlg() : Dialog(IupProgressDlg()) {}
  };
  class ScintillaDlg : public Dialog
  {
  public:
    ScintillaDlg() : Dialog(IupScintillaDlg()) {}
  };
#ifdef LUA_VERSION
  public:
    LuaScripterDlg(lua_State *L) : Dialog(IupLuaScripterDlg(L)) {}
  };
#endif

  class GLCanvas : public Control
  {
  public:
    GLCanvas() : Control(IupGLCanvas(0)) {}
    GLCanvas(Ihandle* _ih) : Control(_ih) {}
    GLCanvas(const Element& elem) : Control(elem.GetHandle()) {}

    static void Open() { IupGLCanvasOpen(); }

    void MakeCurrent() { IupGLMakeCurrent(ih); }
    int IsCurrent() { return IupGLIsCurrent(ih); }
    void SwapBuffers() { IupGLSwapBuffers(ih); }
    void Palette(int index, float r, float g, float b) { IupGLPalette(ih, index, r, g, b); }
    void UseFont(int first, int count, int list_base) { IupGLUseFont(ih, first, count, list_base); }

    static void Wait(int gl) { IupGLWait(gl); }
  };
  class GLBackgroundBox : public Container
  {
  public:
    GLBackgroundBox() : Container(IupGLBackgroundBox(0)) {}
    GLBackgroundBox(Control child) : Container(IupGLBackgroundBox(child.GetHandle())) {}
    GLBackgroundBox(const GLBackgroundBox& container) : Container(container.GetHandle()) {}
    GLBackgroundBox(Ihandle* _ih) : Container(_ih) {}
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
    Dial(Ihandle* _ih) : Control(_ih) {}
    Dial(const Element& elem) : Control(elem.GetHandle()) {}
  };
  class Gauge : public Control
  {
  public:
    Gauge() : Control(IupGauge()) {}
    Gauge(Ihandle* _ih) : Control(_ih) {}
    Gauge(const Element& elem) : Control(elem.GetHandle()) {}
  };
  class ColorBrowser : public Control
  {
  public:
    ColorBrowser() : Control(IupColorBrowser()) {}
    ColorBrowser(Ihandle* _ih) : Control(_ih) {}
    ColorBrowser(const Element& elem) : Control(elem.GetHandle()) {}
  };
  class Cells : public Control
  {
  public:
    Cells() : Control(IupCells()) {}
    Cells(Ihandle* _ih) : Control(_ih) {}
    Cells(const Element& elem) : Control(elem.GetHandle()) {}
  };
  class Colorbar : public Control
  {
  public:
    Colorbar() : Control(IupColorbar()) {}
    Colorbar(Ihandle* _ih) : Control(_ih) {}
    Colorbar(const Element& elem) : Control(elem.GetHandle()) {}
  };
  class Matrix : public Control
  {
  public:
    Matrix() : Control(IupMatrix(0)) {}
    Matrix(Ihandle* _ih) : Control(_ih) {}
    Matrix(const Element& elem) : Control(elem.GetHandle()) {}

    void SetFormula(int col, const char* formula, const char* init = 0) { IupMatrixSetFormula(ih, col, formula, init); }
    void SetDynamic(const char* init = 0) { IupMatrixSetDynamic(ih, init); }
  };
  class MatrixList : public Control
  {
  public:
    MatrixList() : Control(IupMatrixList()) {}
    MatrixList(Ihandle* _ih) : Control(_ih) {}
    MatrixList(const Element& elem) : Control(elem.GetHandle()) {}
  };
  class MatrixEx : public Control
  {
  public:
    MatrixEx() : Control(IupMatrixEx()) {}
    MatrixEx(Ihandle* _ih) : Control(_ih) {}
    MatrixEx(const Element& elem) : Control(elem.GetHandle()) {}
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
    GLSubCanvas(Ihandle* _ih) : Control(_ih) {}
    GLSubCanvas(const Element& elem) : Control(elem.GetHandle()) {}
  };
  class GLSeparator : public Control
  {
  public:
    GLSeparator() : Control(IupGLSeparator()) {}
    GLSeparator(Ihandle* _ih) : Control(_ih) {}
    GLSeparator(const Element& elem) : Control(elem.GetHandle()) {}
  };
  class GLProgressBar : public Control
  {
  public:
    GLProgressBar() : Control(IupGLProgressBar()) {}
    GLProgressBar(Ihandle* _ih) : Control(_ih) {}
    GLProgressBar(const Element& elem) : Control(elem.GetHandle()) {}
  };
  class GLVal : public Control
  {
  public:
    GLVal() : Control(IupGLVal()) {}
    GLVal(Ihandle* _ih) : Control(_ih) {}
    GLVal(const Element& elem) : Control(elem.GetHandle()) {}
  };
  class GLLabel : public Control
  {
  public:
    GLLabel(const char* title = 0) : Control(IupGLLabel(title)) {}
    GLLabel(Ihandle* _ih) : Control(_ih) {}
    GLLabel(const Element& elem) : Control(elem.GetHandle()) {}
  };
  class GLButton : public Control
  {
  public:
    GLButton(const char* title = 0) : Control(IupGLButton(title)) {}
    GLButton(Ihandle* _ih) : Control(_ih) {}
    GLButton(const Element& elem) : Control(elem.GetHandle()) {}
  };
  class GLToggle : public Control
  {
  public:
    GLToggle(const char* title = 0) : Control(IupGLToggle(title)) {}
    GLToggle(Ihandle* _ih) : Control(_ih) {}
    GLToggle(const Element& elem) : Control(elem.GetHandle()) {}
  };
  class GLLink : public Control
  {
  public:
    GLLink(const char *url = 0, const char* title = 0) : Control(IupGLLink(url, title)) {}
    GLLink(Ihandle* _ih) : Control(_ih) {}
    GLLink(const Element& elem) : Control(elem.GetHandle()) {}
  };
  class GLFrame : public Container
  {
  public:
    GLFrame(Control child) : Container(IupGLFrame(child.GetHandle())) {}
    GLFrame() : Container(IupGLFrame(0)) {}
    GLFrame(const GLFrame& container) : Container(container.GetHandle()) {}
    GLFrame(Ihandle* _ih) : Container(_ih) {}
  };
  class GLExpander : public Container
  {
  public:
    GLExpander(Control child) : Container(IupGLExpander(child.GetHandle())) {}
    GLExpander() : Container(IupGLExpander(0)) {}
    GLExpander(const GLExpander& container) : Container(container.GetHandle()) {}
    GLExpander(Ihandle* _ih) : Container(_ih) {}
  };
  class GLScrollBox : public Container
  {
  public:
    GLScrollBox(Control child) : Container(IupGLScrollBox(child.GetHandle())) {}
    GLScrollBox() : Container(IupGLScrollBox(0)) {}
    GLScrollBox(const GLScrollBox& container) : Container(container.GetHandle()) {}
    GLScrollBox(Ihandle* _ih) : Container(_ih) {}
  };
  class GLSizeBox : public Container
  {
  public:
    GLSizeBox(Control child) : Container(IupGLSizeBox(child.GetHandle())) {}
    GLSizeBox() : Container(IupGLSizeBox(0)) {}
    GLSizeBox(const GLSizeBox& container) : Container(container.GetHandle()) {}
    GLSizeBox(Ihandle* _ih) : Container(_ih) {}
  };
  class GLCanvasBox : public Container
  {
  public:
    GLCanvasBox() : Container(IupGLCanvasBox(0)) {}
    GLCanvasBox(Control child0, Control child1 = (Ihandle*)0, Control child2 = (Ihandle*)0, Control child3 = (Ihandle*)0, Control child4 = (Ihandle*)0, Control child5 = (Ihandle*)0, Control child6 = (Ihandle*)0, Control child7 = (Ihandle*)0, Control child8 = (Ihandle*)0, Control child9 = (Ihandle*)0) 
      : Container(IupGLCanvasBox(0), child0, child1, child2, child3, child4, child5, child6, child7, child8, child9) {}
    GLCanvasBox(const Control *child_array, int count) : Container(IupGLCanvasBox(0), child_array, count) {}
    GLCanvasBox(const GLCanvasBox& container) : Container(container.GetHandle()) {}
    GLCanvasBox(Ihandle* _ih) : Container(_ih) {}
  };
  class Plot : public Control
  {
  public:
    Plot() : Control(IupPlot()) {}
    Plot(Ihandle* _ih) : Control(_ih) {}
    Plot(const Element& elem) : Control(elem.GetHandle()) {}

    static void Open() { IupPlotOpen(); }

    void Begin(int strXdata) { IupPlotBegin(ih, strXdata); }
    void Add(double x, double y) { IupPlotAdd(ih, x, y); }
    void AddStr(const char* x, double y) { IupPlotAddStr(ih, x, y); }
    void AddSegment(double x, double y) { IupPlotAddSegment(ih, x, y); }
    int End() { return IupPlotEnd(ih); }

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
    double GetSampleExtra(int ds_index, int sample_index) { return IupPlotGetSampleExtra(ih, ds_index, sample_index); }
    void SetSample(int ds_index, int sample_index, double x, double y) { IupPlotSetSample(ih, ds_index, sample_index, x, y); }
    void SetSampleStr(int ds_index, int sample_index, const char* x, double y) { IupPlotSetSampleStr(ih, ds_index, sample_index, x, y); }
    void SetSampleSelection(int ds_index, int sample_index, int selected) { IupPlotSetSampleSelection(ih, ds_index, sample_index, selected); }
    void SetSampleExtra(int ds_index, int sample_index, double extra) { IupPlotSetSampleExtra(ih, ds_index, sample_index, extra); }

    void Transform(double x, double y, double &cnv_x, double &cnv_y) { IupPlotTransform(ih, x, y, &cnv_x, &cnv_y); }
    void TransformTo(double cnv_x, double cnv_y, double &x, double &y) { IupPlotTransformTo(ih, cnv_x, cnv_y, &x, &y); }

    int FindSample(double cnv_x, double cnv_y, int &ds_index, int &sample_index) { return IupPlotFindSample(ih, cnv_x, cnv_y, &ds_index, &sample_index); }

#ifdef __CD_PLUS_H
    void PaintTo(cd::Canvas& cd_canvas) { IupPlotPaintTo(ih, cd_canvas.GetHandle()); }
#endif
  };
  class MglPlot : public Control
  {
  public:
    MglPlot() : Control(IupMglPlot()) {}
    MglPlot(Ihandle* _ih) : Control(_ih) {}
    MglPlot(const Element& elem) : Control(elem.GetHandle()) {}

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
    MglLabel(Ihandle* _ih) : Control(_ih) {}
    MglLabel(const Element& elem) : Control(elem.GetHandle()) {}
  };
  class OleControl : public Control
  {
  public:
    OleControl(const char* progid) : Control(IupOleControl(progid)) {}
    OleControl(Ihandle* _ih) : Control(_ih) {}
    OleControl(const Element& elem) : Control(elem.GetHandle()) {}

    static void Open() { IupOleControlOpen(); }
  };
  class WebBrowser : public Control
  {
  public:
    WebBrowser() : Control(IupWebBrowser()) {}
    WebBrowser(Ihandle* _ih) : Control(_ih) {}
    WebBrowser(const Element& elem) : Control(elem.GetHandle()) {}

    static void Open() { IupWebBrowserOpen(); }
  };
  class Scintilla : public Control
  {
  public:
    Scintilla(): Control(IupScintilla()) {}
    Scintilla(Ihandle* _ih) : Control(_ih) {}
    Scintilla(const Element& elem) : Control(elem.GetHandle()) {}

    static void Open() { IupScintillaOpen(); }
  };
  class TuioClient : public Element
  {
  public:
    TuioClient(int port) : Element(IupTuioClient(port)) {}
    TuioClient(Ihandle* _ih) : Element(_ih) {}
    TuioClient(const Element& elem) : Element(elem.GetHandle()) {}

    static void Open() { IupTuioOpen(); }
  };

  class Config: public Element
  {
  public:
    Config(): Element(IupConfig()) { }
    Config(Ihandle* _ih) : Element(_ih) {}
    Config(const Element& elem) : Element(elem.GetHandle()) {}

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

    void Copy(const Config& config2, const char* exclude_prefix) { IupConfigCopy(ih, config2.GetHandle(), exclude_prefix); }

    void SetListVariable(const char *group, const char* key, const char* value, int add) { IupConfigSetListVariable(ih, group, key, value, add); }

    void RecentInit(Menu menu, Icallback recent_cb, int max_recent) { IupConfigRecentInit(ih, menu.GetHandle(), recent_cb, max_recent); }
    void RecentUpdate(const char* filename) { IupConfigRecentUpdate(ih, filename); }

    void DialogShow(Dialog dialog, const char* name) { IupConfigDialogShow(ih, dialog.GetHandle(), name); }
    void DialogClosed(Dialog dialog, const char* name) { IupConfigDialogClosed(ih, dialog.GetHandle(), name); }
  };
}

#ifdef __CD_PLUS_H
namespace cd
{
  class CanvasIup : public Canvas
  {
  public:
    CanvasIup(Iup::Canvas& iup_canvas)
      : Canvas() { canvas = cdCreateCanvas(CD_IUP, iup_canvas.GetHandle()); }
  };
  class CanvasIupDoubleBuffer : public Canvas
  {
  public:
    CanvasIupDoubleBuffer(Iup::Canvas& iup_canvas)
      : Canvas() { canvas = cdCreateCanvas(CD_IUPDBUFFER, iup_canvas.GetHandle()); }
  };
  class CanvasIupDoubleBufferRGB : public Canvas
  {
  public:
    CanvasIupDoubleBufferRGB(Iup::Canvas& iup_canvas)
      : Canvas() { canvas = cdCreateCanvas(CD_IUPDBUFFERRGB, iup_canvas.GetHandle()); }
  };
}
#endif

#endif
