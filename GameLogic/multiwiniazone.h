#ifndef _included_multiwiniazone_h
#define _included_multiwiniazone_h

#include "building.h"

class AITarget;
class Flag;

#define BLITZKRIEG_FLAGCAPTURE_RANGE 20.0f

class MultiwiniaZone : public Building
{
  public:
    double m_size;
    double m_life; // -1 = live forever.  > 0 = life remaining until delete

    double m_recountTimer;

    int m_totalCount;
    int m_teamCount[NUM_TEAMS];

    Flag* m_blitzkriegFlag;
    AITarget* m_aiTarget;

    LList<int> m_blitzkriegLinks;

    double m_blitzkriegOwnership;
    int m_blitzkriegCaptureTeam;
    int m_blitzkriegUpOrDown;
    bool m_blitzkriegLocked;

    double m_blitzkriegUnderAttackMessageTimer;

    static int s_blitzkriegBaseZone[NUM_TEAMS];

  protected:
    void Advance_KingOfTheHill();
    void Advance_CaptureTheStatue();
    void Advance_Blitzkrieg();

    void RenderBlitzkrieg();

    static double GetBlitzkriegPriority(int _zoneId, int _teamId);

  public:
    MultiwiniaZone();
    ~MultiwiniaZone() override;

    void Initialise(Building* _template) override;
    void SetDetail(int _detail) override;

    bool Advance() override;
    void Render(double predictionTime) override;
    void RenderAlphas(double predictionTime) override;
    void RenderZoneEdge(double startAngle, double totalAngle, RGBAColour colour, bool animated);

    bool DoesSphereHit(const Vector3& _pos, double _radius) override;
    bool DoesShapeHit(Shape* _shape, Matrix34 _transform) override;
    bool DoesRayHit(const Vector3& _rayStart, const Vector3& _rayDir, double _rayLen = 1e10, Vector3* _pos = nullptr,
                    Vector3* _norm = nullptr) override;

    int GetBuildingLink() override; // Allows a building to link to another
    void SetBuildingLink(int _buildingId) override; // eg control towers

    bool IsBlitzkriegBaseZone();
    bool IsBlitzkriegZoneLocked();
    int GetBaseTeamId();

    void Read(TextReader* _in, bool _dynamic) override;

    static int GetNumZones(int _teamId);
};

#endif
