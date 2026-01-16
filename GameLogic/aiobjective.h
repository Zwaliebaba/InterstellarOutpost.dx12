#ifndef _included_aiobjective_h
#define _included_aiobjective_h

#include "building.h"

/*
An AI Objective is used by the AI to track progress through assault levels
They follow the following rules:
*   An AIObjective may have 1 or more AIObjectiveMarkers
    AIObjectiveMarkers mark positions that the AI will send Darwinians to accomplish that part of the objective
    The position of the AIObjective itself doesnt matter
    Once all the AIObjectiveMarkers connected to an AIObjective have been captured by a single team (or group in coop games), the objective will be complete, and the next one will be activate
    The AIObjective should be linked to the next AIObjective building
    if an AIObjective isnt linked to by another AIObjective, it will become the starting objective for the AI. If there is more than one in this state, one will be selected randomly, allowing for multiple paths across a map
    The only AIObjective that shouldnt be linked to another is the final one, which should have objective markers on each of the Control Stations attached to the PulseBomb for that map
Gary
*/
class AIObjective : public Building
{
  public:
    int m_nextObjective; // the objective which becomes the target once this one has been captured
    LList<int> m_objectiveMarkers;

    bool m_active;
    bool m_defenseObjective;
    double m_timer;

    int m_armourMarker;

    static bool s_objectivesInitialised;

    AIObjective();
    void Initialise(Building* _template) override;

    bool Advance() override;

    void AdvanceStandard();
    void AdvanceDefensive();

    void RegisterObjectiveMarker(int _markerId, bool _armourMarker = false);

    void SetBuildingLink(int _buildingId) override;
    int GetBuildingLink() override;

    void Read(TextReader* _in, bool _dynamic) override;

    static void InitialiseObjectives();

    bool DoesSphereHit(const Vector3& _pos, double _radius) override;
    bool DoesShapeHit(Shape* _shape, Matrix34 _transform) override;
    bool DoesRayHit(const Vector3& _rayStart, const Vector3& _rayDir, double _rayLen = 1e10, Vector3* _pos = nullptr,
                    Vector3* _norm = nullptr) override;
};

class AIObjectiveMarker : public Building
{
  public:
    double m_scanRange;
    double m_timer;

    bool m_registered;
    bool m_pickupAvailable;
    bool m_defenseMarker;

    int m_objectiveId;
    int m_armourObjective;
    int m_pickupOnly;
    int m_objectiveBuildingId;
    int m_defaultTeam;

    AIObjectiveMarker();
    void Initialise(Building* _template) override;

    bool Advance() override;
    void AdvanceStandard();
    void AdvanceDefensive();
    void AdvanceRocketRiot();

    void SetBuildingLink(int _buildingId) override;
    int GetBuildingLink() override;

    void Read(TextReader* _in, bool _dynamic) override;

    bool DoesSphereHit(const Vector3& _pos, double _radius) override;
    bool DoesShapeHit(Shape* _shape, Matrix34 _transform) override;
    bool DoesRayHit(const Vector3& _rayStart, const Vector3& _rayDir, double _rayLen = 1e10, Vector3* _pos = nullptr,
                    Vector3* _norm = nullptr) override;
};

#endif
