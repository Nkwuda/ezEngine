#pragma once

#include <TestFramework/Framework/TestFramework.h>

#ifdef EZ_PLATFORM_WINDOWS
  inline void SetConsoleColorInl (WORD ui)
  {
    SetConsoleTextAttribute (GetStdHandle (STD_OUTPUT_HANDLE), ui);
  }
#else
  inline void SetConsoleColorInl (ezUInt8 ui) { }
#endif

inline void OutputToConsole (ezTestOutput::Enum Type, const char* szMsg)
{
  static ezInt32 iIndentation = 0;
  static bool bAnyError = false;

  switch (Type)
  {
  case ezTestOutput::BeginBlock:
    iIndentation += 2;
    break;
  case ezTestOutput::EndBlock:
    iIndentation -= 2;
    break;
  case ezTestOutput::Info:
    SetConsoleColorInl (0x07);
    break;
  case ezTestOutput::Success:
    SetConsoleColorInl (0x0A);
    break;
  case ezTestOutput::Message:
    SetConsoleColorInl (0x0E);
    break;
  case ezTestOutput::Error:
    SetConsoleColorInl (0x0C);
    bAnyError = true;
    break;
  case ezTestOutput::FinalResult:
    if (bAnyError)
      SetConsoleColorInl (0x0C);
    else
      SetConsoleColorInl (0x0A);

    // reset it for the next test round
    bAnyError = false;
    break;
  }

  printf ("%*s%s\n", iIndentation, "", szMsg);
  SetConsoleColorInl (0x07);
}
