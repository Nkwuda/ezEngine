#include <PCH.h>

#include <Foundation/IO/MemoryStream.h>
#include <Foundation/IO/Stream.h>
#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Containers/StaticArray.h>
#include <Foundation/Containers/HybridArray.h>

EZ_CREATE_SIMPLE_TEST(IO, StreamOperation)
{
  EZ_TEST_BLOCK(ezTestBlock::Enabled, "Binary Stream Basic Operations (built-in types)")
  {
    ezMemoryStreamStorage StreamStorage(4096);

    // Create writer
    ezMemoryStreamWriter StreamWriter(&StreamStorage);

    StreamWriter << (ezUInt8)0x42;
    StreamWriter << (ezUInt16)0x4223;
    StreamWriter << (ezUInt32)0x42232342;
    StreamWriter << (ezUInt64)0x4223234242232342;
    StreamWriter << 42.0f;
    StreamWriter << 23.0;
    StreamWriter << (ezInt8)0x23;
    StreamWriter << (ezInt16)0x2342;
    StreamWriter << (ezInt32)0x23422342;
    StreamWriter << (ezInt64)0x2342234242232342;

    // Arrays
    {
      ezDynamicArray<ezUInt32> DynamicArray;
      DynamicArray.PushBack(42);
      DynamicArray.PushBack(23);
      DynamicArray.PushBack(13);
      DynamicArray.PushBack(5);
      DynamicArray.PushBack(0);

      StreamWriter.WriteArray(DynamicArray);
    }

    // Create reader
    ezMemoryStreamReader StreamReader(&StreamStorage);

    // Read back
    {
      ezUInt8 uiVal;
      StreamReader >> uiVal;
      EZ_TEST_BOOL(uiVal == (ezUInt8)0x42);
    }
    {
      ezUInt16 uiVal;
      StreamReader >> uiVal;
      EZ_TEST_BOOL(uiVal == (ezUInt16)0x4223);
    }
    {
      ezUInt32 uiVal;
      StreamReader >> uiVal;
      EZ_TEST_BOOL(uiVal == (ezUInt32)0x42232342);
    }
    {
      ezUInt64 uiVal;
      StreamReader >> uiVal;
      EZ_TEST_BOOL(uiVal == (ezUInt64)0x4223234242232342);
    }

    {
      float fVal;
      StreamReader >> fVal;
      EZ_TEST_BOOL(fVal == 42.0f);
    }
    {
      double dVal;
      StreamReader >> dVal;
      EZ_TEST_BOOL(dVal == 23.0f);
    }


    {
      ezInt8 iVal;
      StreamReader >> iVal;
      EZ_TEST_BOOL(iVal == (ezInt8)0x23);
    }
    {
      ezInt16 iVal;
      StreamReader >> iVal;
      EZ_TEST_BOOL(iVal == (ezInt16)0x2342);
    }
    {
      ezInt32 iVal;
      StreamReader >> iVal;
      EZ_TEST_BOOL(iVal == (ezInt32)0x23422342);
    }
    {
      ezInt64 iVal;
      StreamReader >> iVal;
      EZ_TEST_BOOL(iVal == (ezInt64)0x2342234242232342);
    }

    {
      ezDynamicArray<ezUInt32> ReadBackDynamicArray;

      // This element will be removed by the ReadArray function
      ReadBackDynamicArray.PushBack(0xAAu);

      StreamReader.ReadArray(ReadBackDynamicArray);

      EZ_TEST_INT(ReadBackDynamicArray.GetCount(), 5);

      EZ_TEST_INT(ReadBackDynamicArray[0], 42);
      EZ_TEST_INT(ReadBackDynamicArray[1], 23);
      EZ_TEST_INT(ReadBackDynamicArray[2], 13);
      EZ_TEST_INT(ReadBackDynamicArray[3], 5);
      EZ_TEST_INT(ReadBackDynamicArray[4], 0);
    }
  }

  EZ_TEST_BLOCK(ezTestBlock::Enabled, "Binary Stream Arrays of Structs")
  {
    ezMemoryStreamStorage StreamStorage(4096);

    // Create writer
    ezMemoryStreamWriter StreamWriter(&StreamStorage);

    struct SerializableStructWithMethods
    {

      EZ_DECLARE_POD_TYPE();

      ezResult Serialize(ezStreamWriter& Stream) const
      {
        Stream << m_uiMember1;
        Stream << m_uiMember2;

        return EZ_SUCCESS;
      }

      ezResult Deserialize(ezStreamReader& Stream)
      {
        Stream >> m_uiMember1;
        Stream >> m_uiMember2;

        return EZ_SUCCESS;
      }

      ezInt32 m_uiMember1 = 0x42;
      ezInt32 m_uiMember2 = 0x23;
    };

    // Write out a couple of the structs
    {
      ezStaticArray<SerializableStructWithMethods, 16> WriteArray;
      WriteArray.ExpandAndGetRef().m_uiMember1 = 0x5;
      WriteArray.ExpandAndGetRef().m_uiMember1 = 0x6;

      StreamWriter.WriteArray(WriteArray);
    }

    // Read back in
    {
      // Create reader
      ezMemoryStreamReader StreamReader(&StreamStorage);

      // This intentionally uses a different array type for the read back
      // to verify that it is a) compatible and b) all arrays are somewhat tested
      ezHybridArray<SerializableStructWithMethods, 1> ReadArray;

      StreamReader.ReadArray(ReadArray);

      EZ_TEST_INT(ReadArray.GetCount(), 2);

      EZ_TEST_INT(ReadArray[0].m_uiMember1, 0x5);
      EZ_TEST_INT(ReadArray[0].m_uiMember2, 0x23);

      EZ_TEST_INT(ReadArray[1].m_uiMember1, 0x6);
      EZ_TEST_INT(ReadArray[1].m_uiMember2, 0x23);
    }
  }
}
