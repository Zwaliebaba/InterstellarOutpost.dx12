#ifndef _included_weapons_h
#define _included_weapons_h

#include "entity.h"
#include "worldobject.h"

// ****************************************************************************
//  Class ThrowableWeapon
// ****************************************************************************

class ThrowableWeapon : public WorldObject
{
  protected:
    Shape* m_shape;
    double m_birthTime;
    double m_force;
    int m_numFlashes;

    Vector3 m_front;
    Vector3 m_up;

    RGBAColour m_colour;

    void TriggerSoundEvent(char* _event);

  public:
    ThrowableWeapon(int _type, const Vector3& _startPos, const Vector3& _front, double _force);

    void Initialise();
    bool Advance() override;
    void Render(double _predictionTime) override;

    void RemoveForce();
    void ResetForce();

    static double GetMaxForce(int _researchLevel);
    static double GetApproxMaxRange(double _maxForce);
};

// ****************************************************************************
//  Class Grenade
// ****************************************************************************

class Grenade : public ThrowableWeapon
{
  public:
    double m_life;
    double m_power;

    bool m_explodeOnImpact; // this grenade will not time out, but will explode as soon as it hits the ground
    bool m_collected; // the grenade has been sucked up by a harvester

    Grenade(const Vector3& _startPos, const Vector3& _front, double _force);
    bool Advance() override;
};

// ****************************************************************************
//  Class AirStrikeMarker
// ****************************************************************************

class AirStrikeMarker : public ThrowableWeapon
{
  public:
    WorldObjectId m_airstrikeUnit;
    bool m_naplamBombers;

    AirStrikeMarker(const Vector3& _startPos, const Vector3& _front, double _force);
    bool Advance() override;
};

// ****************************************************************************
//  Class ControllerGrenade
// ****************************************************************************

class ControllerGrenade : public ThrowableWeapon
{
  public:
    int m_squadTaskId;

    ControllerGrenade(const Vector3& _startPos, const Vector3& _front, double _force);
    bool Advance() override;
};

// ****************************************************************************
//  Class Rocket
// ****************************************************************************

class Rocket : public WorldObject
{
  public:
    unsigned char m_fromTeamId;

    Shape* m_shape;
    double m_timer;
    double m_lifeTime;
    int m_fromBuildingId;
    bool m_firework;

    Vector3 m_target;

    Rocket() {}
    Rocket(Vector3 _startPos, Vector3 _targetPos);

    void Initialise();
    bool Advance() override;
    void Render(double predictionTime) override;

    void Explode();
};

// ****************************************************************************
//  Class Laser
// ****************************************************************************

class Laser : public WorldObject
{
  public:
    unsigned char m_fromTeamId;
    bool m_mindControl;

  protected:
    double m_life;
    bool m_harmless; // becomes true after hitting someone
    bool m_bounced;

  public:
    Laser() {}

    virtual void Initialise(double _lifeTime);
    bool Advance() override;
    void Render(double predictionTime) override;
};

// ****************************************************************************
//  Class TurretShell
// ****************************************************************************

class TurretShell : public WorldObject
{
  protected:
    double m_age;
    double m_life;
    bool m_stopRendering;

  public:
    int m_fromBuildingId;

    TurretShell(double _life);

    bool Advance() override;
    void Render(double predictionTime) override;

    void ExplodeShape(float fraction);
};

// ****************************************************************************
//  Class Shockwave
// ****************************************************************************

class Shockwave : public WorldObject
{
  public:
    Shape* m_shape;
    int m_teamId;
    double m_size;
    double m_life;
    int m_fromBuildingId;

    Shockwave(int _teamId, double _size);

    bool Advance() override;
    void Render(double predictionTime) override;
};

// ****************************************************************************
//  Class MuzzleFlash
// ****************************************************************************

class MuzzleFlash : public WorldObject
{
  public:
    Vector3 m_front;
    double m_size;
    double m_life;

    MuzzleFlash();
    MuzzleFlash(const Vector3& _pos, const Vector3& _front, double _size, double _life);

    bool Advance() override;
    void Render(double _predictionTime) override;
};

// ****************************************************************************
//  Class Missile
// ****************************************************************************

class Missile : public WorldObject
{
  protected:
    double m_life;
    LList<Vector3> m_history;
    Shape* m_shape;
    ShapeMarker* m_booster;
    MuzzleFlash m_fire;

  public:
    WorldObjectId m_tankId; // Who fired me
    Vector3 m_front;
    Vector3 m_up;
    Vector3 m_target;

    Missile();

    bool Advance() override;
    bool AdvanceToTargetPosition(const Vector3& _pos);
    void Explode();
    void Render(double _predictionTime) override;
};

class Fireball : public WorldObject
{
  public:
    double m_life;
    double m_radius;
    bool m_createdParticles;

    int m_fromBuildingId;

    LList<int> m_damaged; // maintains a list of damaged darwinians to make sure it doesnt cause damage more than once

    Fireball();
    bool Advance() override;
    bool Damaged(int _darwinianId);
};

class PulseWave : public WorldObject
{
  public:
    static PulseWave* s_pulseWave;

    double m_radius;
    double m_birthTime;

    PulseWave();
    ~PulseWave() override;
    bool Advance() override;
    void Render(double _predictionTime) override;
};

class LandMine : public WorldObject
{
  public:
    Shape* m_shape;
    Vector3 m_front;
    Vector3 m_up;

    LandMine();
    bool Advance() override;
    void Render(double _predictionTime) override;
};

class FlameGrenade : public Grenade
{
  public:
    FlameGrenade(const Vector3& _startPos, const Vector3& _front, double _force);
    bool Advance() override;

};

class TurretEmptyShell : public WorldObject
{
  public:
    Shape* m_shape;
    Vector3 m_front;
    Vector3 m_up;
    double m_force;

    TurretEmptyShell(const Vector3& pos, const Vector3& front, const Vector3& vel);

    bool Advance() override;
    void Render(double _predictionTime) override;
};

#endif
