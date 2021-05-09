/** \file
 * \brief Class Callback Utilities.
 */

#ifndef __IUP_CLASS_CBS_HPP
#define __IUP_CLASS_CBS_HPP


#define IUP_CLASS_GET_OBJECT(__ih, __class) dynamic_cast<__class*>((__class*)IupGetAttribute(__ih, #__class "->this"))


#define IUP_CLASS_INITCALLBACK(__ih, __class)  \
  IupSetAttribute(__ih, #__class "->this", (char*)this)

#define IUP_CLASS_SETCALLBACK(__ih, __name, __cb)  \
  IupSetCallback(__ih, __name, (Icallback)CB_##__cb)



#ifdef __IUP_PLUS_H

#define IUP_PLUS_GET_OBJECT(__elem, __class) dynamic_cast<__class*>((__class*)IupGetAttribute(__elem.GetHandle(), #__class "->this"))

#define IUP_PLUS_INITCALLBACK(__elem, __class)  \
  IupSetAttribute(__elem.GetHandle(), #__class "->this", (char*)this)

#define IUP_PLUS_SETCALLBACK(__elem, __name, __cb)  \
  IupSetCallback(__elem.GetHandle(), __name, (Icallback)CB_##__cb)

#endif



#define IUP_CLASS_DECLARECALLBACK_IFn(__class, __cb)  \
       int      __cb(Ihandle* ih);         \
static int CB_##__cb(Ihandle* ih)          \
{                                                                   \
  __class* obj = IUP_CLASS_GET_OBJECT(ih, __class);  \
  return obj->__cb(ih);                                  \
}                                                           

#define IUP_CLASS_DECLARECALLBACK_IFni(__class, __cb)  \
       int      __cb(Ihandle* ih, int i1);         \
static int CB_##__cb(Ihandle* ih, int i1)          \
{                                                                   \
  __class* obj = IUP_CLASS_GET_OBJECT(ih, __class);  \
  return obj->__cb(ih, i1);                                  \
}                                                           

#define IUP_CLASS_DECLARECALLBACK_IFnii(__class, __cb)  \
       int      __cb(Ihandle* ih, int i1, int i2);         \
static int CB_##__cb(Ihandle* ih, int i1, int i2)          \
{                                                                   \
  __class* obj = IUP_CLASS_GET_OBJECT(ih, __class);  \
  return obj->__cb(ih, i1, i2);                                  \
}                                                           

#define IUP_CLASS_DECLARECALLBACK_IFniii(__class, __cb)  \
       int      __cb(Ihandle* ih, int i1, int i2, int i3);         \
static int CB_##__cb(Ihandle* ih, int i1, int i2, int i3)          \
{                                                                   \
  __class* obj = IUP_CLASS_GET_OBJECT(ih, __class);  \
  return obj->__cb(ih, i1, i2, i3);                                  \
}                                                           

#define IUP_CLASS_DECLARECALLBACK_IFniiii(__class, __cb)  \
       int      __cb(Ihandle* ih, int i1, int i2, int i3, int i4);         \
static int CB_##__cb(Ihandle* ih, int i1, int i2, int i3, int i4)          \
{                                                                   \
  __class* obj = IUP_CLASS_GET_OBJECT(ih, __class);  \
  return obj->__cb(ih, i1, i2, i3, i4);                                  \
}                                                           

#define IUP_CLASS_DECLARECALLBACK_IFniiiii(__class, __cb)  \
       int      __cb(Ihandle* ih, int i1, int i2, int i3, int i4, int i5);         \
static int CB_##__cb(Ihandle* ih, int i1, int i2, int i3, int i4, int i5)          \
{                                                                   \
  __class* obj = IUP_CLASS_GET_OBJECT(ih, __class);  \
  return obj->__cb(ih, i1, i2, i3, i4, i5);                                  \
}                                                           

#define IUP_CLASS_DECLARECALLBACK_IFniiiiii(__class, __cb)  \
       int      __cb(Ihandle* ih, int i1, int i2, int i3, int i4, int i5, int i6);         \
static int CB_##__cb(Ihandle* ih, int i1, int i2, int i3, int i4, int i5, int i6)          \
{                                                                   \
  __class* obj = IUP_CLASS_GET_OBJECT(ih, __class);  \
  return obj->__cb(ih, i1, i2, i3, i4, i5, i6);                                  \
}                                                           

#define IUP_CLASS_DECLARECALLBACK_IFniiiiiiC(__class, __cb)  \
       int      __cb(Ihandle* ih, int i1, int i2, int i3, int i4, int i5, int i6, struct _cdCanvas* canvas);         \
static int CB_##__cb(Ihandle* ih, int i1, int i2, int i3, int i4, int i5, int i6, struct _cdCanvas* canvas)          \
{                                                                   \
  __class* obj = IUP_CLASS_GET_OBJECT(ih, __class);  \
  return obj->__cb(ih, i1, i2, i3, i4, i5, i6, canvas);             \
}                                                           

#define IUP_CLASS_DECLARECALLBACK_IFnC(__class, __cb)  \
       int      __cb(Ihandle* ih, struct _cdCanvas* canvas);         \
static int CB_##__cb(Ihandle* ih, struct _cdCanvas* canvas)          \
{                                                                   \
  __class* obj = IUP_CLASS_GET_OBJECT(ih, __class);  \
  return obj->__cb(ih, canvas);             \
}                                                           

#define IUP_CLASS_DECLARECALLBACK_dIFnii(__class, __cb)  \
       double      __cb(Ihandle* ih, int i1, int i2);    \
static double CB_##__cb(Ihandle* ih, int i1, int i2)     \
{                                                                   \
  __class* obj = IUP_CLASS_GET_OBJECT(ih, __class);  \
  return obj->__cb(ih, i1, i2);                                     \
}                                                           

#define IUP_CLASS_DECLARECALLBACK_sIFni(__class, __cb)  \
       char*      __cb(Ihandle* ih, int i1);    \
static char* CB_##__cb(Ihandle* ih, int i1)     \
{                                                                   \
  __class* obj = IUP_CLASS_GET_OBJECT(ih, __class);  \
  return obj->__cb(ih, i1);                                     \
}                                                           

#define IUP_CLASS_DECLARECALLBACK_sIFnii(__class, __cb)  \
       char*      __cb(Ihandle* ih, int i1, int i2);    \
static char* CB_##__cb(Ihandle* ih, int i1, int i2)     \
{                                                                   \
  __class* obj = IUP_CLASS_GET_OBJECT(ih, __class);  \
  return obj->__cb(ih, i1, i2);                                     \
}                                                           

#define IUP_CLASS_DECLARECALLBACK_sIFniis(__class, __cb)  \
       char*      __cb(Ihandle* ih, int i1, int i2, char* s);         \
static char* CB_##__cb(Ihandle* ih, int i1, int i2, char* s)          \
{                                                                   \
  __class* obj = IUP_CLASS_GET_OBJECT(ih, __class);  \
  return obj->__cb(ih, i1, i2, s);                                  \
}                                                           

#define IUP_CLASS_DECLARECALLBACK_IFnff(__class, __cb)  \
       int      __cb(Ihandle* ih, float f1, float f2);         \
static int CB_##__cb(Ihandle* ih, float f1, float f2)          \
{                                                                   \
  __class* obj = IUP_CLASS_GET_OBJECT(ih, __class);  \
  return obj->__cb(ih, f1, f2);                                  \
}                                                           

#define IUP_CLASS_DECLARECALLBACK_IFniff(__class, __cb)  \
       int      __cb(Ihandle* ih, int i1, float f1, float f2);         \
static int CB_##__cb(Ihandle* ih, int i1, float f1, float f2)          \
{                                                                   \
  __class* obj = IUP_CLASS_GET_OBJECT(ih, __class);  \
  return obj->__cb(ih, i1, f1, f2);                                  \
}                                                           

#define IUP_CLASS_DECLARECALLBACK_IFnfiis(__class, __cb)  \
       int      __cb(Ihandle* ih, float f1, int i1, int i2, char* s);         \
static int CB_##__cb(Ihandle* ih, float f1, int i1, int i2, char* s)          \
{                                                                   \
  __class* obj = IUP_CLASS_GET_OBJECT(ih, __class);  \
  return obj->__cb(ih, f1, i1, i2, s);                                  \
}                                                           

#define IUP_CLASS_DECLARECALLBACK_IFnd(__class, __cb)  \
       int      __cb(Ihandle* ih, double d1);         \
static int CB_##__cb(Ihandle* ih, double d1)          \
{                                                                   \
  __class* obj = IUP_CLASS_GET_OBJECT(ih, __class);  \
  return obj->__cb(ih, d1);                                  \
}                                                           

#define IUP_CLASS_DECLARECALLBACK_IFndds(__class, __cb)  \
       int      __cb(Ihandle* ih, double d1, double d2, char* s);         \
static int CB_##__cb(Ihandle* ih, double d1, double d2, char* s)          \
{                                                                   \
  __class* obj = IUP_CLASS_GET_OBJECT(ih, __class);  \
  return obj->__cb(ih, d1, d2, s);                                  \
}                                                           

#define IUP_CLASS_DECLARECALLBACK_IFniid(__class, __cb)  \
       int      __cb(Ihandle* ih, int i1, int i2, double d1);         \
static int CB_##__cb(Ihandle* ih, int i1, int i2, double d1)          \
{                                                                   \
  __class* obj = IUP_CLASS_GET_OBJECT(ih, __class);  \
  return obj->__cb(ih, i1, i2, d1);                                  \
}                                                           

#define IUP_CLASS_DECLARECALLBACK_IFniidd(__class, __cb)  \
       int      __cb(Ihandle* ih, int i1, int i2, double d1, double d2);         \
static int CB_##__cb(Ihandle* ih, int i1, int i2, double d1, double d2)          \
{                                                                   \
  __class* obj = IUP_CLASS_GET_OBJECT(ih, __class);  \
  return obj->__cb(ih, i1, i2, d1, d2);                                  \
}                                                           

#define IUP_CLASS_DECLARECALLBACK_IFniiddi(__class, __cb)  \
       int      __cb(Ihandle* ih, int i1, int i2, double d1, double d2, int i3);         \
static int CB_##__cb(Ihandle* ih, int i1, int i2, double d1, double d2, int i3)          \
{                                                                   \
  __class* obj = IUP_CLASS_GET_OBJECT(ih, __class);  \
  return obj->__cb(ih, i1, i2, d1, d2, i3);                                  \
}                                                           

#define IUP_CLASS_DECLARECALLBACK_IFniidds(__class, __cb)  \
       int      __cb(Ihandle* ih, int i1, int i2, double d1, double d2, char* s);         \
static int CB_##__cb(Ihandle* ih, int i1, int i2, double d1, double d2, char* s)          \
{                                                                   \
  __class* obj = IUP_CLASS_GET_OBJECT(ih, __class);  \
  return obj->__cb(ih, i1, i2, d1, d2, s);                                  \
}                                                           

#define IUP_CLASS_DECLARECALLBACK_IFniiIII(__class, __cb)  \
       int      __cb(Ihandle* ih, int i1, int i2, int *I1, int *I2, int *I3);  \
static int CB_##__cb(Ihandle* ih, int i1, int i2, int *I1, int *I2, int *I3)   \
{                                                                   \
  __class* obj = IUP_CLASS_GET_OBJECT(ih, __class);  \
  return obj->__cb(ih, i1, i2, I1, I2, I3);                         \
}                                                           

#define IUP_CLASS_DECLARECALLBACK_IFniIIII(__class, __cb)  \
       int      __cb(Ihandle* ih, int i1, int *I1, int *I2, int *I3, int *I4);  \
static int CB_##__cb(Ihandle* ih, int i1, int *I1, int *I2, int *I3, int *I4)   \
{                                                                   \
  __class* obj = IUP_CLASS_GET_OBJECT(ih, __class);  \
  return obj->__cb(ih, i1, I1, I2, I3, I4);                         \
}                                                           

#define IUP_CLASS_DECLARECALLBACK_IFnIi(__class, __cb)  \
       int      __cb(Ihandle* ih, int *I1, int i1);  \
static int CB_##__cb(Ihandle* ih, int *I1, int i1)   \
{                                                                   \
  __class* obj = IUP_CLASS_GET_OBJECT(ih, __class);  \
  return obj->__cb(ih, I1, i1);                         \
}                                                           

#define IUP_CLASS_DECLARECALLBACK_IFnccc(__class, __cb)  \
       int      __cb(Ihandle* ih, char c1, char c2, char c3);         \
static int CB_##__cb(Ihandle* ih, char c1, char c2, char c3)          \
{                                                                   \
  __class* obj = IUP_CLASS_GET_OBJECT(ih, __class);  \
  return obj->__cb(ih, c1, c2, c3);                                  \
}                                                           

#define IUP_CLASS_DECLARECALLBACK_IFnis(__class, __cb)  \
       int      __cb(Ihandle* ih, int i1, char* s);         \
static int CB_##__cb(Ihandle* ih, int i1, char* s)          \
{                                                                   \
  __class* obj = IUP_CLASS_GET_OBJECT(ih, __class);  \
  return obj->__cb(ih, i1, s);                                  \
}                                                           

#define IUP_CLASS_DECLARECALLBACK_IFniis(__class, __cb)  \
       int      __cb(Ihandle* ih, int i1, int i2, char* s);         \
static int CB_##__cb(Ihandle* ih, int i1, int i2, char* s)          \
{                                                                   \
  __class* obj = IUP_CLASS_GET_OBJECT(ih, __class);  \
  return obj->__cb(ih, i1, i2, s);                                  \
}                                                           

#define IUP_CLASS_DECLARECALLBACK_IFniiis(__class, __cb)  \
       int      __cb(Ihandle* ih, int i1, int i2, int i3, char* s);         \
static int CB_##__cb(Ihandle* ih, int i1, int i2, int i3, char* s)          \
{                                                                   \
  __class* obj = IUP_CLASS_GET_OBJECT(ih, __class);  \
  return obj->__cb(ih, i1, i2, i3, s);                                  \
}                                                           

#define IUP_CLASS_DECLARECALLBACK_IFniiiis(__class, __cb)  \
       int      __cb(Ihandle* ih, int i1, int i2, int i3, int i4, char* s);         \
static int CB_##__cb(Ihandle* ih, int i1, int i2, int i3, int i4, char* s)          \
{                                                                   \
  __class* obj = IUP_CLASS_GET_OBJECT(ih, __class);  \
  return obj->__cb(ih, i1, i2, i3, i4, s);                                  \
}                                                           

#define IUP_CLASS_DECLARECALLBACK_IFniiiiis(__class, __cb)  \
       int      __cb(Ihandle* ih, int i1, int i2, int i3, int i4, int i5, char* s);         \
static int CB_##__cb(Ihandle* ih, int i1, int i2, int i3, int i4, int i5, char* s)          \
{                                                                   \
  __class* obj = IUP_CLASS_GET_OBJECT(ih, __class);  \
  return obj->__cb(ih, i1, i2, i3, i4, i5, s);                                  \
}                                                           

#define IUP_CLASS_DECLARECALLBACK_IFniiiiiis(__class, __cb)  \
       int      __cb(Ihandle* ih, int i1, int i2, int i3, int i4, int i5, int i6, char* s);         \
static int CB_##__cb(Ihandle* ih, int i1, int i2, int i3, int i4, int i5, int i6, char* s)          \
{                                                                   \
  __class* obj = IUP_CLASS_GET_OBJECT(ih, __class);  \
  return obj->__cb(ih, i1, i2, i3, i4, i5, i6, s);                                  \
}                                                           

#define IUP_CLASS_DECLARECALLBACK_IFnss(__class, __cb)  \
       int      __cb(Ihandle* ih, char* s1, char* s2);         \
static int CB_##__cb(Ihandle* ih, char* s1, char* s2)          \
{                                                                   \
  __class* obj = IUP_CLASS_GET_OBJECT(ih, __class);  \
  return obj->__cb(ih, s1, s2);                                  \
}                                                           

#define IUP_CLASS_DECLARECALLBACK_IFns(__class, __cb)  \
       int      __cb(Ihandle* ih, char* s1);         \
static int CB_##__cb(Ihandle* ih, char* s1)          \
{                                                                   \
  __class* obj = IUP_CLASS_GET_OBJECT(ih, __class);  \
  return obj->__cb(ih, s1);                                  \
}                                                           

#define IUP_CLASS_DECLARECALLBACK_IFnsi(__class, __cb)  \
       int      __cb(Ihandle* ih, char* s1, int i1);         \
static int CB_##__cb(Ihandle* ih, char* s1, int i1)          \
{                                                                   \
  __class* obj = IUP_CLASS_GET_OBJECT(ih, __class);  \
  return obj->__cb(ih, s1, i1);                                  \
}                                                           

#define IUP_CLASS_DECLARECALLBACK_IFnsii(__class, __cb)  \
       int      __cb(Ihandle* ih, char* s1, int i1, int i2);         \
static int CB_##__cb(Ihandle* ih, char* s1, int i1, int i2)          \
{                                                                   \
  __class* obj = IUP_CLASS_GET_OBJECT(ih, __class);  \
  return obj->__cb(ih, s1, i1, i2);                                  \
}                                                           

#define IUP_CLASS_DECLARECALLBACK_IFnsiii(__class, __cb)  \
       int      __cb(Ihandle* ih, char* s1, int i1, int i2, int i3);         \
static int CB_##__cb(Ihandle* ih, char* s1, int i1, int i2, int i3)          \
{                                                                   \
  __class* obj = IUP_CLASS_GET_OBJECT(ih, __class);  \
  return obj->__cb(ih, s1, i1, i2, i3);                                  \
}                                                           

#define IUP_CLASS_DECLARECALLBACK_IFnnii(__class, __cb)  \
       int      __cb(Ihandle* ih, Ihandle* ih1, int i1, int i2);         \
static int CB_##__cb(Ihandle* ih, Ihandle* ih1, int i1, int i2)          \
{                                                                   \
  __class* obj = IUP_CLASS_GET_OBJECT(ih, __class);  \
  return obj->__cb(ih, ih1, i1, i2);                                  \
}                                                           

#define IUP_CLASS_DECLARECALLBACK_IFnnn(__class, __cb)  \
       int      __cb(Ihandle* ih, Ihandle* ih1, Ihandle *ih2);         \
static int CB_##__cb(Ihandle* ih, Ihandle* ih1, Ihandle *ih2)          \
{                                                                   \
  __class* obj = IUP_CLASS_GET_OBJECT(ih, __class);  \
  return obj->__cb(ih, ih1, ih2);                                  \
}                                                           

#define IUP_CLASS_DECLARECALLBACK_IFniinsii(__class, __cb)  \
       int      __cb(Ihandle* ih, int i1, int i2, Ihandle* ih1, char* s, int i3, int i4);         \
static int CB_##__cb(Ihandle* ih, int i1, int i2, Ihandle* ih1, char* s, int i3, int i4)          \
{                                                                   \
  __class* obj = IUP_CLASS_GET_OBJECT(ih, __class);  \
  return obj->__cb(ih, i1, i2, ih1, s, i3, i4);                                  \
}                                                           

#define IUP_CLASS_DECLARECALLBACK_IFnsVi(__class, __cb)  \
       int      __cb(Ihandle* ih, char* s1, void* V1, int i1);         \
static int CB_##__cb(Ihandle* ih, char* s1, void* V1, int i1)          \
{                                                                   \
  __class* obj = IUP_CLASS_GET_OBJECT(ih, __class);  \
  return obj->__cb(ih, s1, V1, i1);                                  \
}                                                           

#define IUP_CLASS_DECLARECALLBACK_IFnsViii(__class, __cb)  \
       int      __cb(Ihandle* ih, char* s1, void* V1, int i1, int i2, int i3);         \
static int CB_##__cb(Ihandle* ih, char* s1, void* V1, int i1, int i2, int i3)          \
{                                                                   \
  __class* obj = IUP_CLASS_GET_OBJECT(ih, __class);  \
  return obj->__cb(ih, s1, V1, i1, i2, i3);                                  \
}                                                           



/* #define IUP_CLASS_DEBUG */
#ifdef IUP_CLASS_DEBUG
class IUP_CLASS_DUMMY
{
  // Used to check for errors in the definitions
  IUP_CLASS_DECLARECALLBACK_IFn(IUP_CLASS_DUMMY, IFn);
  IUP_CLASS_DECLARECALLBACK_IFni(IUP_CLASS_DUMMY, IFni);
  IUP_CLASS_DECLARECALLBACK_IFnii(IUP_CLASS_DUMMY, IFnii);
  IUP_CLASS_DECLARECALLBACK_IFniii(IUP_CLASS_DUMMY, IFniii);
  IUP_CLASS_DECLARECALLBACK_IFniiii(IUP_CLASS_DUMMY, IFniiii);
  IUP_CLASS_DECLARECALLBACK_IFniiiii(IUP_CLASS_DUMMY, IFniiiii);
  IUP_CLASS_DECLARECALLBACK_IFniiiiii(IUP_CLASS_DUMMY, IFniiiiii);
  IUP_CLASS_DECLARECALLBACK_IFniiiiiiC(IUP_CLASS_DUMMY, IFniiiiiiC);
  IUP_CLASS_DECLARECALLBACK_IFnC(IUP_CLASS_DUMMY, IFnC);
  IUP_CLASS_DECLARECALLBACK_dIFnii(IUP_CLASS_DUMMY, dIFnii);
  IUP_CLASS_DECLARECALLBACK_sIFni(IUP_CLASS_DUMMY, sIFni);
  IUP_CLASS_DECLARECALLBACK_sIFnii(IUP_CLASS_DUMMY, sIFnii);
  IUP_CLASS_DECLARECALLBACK_sIFniis(IUP_CLASS_DUMMY, sIFniis);
  IUP_CLASS_DECLARECALLBACK_IFnff(IUP_CLASS_DUMMY, IFnff);
  IUP_CLASS_DECLARECALLBACK_IFniff(IUP_CLASS_DUMMY, IFniff);
  IUP_CLASS_DECLARECALLBACK_IFnfiis(IUP_CLASS_DUMMY, IFnfiis);
  IUP_CLASS_DECLARECALLBACK_IFnd(IUP_CLASS_DUMMY, IFnd);
  IUP_CLASS_DECLARECALLBACK_IFndds(IUP_CLASS_DUMMY, IFndds);
  IUP_CLASS_DECLARECALLBACK_IFniid(IUP_CLASS_DUMMY, IFniid);
  IUP_CLASS_DECLARECALLBACK_IFniidd(IUP_CLASS_DUMMY, IFniidd);
  IUP_CLASS_DECLARECALLBACK_IFniiddi(IUP_CLASS_DUMMY, IFniiddi);
  IUP_CLASS_DECLARECALLBACK_IFniidds(IUP_CLASS_DUMMY, IFniidds);
  IUP_CLASS_DECLARECALLBACK_IFniiIII(IUP_CLASS_DUMMY, IFniiIII);
  IUP_CLASS_DECLARECALLBACK_IFniIIII(IUP_CLASS_DUMMY, IFniIIII);
  IUP_CLASS_DECLARECALLBACK_IFnIi(IUP_CLASS_DUMMY, IFnIi);
  IUP_CLASS_DECLARECALLBACK_IFnccc(IUP_CLASS_DUMMY, IFnccc);
  IUP_CLASS_DECLARECALLBACK_IFnis(IUP_CLASS_DUMMY, IFnis);
  IUP_CLASS_DECLARECALLBACK_IFniis(IUP_CLASS_DUMMY, IFniis);
  IUP_CLASS_DECLARECALLBACK_IFniiis(IUP_CLASS_DUMMY, IFniiis);
  IUP_CLASS_DECLARECALLBACK_IFniiiis(IUP_CLASS_DUMMY, IFniiiis);
  IUP_CLASS_DECLARECALLBACK_IFniiiiis(IUP_CLASS_DUMMY, IFniiiiis);
  IUP_CLASS_DECLARECALLBACK_IFniiiiiis(IUP_CLASS_DUMMY, IFniiiiiis);
  IUP_CLASS_DECLARECALLBACK_IFnss(IUP_CLASS_DUMMY, IFnss);
  IUP_CLASS_DECLARECALLBACK_IFns(IUP_CLASS_DUMMY, IFns);
  IUP_CLASS_DECLARECALLBACK_IFnsi(IUP_CLASS_DUMMY, IFnsi);
  IUP_CLASS_DECLARECALLBACK_IFnsii(IUP_CLASS_DUMMY, IFnsii);
  IUP_CLASS_DECLARECALLBACK_IFnsiii(IUP_CLASS_DUMMY, IFnsiii);
  IUP_CLASS_DECLARECALLBACK_IFnnii(IUP_CLASS_DUMMY, IFnnii);
  IUP_CLASS_DECLARECALLBACK_IFnnn(IUP_CLASS_DUMMY, IFnnn);
  IUP_CLASS_DECLARECALLBACK_IFniinsii(IUP_CLASS_DUMMY, IFniinsii);
  IUP_CLASS_DECLARECALLBACK_IFnsVi(IUP_CLASS_DUMMY, IFnsVi);
  IUP_CLASS_DECLARECALLBACK_IFnsViii(IUP_CLASS_DUMMY, IFnsViii);
};

class SampleClass
{
  int sample_count;
  
public:
  SampleClass()
  {
    sample_count = 0;
    
    Ihandle* button1 = IupButton("Inc", NULL);
    Ihandle* button2 = IupButton("Dec", NULL);
    Ihandle* dialog = IupDialog(IupHbox(button1, button2, NULL));

    // 1) Register "this" object as a callback receiver (need only once)
    IUP_CLASS_INITCALLBACK(dialog, SampleClass);

    // 2) Associate the callback with the button
    IUP_CLASS_SETCALLBACK(button1, "ACTION", ButtonAction1);
    IUP_CLASS_SETCALLBACK(button2, "ACTION", ButtonAction2);

    IupShow(dialog);
  };

protected:
  // 3) Declare the callback as a member function
  IUP_CLASS_DECLARECALLBACK_IFn(SampleClass, ButtonAction1);
  IUP_CLASS_DECLARECALLBACK_IFn(SampleClass, ButtonAction2);
};

// 4) Define the callback as a member function
int SampleClass::ButtonAction1(Ihandle*)
{
  sample_count++;
  return IUP_DEFAULT;
}
int SampleClass::ButtonAction2(Ihandle*)
{
  sample_count--;
  return IUP_DEFAULT;
}

#endif // IUP_CLASS_DEBUG

#endif
