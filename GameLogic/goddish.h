
#ifndef _included_goddish_h
#define _included_goddish_h

#include "building.h"


class GodDish : public Building
{
public:
    bool    m_activated;
    double   m_timer;

  public:
    GodDish();

    void Initialise( Building *_template );

    bool Advance        ();
    void Render         ( double _predictionTime );
    void RenderAlphas   ( double _predictionTime );

    bool IsInView       ();

    void Activate();
    void DeActivate();
    void SpawnSpam( bool _isResearch );

    void TriggerSpam();

    void ListSoundEvents( LList<char *> *_list );
};


#endif