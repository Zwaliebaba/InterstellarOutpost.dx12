#ifndef _included_scripttrigger_h
#define _included_scripttrigger_h

#include "building.h"

#define SCRIPTRIGGER_RUNALWAYS              999
#define SCRIPTRIGGER_RUNNEVER               998
#define SCRIPTRIGGER_RUNCAMENTER            997
#define SCRIPTRIGGER_RUNCAMVIEW             996

class ScriptTrigger : public Building
{
  public:
    char m_scriptFilename[256];
    double m_range;
    int m_entityType;
    int m_linkId;

  protected:
    int m_triggered;
    double m_timer;

  public:
    ScriptTrigger();

    void Initialise(Building* _template) override;
    bool Advance() override;

    void Trigger();

    bool DoesSphereHit(const Vector3& _pos, double _radius) override;
    bool DoesShapeHit(Shape* _shape, Matrix34 _transform) override;
    bool DoesRayHit(const Vector3& _rayStart, const Vector3& _rayDir, double _rayLen = 1e10, Vector3* _pos = nullptr,
                    Vector3* _norm = nullptr) override;

    int GetBuildingLink() override;
    void SetBuildingLink(int _buildingId) override;

    void Read(TextReader* _in, bool _dynamic) override;
};

#endif
