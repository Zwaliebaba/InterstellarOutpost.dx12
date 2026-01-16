#ifndef _included_radardish_h
#define _included_radardish_h

#define RADARDISH_TRANSPORTPERIOD    0.1                        // Minimum wait time between sends
#define RADARDISH_TRANSPORTSPEED     50.0                       // Speed of in-transit entities (m/s)

#include "ai.h"
#include "teleport.h"

class RadarDish : public Teleport
{
  protected:
    ShapeFragment* m_dish;
    ShapeFragment* m_upperMount;
    ShapeMarker* m_focusMarker;

    Vector3 m_entrancePos;
    Vector3 m_entranceFront;

    Vector3 m_target;
    int m_receiverId;
    double m_range;
    double m_signal;

    bool m_newlyCreated;

    bool m_horizontallyAligned;
    bool m_verticallyAligned;
    bool m_movementSoundsPlaying;

    int m_oldTeamId;

    DArray<int> m_validLinkList;

    void RenderSignal(double _predictionTime, double _radius, double _alpha);

  public:
    AITarget* m_aiTarget;
    int m_forceConnection;
    int m_autoConnectAtStart;
    bool m_forceTeamMatch;

    RadarDish();
    ~RadarDish() override;

    void SetDetail(int _detail) override;
    void Initialise(Building* _template) override;

    bool Advance() override;
    void Render(double _predictionTime) override;
    void RenderAlphas(double _predictionTime) override;

    void Aim(Vector3 _worldPos);

    bool Connected() override;
    bool ReadyToSend() override;

    int GetConnectedDishId();

    Vector3 GetStartPoint() override;
    Vector3 GetEndPoint() override;
    bool GetEntrance(Vector3& _pos, Vector3& _front) override;
    bool GetExit(Vector3& _pos, Vector3& _front) override;

    Vector3 GetDishFront(double _predictionTime); // Returns the front vector of the dish
    Vector3 GetDishPos(double _predictionTime); // Returns the position of the transmission point

    bool DoesSphereHit(const Vector3& _pos, double _radius) override;
    bool DoesRayHit(const Vector3& rayStart, const Vector3& rayDir, float rayLen = 1e10, Vector3* pos = nullptr, Vector3* norm = nullptr);

    bool UpdateEntityInTransit(Entity* _entity) override;

    void AdvanceAITarget();

    void Read(TextReader* _in, bool _dynamic) override;

    void SetBuildingLink(int _buildingId) override;
    void ClearLinks();
    int CountValidLinks();
    void AddValidLink(int _id);
    bool ValidReceiverDish(int _buildingId);
    bool NotAligned();
};

#endif
