#include "pch.h"
#include "hi_res_time.h"
#include "text_renderer.h"
#include "input.h"
#include "resource.h"
#include "language_table.h"
#include "eclipse.h"
#include "scrollbar.h"
#include "filedialog.h"
#include "app.h"

//*****************************************************************************
// Class FileOKButton
//*****************************************************************************

class FileOKButton : public DarwiniaButton
{
  public:
    void MouseUp() override
    {
      auto fd = static_cast<FileDialog*>(m_parent);

      for (int i = 0; i < fd->m_selected.Size(); ++i)
      {
        int index = fd->m_selected[i];
        const char* filename = fd->m_files[index].c_str();
        fd->FileSelected(filename);
      }

      EclRemoveWindow(m_parent->m_name);
    }
};

//*****************************************************************************
// Class FileButton
//*****************************************************************************

class FileButton : public EclButton
{
  public:
    int m_index;
    double m_lastClickTime;

    FileButton(int _index)
      : m_index(_index),
        m_lastClickTime(-1.0) {}

    void MouseUp() override
    {
      auto fd = static_cast<FileDialog*>(m_parent);
      int index = m_index + fd->m_scrollBar->m_currentValue;

      if (!fd->m_files.empty())
        fd->FileClicked(index);

      double timeNow = GetHighResTime();
      double delta = timeNow - m_lastClickTime;
      if (delta < 0.2)
      {
        auto ok = static_cast<FileOKButton*>(fd->GetButton("dialog_ok"));
        ok->MouseUp();
        return;
      }
      m_lastClickTime = timeNow;
    }

    void Render(int realX, int realY, bool highlighted, bool clicked) override
    {
      auto fd = static_cast<FileDialog*>(m_parent);
      int index = m_index + fd->m_scrollBar->m_currentValue;

      if (!fd->m_files.empty())
      {
        if (fd->IsFileSelected(index) != -1)
        {
          glColor4f(0.3, 0.3, 1.0, 0.5);
          glBegin(GL_QUADS);
          glVertex2i(realX, realY);
          glVertex2i(realX + m_w, realY);
          glVertex2i(realX + m_w, realY + m_h);
          glVertex2i(realX, realY + m_h);
          glEnd();
        }

        glColor4f(1.0, 1.0, 1.0, 1.0);

        if (clicked || highlighted)
        {
          glBegin(GL_LINE_LOOP);
          glVertex2i(realX, realY);
          glVertex2i(realX + m_w, realY);
          glVertex2i(realX + m_w, realY + m_h);
          glVertex2i(realX, realY + m_h);
          glEnd();
        }

        const char* fileName = fd->m_files[index].c_str();
        g_editorFont.DrawText2D(realX + 30, realY + 8, 11, fileName);
      }
    }
};

class FileCancelButton : public DarwiniaButton
{
  void MouseUp() override { EclRemoveWindow(m_parent->m_name); }
};

//*****************************************************************************
// Class SelectedButton
//*****************************************************************************

class SelectedButton : public DarwiniaButton
{
  void Render(int realX, int realY, bool highlighted, bool clicked) override
  {
    auto fd = static_cast<FileDialog*>(m_parent);
    if (fd->m_selected.Size() > 1)
      SetCaption(LANGUAGEPHRASE("dialog_multiplefiles"));
    else if (fd->m_selected.Size() == 1)
    {
      int index = fd->m_selected[0];
      const char* filename = fd->m_files[index].c_str();
      SetCaption(UnicodeString(filename));
    }
    else
      SetCaption(UnicodeString(" "));

    DarwiniaButton::Render(realX, realY, highlighted, clicked);
  }
};

//*****************************************************************************
// Class FileDialog
//*****************************************************************************

FileDialog::FileDialog(const char* name, const char* parent, const char* path, const char* filter, bool allowMultiSelect)
  : DarwiniaWindow(name),
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

FileDialog::~FileDialog()
{
  free(m_path);
  free(m_filter);
  free(m_parent);

  m_selected.Empty();

  delete m_scrollBar;
}

void FileDialog::Create()
{
  DarwiniaWindow::Create();

  int numRows = (m_h - 60) / 13;

  for (int i = 0; i < numRows; ++i)
  {
    char name[32];
    sprintf(name, "File %d", i);
    auto button = new FileButton(i);
    button->SetProperties(name, 5, 25 + i * 13, m_w - 25, 12, UnicodeString(" "), UnicodeString(" "));
    RegisterButton(button);
  }

  auto selected = new SelectedButton();
  selected->SetProperties("Selected", 10, m_h - 30, m_w - 140, 20, UnicodeString(""), UnicodeString(" "));
  RegisterButton(selected);

  auto cancel = new FileCancelButton();
  cancel->SetProperties("dialog_cancel", m_w - 60, m_h - 30, 55, 20, LANGUAGEPHRASE("dialog_cancel"));
  RegisterButton(cancel);

  auto ok = new FileOKButton();
  ok->SetProperties("dialog_ok", m_w - 120, m_h - 30, 55, 20, LANGUAGEPHRASE("dialog_ok"));
  RegisterButton(ok);

  m_scrollBar->Create("FileScroll", m_w - 20, 25, 15, numRows * 13, m_files.size(), numRows);
}

void FileDialog::Remove()
{
  DarwiniaWindow::Remove();

  m_scrollBar->Remove();
}

void FileDialog::SetDirectory(const char* path)
{
  free(m_path);
  m_path = _strdup(path);
  SetTitle(UnicodeString(path));
  RefreshFileList();
}

void FileDialog::SetFilter(const char* filter)
{
  free(m_filter);
  m_filter = _strdup(filter);
}

void FileDialog::SetParent(const char* parent)
{
  free(m_parent);
  m_parent = _strdup(parent);
}

void FileDialog::FileSelected(const char* filename) {}

void FileDialog::RefreshFileList()
{
  m_selected.Empty();

  m_files = g_app->m_resource->ListResources(m_path, m_filter, false);

  EclDirtyWindow(m_name);
}

void FileDialog::FileClicked(int index)
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

int FileDialog::IsFileSelected(int index)
{
  for (int i = 0; i < m_selected.Size(); ++i)
  {
    if (m_selected[i] == index)
      return i;
  }

  return -1;
}
