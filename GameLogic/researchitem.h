#ifndef _included_researchitem_h
#define _included_researchitem_h

#include "building.h"

class ResearchItem : public Building
{
  protected:
    double m_reprogrammed;
    ShapeMarker* m_end1;
    ShapeMarker* m_end2;

  public:
    int m_researchType; // indexes into GlobalResearch::m_type
    int m_level;
    double m_percentResearchedSmooth;

    ResearchItem();

    void Initialise(Building* _template) override;
    void SetDetail(int _detail) override;

    bool Advance() override;
    void Render(double _predictionTime) override;
    void RenderAlphas(double _predictionTime) override;

    bool NeedsReprogram();
    double GetReprogrammed();
    bool Reprogram();

    void Read(TextReader* _in, bool _dynamic) override;
    void Write(TextWriter* _out) override;

    void GetEndPositions(Vector3& _end1, Vector3& _end2);

    bool DoesSphereHit(const Vector3& _pos, double _radius) override;
    bool DoesShapeHit(Shape* _shape, Matrix34 _transform) override;
    bool DoesRayHit(const Vector3& _rayStart, const Vector3& _rayDir, double _rayLen, Vector3* _pos, Vector3* norm) override;

    bool IsInView() override;

    void ListSoundEvents(LList<char*>* _list) override;
};

#endif
