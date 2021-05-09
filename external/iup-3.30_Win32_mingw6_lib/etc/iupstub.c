/** 
 * When using IUP as a DLL you must link with the "iupstub.lib" static library.
 * But if you are using a compiler that is different 
 * from the one that built the "iupstub.lib" static library 
 * it can occour linking problems, usually argc and argv not being found.
 * If that happen do not use the "iupstub.lib" static library 
 * and use this file instead.
 *
 * See Copyright Notice in "iup.h"
 */

#include <windows.h>

#include <stdlib.h> /* declaration of __argc and __argv */

extern int main(int, char **);

/* WinMain is NOT called for Console applications */
int WINAPI WinMain (HINSTANCE hinst, HINSTANCE hprev, LPSTR cmdline, int ncmdshow)
{
  (void)hinst;  /* NOT used */
  (void)hprev;
  (void)cmdline;
  (void)ncmdshow;
  
  /* this seems to work for all the compilers we tested, except Watcom compilers */
  /* These are declared in <stdlib.h>, except for Cygwin. */
#ifdef __GNUC__
  extern int __argc;
  extern char** __argv;
#endif

  return main(__argc, __argv);
}
