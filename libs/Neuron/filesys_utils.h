#ifndef INCLUDED_FILESYS_UTILS
#define INCLUDED_FILESYS_UTILS

#include "llist.h"

#undef CreateDirectory

//*****************************************************************************
// Misc directory and filename functions
//*****************************************************************************

LList<char *> *ListDirectory(const char *_dir, const char *_filter, bool fullFilename = true);
LList<char *> *ListSubDirectoryNames(const char *_dir);

bool DoesFileExist(const char *_fullPath);

const char *GetDirectoryPart(const char *_fullFilePath);
const char *GetFilenamePart(const char *_fullFilePath);
const char *GetExtensionPart(const char *_fileFilePath);
const char *RemoveExtension(const char *_fullFileName);

bool AreFilesIdentical(const char *_name1, const char *_name2);

bool CreateDirectory(const char *_directory);
void DeleteThisFile(const char *_filename);


class EncryptedFileWriter
{
protected:
  int m_offsetIndex;
  FILE *m_out;

public:
  EncryptedFileWriter(const char *_name);
  ~EncryptedFileWriter();

  void WriteLine(char *_line);// _line gets modified
};

#endif