#include <TestFramework/Framework/TestFramework.h>
#include <Foundation/Containers/HybridArray.h>
#include <TestFramework/Utilities/ConstructionCounter.h>

typedef ezConstructionCounter st;

namespace
{
  class Dummy
  {
  public:
    int a;
    std::string s;

    Dummy() : a(0), s("Test") { }
    Dummy(int a) : a(a), s("Test") { }
    Dummy(const Dummy& other) : a(other.a), s(other.s) { }
    ~Dummy() { }

    Dummy& operator=(const Dummy& other)
    {
      a = other.a;
      s = other.s;
      return *this;
    }

    bool operator<=(const Dummy& dummy) const { return a <= dummy.a; }
    bool operator>=(const Dummy& dummy) const { return a >= dummy.a; }
    bool operator>(const Dummy& dummy) const { return a > dummy.a; }
    bool operator<(const Dummy& dummy) const { return a < dummy.a; }
    bool operator==(const Dummy& dummy) const { return a == dummy.a; }
  };
}

#if EZ_PLATFORM_64BIT
  EZ_CHECK_AT_COMPILETIME(sizeof(ezHybridArray<ezInt32, 1>) == 32);
#else
  EZ_CHECK_AT_COMPILETIME(sizeof(ezHybridArray<ezInt32, 1>) == 20);
#endif


EZ_CREATE_SIMPLE_TEST(Containers, HybridArray)
{
  EZ_TEST_BLOCK(true, "Constructor")
  {
    ezHybridArray<ezInt32, 16> a1;
    ezHybridArray<st, 16> a2;

    EZ_TEST(a1.GetCount() == 0);
    EZ_TEST(a2.GetCount() == 0);
    EZ_TEST(a1.IsEmpty());
    EZ_TEST(a2.IsEmpty());
  }

  EZ_TEST_BLOCK(true, "Copy Constructor")
  {
    ezHybridArray<ezInt32, 16> a1;

    for (ezInt32 i = 0; i < 32; ++i)
      a1.Append(rand() % 100000);

    ezHybridArray<ezInt32, 16> a2 = a1;
    ezHybridArray<ezInt32, 16> a3 (a1);

    EZ_TEST(a1 == a2);
    EZ_TEST(a1 == a3);
    EZ_TEST(a2 == a3);
  }

  EZ_TEST_BLOCK(true, "Convert to ArrayPtr")
  {
    ezHybridArray<ezInt32, 16> a1;

    for (ezInt32 i = 0; i < 100; ++i)
    {
      ezInt32 r = rand() % 100000;
      a1.Append(r);
    }

    ezArrayPtr<ezInt32> ap = a1;

    EZ_TEST(ap.GetCount () == a1.GetCount());
  }

  EZ_TEST_BLOCK(true, "operator =")
  {
    ezHybridArray<ezInt32, 16> a1, a2;

    for (ezInt32 i = 0; i < 100; ++i)
      a1.Append(i);

    a2 = a1;

    EZ_TEST(a1 == a2);
  }

  EZ_TEST_BLOCK(true, "operator == / !=")
  {
    ezHybridArray<ezInt32, 16> a1, a2;
    
    EZ_TEST(a1 == a1);
    EZ_TEST(a2 == a2);
    EZ_TEST(a1 == a2);

    EZ_TEST((a1 != a1) == false);
    EZ_TEST((a2 != a2) == false);
    EZ_TEST((a1 != a2) == false);

    for (ezInt32 i = 0; i < 100; ++i)
    {
      ezInt32 r = rand() % 100000;
      a1.Append(r);
      a2.Append(r);
    }

    EZ_TEST(a1 == a1);
    EZ_TEST(a2 == a2);
    EZ_TEST(a1 == a2);

    EZ_TEST((a1 != a2) == false);
  }

  EZ_TEST_BLOCK(true, "Index operator")
  {
    ezHybridArray<ezInt32, 16> a1;
    a1.SetCount(100);

    for (ezInt32 i = 0; i < 100; ++i)
      a1[i] = i;;

    for (ezInt32 i = 0; i < 100; ++i)
      EZ_TEST_INT(a1[i], i);

    const ezHybridArray<ezInt32, 16> ca1 = a1;

    for (ezInt32 i = 0; i < 100; ++i)
      EZ_TEST_INT(ca1[i], i);
  }

  EZ_TEST_BLOCK(true, "SetCount / GetCount / IsEmpty")
  {
    ezHybridArray<ezInt32, 16> a1;

    EZ_TEST(a1.IsEmpty());

    for (ezInt32 i = 0; i < 128; ++i)
    {
      a1.SetCount(i + 1);
      a1[i] = i;

      EZ_TEST_INT(a1.GetCount(), i + 1);
      EZ_TEST(!a1.IsEmpty());
    }

    for (ezInt32 i = 0; i < 128; ++i)
      EZ_TEST_INT(a1[i], i);

    for (ezInt32 i = 128; i >= 0; --i)
    {
      a1.SetCount(i);

      EZ_TEST_INT(a1.GetCount(), i);

      for (ezInt32 i2 = 0; i2 < i; ++i2)
        EZ_TEST_INT(a1[i2], i2);
    }

    EZ_TEST(a1.IsEmpty());
  }

  EZ_TEST_BLOCK(true, "Clear")
  {
    ezHybridArray<ezInt32, 16> a1;
    a1.Clear();

    a1.Append(3);
    a1.Clear();

    EZ_TEST(a1.IsEmpty());
  }

  EZ_TEST_BLOCK(true, "Contains / IndexOf / LastIndexOf")
  {
    ezHybridArray<ezInt32, 16> a1;

    for (ezInt32 i = -100; i < 100; ++i)
      EZ_TEST(!a1.Contains(i));

    for (ezInt32 i = 0; i < 100; ++i)
      a1.Append(i);

    for (ezInt32 i = 0; i < 100; ++i)
    {
      EZ_TEST(a1.Contains(i));
      EZ_TEST_INT(a1.IndexOf(i), i);
      EZ_TEST_INT(a1.LastIndexOf(i), i);
    }
  }

  EZ_TEST_BLOCK(true, "Insert")
  {
    ezHybridArray<ezInt32, 16> a1;

    // always inserts at the front
    for (ezInt32 i = 0; i < 100; ++i)
      a1.Insert(i, 0);

    for (ezInt32 i = 0; i < 100; ++i)
      EZ_TEST_INT(a1[i], 99 - i);
  }

  EZ_TEST_BLOCK(true, "Remove")
  {
    ezHybridArray<ezInt32, 16> a1;

    for (ezInt32 i = 0; i < 100; ++i)
      a1.Push(i % 2);

    while (a1.Remove(1));

    EZ_TEST(a1.GetCount() == 50);

    for (ezUInt32 i = 0; i < a1.GetCount(); ++i)
      EZ_TEST_INT(a1[i], 0);
  }

  EZ_TEST_BLOCK(true, "RemoveAt")
  {
    ezHybridArray<ezInt32, 16> a1;

    for (ezInt32 i = 0; i < 10; ++i)
      a1.Insert(i, i); // inserts at the end

    a1.RemoveAt(9);
    a1.RemoveAt(7);
    a1.RemoveAt(5);
    a1.RemoveAt(3);
    a1.RemoveAt(1);

    EZ_TEST_INT(a1.GetCount(), 5);

    for (ezInt32 i = 0; i < 5; ++i)
      EZ_TEST_INT(a1[i], i * 2);
  }

  EZ_TEST_BLOCK(true, "RemoveAtSwap")
  {
    ezHybridArray<ezInt32, 16> a1;

    for (ezInt32 i = 0; i < 10; ++i)
      a1.Insert(i, i); // inserts at the end

    a1.RemoveAtSwap(9);
    a1.RemoveAtSwap(7);
    a1.RemoveAtSwap(5);
    a1.RemoveAtSwap(3);
    a1.RemoveAtSwap(1);

    EZ_TEST_INT(a1.GetCount(), 5);

    for (ezInt32 i = 0; i < 5; ++i)
      EZ_TEST(ezMath::IsEven(a1[i]));
  }

  EZ_TEST_BLOCK(true, "Push / Pop / Peek")
  {
    ezHybridArray<ezInt32, 16> a1;

    for (ezInt32 i = 0; i < 10; ++i)
    {
      a1.Push(i);
      EZ_TEST_INT(a1.Peek(), i);
    }

    for (ezInt32 i = 9; i >= 0; --i)
    {
      EZ_TEST_INT(a1.Peek(), i);
      a1.Pop();
    }

    a1.Push(23);
    a1.Push(2);
    a1.Push(3);

    a1.Pop(2);
    EZ_TEST_INT(a1.Peek(), 23);
  }

  EZ_TEST_BLOCK(true, "Construction / Destruction")
  {
    {
      EZ_TEST(st::HasAllDestructed());

      ezHybridArray<st, 16> a1;
      ezHybridArray<st, 16> a2;

      EZ_TEST(st::HasDone(0, 0)); // nothing has been constructed / destructed in between
      EZ_TEST(st::HasAllDestructed());

      a1.Append(st(1));
      EZ_TEST(st::HasDone(2, 1)); // one temporary, one final (copy constructed)

      a1.Insert(st(2), 0);
      EZ_TEST(st::HasDone(2, 1)); // one temporary, one final (copy constructed)

      a2 = a1;
      EZ_TEST(st::HasDone(2, 0)); // two copies

      a1.Clear();
      EZ_TEST(st::HasDone(0, 2));

      a1.Push(st(3));
      a1.Push(st(4));
      a1.Push(st(5));
      a1.Push(st(6));

      EZ_TEST(st::HasDone(8, 4)); // four temporaries

      a1.Remove(st(3));
      EZ_TEST(st::HasDone(1, 2)); // one temporary, one destroyed

      a1.Remove(st(3));
      EZ_TEST(st::HasDone(1, 1)); // one temporary, none destroyed

      a1.RemoveAt(0);
      EZ_TEST(st::HasDone(0, 1)); // one destroyed

      a1.RemoveAtSwap(0);
      EZ_TEST(st::HasDone(0, 1)); // one destroyed
    }

    // tests the destructor of a2 and a1
    EZ_TEST(st::HasAllDestructed());
  }

  EZ_TEST_BLOCK(true, "Compact")
  {
    ezHybridArray<ezInt32, 16> a;

    for (ezInt32 i = 0; i < 1000; ++i)
    {
      a.Push(i);
      EZ_TEST_INT(a.GetCount(), i + 1);
    }

    a.Compact();

    for (ezInt32 i = 0; i < 1000; ++i)
      EZ_TEST_INT(a[i], i);

    // this tests whether the static array is reused properly (well, I stepped through it in the debugger to check it)
    a.SetCount(15);
    a.Compact();

    for (ezInt32 i = 0; i < 15; ++i)
      EZ_TEST_INT(a[i], i);
  }

  EZ_TEST_BLOCK(true, "SortingPrimitives")
  {
    ezHybridArray<ezUInt32, 16> list;

    list.Sort();

    for (ezUInt32 i = 0; i < 45; i++)
    {
      list.Append(std::rand());
    }
    list.Sort();

    ezUInt32 last = 0;
    for (ezUInt32 i = 0; i < list.GetCount(); i++)
    {
      EZ_TEST(last <= list[i]);
      last = list[i];
    }
  }

  EZ_TEST_BLOCK(true, "SortingObjects")
  {
    ezHybridArray<Dummy, 16> list;
    list.Reserve(128);

    for (ezUInt32 i = 0; i < 100; i++)
    {
      list.Append(Dummy(rand()));
    }
    list.Sort();

    Dummy last = 0;
    for (ezUInt32 i = 0; i < list.GetCount(); i++)
    {
      EZ_TEST(last <= list[i]);
      last = list[i];
    }
  }

  EZ_TEST_BLOCK(true, "Various")
  {
    ezHybridArray<Dummy, 16> list;
    list.Append(1);
    list.Append(2);
    list.Append(3);
    list.Insert(4, 3);
    list.Insert(0, 1);
    list.Insert(0, 5);

    EZ_TEST(list[0].a == 1);
    EZ_TEST(list[1].a == 0);
    EZ_TEST(list[2].a == 2);
    EZ_TEST(list[3].a == 3);
    EZ_TEST(list[4].a == 4);
    EZ_TEST(list[5].a == 0);
    EZ_TEST(list.GetCount() == 6);

    list.RemoveAt(3);
    list.RemoveAtSwap(2);

    EZ_TEST(list[0].a == 1);
    EZ_TEST(list[1].a == 0);
    EZ_TEST(list[2].a == 0);
    EZ_TEST(list[3].a == 4);
    EZ_TEST(list.GetCount() == 4);
    EZ_TEST(list.IndexOf(0) == 1);
    EZ_TEST(list.LastIndexOf(0) == 2);

    list.Push(5);
    EZ_TEST(list[4].a == 5);
    Dummy d = list.Peek();
    list.Pop();
    EZ_TEST(d.a == 5);
    EZ_TEST(list.GetCount() == 4);
  }

  EZ_TEST_BLOCK(true, "Assignment")
  {
    ezHybridArray<Dummy, 16> list;
    for (int i = 0; i < 16; i++)
    {
      list.Append(Dummy(rand()));
    }

    ezHybridArray<Dummy, 16> list2;
    for (int i = 0; i < 8; i++)
    {
      list2.Append(Dummy(rand()));
    }

    list = list2;
    EZ_TEST(list.GetCount() == list2.GetCount());

    list2.Clear();
    EZ_TEST(list2.GetCount() == 0);

    list2 = list;
    EZ_TEST(list.Peek() == list2.Peek());
    EZ_TEST(list == list2);

    for (int i = 0; i < 16; i++)
    {
      list2.Append(Dummy(rand()));
    }

    list = list2;
    EZ_TEST(list.Peek() == list2.Peek());
    EZ_TEST(list == list2);
  }

  EZ_TEST_BLOCK(true, "Count")
  {
    ezHybridArray<Dummy, 16> list;
    for (int i = 0; i < 16; i++)
    {
      list.Append(Dummy(rand()));
    }
    list.SetCount(32);
    list.SetCount(4);

    list.Compact();
  }
}