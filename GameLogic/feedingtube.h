#ifndef _included_feedingtube_h
#define _included_feedingtube_h

class FeedingTube : public Building
{
  protected:
    ShapeMarker* m_focusMarker;

    int m_receiverId;
    double m_range;
    double m_signal;

    Vector3 GetDishPos(double _predictionTime); // Returns the position of the transmission point
    Vector3 GetDishFront(double _predictionTime); // Returns the front vector of the dish
    Vector3 GetForwardsClippingDir(double _predictionTime, FeedingTube* _sender); // Returns a good clipping direction for signal

    void RenderSignal(double _predictionTime, double _radius, double _alpha);

  public:
    FeedingTube();

    void Initialise(Building* _template) override;
    void Read(TextReader* _in, bool _dynamic) override;

    bool Advance() override;
    void Render(double _predictionTime) override;
    void RenderAlphas(double _predictionTime) override;

    Vector3 GetStartPoint();
    Vector3 GetEndPoint();

    int GetBuildingLink() override;
    void SetBuildingLink(int _buildingId) override;

    bool DoesSphereHit(const Vector3& _pos, double _radius) override;

    bool IsInView() override;
};

#endif
