#ifndef _included_tree_h
#define _included_tree_h

#include "building.h"

class Tree : public Building
{
  protected:
    int m_branchDisplayListId;
    int m_leafDisplayListId;
    void RenderBranch(Vector3 _from, Vector3 _to, int _iterations, bool _calcRadius, bool _renderBranch, bool _renderLeaf);

    void GenerateLeaves();
    void GenerateBranches();

    Vector3 m_hitcheckCentre;
    double m_hitcheckRadius;
    int m_numLeafs;

    double m_fireDamage;
    double m_onFire;
    bool m_burnSoundPlaying;

    double GetActualHeight(double _predictionTime);

    unsigned char m_leafColourArray[4];
    unsigned char m_branchColourArray[4];

  public:
    double m_height;
    double m_budsize;
    double m_pushUp;
    double m_pushOut;
    int m_iterations;
    int m_seed;
    int m_leafColour;
    int m_branchColour;
    int m_leafDropRate;
    int m_spiritDropRate;

    int m_spawnsRemaining; // Time to spread and create another tree
    double m_spawnTimer;
    bool m_destroyable;
    bool m_evil;
    float m_corruptCheckTimer;
    bool m_renderCorruptShadow;
    bool m_corrupted;

    Tree();
    ~Tree() override;

    void Initialise(Building* _template) override;
    void SetDetail(int _detail) override;
    void Clone(Tree* tree);

    bool Advance() override;

    void DeleteDisplayLists();
    void Generate();
    void RenderAlphas(double _predictionTime) override;

    bool PerformDepthSort(Vector3& _centrePos) override;

    void Damage(double _damage) override;
    void SetFireAmount(double _amount);
    double GetFireAmount();
    bool IsOnFire();
    double GetBurnRange();

    void CreateAnotherTree();

    bool DoesSphereHit(const Vector3& _pos, double _radius) override;
    bool DoesShapeHit(Shape* _shape, Matrix34 _transform) override;
    bool DoesRayHit(const Vector3& _rayStart, const Vector3& _rayDir, double _rayLen = 1e10, Vector3* _pos = nullptr,
                    Vector3* _norm = nullptr) override; // pos/norm will not always be available

    void ListSoundEvents(LList<char*>* _list) override;

    void Read(TextReader* _in, bool _dynamic) override;
    void Write(TextWriter* _out) override;

    char* LogState(char* message = nullptr) override;

    bool IsInView() override;
};

#endif
