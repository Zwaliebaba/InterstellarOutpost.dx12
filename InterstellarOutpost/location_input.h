#ifndef INCLUDED_LOCATION_INPUT
#define INCLUDED_LOCATION_INPUT

class LocationInput
{
  void AdvanceRadarDishControl(Building* _building);
  void AdvanceCarryableControl(Building* _building);
  void AdvanceNoSelection();
  void AdvanceTeamControl();

  void IssueDarwinianOrders(const Vector3& pos);

  void SelectObjectUnderMouse(WorldObjectId& objId);

  public:
    int m_routeId;
    bool m_chatToggledThisUpdate;

    LocationInput();

    bool GetObjectUnderMouse(WorldObjectId& _id, int _teamId);

    void Advance();
    void Render();
};

#endif
