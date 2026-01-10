#include "pch.h"
#include "text_file_writer.h"
#include "text_stream_readers.h"
#include "restrictionzone.h"
#include "app.h"
#include "location.h"

// ===============================================================
// The Restriction Zone doesn't do anything itself, although it
// prevents crates from dropping inside their area
// This lets you prevent crates from falling on a players starting
// island, for example
// Additional functionality can be added later (restriction of
// powerups running, for example)
// Gary

LList<int> RestrictionZone::s_restrictionZones;

RestrictionZone::RestrictionZone()
  : Building(),
    m_size(300.0f),
    m_blockPowerups(false) { m_type = TypeRestrictionZone; }

RestrictionZone::~RestrictionZone()
{
  int index = s_restrictionZones.FindData(m_id.GetUniqueId());
  if (index != -1)
    s_restrictionZones.RemoveData(index);
}

void RestrictionZone::Initialise(Building* _template)
{
  Building::Initialise(_template);
  m_size = static_cast<RestrictionZone*>(_template)->m_size;
  m_blockPowerups = static_cast<RestrictionZone*>(_template)->m_blockPowerups;

  s_restrictionZones.PutData(m_id.GetUniqueId());
}

bool RestrictionZone::Advance() { return false; }

void RestrictionZone::RenderAlphas(double _predictionTime) {}

void RestrictionZone::Read(TextReader* _in, bool _dynamic)
{
  Building::Read(_in, _dynamic);
  m_size = atof(_in->GetNextToken());
  if (_in->TokenAvailable())
    m_blockPowerups = atoi(_in->GetNextToken());
}

void RestrictionZone::Write(TextWriter* _out)
{
  Building::Write(_out);
  _out->printf("%-2.2f", m_size);
  _out->printf("%5d", m_blockPowerups);
}

bool RestrictionZone::IsRestricted(Vector3 _pos, bool _powerups)
{
  Vector3 pos = _pos;
  for (int i = 0; i < s_restrictionZones.Size(); ++i)
  {
    auto rz = static_cast<RestrictionZone*>(g_app->m_location->GetBuilding(s_restrictionZones[i]));
    if (rz && rz->m_type == TypeRestrictionZone)
    {
      pos.y = rz->m_pos.y;
      if ((rz->m_pos - pos).Mag() < rz->m_size)
      {
        if (!_powerups || rz->m_blockPowerups == 1)
          return true;
      }
    }
  }

  return false;
}

bool RestrictionZone::DoesSphereHit(const Vector3& _pos, double _radius) { return false; }

bool RestrictionZone::DoesShapeHit(Shape* _shape, Matrix34 _transform) { return false; }

bool RestrictionZone::DoesRayHit(const Vector3& _rayStart, const Vector3& _rayDir, double _rayLen, Vector3* _pos, Vector3* _norm)
{
  if (g_app->m_editing)
    return Building::DoesRayHit(_rayStart, _rayDir, _rayLen, _pos, _norm);
  return false;
}
