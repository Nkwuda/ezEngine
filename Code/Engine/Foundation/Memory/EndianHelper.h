#pragma once

#include <Foundation/Basics.h>

/// Collection of helper methods when working with endianess "problems"
struct EZ_FOUNDATION_DLL ezEndianHelper
{

  /// Returns true if called on a big endian system, false otherwise
  /// Note that usually the compile time decisions with the defines EZ_LITTLE_ENDIAN, EZ_BIG_ENDIAN is preferred
  static inline bool IsBigEndian()
  {
    const int i = 1;
    return (*(char*)&i) == 0;
  }

  /// Returns true if called on a little endian endian system, false otherwise
  /// Note that usually the compile time decisions with the defines EZ_LITTLE_ENDIAN, EZ_BIG_ENDIAN is preferred
  static inline bool IsLittleEndian()
  {
    return !IsBigEndian();
  }

  /// Switches endianess of the given array of words (16 bit values)
  static inline void SwitchWords(ezUInt16* pWords, ezUInt32 uiCount)
  {
    for(ezUInt32 i = 0; i < uiCount; i++)
      pWords[i] = Switch(pWords[i]);
  }

  /// Switches endianess of the given array of double words (32 bit values)
  static inline void SwitchDWords(ezUInt32* pDWords, ezUInt32 uiCount)
  {
    for(ezUInt32 i = 0; i < uiCount; i++)
      pDWords[i] = Switch(pDWords[i]);
  }

  /// Switches endianess of the given array of quad words (64 bit values)
  static inline void SwitchQWords(ezUInt64* pQWords, ezUInt32 uiCount)
  {
    for(ezUInt32 i = 0; i < uiCount; i++)
      pQWords[i] = Switch(pQWords[i]);
  }

  /// Returns a single switched word (16 bit value)
  static EZ_FORCE_INLINE ezUInt16 Switch(ezUInt16 uiWord)
  {
    return (((uiWord & 0xFF) << 8) | ((uiWord >> 8) & 0xFF));
  }

  /// Returns a single switched double word (32 bit value)
  static EZ_FORCE_INLINE ezUInt32 Switch(ezUInt32 uiDWord)
  {
    return (((uiDWord & 0xFF) << 24) | (((uiDWord >> 8) & 0xFF) << 16) | (((uiDWord >> 16) & 0xFF) << 8) | (uiDWord >> 24) & 0xFF);
  }

  /// Returns a single switched quad word (64 bit value)
  static EZ_FORCE_INLINE ezUInt64 Switch(ezUInt64 uiQWord)
  {
    return (((uiQWord & 0xFF) << 56) | ((uiQWord & 0xFF00) << 40) | ((uiQWord & 0xFF0000) << 24) | ((uiQWord & 0xFF000000) << 8) | ((uiQWord & 0xFF00000000) >> 8) | ((uiQWord & 0xFF0000000000) >> 24) | ((uiQWord & 0xFF000000000000) >> 40) | ((uiQWord & 0xFF00000000000000) >> 56));
  }

  /*
  template <typename T> static void SwitchInPlace(T* pValue)
  {
    EZ_CHECK_AT_COMPILETIME_MSG((sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8), "Switch in place only works for type equivalents of ezUInt16, ezUInt32, ezUInt64!");

    if(sizeof(T) == 2)
    {
      struct TAnd16BitUnion
      {
        union { ezUInt16 BitValue; T TValue; };
      };

      TAnd16BitUnion Temp;
      Temp.TValue = *pValue;
      Temp.BitValue = Switch(Temp.BitValue);

      *pValue = Temp.TValue;
    }
    else if(sizeof(T) == 4)
    {
      struct TAnd32BitUnion
      {
        union { ezUInt32 BitValue; T TValue; };
      };

      TAnd32BitUnion Temp;
      Temp.TValue = *pValue;
      Temp.BitValue = Switch(Temp.BitValue);

      *pValue = Temp.TValue;
    }
    else if(sizeof(T) == 8)
    {
      struct TAnd64BitUnion
      {
        union { ezUInt64 BitValue; T TValue; };
      };

      TAnd64BitUnion Temp;
      Temp.TValue = *pValue;
      Temp.BitValue = Switch(Temp.BitValue);

      *pValue = Temp.TValue;
    }
  }*/

  #if defined(EZ_LITTLE_ENDIAN)

  static EZ_FORCE_INLINE void LittleEndianToNative(ezUInt16* pWords, ezUInt32 uiCount)
  {
  }

  static EZ_FORCE_INLINE void NativeToLittleEndian(ezUInt16* pWords, ezUInt32 uiCount)
  {
  }

  static EZ_FORCE_INLINE void LittleEndianToNative(ezUInt32* pDWords, ezUInt32 uiCount)
  {
  }

  static EZ_FORCE_INLINE void NativeToLittleEndian(ezUInt32* pDWords, ezUInt32 uiCount)
  {
  }

  static EZ_FORCE_INLINE void LittleEndianToNative(ezUInt64* pQWords, ezUInt32 uiCount)
  {
  }

  static EZ_FORCE_INLINE void NativeToLittleEndian(ezUInt64* pQWords, ezUInt32 uiCount)
  {
  }

  static EZ_FORCE_INLINE void BigEndianToNative(ezUInt16* pWords, ezUInt32 uiCount)
  {
    SwitchWords(pWords, uiCount);
  }

  static EZ_FORCE_INLINE void NativeToBigEndian(ezUInt16* pWords, ezUInt32 uiCount)
  {
    SwitchWords(pWords, uiCount);
  }

  static EZ_FORCE_INLINE void BigEndianToNative(ezUInt32* pDWords, ezUInt32 uiCount)
  {
    SwitchDWords(pDWords, uiCount);
  }

  static EZ_FORCE_INLINE void NativeToBigEndian(ezUInt32* pDWords, ezUInt32 uiCount)
  {
    SwitchDWords(pDWords, uiCount);
  }

  static EZ_FORCE_INLINE void BigEndianToNative(ezUInt64* pQWords, ezUInt32 uiCount)
  {
    SwitchQWords(pQWords, uiCount);
  }

  static EZ_FORCE_INLINE void NativeToBigEndian(ezUInt64* pQWords, ezUInt32 uiCount)
  {
    SwitchQWords(pQWords, uiCount);
  }

  #elif defined(EZ_BIG_ENDIAN)

  static EZ_FORCE_INLINE void LittleEndianToNative(ezUInt16* pWords, ezUInt32 uiCount)
  {
    SwitchWords(pWords, uiCount);
  }

  static EZ_FORCE_INLINE void NativeToLittleEndian(ezUInt16* pWords, ezUInt32 uiCount)
  {
    SwitchWords(pWords, uiCount);
  }

  static EZ_FORCE_INLINE void LittleEndianToNative(ezUInt32* pDWords, ezUInt32 uiCount)
  {
    SwitchDWords(pDWords, uiCount);
  }

  static EZ_FORCE_INLINE void NativeToLittleEndian(ezUInt32* pDWords, ezUInt32 uiCount)
  {
    SwitchDWords(pDWords, uiCount);
  }

  static EZ_FORCE_INLINE void LittleEndianToNative(ezUInt64* pQWords, ezUInt32 uiCount)
  {
    SwitchQWords(pQWords, uiCount);
  }

  static EZ_FORCE_INLINE void NativeToLittleEndian(ezUInt64* pQWords, ezUInt32 uiCount)
  {
    SwitchQWords(pQWords, uiCount);
  }

  static EZ_FORCE_INLINE void BigEndianToNative(ezUInt16* pWords, ezUInt32 uiCount)
  {
  }

  static EZ_FORCE_INLINE void NativeToBigEndian(ezUInt16* pWords, ezUInt32 uiCount)
  {
  }

  static EZ_FORCE_INLINE void BigEndianToNative(ezUInt32* pDWords, ezUInt32 uiCount)
  {
  }

  static EZ_FORCE_INLINE void NativeToBigEndian(ezUInt32* pDWords, ezUInt32 uiCount)
  {
  }

  static EZ_FORCE_INLINE void BigEndianToNative(ezUInt64* pQWords, ezUInt32 uiCount)
  {
  }

  static EZ_FORCE_INLINE void NativeToBigEndian(ezUInt64* pQWords, ezUInt32 uiCount)
  {
  }

  #endif


  /// Switches a given struct according to the layout described in the szFormat parameter
  /// The format string may contain the characters:
  ///  - c, b for a member of 1 byte
  ///  - w, s for a member of 2 bytes (word, ezUInt16)
  ///  - d for a member of 4 bytes (dword, ezUInt32)
  ///  - q for a member of 8 bytes (qword, ezUInt64)
  static void SwitchStruct(void* pDataPointer, const char* szFormat);

  /// Templated helper method for SwitchStruct
  template <typename T> static void SwitchStruct(T* pDataPointer, const char* szFormat)
  {
    SwitchStruct(static_cast<void*>(pDataPointer), szFormat);
  }

  /// Switches a given set of struct according to the layout described in the szFormat parameter
  /// The format string may contain the characters:
  ///  - c, b for a member of 1 byte
  ///  - w, s for a member of 2 bytes (word, ezUInt16)
  ///  - d for a member of 4 bytes (dword, ezUInt32)
  ///  - q for a member of 8 bytes (qword, ezUInt64)
  static void SwitchStructs(void* pDataPointer, const char* szFormat, ezUInt32 uiStride, ezUInt32 uiCount);

  /// Templated helper method for SwitchStructs
  template <typename T> static void SwitchStructs(T* pDataPointer, const char* szFormat, ezUInt32 uiCount)
  {
    SwitchStructs(static_cast<void*>(pDataPointer), szFormat, sizeof(T), uiCount);
  }


};
