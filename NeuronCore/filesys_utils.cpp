#include "pch.h"
#include "string_utils.h"
#include "filesys_utils.h"

#include <direct.h>

std::vector<std::string> ListDirectory(const char* _dir, const char* _filter, bool _fullFilename)
{
  std::vector<std::string> result;

  if (_filter == nullptr || _filter[0] == '\0')
    _filter = "*";

  // Now add on all files found locally
  char searchstring[256];
  DEBUG_ASSERT(strlen(_dir) + strlen(_filter) < sizeof(searchstring) - 1);

  sprintf(searchstring, "%s%s", _dir, _filter);

  WIN32_FIND_DATAA thisfile;
  const HANDLE hFind = FindFirstFileA(searchstring, &thisfile);
  if (hFind == INVALID_HANDLE_VALUE)
    return result;

  do
  {
    if (!(thisfile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
    {
      std::string newname;
      if (_fullFilename)
        newname = std::string(_dir) + thisfile.cFileName;
      else
        newname = thisfile.cFileName;

      result.emplace_back(newname);
    }
  }
  while (FindNextFileA(hFind, &thisfile) != 0);

  return result;
}

std::vector<std::string> ListSubDirectoryNames(const char* _dir)
{
  std::vector<std::string> result;

  WIN32_FIND_DATAA thisfile;
  const HANDLE hFind = FindFirstFileA(_dir, &thisfile);

  do
  {
    if (strcmp(thisfile.cFileName, ".") != 0 && strcmp(thisfile.cFileName, "..") != 0 &&
        (thisfile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
    {
      std::string newname = thisfile.cFileName;
      result.emplace_back(newname);
    }
  }
  while (FindNextFileA(hFind, &thisfile) != 0);
  return result;
}

bool DoesFileExist(const char *_fullPath)
{
#ifndef WIN32
	std::string path = FindCaseInsensitive( _fullPath );
	FILE *f = fopen( path.c_str(), "r" );
#else
    FILE *f = fopen(_fullPath, "r");
#endif
    if(f) 
    {
        fclose(f);
        return true;
    }

    return false;
}


#define FILE_PATH_BUFFER_SIZE 256
static char s_filePathBuffer[FILE_PATH_BUFFER_SIZE + 1];

char *ConcatPaths(const char *_firstComponent, ...)
{
	va_list components;
	const char *component;
	char *buffer, *returnBuffer;
	
	buffer = _strdup(_firstComponent);
	
	va_start(components, _firstComponent);
	while ((component = va_arg(components, const char *)) != 0)
	{
		buffer = (char *)realloc(buffer, strlen(buffer) + 1 + strlen(component) + 1);
		strcat(buffer, "/");
		strcat(buffer, component);
	}
	va_end(components);
	
	returnBuffer = newStr(buffer);
	free(buffer);
	return returnBuffer;
}

char *GetDirectoryPart(const char *_fullFilePath)
{
    strncpy(s_filePathBuffer, _fullFilePath, FILE_PATH_BUFFER_SIZE);
    
    char *finalSlash = strrchr(s_filePathBuffer, '/');
    if( finalSlash )
    {
        *(finalSlash+1) = '\x0';
        return s_filePathBuffer;
    }

    return NULL;
}


char *GetFilenamePart(const char  *_fullFilePath)
{
    const char *filePart = strrchr(_fullFilePath, '/') + 1;
    if( filePart )
    {
        strncpy(s_filePathBuffer, filePart, FILE_PATH_BUFFER_SIZE);
        return s_filePathBuffer;
    }
    return NULL;
}


const char *GetExtensionPart(const char *_fullFilePath)
{
    if( !strrchr(_fullFilePath, '.') ) return NULL;
    return strrchr(_fullFilePath, '.') + 1;
}


char *RemoveExtension(const char *_fullFileName)
{
    strcpy( s_filePathBuffer, _fullFileName );

    char *dot = strrchr(s_filePathBuffer, '.');
    if( dot ) *dot = '\x0';
        
    return s_filePathBuffer;
}


bool AreFilesIdentical(const char *_name1, const char *_name2)
{
    FILE *in1 = fopen(_name1, "r");
    FILE *in2 = fopen(_name2, "r");
    bool rv = true;
    bool exitNow = false;

    if (!in1 || !in2)
    {
        rv = false;
        exitNow = true;
    }

    while (exitNow == false && !feof(in1) && !feof(in2))
    {
        int a = fgetc(in1);
        int b = fgetc(in2);
        if (a != b) 
        {
            rv = false;
            exitNow = true;
            break;
        }
    }

    if (exitNow == false && feof(in1) != feof(in2))
    {
        rv = false;
    }

    if (in1) fclose(in1);
    if (in2) fclose(in2);

    return rv;
}


bool CreateDirectory(const char *_directory)
{
    int result = _mkdir ( _directory );
    if( result == 0 ) return true;                              // Directory was created
    if( result == -1 && errno == 17 /* EEXIST */ ) return true;              // Directory already exists    
    else return false;
}

bool CreateDirectoryRecursively(const char *_directory)
{
	const char *p;
	char *buffer;
	bool error = false;
	
	if ( strlen(_directory) == 0 )
		return false;

	buffer = new char[ strlen(_directory) + 1 ];
	p = _directory;
	if (*p == '/')
		p++;
	
	p = strchr(p, '/');
	while (p != NULL && !error)
	{
		memcpy(buffer, _directory, p - _directory);
		buffer[ p-_directory ] = '\0';
		error = !CreateDirectory(buffer);
		p = strchr(p+1, '/');
	}
	
	if (error)
		return false;
	else
		return CreateDirectory(_directory);
}

void DeleteThisFile(const char *_filename)
{
#ifdef WIN32
    DeleteFileA( _filename );
#else
    unlink( _filename );
#endif
}

bool IsDirectory(const char *_fullPath)
{
#ifdef WIN32
	DWORD dwAtts = ::GetFileAttributesA( _fullPath );
	if ( dwAtts == INVALID_FILE_ATTRIBUTES )
		return false;
	return (dwAtts & FILE_ATTRIBUTE_DIRECTORY) != 0;
#else
    struct stat s;
    int rc = stat(_fullPath, &s);
    if (rc != 0)
        return false;
    return (s.st_mode & S_IFDIR);
#endif
}

std::string FindCaseInsensitive ( const std::string &_fullPath )
{
    return _fullPath;
}
