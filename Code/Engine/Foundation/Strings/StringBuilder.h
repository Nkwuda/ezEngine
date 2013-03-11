#pragma once

#include <utf8/utf8.h>
#include <Foundation/Containers/HybridArray.h>
#include <Foundation/Memory/MemoryUtils.h>
#include <Foundation/Strings/StringUtils.h>
#include <Foundation/Strings/Implementation/StringBase.h>
#include <Foundation/Strings/StringIterator.h>
#include <Foundation/Strings/PathUtils.h>

/// ezStringBuilder is a class that is meant for creating and modifying strings.
/// It is not meant to store strings for a longer duration.
/// Each ezStringBuilder uses an ezHybridArray to allocate a large buffer on the stack, such that string manipulations
/// are possible without memory allocations, unless the string is too large.
/// No sharing of data happens between ezStringBuilder instances, as it is expected that they will be modified anyway.
/// Instead all data is always copied, therefore instances should not be passed by copy.
/// All string data is stored Utf8 encoded, just as all other string classes, too.
/// That makes it difficult to modify individual characters. Instead you should prefer high-level functions
/// such as 'ReplaceSubString'. If individual characters must be modified, it might make more sense to create
/// a second ezStringBuilder, and iterate over the first while rebuilding the desired result in the second.
/// For very convenient string creation, printf functionality is also available via the 'Format', 'AppendFormat' 
/// and 'PrependFormat' functions.
/// Once a string is built and should only be stored for read access, it should be stored in an ezSharedString instance.
class EZ_FOUNDATION_DLL ezStringBuilder : public ezStringBase<ezStringBuilder>
{
public:

  /// Initializes the string to be empty. No data is allocated, but the ezStringBuilder ALWAYS creates an array on the stack.
  ezStringBuilder(ezIAllocator* pAllocator = ezFoundation::GetDefaultAllocator()); // [tested]

  /// Copies the given string into this one.
  ezStringBuilder(const ezStringBuilder& rhs); // [tested]

  /// Copies the given Utf8 string into this one.
  /* implicit */ ezStringBuilder(const char* szUTF8, ezIAllocator* pAllocator = ezFoundation::GetDefaultAllocator()); // [tested]

  /// Copies the given wchar_t string into this one.
  /* implicit */ ezStringBuilder(const wchar_t* szWChar, ezIAllocator* pAllocator = ezFoundation::GetDefaultAllocator()); // [tested]

  /// Copies the given substring into this one. The ezStringIterator might actually be a substring of this very string.
  /* implicit */ ezStringBuilder(const ezStringIterator& rhs, ezIAllocator* pAllocator = ezFoundation::GetDefaultAllocator()); // [tested]

  /// Copies the given string into this one.
  void operator=(const ezStringBuilder& rhs); // [tested]

  /// Copies the given Utf8 string into this one.
  void operator=(const char* szUTF8); // [tested]

  /// Copies the given wchar_t string into this one.
  void operator=(const wchar_t* szWChar); // [tested]

  /// Copies the given substring into this one. The ezStringIterator might actually be a substring of this very string.
  void operator=(const ezStringIterator& rhs);

  /// Returns the allocator that is used by this object.
  ezIAllocator* GetAllocator() const;

  /// Resets this string to be empty. Does not deallocate any previously allocated data, as it might be reused later again.
  void Clear(); // [tested]

  /// Returns a char pointer to the internal Utf8 data.
  const char* GetData() const; // [tested]

  /// Returns the number of bytes that this string takes up.
  ezUInt32 GetElementCount() const; // [tested]

  /// Returns the number of characters of which this string consists. Might be less than GetElementCount, if it contains Utf8 multi-byte characters.
  ezUInt32 GetCharacterCount() const; // [tested]

  /// Returns whether this string only contains ASCII characters, which means that GetElementCount() == GetCharacterCount()
  bool IsPureASCII() const; // [tested]

  /// Returns an iterator to the entire string, which starts at the first character.
  ezStringIterator GetIteratorFront() const; // [tested]

  /// Returns an iterator to the entire string, which starts at the last character.
  ezStringIterator GetIteratorBack() const; // [tested]

  /// Converts all characters to upper case. Might move the string data around, so all iterators to the data will be invalid afterwards.
  void ToUpper(); // [tested]

  /// Converts all characters to lower case. Might move the string data around, so all iterators to the data will be invalid afterwards.
  void ToLower(); // [tested]

  /// Changes the single character in this string, to which the iterator currently points.
  /// The string might need to be moved around, if its encoding size changes, however the given iterator will be adjusted
  /// so that it will always stay valid.
  /// \note
  /// This can be a very costly operation (unless this string is pure ASCII).
  /// It is only provided for the few rare cases where it is more convenient and performance is not of concern.
  /// If possible, do not use this function, at all.
  void ChangeCharacter(ezStringIterator& Pos, ezUInt32 uiCharacter); // [tested]

  /// Appends a single Utf32 character.
  void Append(ezUInt32 uiChar);

  /// Appends all the given strings at the back of this string in one operation.
  void Append(const wchar_t* pData1, const wchar_t* pData2 = NULL, const wchar_t* pData3 = NULL, const wchar_t* pData4 = NULL, const wchar_t* pData5 = NULL, const wchar_t* pData6 = NULL); // [tested]

  /// Appends all the given strings at the back of this string in one operation.
  void Append(const char* pData1, const char* pData2 = NULL, const char* pData3 = NULL, const char* pData4 = NULL, const char* pData5 = NULL, const char* pData6 = NULL); // [tested]

  /// Appends the formatted string.
  void AppendFormat(const char* szUtf8Format, ...); // [tested]

  /// Appends the formatted string.
  void AppendFormat(const char* szUtf8Format, va_list& args); // [tested]

  /// Prepends all the given strings to the front of this string in one operation.
  void Prepend(const wchar_t* pData1, const wchar_t* pData2 = NULL, const wchar_t* pData3 = NULL, const wchar_t* pData4 = NULL, const wchar_t* pData5 = NULL, const wchar_t* pData6 = NULL); // [tested]

  /// Prepends all the given strings to the front of this string in one operation.
  void Prepend(const char* pData1, const char* pData2 = NULL, const char* pData3 = NULL, const char* pData4 = NULL, const char* pData5 = NULL, const char* pData6 = NULL); // [tested]

  /// Prepens the formatted string.
  void PrependFormat(const char* szUtf8Format, ...); // [tested]

  /// Prepens the formatted string.
  void PrependFormat(const char* szUtf8Format, va_list& args); // [tested]

  /// Sets this string to the formatted string.
  void Format(const char* szUtf8Format, ...); // [tested]

  /// Sets this string to the formatted string.
  void Format(const char* szUtf8Format, va_list& args); // [tested]

  /// Removes the first n and last m characters from this string.
  /// This function will never reallocate data.
  /// Removing characters at the back is very cheap.
  /// Removing characters at the front needs to move data around, so can be quite costly.
  void Shrink(ezUInt32 uiShrinkCharsFront, ezUInt32 uiShrinkCharsBack); // [tested]
  

  /// Replaces the string that starts at szStartPos and ands at szEndPos with the string szReplaceWith.
  void ReplaceSubString(const char* szStartPos, const char* szEndPos, const char* szReplaceWith, const char* szReplaceWithEnd = ezMaxStringEnd); // [tested]

  /// A wrapper around ReplaceSubString. Will insert the given string at szInsertAtPos.
  void Insert(const char* szInsertAtPos, const char* szTextToInsert, const char* szTextToInsertEnd = ezMaxStringEnd); // [tested]

  /// A wrapper around ReplaceSubString. Will remove the substring which starts at szRemoveFromPos and ends at szRemoveToPos.
  void Remove(const char* szRemoveFromPos, const char* szRemoveToPos); // [tested]

  /// Replaces the first occurrence of szSearchFor by szReplacement. Optionally starts searching at szStartSearchAt (or the beginning).
  /// Returns the first position where szSearchFor was found, or NULL if nothing was found (and replaced).
  const char* ReplaceFirst(const char* szSearchFor, const char* szReplacement, const char* szStartSearchAt = NULL); // [tested]

  /// Case-insensitive version of ReplaceFirst.
  const char* ReplaceFirst_NoCase(const char* szSearchFor, const char* szReplacement, const char* szStartSearchAt = NULL); // [tested]

  /// Replaces the last occurrence of szSearchFor by szReplacement. Optionally starts searching at szStartSearchAt (or the end).
  /// Returns the last position where szSearchFor was found, or NULL if nothing was found (and replaced).
  const char* ReplaceLast(const char* szSearchFor, const char* szReplacement, const char* szStartSearchAt = NULL); // [tested]

  /// Case-insensitive version of ReplaceLast.
  const char* ReplaceLast_NoCase(const char* szSearchFor, const char* szReplacement, const char* szStartSearchAt = NULL); // [tested]

  /// Replaces all occurrences of szSearchFor by szReplacement. Returns the number of replacements.
  ezUInt32 ReplaceAll(const char* szSearchFor, const char* szReplacement); // [tested]

  /// Case-insensitive version of ReplaceAll.
  ezUInt32 ReplaceAll_NoCase(const char* szSearchFor, const char* szReplacement); // [tested]

  /// Replaces the first occurrence of szSearchFor by szReplaceWith, if szSearchFor was found to be a 'whole word', as indicated by the delimiter function IsDelimiterCB.
  const char* ReplaceWholeWord(const char* szSearchFor, const char* szReplaceWith, ezStringUtils::EZ_IS_WORD_DELIMITER IsDelimiterCB); // [tested]

  /// Case-insensitive version of ReplaceWholeWord.
  const char* ReplaceWholeWord_NoCase(const char* szSearchFor, const char* szReplaceWith, ezStringUtils::EZ_IS_WORD_DELIMITER IsDelimiterCB); // [tested]

  /// Replaces all occurrences of szSearchFor by szReplaceWith, if szSearchFor was found to be a 'whole word', as indicated by the delimiter function IsDelimiterCB.
  ezUInt32 ReplaceWholeWordAll(const char* szSearchFor, const char* szReplaceWith, ezStringUtils::EZ_IS_WORD_DELIMITER IsDelimiterCB); // [tested]

  /// Case-insensitive version of ReplaceWholeWordAll.
  ezUInt32 ReplaceWholeWordAll_NoCase(const char* szSearchFor, const char* szReplaceWith, ezStringUtils::EZ_IS_WORD_DELIMITER IsDelimiterCB); // [tested]

  /// Fills the given container with ezStringIterator's which represent each found substring.
  /// If bReturnEmptyStrings is true, even empty strings between separators are returned.
  /// Output must be a container that stores ezStringIterator's and provides the functions 'Clear' and 'Append'.
  /// szSeparator1 to szSeparator6 are strings which act as separators and indicate where to split the string.
  /// This string itself will not be modified.
  template <typename Container>
  void Split(bool bReturnEmptyStrings, Container& Output, const char* szSeparator1, const char* szSeparator2 = NULL, const char* szSeparator3 = NULL, const char* szSeparator4 = NULL, const char* szSeparator5 = NULL, const char* szSeparator6 = NULL) const; // [tested]



  // ******* Path Functions ********

  /// Checks whether the given path has any file extension
  bool HasAnyExtension() const; // [tested]

  /// Checks whether the given path ends with the given extension. szExtension should start with a '.' for performance reasons, but it will work without a '.' too.
  bool HasExtension(const char* szExtension) const; // [tested]

  /// Returns the file extension of the given path. Will be empty, if the path does not end with a proper extension.
  ezStringIterator GetFileExtension() const; // [tested]

  /// Returns the file name of a path, exluding the path and extension.
  /// If the path already ends with a path separator, the result will be empty.
  ezStringIterator GetFileName() const; // [tested]

  /// Returns the substring that represents the file name including the file extension.
  /// Returns an empty string, if sPath already ends in a path separator, or is empty itself.
  ezStringIterator GetFileNameAndExtension() const; // [tested]

  /// Returns the directory of the given file, which is the substring up to the last path separator.
  /// If the path already ends in a path separator, and thus points to a folder, instead of a file, the unchanged path is returned.
  /// "path/to/file" -> "path/to/"
  /// "path/to/folder/" -> "path/to/folder/"
  /// "filename" -> ""
  /// "/file_at_root_level" -> "/"
  ezStringIterator GetFileDirectory() const; // [tested]

  /// Returns true, if the given path represents an absolute path on the current OS.
  bool IsAbsolutePath() const; // [tested]

  /// Returns true, if the given path represents a relative path on the current OS.
  bool IsRelativePath() const; // [tested]




  /// Removes double path-separators (ie. folder//folder -> folder/folder), removes "../" where possible, replaces all path separators with /
  /// All paths use slashes on all platforms. If you need to convert a path to the OS specific representation, use 'MakePathOsSpecific'
  /// 'MakeCleanPath' will in rare circumstances grow the string by one character. That means it is quite safe to assume that
  /// it will not waste time on memory allocations.
  /// If it is repeatedly called on the same string, it has a minor overhead for computing the same string over and over, 
  /// but no memory allocations will be done (everything is in-place).
  void MakeCleanPath(); // [tested]

  /// Modifies this string to point to the parent directory.
  /// 'uiLevelsUp' can be used to go several folders upwards. It has to be at least one.
  /// If there are no more folders to go up, "../" is appended as much as needed.
  void PathParentDirectory(ezUInt32 uiLevelsUp = 1); // [tested]

  /// Appends several path pieces. Makes sure they are always properly separated by a slash.
  /// Will call 'MakeCleanPath' internally, so the representation of the path might change.
  void AppendPath(const char* szPath1, const char* szPath2 = NULL, const char* szPath3 = NULL, const char* szPath4 = NULL); // [tested]

  /// Changes the file name part of the path, keeps the extension intact (if there is any).
  void ChangeFileName(const char* szNewFileName); // [tested]

  /// Changes the file name and the extension part of the path.
  void ChangeFileNameAndExtension(const char* szNewFileNameWithExtension); // [tested]

  /// Only changes the file extension of the path. If there is no extension yet, one is appended.
  /// szNewExtension must not start with a dot.
  void ChangeFileExtension(const char* szNewExtension); // [tested]

  /// Sets this path to the absolute path by concatenating the absolute base path and the relative path.
  /// The result is already cleaned up by 'MakeCleanPath'
  void MakeAbsolutePath(const char* szAbsoluteBasePath); // [tested]

  /// Converts this path into a relative path to the path with the awesome variable name 'szAbsolutePathToMakeThisRelativeTo'
  void MakeRelativePath(const char* szAbsolutePathToMakeThisRelativeTo); // [tested]

  /// Cleans this path up and replaces all path separators by the OS specific separator.
  /// This can be used, if you want to present paths in the OS specific form to the user in GUI.
  /// In all other cases the internal representation uses slashes, no matter on which operating system.
  void MakePathOsSpecific();

  /// Checks whether this path is a subpath of the given path.
  /// This function will call 'MakeCleanPath' to be able to compare both paths, thus it might modify the data of this instance.
  bool IsPathBelowFolder (const char* szPathToFolder); // [tested]

  /// Will remove all double path separators (slashes and backslashes) in a path, 
  /// except if the path starts with two (back-)slashes, those are kept, as they might indicate a UNC path.
  void RemoveDoubleSlashesInPath();

private:
  void ChangeCharacterNonASCII(ezStringIterator& Pos, ezUInt32 uiCharacter);
  void AppendTerminator();

  ezUInt32 m_uiCharacterCount;
  ezHybridArray<char, 256> m_Data;
};

#include <Foundation/Strings/Implementation/StringBuilder_inl.h>
