#pragma once

#include <CoreUtils/Image/Formats/ImageFileFormat.h>

class EZ_COREUTILS_DLL ezDdsFileFormat : public ezImageFileFormatBase
{
public:
  virtual ezResult ReadImage(ezStreamReaderBase& stream, ezImage& image, ezLogInterface* pLog) const EZ_OVERRIDE;
  virtual ezResult WriteImage(ezStreamWriterBase& stream, const ezImage& image, ezLogInterface* pLog) const EZ_OVERRIDE;

  virtual bool CanReadFileType (const char* szExtension) const EZ_OVERRIDE;
  virtual bool CanWriteFileType(const char* szExtension) const EZ_OVERRIDE;

};
