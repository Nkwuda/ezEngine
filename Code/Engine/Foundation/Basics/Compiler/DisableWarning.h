
// Does nothing when not on MSVC
#if defined(BUILDSYSTEM_COMPILER_MSVC) && defined(EZ_MSVC_WARNING_NUMBER)
  #include <Foundation/Basics/Compiler/MSVC/DisableWarning_MSVC.h>
#endif