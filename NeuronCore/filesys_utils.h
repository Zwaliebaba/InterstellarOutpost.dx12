#ifndef INCLUDED_FILESYS_UTILS
#define INCLUDED_FILESYS_UTILS

#undef CreateDirectory

std::vector<std::string> ListDirectory(const char* _dir, const char* _filter, bool fullFilename = true);
std::vector<std::string> ListSubDirectoryNames(const char* _dir);

bool IsDirectory(const char* _fullPath);
bool DoesFileExist(const char* _fullPath);
bool AreFilesIdentical(const char* _name1, const char* _name2);

const char* GetExtensionPart(const char* _fileFilePath);
char* RemoveExtension(const char* _fullFileName);

bool CreateDirectory(const char* _directory);
void DeleteThisFile(const char* _filename);

std::string FindCaseInsensitive(const std::string& _fullPath);

#endif
