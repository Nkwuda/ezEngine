#include <TestFramework/Framework/TestFramework.h>
#include <Foundation/IO/OSFile.h>

EZ_CREATE_SIMPLE_TEST(IO, OSFile)
{
  ezStringBuilder sFileContent = "Lyrics to Taste The Cake:\n\
Turret: Who's there?\n\
Turret: Is anyone there?\n\
Turret: I see you.\n\
\n\
Chell rises from a stasis inside of a glass box\n\
She isn't greeted by faces,\n\
Only concrete and clocks.\n\
...";

  const ezUInt32 uiTextLen = sFileContent.GetElementCount();

  ezStringBuilder sOutputFile = BUILDSYSTEM_OUTPUT_FOLDER;
  sOutputFile.AppendPath("FoundationTest", "SubFolder");
  sOutputFile.AppendPath("OSFile_TestFile.txt");

  ezStringBuilder sOutputFile2 = BUILDSYSTEM_OUTPUT_FOLDER;
  sOutputFile2.AppendPath("FoundationTest", "SubFolder2");
  sOutputFile2.AppendPath("OSFile_TestFileCopy.txt");

  EZ_TEST_BLOCK(true, "Write File")
  {
    ezOSFile f;
    EZ_TEST(f.Open(sOutputFile.GetData(), ezFileMode::Write) == EZ_SUCCESS);
    EZ_TEST(f.IsOpen());
    EZ_TEST_INT(f.GetFilePosition(), 0);
    EZ_TEST_INT(f.GetFileSize(), 0);

    for (ezUInt32 i = 0; i < uiTextLen; ++i)
    {
      EZ_TEST(f.Write(&sFileContent.GetData()[i], 1) == EZ_SUCCESS);
      EZ_TEST_INT(f.GetFilePosition(), i + 1);
      EZ_TEST_INT(f.GetFileSize(), i + 1);
    }

    EZ_TEST_INT(f.GetFilePosition(), uiTextLen);
    f.SetFilePosition(5, ezFilePos::FromStart);
    EZ_TEST_INT(f.GetFileSize(), uiTextLen);

    EZ_TEST_INT(f.GetFilePosition(), 5);
    // f.Close(); // The file should be closed automatically
  }

  EZ_TEST_BLOCK(true, "Append File")
  {
    ezOSFile f;
    EZ_TEST(f.Open(sOutputFile.GetData(), ezFileMode::Append) == EZ_SUCCESS);
    EZ_TEST(f.IsOpen());
    EZ_TEST_INT(f.GetFilePosition(), uiTextLen);
    EZ_TEST(f.Write(sFileContent.GetData(), uiTextLen) == EZ_SUCCESS);
    EZ_TEST_INT(f.GetFilePosition(), uiTextLen * 2);
    f.Close();
    EZ_TEST(!f.IsOpen());
  }

  EZ_TEST_BLOCK(true, "Read File")
  {
    const ezUInt32 FS_MAX_PATH = 1024;
    char szTemp[FS_MAX_PATH];

    ezOSFile f;
    EZ_TEST(f.Open(sOutputFile.GetData(), ezFileMode::Read) == EZ_SUCCESS);
    EZ_TEST(f.IsOpen());
    EZ_TEST_INT(f.GetFilePosition(), 0);

    EZ_TEST_INT(f.Read(szTemp, FS_MAX_PATH), uiTextLen * 2);
    EZ_TEST_INT(f.GetFilePosition(), uiTextLen * 2);

    EZ_TEST(ezMemoryUtils::IsEqual(szTemp, sFileContent.GetData(), uiTextLen));
    EZ_TEST(ezMemoryUtils::IsEqual(&szTemp[uiTextLen], sFileContent.GetData(), uiTextLen));

    f.Close();
  }

  EZ_TEST_BLOCK(true, "Copy File")
  {
    ezOSFile::CopyFile(sOutputFile.GetData(), sOutputFile2.GetData());

    ezOSFile f;
    EZ_TEST(f.Open(sOutputFile2.GetData(), ezFileMode::Read) == EZ_SUCCESS);

    const ezUInt32 FS_MAX_PATH = 1024;
    char szTemp[FS_MAX_PATH];

    EZ_TEST_INT(f.Read(szTemp, FS_MAX_PATH), uiTextLen * 2);

    EZ_TEST(ezMemoryUtils::IsEqual(szTemp, sFileContent.GetData(), uiTextLen));
    EZ_TEST(ezMemoryUtils::IsEqual(&szTemp[uiTextLen], sFileContent.GetData(), uiTextLen));

    f.Close();
  }

#ifdef EZ_SUPPORTS_FILE_STATS
  EZ_TEST_BLOCK(true, "File Stats")
  {
    ezFileStats s;

    ezStringBuilder dir = sOutputFile2.GetFileDirectory();
    dir.ToLower();

    EZ_TEST(ezOSFile::GetFileStats(sOutputFile2.GetData(), s) == EZ_SUCCESS);
    //printf("%s Name: '%s' (%lli Bytes), Modified Time: %lli\n", s.m_bIsDirectory ? "Directory" : "File", s.m_sFileName.GetData(), s.m_uiFileSize, s.m_uiLastModificationTime);

    EZ_TEST(ezOSFile::GetFileStats(dir.GetData(), s) == EZ_SUCCESS);
    //printf("%s Name: '%s' (%lli Bytes), Modified Time: %lli\n", s.m_bIsDirectory ? "Directory" : "File", s.m_sFileName.GetData(), s.m_uiFileSize, s.m_uiLastModificationTime);

    ezStringBuilder sOutputFile3 = BUILDSYSTEM_OUTPUT_FOLDER;
    sOutputFile3.AppendPath("FoundationTest", "SubFolder2");
    sOutputFile3.AppendPath("*.txt");

    EZ_TEST(ezOSFile::GetFileStats(sOutputFile3.GetData(), s) == EZ_SUCCESS);
    //printf("%s Name: '%s' (%lli Bytes), Modified Time: %lli\n", s.m_bIsDirectory ? "Directory" : "File", s.m_sFileName.GetData(), s.m_uiFileSize, s.m_uiLastModificationTime);
  }

  EZ_TEST_BLOCK(true, "GetFileCasing")
  {
    ezStringBuilder dir = sOutputFile2;
    dir.ToLower();

    ezStringBuilder sCorrected;
    EZ_TEST(ezOSFile::GetFileCasing(dir.GetData(), sCorrected) == EZ_SUCCESS);

    // On Windows the drive letter will always be made to upper case
    EZ_TEST_STRING(sCorrected.GetData(), sOutputFile2.GetData());
  }

#endif

#ifdef EZ_SUPPORTS_FILE_ITERATORS

  EZ_TEST_BLOCK(true, "File Iterator")
  {
    // It is not really possible to test this stuff (with a guaranteed result), as long as we do not have
    // a test data folder with deterministic content
    // Therefore I tested it manually, and leave the code in, such that it is at least a 'does it compile and link' test.

    ezStringBuilder sOutputFolder = BUILDSYSTEM_OUTPUT_FOLDER;
    sOutputFolder.AppendPath("Debug*");

    ezStringBuilder sFullPath;

    ezFileSystemIterator it;
    if (it.StartSearch(sOutputFolder.GetData(), true, true))
    {
      int SkipFolder = 1;

      do
      {
        sFullPath = it.GetCurrentPath();
        sFullPath.AppendPath(it.GetStats().m_sFileName.GetData());

        it.GetStats();
        it.GetCurrentPath();

        if (it.GetStats().m_bIsDirectory)
        {
          SkipFolder++;

          if ((SkipFolder % 2 == 0) && (!it.SkipFolder()))
            break;
        }

        //printf("%s: '%s'\n", it.GetStats().m_bIsDirectory ? "[Dir] " : "[File]", sFullPath.GetData());
      }
      while(it.Next());
    }
  }

#endif

  EZ_TEST_BLOCK(true, "Delete File")
  {
    EZ_TEST(ezOSFile::DeleteFile(sOutputFile.GetData()) == EZ_SUCCESS);
    EZ_TEST(ezOSFile::DeleteFile(sOutputFile.GetData()) == EZ_SUCCESS); // second time should still 'succeed'

    EZ_TEST(ezOSFile::DeleteFile(sOutputFile2.GetData()) == EZ_SUCCESS);
    EZ_TEST(ezOSFile::DeleteFile(sOutputFile2.GetData()) == EZ_SUCCESS); // second time should still 'succeed'

    ezOSFile f;
    EZ_TEST(f.Open(sOutputFile.GetData(), ezFileMode::Read) == EZ_FAILURE); // file should not exist anymore
    EZ_TEST(f.Open(sOutputFile2.GetData(), ezFileMode::Read) == EZ_FAILURE); // file should not exist anymore
  }
}
