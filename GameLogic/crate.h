#ifndef _included_crate_h
#define _included_crate_h

#include "building.h"
#include "worldobject.h"
#include "unicode_string.h"

#define CRATE_START_HEIGHT 1300.0
#define CRATE_SCAN_RANGE 35.0

#ifdef TARGET_DEBUG
#define INCLUDE_THEBEAST
#endif

class AITarget;

class Crate : public Building
{
  protected:
    double m_captureTimer;
    double m_life; // ticks down if the crate isnt being captured
    double m_bubbleTimer;

  public:
    static int s_crates; // number of crates currently in the level
    static bool s_spaceShipUsed;

    enum
    {
      CrateRewardSquad,
      // gives the player a squad task to run at their leisure
      CrateRewardHarvester,
      // gives the player a harvester task
      CrateRewardEngineer,
      CrateRewardArmour,
      // gives the player an armour task
      CrateRewardAirstrike,
      // spawns a number of airstrike markers around the crates location
      CrateRewardNuke,
      // Launches a nuke at the target area
      CrateRewardDarwinians,
      // Spawn a bunch of Darwinians
      CrateRewardBooster,
      // All nearby Darwinians become immune to laser fire, and attack more often
      CrateRewardSubversion,
      // Gives all nearby Darwinians the subversion ability
      CrateRewardHotFeet,
      // Doubles the speed of all Darwinians in range
      CrateRewardGunTurret,
      // Gives the player a placable gun turret
      CrateRewardSpam,
      // spawns a Spam object at the crates location
      CrateRewardForest,
      // Spawns 1 - 3 trees around the crates area
      CrateRewardRandomiser,
      // Randomises the team of all Darwinians within range
      CrateRewardAntNest,
      // Creates an Ant nest
      CrateRewardPlague,
      // Infects all nearby Darwinians with the plague
      CrateRewardEggs,
      // Spawns between 1 and 3 eggs filled with a random monster
      CrateRewardMeteor,
      // Spawns a number of meteors in the sky which hit the ground causing explosions
      CrateRewardDarkForest,
      // Creates an evil forest that kills any darwinians inside, turning them into ghosts
      CrateRewardCrateMania,
      // Loads of crates are spawned  
      CrateRewardSpawnMania,
      // causes all active spawn points to spawn constantly for 15 seconds
      CrateRewardBitzkreig,
      // All Darwinians will always throw grenades for 10 seconds
      //CrateRewardFormationIncrease,   // Max size of the teams formations increases
      CrateRewardRocketDarwinians,
      // all darwinians replace their grenades with rockets
      //CrateRewardExtinguisher,    // Puts out any nearby burning darwinians and trees
      CrateRewardRage,
      // greatly increases the rate at which Darwinians attack for 60 seconds
      //CrateRewardTank,        // Gives the player a bitchin tank for 30 seconds
      CrateRewardRocketTurret,
      // Gives the player a rocket turret to play
      CrateRewardFlameTurret,
      // gives the player a flame turret to place
      CrateRewardSpaceShip,
      // a futurewinian space ship appears and starts abducting darwinians
      CrateRewardTriffids,
      // spawn 1 - 3 triffids on the spot and make them fire random eggs
      CrateRewardNapalmStrike,
      // summons airstrike units which drop naplam. lots and lots of napalm
      CrateRewardSlowDownTime,
      CrateRewardSpeedUpTime,
#ifdef INCLUDE_THEBEAST
      CrateRewardTheBeast,
#endif
      NumCrateRewards
    };

    AITarget* m_aiTarget;

    int m_rewardId; // used if the crate has a fixed reward, otherwise, random
    double m_percentCapturedSmooth; // for smoothing only

  protected:
    static void GenerateTrees(Vector3 _pos);
    static void GenerateEvilTrees(Vector3 _pos);

  public:
    Crate();
    ~Crate() override;

    void Initialise(Building* _template) override;
    void SetDetail(int _detail) override;

    bool Advance() override;
    void AdvanceOnGround();
    void AdvanceFloating();

    Vector3 PushFromObstructions(Vector3 _pos);

    void Render(double _predictionTime) override;
    void RenderAlphas(double _predictionTime) override;
    void RenderAlphasOnGround(double _predictionTime);
    void RenderAlphasFloating(double _predictionTime);
    void RenderBubble(double _predictionTime);
    void RenderGroundMarker(double _predictionTime);
    void RenderGlow(double _predictionTime);

    void CaptureCrate();
    void RunUncapturedCrate(int _teamId);
    static void CaptureCrate(int _crateId, int _teamId);

    static bool IsBasicCrate(int _type);
    static bool IsAdvancedCrate(int _type);
    static bool IsValid(int _type);

    bool IsInView() override;

    static void TriggerSoundEvent(int _crateType, const Vector3& pos, int _teamId);

    bool DoesSphereHit(const Vector3& _pos, double _radius) override;
    bool DoesShapeHit(Shape* _shape, Matrix34 _transform) override;
    bool DoesRayHit(const Vector3& _rayStart, const Vector3& _rayDir, double _rayLen, Vector3* _pos, Vector3* norm) override;

    int RecalculateOwnership(int& _num);

    double GetPercentCaptured(); // returns 0 - 1

    char* LogState(char* _message) override;

    static char* GetName(int _type);
    static void GetNameTranslated(int _type, UnicodeString& _dest);

    void ListSoundEvents(LList<char*>* _list) override;
    void StopCaptureSounds();

    static WorldObjectId RunAirStrike(const Vector3& _pos, int _teamId, bool _special = false);
    static WorldObjectId RunGunTurret(const Vector3& _pos, int _teamId, int _turretType);
    static void RunNuke(const Vector3& _pos, int _teamId);
    static void RunDarwinians(const Vector3& _pos, int _teamId);
    static void RunSubversion(const Vector3& _pos, int _teamId, int _taskId);
    static void RunBooster(const Vector3& _pos, int _teamId, int _taskId);
    static void RunHotFeet(const Vector3& _pos, int _teamId, int _taskId);
    static void RunInfection(const Vector3& _pos, int _teamId);
    static void RunMagicalForest(const Vector3& _pos, int _teamId);
    static void RunDarkForest(const Vector3& _pos, int _teamId);
    static void RunAntNest(const Vector3& _pos, int _teamId);
    static void RunPlague(const Vector3& _pos, int _teamId);
    static void RunEggs(const Vector3& _pos, int _teamId);
    static void RunMeteorShower(const Vector3& _pos, int _teamId);
    static void RunRandomiser(const Vector3& _pos, int _teamId);
    static void RunRage(const Vector3& _pos, int _teamId, int _taskId);
    static void RunSpawnMania();
    static void RunBitzkreig();
    static void RunCrateMania();
    static void RunRocketDarwinians(int _teamId);
    static void RunSpaceShip();
    static void RunTriffids(const Vector3& _pos, int _teamId);
    static void RunSlowDownTime();
    static void RunSpeedUpTime();
    static void RunTheBeast(const Vector3& _pos);
};

#endif
