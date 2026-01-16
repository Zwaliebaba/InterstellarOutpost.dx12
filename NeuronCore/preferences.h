#ifndef INCLUDED_PREFERENCES_H
#define INCLUDED_PREFERENCES_H

#include "unicode_string.h"

class PrefsItem;

// ******************
// Class PrefsManager
// ******************

class PrefsManager
{
  using PrefsItemMap = std::map<UnicodeString, PrefsItem>;
  using FileTextList = std::list<UnicodeString>;

  PrefsItemMap m_items;
  FileTextList m_fileText;

  char* m_filename;

  bool IsLineEmpty(const UnicodeString& _line);

  void CreateDefaultValues();

  public:
    PrefsManager(const char* _filename);
    PrefsManager(const std::string& _filename);
    ~PrefsManager();

    void Load(const char* _filename = nullptr); // If filename is NULL, then m_filename is used

    void Clear();

    const char* GetString(const char* _key, const char* _default = nullptr) const;
    UnicodeString GetUnicodeString(const char* _key, const UnicodeString& _default) const;
    float GetFloat(const char* _key, float _default = -1.0f) const;
    int GetInt(const char* _key, int _default = -1) const;

    // Set functions update existing PrefsItems or create new ones if key doesn't yet exist
    void SetString(const char* _key, const UnicodeString& _val);
    void SetFloat(const char* _key, float _val);
    void SetInt(const char* _key, int _val);

    void AddLine(UnicodeString _line);

    bool DoesKeyExist(const char* _key) const;
};

extern PrefsManager* g_prefsManager;

#endif
