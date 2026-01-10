/*
 * =========
 * SCROLLBAR
 * =========
 *
 * This is a container class for scrollbar creation/deletion/management,
 * and all associated buttons that are required.
 *
 */

#ifndef _included_scrollbar_h
#define _included_scrollbar_h

#include "eclipse.h"
#include "darwinia_window.h"

class ScrollBar
{

  public:
    char m_parentWindow[SIZE_ECLWINDOW_NAME];
    char m_name[SIZE_ECLBUTTON_NAME];

    int m_x;
    int m_y;
    int m_w;
    int m_h;
    int m_numRows;
    int m_winSize;
    int m_currentValue;

    ScrollBar(EclWindow* parent);
    ~ScrollBar();

    void Create(const char* name, int x, int y, int w, int h, int numRows, int winSize, int stepSize = 1);

    void Remove();

    void SetNumRows(int newValue);
    void SetWinSize(int newValue);

    void SetCurrentValue(int newValue);
    void ChangeCurrentValue(int newValue);

};

class ScrollBarButton : public EclButton
{
  protected:
    ScrollBar* m_scrollBar;
    int m_grabOffset;

  public:
    ScrollBarButton(ScrollBar* scrollBar);
    void Render(int realX, int realY, bool highlighted, bool clicked) override;
    void MouseUp() override;
    void MouseDown() override;
};

class ScrollChangeButton : public DarwiniaButton
{
  protected:
    ScrollBar* m_scrollBar;
    int m_amount;

  public:
    ScrollChangeButton(ScrollBar* scrollbar, int amount);
    void MouseDown() override;
    void Render(int realX, int realY, bool highlighted, bool clicked) override;
};

#endif
