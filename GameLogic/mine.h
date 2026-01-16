#ifndef _included_mine_h
#define _included_mine_h

#include "building.h"

class Refinery;
class MineCart;

// ****************************************************************************
// Class MineBuilding
// ****************************************************************************

class MineBuilding : public Building
{
  protected:
    int m_trackLink;
    ShapeMarker* m_trackMarker1;
    ShapeMarker* m_trackMarker2;

    Matrix34 m_trackMatrix1;
    Matrix34 m_trackMatrix2;

    LList<MineCart*> m_carts;

    double m_previousMineSpeed;
    double m_wheelRotate;

    static Shape* s_wheelShape;
    static Shape* s_cartShape;
    static ShapeMarker* s_cartMarker1;
    static ShapeMarker* s_cartMarker2;
    static ShapeMarker* s_cartContents[3];
    static Shape* s_polygon1;
    static Shape* s_primitive1;

    static double s_refineryPopulation;
    static double s_refineryRecalculateTimer;
    static double RefinerySpeed();

  public:
    MineBuilding();

    void Initialise(Building* _template) override;
    bool Advance() override;

    bool IsInView() override;

    void Render(double _predictionTime) override;
    void RenderAlphas(double _predictionTime) override;
    void RenderCart(MineCart* _cart, double _predictionTime);

    Vector3 GetTrackMarker1();
    Vector3 GetTrackMarker2();

    virtual void TriggerCart(MineCart* _cart, double _initValue);

    void Read(TextReader* _in, bool _dynamic) override;

    int GetBuildingLink() override;
    void SetBuildingLink(int _buildingId) override;

};

class MineCart
{
  public:
    double m_progress; // Progress down current line, 0.0 - 1.0

    bool m_polygons[3];
    bool m_primitives[3];

    MineCart();
};

// ****************************************************************************
// Class TrackLink
// ****************************************************************************

class TrackLink : public MineBuilding
{
  public:
    TrackLink();

    bool Advance() override;
};

// ****************************************************************************
// Class TrackJunction
// ****************************************************************************

class TrackJunction : public MineBuilding
{
  public:
    LList<int> m_trackLinks;

    TrackJunction();

    void Initialise(Building* _template) override;

    void TriggerCart(MineCart* _cart, double _initValue) override;

    void SetBuildingLink(int _buildingId) override;

    void Read(TextReader* _in, bool _dynamic) override;
};

// ****************************************************************************
// Class TrackStart
// ****************************************************************************

class TrackStart : public MineBuilding
{
  public:
    int m_reqBuildingId; // This building must be online

    TrackStart();

    void Initialise(Building* _template) override;
    bool Advance() override;

    void Read(TextReader* _in, bool _dynamic) override;
};

// ****************************************************************************
// Class TrackEnd
// ****************************************************************************

class TrackEnd : public MineBuilding
{
  public:
    int m_reqBuildingId; // This building must be online

    TrackEnd();

    void Initialise(Building* _template) override;
    bool Advance() override;

    void Read(TextReader* _in, bool _dynamic) override;
};

// ****************************************************************************
// Class Refinery
// ****************************************************************************

class Refinery : public MineBuilding
{
  protected:
    ShapeMarker* m_wheel1;
    ShapeMarker* m_wheel2;
    ShapeMarker* m_wheel3;
    ShapeMarker* m_counter1;

  public:
    Refinery();

    bool Advance() override;
    void Render(double _predictionTime) override;

    void GetObjectiveCounter(UnicodeString& _dest) override;

    void TriggerCart(MineCart* _cart, double _initValue) override;
};

// ****************************************************************************
// Class Mine
// ****************************************************************************

class Mine : public MineBuilding
{
  protected:
    ShapeMarker* m_wheel1;
    ShapeMarker* m_wheel2;

  public:
    Mine();

    void Render(double _predictionTime) override;

    void TriggerCart(MineCart* _cart, double _initValue) override;
};

#endif
