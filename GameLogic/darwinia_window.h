/*
 * ===============
 * DARWINIA WINDOW
 * ===============
 *
 * A basic window for use in Darwinia
 *
 */

#ifndef _included_darwiniawindow_h
#define _included_darwiniawindow_h

#include "eclipse.h"
#include "unicode_string.h"

class DarwiniaButton;

class DarwiniaWindow : public EclWindow
{
  public:
    LList<EclButton*> m_buttonOrder;
    int m_currentButton;
    bool m_buttonChangedThisUpdate;
    bool m_skipUpdate;
    bool m_closeable;
    bool m_lockMenu;
    InputDetails m_details;

    EclButton* m_backButton;

    DarwiniaWindow(const char* name);
    ~DarwiniaWindow() override;

    void Create() override;
    void Remove() override;
    void Render(bool hasFocus) override;
    void Update() override;

    void CreateValueControl(const char* name, bool* value, int y, float change, float _lowBound, float _highBound,
                            DarwiniaButton* callback = nullptr, int x = -1, int w = -1);

    void CreateValueControl(const char* name, unsigned char* value, int y, float change, float _lowBound, float _highBound,
                            DarwiniaButton* callback = nullptr, int x = -1, int w = -1);

    void CreateValueControl(const char* name, int* value, int y, float change, float _lowBound, float _highBound,
                            DarwiniaButton* callback = nullptr, int x = -1, int w = -1);

    void CreateValueControl(const char* name, float* value, int y, float change, float _lowBound, float _highBound,
                            DarwiniaButton* callback = nullptr, int x = -1, int w = -1);

    void CreateValueControl(const char* name, double* value, int y, float change, float _lowBound, float _highBound,
                            DarwiniaButton* callback = nullptr, int x = -1, int w = -1);

    void CreateValueControl(const char* name, char* value, int y, float change, float _lowBound, float _highBound,
                            DarwiniaButton* callback = nullptr, int x = -1, int w = -1);

    void CreateColourControl(const char* name, int* value, int y, DarwiniaButton* callback = nullptr, int x = -1, int w = -1);

    void RemoveValueControl(char* name);

    int GetMenuSize(int _value);
    void SetMenuSize(int _w, int _h);

    int GetClientRectX1();
    int GetClientRectY1();
    int GetClientRectX2();
    int GetClientRectY2();

    void SetCurrentButton(EclButton* button);

  private:
    void CreateValueControl(const char* name, int dataType, void* value, int y, float change, float _lowBound, float _highBound,
                            DarwiniaButton* callback = nullptr, int x = -1, int w = -1);
};

class DarwiniaButton : public EclButton
{
  public:
    float m_fontSize;
    bool m_centered;
    bool m_disabled;
    bool m_highlightedThisFrame;
    bool m_mouseHighlightMode;

    DarwiniaButton();

    void Render(int realX, int realY, bool highlighted, bool clicked) override;
    virtual void SetShortProperties(const char* _name, int x, int y, int w, int h, const UnicodeString& _caption,
                                    const UnicodeString& _tooltip = UnicodeString());
    void SetDisabled(bool _disabled = true);
    void UpdateButtonHighlight();

    virtual bool SpecialScroll(int direction = 1);
};

class BorderlessButton : public DarwiniaButton
{
  public:
    BorderlessButton();
    void Render(int realX, int realY, bool highlighted, bool clicked) override;
    void SetShortProperties(const char* _name, int x, int y, int w, int h, const UnicodeString& _caption,
                            const UnicodeString& _tooltip = UnicodeString()) override;
};

class CloseButton : public DarwiniaButton
{
  public:
    bool m_iconised;
    CloseButton();
    void Render(int realX, int realY, bool highlighted, bool clicked) override;
    void MouseUp() override;
};

class GameExitButton : public DarwiniaButton
{
  void MouseUp() override;
};

class InvertedBox : public DarwiniaButton
{
  public:
    void Render(int realX, int realY, bool highlighted, bool clicked) override;
};

class LabelButton : public DarwiniaButton
{
  public:
    void Render(int realX, int realY, bool highlighted, bool clicked) override;
};

#endif
