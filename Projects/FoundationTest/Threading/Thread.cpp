#include <TestFramework/Framework/TestFramework.h>
#include <Foundation/Threading/Thread.h>
#include <Foundation/Threading/ThreadLocalStorage.h>
#include <Foundation/Threading/AtomicUtils.h>
#include <Foundation/Time/Time.h>

namespace 
{
  volatile ezInt32 g_iCrossThreadVariable = 0;
  const ezUInt32 g_uiIncrementSteps = 160000;

  class TestThread : public ezThread
  {
  public:

    TestThread()
      : ezThread("Test Thread")
    {
    }

    virtual ezUInt32 Run()
    {

      for(ezUInt32 i = 0; i < g_uiIncrementSteps; i++)
      {
        ezAtomicUtils::Increment(g_iCrossThreadVariable);

        ezThreadUtils::YieldTimeSlice();
      }

      return 0;
    }
  };
}

EZ_CREATE_SIMPLE_TEST_GROUP(Threading);

EZ_CREATE_SIMPLE_TEST(Threading, Thread)
{
  EZ_TEST_BLOCK(true, "Thread")
  {
    TestThread* pTestThread = NULL;
    TestThread* pTestThread2 = NULL;

    // the try-catch is necessary to quite the static code analysis
    // TODO (Marc): The thread internally throws an exception, maybe we can convince the code analysis there, that this is not a bug.
    try
    {
      pTestThread = new TestThread;
      pTestThread2 = new TestThread;
    }
    catch(...)
    {
    }

    EZ_TEST(pTestThread != NULL);
    EZ_TEST(pTestThread2 != NULL);

    // Both thread will increment via atomic operations the global variable
    pTestThread->Start();
    pTestThread2->Start();

    //pTestThread->WaitUntilRunningOrFinished();
    //pTestThread2->WaitUntilRunningOrFinished();

    // Main thread will also increment the test variable
    ezAtomicUtils::Increment(g_iCrossThreadVariable);

    // Join with both threads
    pTestThread->Join();
    pTestThread2->Join();

    // Test deletion
    delete pTestThread;
    delete pTestThread2;

    EZ_TEST_INT(g_iCrossThreadVariable, g_uiIncrementSteps * 2 + 1);
  }

  EZ_TEST_BLOCK(true, "Thread Sleeping")
  {
    const ezTime start = ezSystemTime::Now();

    ezTime sleepTime(ezTime::Seconds(0.3));

    ezThreadUtils::Sleep((ezUInt32)sleepTime.GetMilliSeconds());

    const ezTime stop = ezSystemTime::Now();

    const ezTime duration = stop - start;

    // We test for 0.25 - 0.35 since the threading functions are a bit varying in their precision
    EZ_TEST(duration.GetSeconds() > 0.25);
    EZ_TEST(duration.GetSeconds() < 0.35);
  }
}
