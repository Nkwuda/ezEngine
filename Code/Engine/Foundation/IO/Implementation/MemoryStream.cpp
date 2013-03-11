
#include <Foundation/PCH.h>
#include <Foundation/Math/Math.h>
#include <Foundation/IO/MemoryStream.h>


ezMemoryStreamStorage::ezMemoryStreamStorage(ezUInt32 uiInitialCapacity /*= 0*/, ezIAllocator* pAllocator /*= ezFoundation::GetDefaultAllocator()*/)
  : m_Storage(pAllocator)
{
  m_Storage.Reserve(uiInitialCapacity);
}

ezMemoryStreamStorage::~ezMemoryStreamStorage()
{
  EZ_ASSERT_API(!IsReferenced(), "Memory stream storage destroyed while there are still references by reader / writer object(s)!");
}


// Reader implementation
ezMemoryStreamReader::ezMemoryStreamReader(ezMemoryStreamStorage* pStreamStorage)
  : m_pStreamStorage(pStreamStorage), m_uiReadPosition(0)
{
  EZ_ASSERT_API(pStreamStorage != NULL, "The memory stream reader needs a valid memory storage object!");
}

ezMemoryStreamReader::~ezMemoryStreamReader()
{
}

ezUInt64 ezMemoryStreamReader::ReadBytes(void* pReadBuffer, ezUInt64 uiBytesToRead)
{
  const ezUInt32 uiBytes = ezMath::Min<ezUInt32>(static_cast<ezUInt32>(uiBytesToRead), m_pStreamStorage->m_Storage.GetCount() - m_uiReadPosition);

  if (uiBytes == 0)
    return 0;

  if (pReadBuffer)
    ezMemoryUtils::Copy(static_cast<ezUInt8*>(pReadBuffer), &m_pStreamStorage->m_Storage[m_uiReadPosition], uiBytes);

  m_uiReadPosition += uiBytes;

  return uiBytes;
}

ezUInt64 ezMemoryStreamReader::SkipBytes(ezUInt64 uiBytesToSkip)
{
  const ezUInt32 uiBytes = ezMath::Min<ezUInt32>(static_cast<ezUInt32>(uiBytesToSkip), m_pStreamStorage->m_Storage.GetCount() - m_uiReadPosition);

  m_uiReadPosition += uiBytes;

  return uiBytes;
}

void ezMemoryStreamReader::Rewind()
{
  m_uiReadPosition = 0;
}


// Writer implementation
ezMemoryStreamWriter::ezMemoryStreamWriter(ezMemoryStreamStorage* pStreamStorage)
  : m_pStreamStorage(pStreamStorage), m_uiWritePosition(0)
{
  EZ_ASSERT_API(pStreamStorage != NULL, "The memory stream writer needs a valid memory storage object!");
}

ezMemoryStreamWriter::~ezMemoryStreamWriter()
{
}

ezResult ezMemoryStreamWriter::WriteBytes(const void* pWriteBuffer, ezUInt64 uiBytesToWrite)
{
  if(uiBytesToWrite == 0)
    return EZ_SUCCESS;

  EZ_ASSERT(pWriteBuffer != NULL, "No valid buffer containing data given!");
  
  ezUInt64 uiNewSizeInBytes = m_uiWritePosition + uiBytesToWrite;
  EZ_ASSERT(uiNewSizeInBytes < ezInvalidIndex, "Memory stream only supports up to 4GB of data");

  const ezUInt32 uiBytesToWrite32 = static_cast<ezUInt32>(uiBytesToWrite);

  // Reserve the memory in the storage object
  m_pStreamStorage->m_Storage.SetCount(uiBytesToWrite32 + m_uiWritePosition);

  ezUInt8* pWritePointer = &m_pStreamStorage->m_Storage[m_uiWritePosition];

  ezMemoryUtils::Copy(pWritePointer, static_cast<const ezUInt8*>(pWriteBuffer), uiBytesToWrite32);
  
  m_uiWritePosition += uiBytesToWrite32;

  return EZ_SUCCESS;
}

void ezMemoryStreamWriter::Rewind()
{
  m_uiWritePosition = 0;
}
