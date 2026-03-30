#include <CppUnitTest.h>
#include "entity.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace GameLogicTests
{
  TEST_CLASS(EntityFactoryTests)
  {
  public:
    TEST_METHOD_CLEANUP(Cleanup)
    {
      // Derived tests must delete any entity they allocate to avoid leaks.
    }

    TEST_METHOD(NewEntity_ReturnsNonNull_ForEveryKnownType)
    {
      for (int type = Entity::TypeLaserTroop; type < Entity::NumEntityTypes; ++type)
      {
        Entity* e = Entity::NewEntity(type);
        Assert::IsNotNull(e,
          (L"NewEntity returned null for type " + std::to_wstring(type)).c_str());
        delete e;
      }
    }

    TEST_METHOD(NewEntity_ReturnsNull_ForInvalidType)
    {
      Entity* e = Entity::NewEntity(Entity::TypeInvalid);
      Assert::IsNull(e);
    }

    TEST_METHOD(NewEntity_ReturnsNull_ForOutOfRangeType)
    {
      Entity* e = Entity::NewEntity(Entity::NumEntityTypes);
      Assert::IsNull(e);
    }

    TEST_METHOD(NewEntity_ReturnsCorrectType_ForLaserTroop)
    {
      Entity* e = Entity::NewEntity(Entity::TypeLaserTroop);
      Assert::IsNotNull(e);
      Assert::AreEqual(static_cast<int>(Entity::TypeLaserTroop), e->m_type);
      delete e;
    }

    TEST_METHOD(NewEntity_ReturnsCorrectType_ForDarwinian)
    {
      Entity* e = Entity::NewEntity(Entity::TypeDarwinian);
      Assert::IsNotNull(e);
      Assert::AreEqual(static_cast<int>(Entity::TypeDarwinian), e->m_type);
      delete e;
    }
  };

  TEST_CLASS(EntityBlueprintTests)
  {
  public:
    TEST_METHOD(GetStat_Health_IsPositive_ForAllTypes)
    {
      for (int type = Entity::TypeLaserTroop; type < Entity::NumEntityTypes; ++type)
      {
        double health = EntityBlueprint::m_stats[type][Entity::StatHealth];
        Assert::IsTrue(health > 0.0,
          (L"Health stat must be positive for type " + std::to_wstring(type)).c_str());
      }
    }

    TEST_METHOD(GetName_IsNonEmpty_ForAllTypes)
    {
      for (int type = Entity::TypeLaserTroop; type < Entity::NumEntityTypes; ++type)
      {
        char* name = EntityBlueprint::m_names[type];
        Assert::IsNotNull(name,
          (L"Name must not be null for type " + std::to_wstring(type)).c_str());
        Assert::IsTrue(strlen(name) > 0,
          (L"Name must not be empty for type " + std::to_wstring(type)).c_str());
      }
    }
  };
}
