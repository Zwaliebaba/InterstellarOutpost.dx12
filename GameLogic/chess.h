#ifndef _included_chess_h
#define _included_chess_h

#include "building.h"
#include "carryablebuilding.h"

class ChessBase : public Building
{
  public:
    double m_width;
    double m_depth;
    double m_sparePoints;
    double m_timer;

    ChessBase();

    void Initialise(Building* _template) override;

    void SpawnPiece(double _scale);

    bool Advance() override;
    void Render(double _precictionTime) override;

    void Read(TextReader* _in, bool _dynamic) override;
};

class ChessPiece : public CarryableBuilding
{
  public:
    int m_chessBaseId;
    double m_damage;

    ChessPiece();

    void Initialise(Building* _template) override;

    bool Advance() override;

    void ExplodeShape(double _fraction);

    void SetWaypoint(const Vector3& _waypoint);
    void HandleCollision(double _force) override;
    void HandleSuccess(); // we arrived at an enemy's chess base
};

#endif
