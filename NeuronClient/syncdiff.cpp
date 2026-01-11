#include "pch.h"
#include "syncdiff.h"
#include "string_utils.h"

SyncDiff::SyncDiff(const WorldObjectId& _id, const Vector3& _pos, const RGBAColour& _colour, const char* _name, const char* _description)
  : m_id(_id),
    m_pos(_pos),
    m_name(newStr(_name)),
    m_description(newStr(_description)),
    m_colour(_colour) {}

SyncDiff::~SyncDiff()
{
  delete[] m_description;
  delete[] m_name;
}

void SyncDiff::Print(std::ostream& _o)
{
  _o << "syncdiff: " << "id: (team: " << static_cast<int>(m_id.GetTeamId()) << ", unit: " << m_id.GetUnitId() << ", index: " << m_id.
    GetIndex() << ", uniqueId: " << m_id.GetUniqueId() << ") " << "(" << m_pos.x << ", " << m_pos.y << ", " << m_pos.z << ") " << " - " <<
    m_name << " - " << m_description << "\n";
}
