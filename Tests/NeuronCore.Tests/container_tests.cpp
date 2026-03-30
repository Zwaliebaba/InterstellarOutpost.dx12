#include <CppUnitTest.h>
#include "darray.h"
#include "llist.h"
#include "hash_table.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace NeuronCoreTests
{
  TEST_CLASS(DArrayTests)
  {
  public:
    TEST_METHOD(PushBack_IncreasesSize)
    {
      DArray<int> arr;
      arr.PushBack(1);
      arr.PushBack(2);
      Assert::AreEqual(2, arr.Size());
    }

    TEST_METHOD(Operator_ReturnsInsertedValue)
    {
      DArray<int> arr;
      arr.PushBack(42);
      Assert::AreEqual(42, arr[0]);
    }

    TEST_METHOD(Empty_ReturnsTrueWhenNoElements)
    {
      DArray<int> arr;
      Assert::IsTrue(arr.Empty());
    }

    TEST_METHOD(Empty_ReturnsFalseAfterPushBack)
    {
      DArray<int> arr;
      arr.PushBack(1);
      Assert::IsFalse(arr.Empty());
    }

    TEST_METHOD(RemoveData_DecreasesSize)
    {
      DArray<int> arr;
      arr.PushBack(10);
      arr.PushBack(20);
      arr.RemoveData(10);
      Assert::AreEqual(1, arr.Size());
    }
  };

  TEST_CLASS(LListTests)
  {
  public:
    TEST_METHOD(PushBack_IncreasesSize)
    {
      LList<int> list;
      list.PushBack(1);
      list.PushBack(2);
      Assert::AreEqual(2, list.Size());
    }

    TEST_METHOD(PopFront_ReturnsFirstInserted)
    {
      LList<int> list;
      list.PushBack(10);
      list.PushBack(20);
      Assert::AreEqual(10, list.GetData(0));
    }

    TEST_METHOD(Empty_ReturnsTrueWhenNoElements)
    {
      LList<int> list;
      Assert::IsTrue(list.Size() == 0);
    }
  };

  TEST_CLASS(HashTableTests)
  {
  public:
    TEST_METHOD(GetData_ReturnsInsertedValue)
    {
      HashTable<char const*, int> table;
      table.PutData("key", 99);
      int* val = table.GetData("key");
      Assert::IsNotNull(val);
      Assert::AreEqual(99, *val);
    }

    TEST_METHOD(GetData_ReturnsNull_ForMissingKey)
    {
      HashTable<char const*, int> table;
      int* val = table.GetData("missing");
      Assert::IsNull(val);
    }

    TEST_METHOD(RemoveData_MakesKeyUnreachable)
    {
      HashTable<char const*, int> table;
      table.PutData("key", 1);
      table.RemoveData("key");
      Assert::IsNull(table.GetData("key"));
    }
  };
}
