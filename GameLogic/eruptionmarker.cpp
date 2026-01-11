#include "pch.h"

#include "random_number.h"
#include "particle_system.h"
#include "app.h"
#include "main.h"
#include "eruptionmarker.h"

EruptionMarker::EruptionMarker()
:   Building(),
    m_erupting(false),
    m_timer(0.0)
{
    m_type = Building::TypeEruptionMarker;
}

bool EruptionMarker::Advance()
{
    if( m_erupting )
    {
        m_timer -= SERVER_ADVANCE_PERIOD;
        if( m_timer <= 0.0 )
        {
            m_erupting = false;
            m_timer = 0.0;
        }

        for( int i = 0; i < 10; ++i )
        {
            Vector3 vel = g_upVector;
            vel.x += sfrand();
            vel.z += sfrand();
            vel.SetLength( 100.0 );

            double size = 300.0 + frand(200.0);

            g_app->m_particleSystem->CreateParticle( m_pos, vel, Particle::TypeVolcano, size );
        }
    }
    else
    {
        if( syncrand() % 1000 == 0 )
        {
            m_erupting = true;
            m_timer = 3.0 + syncfrand(4.0);
        }
    }

    return false;
}

bool EruptionMarker::DoesSphereHit(Vector3 const &_pos, double _radius)
{
    return false;
}


bool EruptionMarker::DoesShapeHit(Shape *_shape, Matrix34 _transform)
{
    return false;
}


bool EruptionMarker::DoesRayHit(Vector3 const &_rayStart, Vector3 const &_rayDir, 
                                double _rayLen, Vector3 *_pos, Vector3 *_norm)
{
        return false;
}
