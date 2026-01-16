#ifndef _included_triffid_h
#define _included_triffid_h

#include "building.h"

class Triffid : public Building
{
  protected:
    ShapeMarker* m_launchPoint;
    ShapeMarker* m_stem;
    double m_timerSync;
    double m_damage;
    bool m_triggered;
    double m_triggerTimer;
    bool m_renderDamaged;

    int m_numEggs;

  public:
    enum
    {
      SpawnVirii,
      SpawnCentipede,
      SpawnSpider,
      SpawnSpirits,
      SpawnEggs,
      SpawnTriffidEggs,
      SpawnAnts,
      SpawnSporeGenerator,
      SpawnSoulDestroyer,
      SpawnDarwinians,
      NumSpawnTypes
    };

    bool m_spawn[NumSpawnTypes];
    double m_size;
    double m_reloadTime;
    double m_pitch;
    double m_force;
    double m_variance; // Horizontal 

    int m_useTrigger; // Num enemies required to trigger
    Vector3 m_triggerLocation; // Offset from m_pos
    double m_triggerRadius;

    Matrix34 GetHead(double _predictionTime); // So to speak

    Triffid();

    void Initialise(Building* _template) override;
    bool Advance() override;
    void Launch();
    void Render(double _predictionTime) override;

    void Damage(double _damage) override;

    bool DoesRayHit(const Vector3& _rayStart, const Vector3& _rayDir, double _rayLen = 1e10, Vector3* _pos = nullptr,
                    Vector3* _norm = nullptr) override;

    static char* GetSpawnName(int _spawnType);
    static void GetSpawnNameTranslated(int _spawnType, UnicodeString& _dest);

    void Read(TextReader* _in, bool _dynamic) override;
};

// Triffid Egg

#define TRIFFIDEGG_BOUNCEFRICTION 0.65

class TriffidEgg : public Entity
{
  protected:
    Vector3 m_up;
    double m_force;
    double m_timerSync;
    double m_life;

  public:
    double m_size;
    int m_spawnType;
    Vector3 m_spawnPoint;
    double m_spawnRange;

    TriffidEgg();
    void Begin() override;

    bool ChangeHealth(int _amount, int _damageType) override;
    void Spawn();
    bool Advance(Unit* _unit) override;
    void Render(double _predictionTime) override;

    void ResetTimer();
    void SetTimer(double _time);
};

#endif
