#ifndef _included_multiwinia_filedialog_h
#define _included_multiwinia_filedialog_h

#include "mainmenus.h"

class ScrollBar;

class MultiwiniaFileDialog : public GameOptionsWindow
{
  public:
    char* m_path;
    char* m_filter;
    char* m_parent;
    bool m_allowMultiSelect;
    bool m_okPressed;

    std::vector<std::string> m_files;
    LList<int> m_selected;

    ScrollBar* m_scrollBar;

    MultiwiniaFileDialog(char* name, const char* parent, const char* path = nullptr, const char* filter = nullptr,
                         bool allowMultiSelect = false);
    ~MultiwiniaFileDialog() override;

    void Create() override;
    void Remove() override;

    void SetDirectory(const char* path);
    void SetParent(const char* parent);
    void SetFilter(const char* filter);

    void FileClicked(int index);
    int IsFileSelected(int index); // Returns index within m_selected, or -1 if not found

    void RefreshFileList();

    void FileSelected(const char* filename); // This will be called for all selected files
};

#endif
