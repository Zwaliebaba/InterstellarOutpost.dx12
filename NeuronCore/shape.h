#ifndef INCLUDED_SHAPE_H
#define INCLUDED_SHAPE_H

#include "llist.h"
#include "matrix34.h"
#include "rgb_colour.h"
#include "vector3.h"

class TextReader;
class ShapeFragment;
class Matrix34;
class Shape;

// ****************
// Class RayPackage
// ****************

class RayPackage
{
  public:
    Vector3 m_rayStart;
    Vector3 m_rayDir;
    double m_rayLen;

    RayPackage(const Vector3& _start, const Vector3& _dir, double _length = 1e10)
      : m_rayStart(_start),
        m_rayDir(_dir)
    {
      m_rayLen = _length; }
};

// *******************
// Class SpherePackage
// *******************

class SpherePackage
{
  public:
    Vector3 m_pos;
    double m_radius;

    SpherePackage(const Vector3& _pos, double _radius)
      : m_pos(_pos),
        m_radius(_radius) {}
};

// *****************
// Class ShapeMarker
// *****************

class ShapeMarker
{
  public:
    Matrix34 m_transform;
    char* m_name;
    char* m_parentName;
    int m_depth; // Number of levels in the shape fragment tree from root to self
    ShapeFragment** m_parents; // An array with m_depth elements

    ShapeMarker(const char* _name, char* _parentName, int _depth, const Matrix34& _transform);
    ShapeMarker(TextReader* _in, const char* _name);
    ~ShapeMarker();

    Matrix34 GetWorldMatrix(const Matrix34& _rootTransform);

    void WriteToFile(FILE* _out) const;
};

// ******************
// Class VertexPosCol
// ******************

class VertexPosCol
{
  public:
    unsigned short m_posId;
    unsigned short m_colId;
};

// *******************
// Class ShapeTriangle
// *******************

class ShapeTriangle
{
  public:
    unsigned short v1, v2, v3; // Vertex indices (into ShapeFragment::m_vertices)
};

// *******************
// Class ShapeFragment
// *******************

class ShapeFragment
{
  friend class ShapeExporter;

  protected:
    char* m_displayListName;

    void ParsePositionBlock(TextReader* in, unsigned int numPositions);
    void ParseNormalBlock(TextReader* in, unsigned int numNorms);
    void ParseColourBlock(TextReader* in, unsigned int numColours);
    void ParseVertexBlock(TextReader* in, unsigned int numVerts);
    void ParseStripBlock(TextReader* in);
    void ParseAllStripBlocks(TextReader* in, unsigned int numStrips);
    void ParseTriangleBlock(TextReader* in, unsigned int numTriangles);

    void GenerateNormals();

  public:
    unsigned int m_numPositions;
    Vector3* m_positions;
    Vector3* m_positionsInWS; // Temp storage space used to cache World Space versions of all the vertex positions in the hit check routines
    unsigned int m_numNormals;
    Vector3* m_normals;
    unsigned int m_numColours;
    RGBAColour* m_colours;
    unsigned int m_numVertices; // Each element contains an index into m_positions and an index into m_colours
    VertexPosCol* m_vertices;
    unsigned int m_numTriangles;
    unsigned int m_maxTriangles;
    ShapeTriangle* m_triangles;

    char* m_name;
    char* m_parentName;
    Matrix34 m_transform;
    Vector3 m_angVel;
    Vector3 m_vel;

    bool m_useCylinder; // If true then use cylinder hit check instead of sphere
    Vector3 m_centre;
    double m_radius;
    double m_mostPositiveY;
    double m_mostNegativeY;

    LList<ShapeFragment*> m_childFragments;
    LList<ShapeMarker*> m_childMarkers;

    ShapeFragment(TextReader* _in, const char* _name);
    ShapeFragment(const char* _name, const char* _parentName);
    ~ShapeFragment();

    void RegisterPositions(Vector3* positions, unsigned int numPositions);
    void RegisterColours(RGBAColour* colours, unsigned int numColours);

    void WriteToFile(FILE* _out) const;

    void Render(double _predictionTime); // Uses display list
    void RenderSlow(); // Doesn't use display list
    void RenderHitCheck(const Matrix34& _transform);
    void RenderMarkers(const Matrix34& _transform);

    ShapeFragment* LookupFragment(const char* _name); // Recurses into child fragments
    ShapeMarker* LookupMarker(const char* _name); // Recurses into child fragments

    void CalculateCentre(const Matrix34& _transform, Vector3& _centre, int& _numFragments); // Recursive
    void CalculateRadius(const Matrix34& _transform, const Vector3& _centre, double& _radius); // Recursive

    void ScaleRadius(double _scale);

    bool RayHit(RayPackage* _package, const Matrix34& _transform, bool _accurate = false);
    bool SphereHit(SpherePackage* _package, const Matrix34& _transform, bool _accurate = false);
    bool ShapeHit(Shape* _shape, const Matrix34& _theTransform, // Transform of _shape
                  const Matrix34& _ourTransform, // Transform of this
                  bool _accurate = false);
};

// ***********
// Class Shape
// ***********

class Shape
{
  protected:
    void Load(TextReader* _in);
    bool m_animating; // If false then whole shape in one display list otherwise one display list per fragment
    char* m_displayListName;

    void RenderSlow();

  public:
    ShapeFragment* m_rootFragment;
    char* m_name;

    Shape();
    Shape(const char* filename, bool _animating, bool _buildDisplayList = true);
    Shape(TextReader* in, bool _animating, bool _buildDisplayList = true);
    ~Shape();

    void BuildDisplayList();
    void FlushDisplayList();

    void Render(double _predictionTime, const Matrix34& _transform);
    void RenderHitCheck(const Matrix34& _transform);

    bool RayHit(RayPackage* _package, const Matrix34& _transform, bool _accurate = false);
    bool SphereHit(SpherePackage* _package, const Matrix34& _transform, bool _accurate = false);
    bool ShapeHit(Shape* _shape, const Matrix34& _theTransform, // Transform of _shape
                  const Matrix34& _ourTransform, // Transform of this
                  bool _accurate = false);

    Vector3 CalculateCentre(const Matrix34& _transform);
    double CalculateRadius(const Matrix34& _transform, const Vector3& _centre);

    void ScaleRadius(double _scale);
};

#endif
