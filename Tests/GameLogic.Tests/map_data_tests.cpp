#include <CppUnitTest.h>
#include "MapData.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace GameLogicTests
{
  TEST_CLASS(MapDataTests)
  {
  public:
    TEST_METHOD(DefaultConstructor_InitializesNumPlayersToZero)
    {
      MapData map;
      Assert::AreEqual(0, map.m_numPlayers);
    }

    TEST_METHOD(DefaultConstructor_InitializesCoopFalse)
    {
      MapData map;
      Assert::IsFalse(map.m_coop);
    }

    TEST_METHOD(DefaultConstructor_InitializesCustomMapFalse)
    {
      MapData map;
      Assert::IsFalse(map.m_customMap);
    }

    TEST_METHOD(DefaultConstructor_InitializesOfficialMapFalse)
    {
      MapData map;
      Assert::IsFalse(map.m_officialMap);
    }

    TEST_METHOD(DefaultConstructor_InitializesPointerFieldsNull)
    {
      MapData map;
      Assert::IsNull(map.m_fileName);
      Assert::IsNull(map.m_levelName);
      Assert::IsNull(map.m_description);
      Assert::IsNull(map.m_author);
    }

    TEST_METHOD(CalculateMapId_ProducesSameId_ForSameFileName)
    {
      MapData map1;
      MapData map2;
      map1.m_fileName = const_cast<char*>("level_01.txt");
      map2.m_fileName = const_cast<char*>("level_01.txt");
      map1.CalculeMapId();
      map2.CalculeMapId();
      Assert::AreEqual(map1.m_mapId, map2.m_mapId);
    }

    TEST_METHOD(CalculateMapId_ProducesDifferentIds_ForDifferentFileNames)
    {
      MapData map1;
      MapData map2;
      map1.m_fileName = const_cast<char*>("level_01.txt");
      map2.m_fileName = const_cast<char*>("level_02.txt");
      map1.CalculeMapId();
      map2.CalculeMapId();
      Assert::AreNotEqual(map1.m_mapId, map2.m_mapId);
    }
  };
}
