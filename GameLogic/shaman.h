
#ifndef _included_shaman_h
#define _included_shaman_h

#include "entity.h"

struct ShamanTargetArea
{
    Vector3 m_pos;
    double   m_radius;
};

class Shaman : public Entity
{
protected:
	Vector3 m_wayPoint;
	int		m_teleportId;

	bool	m_sacraficeDarwinians;
	double	m_sacraficeTimer;

  int     m_summonType;

    int     m_mode;
    double   m_lastModeChange;

public:

    enum
    {
        ModeNone,
        ModeCreateDarwinians,
        ModeFollow,
        NumModes
    };

    int     m_sacrafices;

    bool    m_vunerable;
    bool    m_paralyzed;        // the final summon is being cast on him, so he can no longer act

    bool    m_renderTeleportTargets;

public:
	Shaman();
	~Shaman();

	void Begin();

	bool Advance					( Unit *_unit );
	bool AdvanceToTargetPosition	();

  void Render						( double predictionTime );
    void RenderTeleportTargets      ();

    bool ChangeHealth               ( int _amount, int _damageType = DamageTypeUnresistable );

	void SetWaypoint				( Vector3 const &_wayPoint );

	void BeginSummoning				( int _type );
	void SummonEntity				( int _type );

  bool IsSacraficing				();

    bool CallingDarwinians          ();

	int GetSummonType				();
    int GetNearestPortal            ();
    void Paralyze                   ();

    void ChangeMode                 ( Vector3 _mousePos );
    void TeleportToPos              ( Vector3 _mousePos );
    bool ValidTeleportPos           ( Vector3 _pos );
    LList<ShamanTargetArea> *GetTargetArea();

    bool IsSelectable               ();
	
	bool		    CanSummonEntity( int _entityType );
    bool            CanCapturePortal();

    static bool     IsSummonable ( int _entityType );
};

#endif