#ifndef _included_incubator_h
#define _included_incubator_h

#include "fast_darray.h"

#include "building.h"
#include "spirit.h"

class ShapeMarker;

#define INCUBATOR_PROCESSTIME                  5.0
#define INCUBATOR_PROCESSTIME_MULTIPLAYER      0.2

struct IncubatorIncoming
{
  Vector3 m_pos;
  int m_entrance;
  double m_alpha;
};

class Incubator : public Building
{
  protected:
    FastDArray<Spirit> m_spirits;
    ShapeMarker* m_spiritCentre;
    ShapeMarker* m_exit;
    ShapeMarker* m_dock;
    ShapeMarker* m_spiritEntrance[3];

    int m_troopType;
    double m_timer;

    LList<IncubatorIncoming*> m_incoming;

  public:
    int m_numStartingSpirits;

    Incubator();
    ~Incubator() override;

    void Initialise(Building* _template) override;

    bool Advance() override;
    void SpawnEntity();
    void AddSpirit(Spirit* _spirit);

    void RenderAlphas(double _predictionTime) override;

    int NumSpiritsInside();

    void Read(TextReader* _in, bool _dynamic) override;
    void Write(TextWriter* _out) override;

    void GetDockPoint(Vector3& _pos, Vector3& _front);

    void ListSoundEvents(LList<char*>* _list) override;
};

#endif
