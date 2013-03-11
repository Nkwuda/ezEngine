#pragma once

#ifndef NULL
  #error "NULL is not defined."
#endif

#ifndef EZ_FORCE_INLINE
  #error "EZ_FORCE_INLINE is not defined."
#endif

#ifndef EZ_RESTRICT
  #error "EZ_RESTRICT is not defined."
#endif

#ifndef EZ_ANALYSIS_ASSUME
  #error "EZ_ANALYSIS_ASSUME is not defined."
#endif

#ifndef EZ_ANALYSIS_IGNORE_WARNING_ONCE
  #error "EZ_ANALYSIS_IGNORE_WARNING_ONCE is not defined."
#endif

#ifndef EZ_ANALYSIS_IGNORE_WARNING_START
  #error "EZ_ANALYSIS_IGNORE_WARNING_START is not defined."
#endif

#ifndef EZ_ANALYSIS_IGNORE_WARNING_END
  #error "EZ_ANALYSIS_IGNORE_WARNING_END is not defined."
#endif

#ifndef EZ_ANALYSIS_IGNORE_ALL_START
  #error "EZ_ANALYSIS_IGNORE_ALL_START is not defined."
#endif

#ifndef EZ_ANALYSIS_IGNORE_ALL_END
  #error "EZ_ANALYSIS_IGNORE_ALL_END is not defined."
#endif

#ifndef EZ_OVERRIDE
  #error "EZ_OVERRIDE is not defined."
#endif

#ifndef EZ_ALIGN
  #error "EZ_ALIGN is not defined."
#endif

#ifndef EZ_ALIGNMENT_OF
  #error "EZ_ALIGNMENT_OF is not defined."
#endif

#if !defined (EZ_PLATFORM_32BIT) && !defined (EZ_PLATFORM_64BIT)
  #error "Platform is not defined as 32 Bit or 64 Bit"
#endif

#ifndef EZ_DEBUG_BREAK
  #error "EZ_DEBUG_BREAK is not defined."
#endif

#ifndef EZ_SOURCE_FUNCTION
  #error "EZ_SOURCE_FUNCTION is not defined."
#endif

#ifndef EZ_SOURCE_FILE
  #error "EZ_SOURCE_FILE is not defined."
#endif

#ifndef EZ_SOURCE_LINE
  #error "EZ_SOURCE_LINE is not defined."
#endif

#if !defined(EZ_LITTLE_ENDIAN) && !defined(EZ_BIG_ENDIAN)
  #error "No Endianess is defined!"
#endif