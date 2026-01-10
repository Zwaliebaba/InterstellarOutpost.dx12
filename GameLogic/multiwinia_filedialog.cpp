#include "pch.h"

#include <string.h>

#include "filesys_utils.h"
#include "hi_res_time.h"
#include "text_renderer.h"
#include "input.h"
#include "resource.h"
#include "language_table.h"

#include "eclipse.h"

#include "scrollbar.h"
#include "multiwinia_filedialog.h"
#include "filedialog.h"
#include "multiwinia_window.h"

#include "app.h"
#include "game_menu.h"

//*****************************************************************************
// Class FileOKButton
//*****************************************************************************

class MultiwiniaFileOKButton : public GameMenuButton
{
  public:
    MultiwiniaFileOKButton()
      : GameMenuButton(UnicodeString()) {}

    void MouseUp() override
    {
      auto fd = static_cast<MultiwiniaFileDialog*>(m_parent);

      for (int i = 0; i < fd->m_selected.Size(); ++i)
      {
        int index = fd->m_selected[i];
        fd->FileSelected(fd->m_files[index].c_str());
      }

      EclRemoveWindow(m_parent->m_name);
    }
};

//*****************************************************************************
// Class FileButton
//*****************************************************************************

class MultiwiniaFileButton : public DarwiniaButton
{
  public:
    int m_index;
    double m_lastClickTime;

    MultiwiniaFileButton(int _index)
      : DarwiniaButton(),
        m_index(_index),
        m_lastClickTime(-1.0) {}

    void MouseUp() override
    {
      auto fd = static_cast<MultiwiniaFileDialog*>(m_parent);
      int index = m_index + fd->m_scrollBar->m_currentValue;

      if (!fd->m_files.empty())
        fd->FileClicked(index);

      double timeNow = GetHighResTime();
      double delta = timeNow - m_lastClickTime;
      if (delta < 0.2)
      {
        auto ok = static_cast<MultiwiniaFileOKButton*>(fd->GetButton("dialog_ok"));
        ok->MouseUp();
        return;
      }
      m_lastClickTime = timeNow;
    }

    void Render(int realX, int realY, bool highlighted, bool clicked) override
    {
      auto fd = static_cast<MultiwiniaFileDialog*>(m_parent);
      if (!m_mouseHighlightMode)
        highlighted = false;

      if (fd->m_buttonOrder[fd->m_currentButton] == this)
        highlighted = true;

      UpdateButtonHighlight();

      int index = m_index + fd->m_scrollBar->m_currentValue;

      if (!fd->m_files.empty())
      {
        if (fd->IsFileSelected(index) != -1)
        {
          glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
          glColor4f(0.75f, 0.75f, 1.0f, 0.5f);
          glBegin(GL_QUADS);
          glVertex2i(realX, realY);
          glVertex2i(realX + m_w, realY);
          glVertex2i(realX + m_w, realY + m_h);
          glVertex2i(realX, realY + m_h);
          glEnd();
        }

        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

        if (clicked || highlighted)
        {
          glBegin(GL_LINE_LOOP);
          glVertex2i(realX, realY);
          glVertex2i(realX + m_w, realY);
          glVertex2i(realX + m_w, realY + m_h);
          glVertex2i(realX, realY + m_h);
          glEnd();
        }

        float texY = realY + m_h / 2 - m_fontSize / 2 + 7.0f;

        const char* fileName = fd->m_files[index].c_str();
        glColor4f(1.0f, 1.0f, 1.0f, 0.0f);
        g_titleFont.SetRenderOutline(true);
        g_titleFont.DrawText2D(realX + m_w * 0.05f, texY, m_fontSize, fileName);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        g_titleFont.SetRenderOutline(false);
        g_titleFont.DrawText2D(realX + m_w * 0.05f, texY, m_fontSize, fileName);
      }
    }

    /*bool SpecialScroll( int direction )
	{
		MultiwiniaFileDialog *parent = (MultiwiniaFileDialog *)m_parent;
		int realNum = m_index + parent->m_scrollBar->m_currentValue;
		if( direction == 1 &&
			m_index == 0 &&
            parent->m_files && parent->m_files->ValidIndex( realNum + 10 ) ) 
		{
			if( parent->m_scrollBar->m_currentValue + 1 <  parent->m_scrollBar->m_numRows )
			{
				 parent->m_scrollBar->m_currentValue++;
				 return true;
			}
		}
		else if( direction == -1 &&
			m_index + 1 == 10 )
		{
			if( parent->m_scrollBar->m_currentValue > 0 )
			{
				parent->m_scrollBar->m_currentValue--;
				return true;
			}

		}

		return false;
	}*/
};

MultiwiniaFileDialog::MultiwiniaFileDialog(char* name, const char* parent, const char* path, const char* filter, bool allowMultiSelect)
  : GameOptionsWindow(name),
    m_path(nullptr),
    m_filter(nullptr),
    m_parent(nullptr),
    m_allowMultiSelect(allowMultiSelect),
    m_files(NULL),
    m_scrollBar(nullptr)
{
  SetFilter(filter ? filter : "*");
  SetDirectory(path ? path : "c:\\");
  SetParent(parent);

  m_scrollBar = new ScrollBar(this);
}

MultiwiniaFileDialog::~MultiwiniaFileDialog()
{
  free(m_path);
  free(m_filter);
  free(m_parent);

  m_selected.Empty();

  delete m_scrollBar;
}

void MultiwiniaFileDialog::Create()
{
  GameOptionsWindow::Create();

  SetTitle(LANGUAGEPHRASE("multiwinia_mainmenu_title"));

  int w = g_app->m_renderer->ScreenW();
  int h = g_app->m_renderer->ScreenH();

  float leftX, leftY, leftW, leftH;
  float fontLarge, fontMed, fontSmall;
  float buttonW, buttonH, gap;
  GetPosition_LeftBox(leftX, leftY, leftW, leftH);
  GetFontSizes(fontLarge, fontMed, fontSmall);
  GetButtonSizes(buttonW, buttonH, gap);

  float x = leftX + (leftW - buttonW) / 2.0f;
  float y = leftY + buttonH;
  float fontSize = fontMed;

  y += buttonH * 0.5f;
  auto title = new GameMenuTitleButton();
  title->SetShortProperties("title", leftX + 10, leftY + 10, leftW - 20, buttonH * 1.5f, LANGUAGEPHRASE("multiwinia_editor_selectmap"));
  title->m_fontSize = fontMed;
  RegisterButton(title);
  y += gap;
  buttonH *= 0.65f;

  int numRows = 10;

  int scrollbarW = buttonH / 2.0f;
  int scrollbarX = leftX + leftW - (scrollbarW + 10); // 10 + buttonW;
  int scrollbarY = y + buttonH; //leftY+10+buttonH*1.6f;
  int scrollbarH = (leftH - (buttonH * 1.5f)) - (buttonH * 7);

  m_scrollBar = new ScrollBar(this);
  m_scrollBar->Create("scrollBar", scrollbarX, scrollbarY, scrollbarW, scrollbarH, m_files.size(), numRows);

  for (int i = 0; i < numRows; ++i)
  {
    char name[32];
    sprintf(name, "File %d", i);
    auto button = new MultiwiniaFileButton(i);
    button->m_fontSize = fontSmall;
    button->SetShortProperties(name, x, y += buttonH + gap, buttonW, buttonH, UnicodeString(" "));
    RegisterButton(button);
    //m_buttonOrder.PutData( button );
  }

  buttonH /= 0.65f;

  y = leftY + leftH - buttonH * 3;

  auto ok = new MultiwiniaFileOKButton();
  ok->SetProperties("dialog_ok", x, y, buttonW, buttonH, LANGUAGEPHRASE("dialog_ok"));
  ok->m_fontSize = fontSize;
  RegisterButton(ok);
  m_buttonOrder.PutData(ok);

  auto close = new MenuCloseButton("dialog_back");
  close->SetShortProperties("dialog_close", x, y += buttonH + gap, buttonW, buttonH, LANGUAGEPHRASE("multiwinia_menu_back"));
  close->m_fontSize = fontMed;
  RegisterButton(close);
  m_buttonOrder.PutData(close);
  m_backButton = close;

  //m_scrollBar->Create( "FileScroll", m_w - 20, 25, 15, numRows * 13, m_files->Size(), numRows );
}

void MultiwiniaFileDialog::Remove()
{
  DarwiniaWindow::Remove();

  m_scrollBar->Remove();
}

void MultiwiniaFileDialog::SetDirectory(const char* path)
{
  free(m_path);
  m_path = _strdup(path);
  SetTitle(UnicodeString(path));
  RefreshFileList();
}

void MultiwiniaFileDialog::SetFilter(const char* filter)
{
  free(m_filter);
  m_filter = _strdup(filter);
}

void MultiwiniaFileDialog::SetParent(const char* parent)
{
  free(m_parent);
  m_parent = _strdup(parent);
}

void MultiwiniaFileDialog::FileSelected(const char* filename)
{
  auto window = static_cast<MultiwiniaWindow*>(EclGetWindow(m_parent));
  window->m_mapFilename.Set(filename);
}

void MultiwiniaFileDialog::RefreshFileList()
{
  m_selected.Empty();
  m_files = g_app->m_resource->ListResources(m_path, m_filter, false);
  EclDirtyWindow(m_name);
}

void MultiwiniaFileDialog::FileClicked(int index)
{
  bool ctrlKey = g_inputManager->controlEvent(ControlFileMultiSelect);

  if (!m_allowMultiSelect || !ctrlKey)
    m_selected.Empty();

  int alreadySelected = IsFileSelected(index);
  if (alreadySelected != -1 && m_allowMultiSelect)
    m_selected.RemoveData(alreadySelected);
  else
    m_selected.PutData(index);
}

int MultiwiniaFileDialog::IsFileSelected(int index)
{
  for (int i = 0; i < m_selected.Size(); ++i)
  {
    if (m_selected[i] == index)
      return i;
  }

  return -1;
}
