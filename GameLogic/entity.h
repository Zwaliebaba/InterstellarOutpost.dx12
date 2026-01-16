#ifndef _included_entity_h
#define _included_entity_h

#include "llist.h"
#include "worldobject.h"
#include "unicode_string.h"
#include "shape.h"

class TeamControls;
class AI;
class Unit;

class Entity : public WorldObject
{
  public:
    enum
    {
      TypeInvalid = 0,
      // Remember to update Entity::GetTypeName and
      TypeLaserTroop,
      // Entity::NewEntity if you update this table
      TypeEngineer,
      TypeVirii,
      TypeInsertionSquadie,
      TypeEgg,
      TypeSporeGenerator,
      TypeLander,
      TypeTripod,
      TypeCentipede,
      TypeSpaceInvader,
      TypeSpider,
      TypeDarwinian,
      TypeOfficer,
      TypeArmyAnt,
      TypeArmour,
      TypeSoulDestroyer,
      TypeTriffidEgg,
      TypeAI,
      TypeShaman,
      TypeHarvester,
      TypeNuke,
      TypeTentacle,
      TypeMeteor,
      TypeTank,
      TypeSpaceShip,
      TypeDropShip,
      NumEntityTypes
    };

    enum
    {
      StatHealth = 0,
      StatSpeed,
      StatRate,
      NumStats
    };

    enum
    {
      DamageTypeUnresistable,
      // non-typed damage generally used for just killing entities outright
      DamageTypeLaser,
      DamageTypeExplosive,
      DamageTypeTurretShell,
      DamageTypeFire,
      NumDamageTypes
    };

    int m_formationIndex; // Our offset within the unit (NOT our index in the array)    
    int m_buildingId; // Which building created us

    Vector3 m_spawnPoint; // Where I was created
    double m_roamRange; // How far can I roam

    unsigned char m_stats[NumStats];
    bool m_dead; // Used to fade out dead creatures
    bool m_justFired;
    double m_reloading; // Time left to reload
    double m_inWater; // -1 = no, otherwise = time in water

    bool m_destroyed; // something has removed us entirely - just bail out

    Vector3 m_front;
    Vector3 m_angVel;

    Shape* m_shape; // Might be NULL
    Vector3 m_centrePos;
    double m_radius; // Can be Zero, which means its a sprite

    bool m_renderDamaged;

    int m_routeId;
    int m_routeWayPointId;
    double m_routeTriggerDistance; // distance the unit must be from the route node to move on to the next

    double m_aiTimer;

    static int s_entityPopulation;

  protected:
    static std::vector<WorldObjectId> s_neighbours;

  public:
    Entity();
    ~Entity() override;

    void SetType(unsigned char _type); // Loads default stats from blueprint

    void Render(double predictionTime) override;

    virtual void Begin();
    virtual bool Advance(Unit* _unit);
    virtual bool AdvanceDead(Unit* _unit);
    virtual void AdvanceInAir(Unit* _unit);
    virtual void AdvanceInWater(Unit* _unit);

    virtual void RunAI(AI* _ai);

    virtual bool ChangeHealth(int amount, int _damageType = DamageTypeUnresistable);
    virtual void Attack(const Vector3& pos);

    virtual bool IsInView();

    virtual void SetWaypoint(Vector3 _waypoint);

    virtual Vector3 PushFromObstructions(const Vector3& pos, bool killem = true);
    virtual Vector3 PushFromCliffs(const Vector3& pos, const Vector3& oldPos);
    virtual Vector3 PushFromEachOther(const Vector3& _pos);
    virtual int EnterTeleports(int _requiredId = -1); // Searches for valid nearby teleport entrances. Returns which one entered

    virtual void DirectControl(const TeamControls& _teamControls);

    virtual bool IsSelectable();

    static void BeginRenderShadow();
    static void RenderShadow(const Vector3& _pos, double _size);
    static void EndRenderShadow();

    static char* GetTypeName(int _troopType);
    static int GetTypeId(const char* _typeName);
    static Entity* NewEntity(int _troopType);

    static void GetTypeNameTranslated(int _troopType, UnicodeString& _dest);

    virtual bool RayHit(const Vector3& _rayStart, const Vector3& _rayDir);

    virtual Vector3 GetCameraFocusPoint(); // used in unit tracking to determine the position the camera should look at
    virtual void FollowRoute();

    char* LogState(char* _message = nullptr) override;

    virtual void SetShape(const char* _shapeName, bool _colourAll = false);
    // If you want your shape to be coloured by team, SetShape must be called; It should only be called from the Begin() function, not the class constructor

    static void ConvertShapeColoursToTeam(Shape* _shape, int _teamId, bool _convertAll = false);
    static void ConvertFragmentColours(ShapeFragment* _frag, RGBAColour _col, bool _convertAll = false);
    void SetupTeamShape(const char* _name, bool _convertAll = false);

    static bool EntityIsBlocked(int _entityId);
};

// ****************************************************************************
//  Class EntityBlueprint
// ****************************************************************************

class EntityBlueprint
{
  protected:
    static char* m_names[Entity::NumEntityTypes];
    static double m_stats[Entity::NumEntityTypes][Entity::NumStats];

  public:
    static void Initialise();

    static char* GetName(unsigned char _type);
    static double GetStat(unsigned char _type, int _stat);
};

#endif
