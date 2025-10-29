#include "pch.h"

#ifdef WIN32
#include <io.h>
#include <direct.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>                     // needed for errno definition


#include "filesys_utils.h"
#include "string_utils.h"


//*****************************************************************************
// Misc directory and filename functions
//*****************************************************************************


static bool FilterMatch( const char *_filename, const char *_filter )
{
	while (*_filename && *_filename == *_filter) {
		_filename++;
		_filter++;
	}

	if (*_filename == *_filter)
		return true;

	switch(*_filter++) {
		case '*':
			while (*_filename) {
				_filename++;
				if (FilterMatch(_filename, _filter))
					return true;
			}
			return false;

		case '?':
			if (!*_filename)
				return false;
			_filename++;
			return FilterMatch(_filename, _filter);

		default:
			return false;
	}
}

// Finds all the filenames in the specified directory that match the specified
// filter. Directory should be like "gamedata/textures" or "gamedata/textures/".
// Filter can be NULL or "" or "*.bmp" or "map_*" or "map_*.txt"
// Set FullFilename to true if you want results like "gamedata/textures/blah.bmp"
// or false for "blah.bmp"
LList <char *> *ListDirectory( char const *_dir, char const *_filter, bool _fullFilename )
{
	if (_filter == nullptr || _filter[0] == '\0') { _filter = "*"; }

	// Create a DArray for our results
	LList<char*>* result = new LList<char*>();

	// Now add on all files found locally
	char searchstring[256];
	ASSERT(strlen(_dir) + strlen(_filter) < sizeof(searchstring) - 1);
	sprintf(searchstring, "%s%s", _dir, _filter);

	WIN32_FIND_DATA thisfile;
	HANDLE hFind = FindFirstFile(searchstring, &thisfile);

	do
	{
		if (strcmp(thisfile.cFileName, ".") != 0 &&
			strcmp(thisfile.cFileName, "..") != 0 &&
			!(thisfile.dwFileAttributes & _A_SUBDIR))
		{
			char* newname = nullptr;
			if (_fullFilename)
			{
				size_t len = strlen(_dir) + strlen(thisfile.cFileName);
				newname = new char[len + 1];
				sprintf(newname, "%s%s", _dir, thisfile.cFileName);
			}
			else
			{
				int len = strlen(thisfile.cFileName);
				newname = new char[len + 1];
				sprintf(newname, "%s", thisfile.cFileName);
			}

			result->PutData(newname);
		}
	} while (FindNextFile(hFind, &thisfile) != 0);

	return result;
}


LList <char *> *ListSubDirectoryNames(char const *_dir)
{
	auto* result = new LList<char*>();

	WIN32_FIND_DATA thisfile;
	HANDLE hFind = FindFirstFile(_dir, &thisfile);

	do
	{
		if (strcmp(thisfile.cFileName, ".") != 0 && strcmp(thisfile.cFileName, "..") != 0
			&& (thisfile.dwFileAttributes & _A_SUBDIR))
		{
			char* newname = _strdup(thisfile.cFileName);
			result->PutData(newname);
		}
	} while (FindNextFile(hFind, &thisfile) != 0);
	return result;
}


bool DoesFileExist(char const *_fullPath)
{
	FILE *f = fopen(_fullPath, "r");
	if(f)
	{
		fclose(f);
		return true;
	}

	return false;
}


#define FILE_PATH_BUFFER_SIZE 256
static char s_filePathBuffer[FILE_PATH_BUFFER_SIZE + 1];

char const *GetDirectoryPart(char const *_fullFilePath)
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


char const *GetFilenamePart(char const *_fullFilePath)
{
    const char *filePart = strrchr(_fullFilePath, '/') + 1;
    if( filePart )
    {
	    strncpy(s_filePathBuffer, filePart, FILE_PATH_BUFFER_SIZE);
        return s_filePathBuffer;
    }
	return NULL;
}


char const *GetExtensionPart(char const *_fullFilePath)
{
	return strrchr(_fullFilePath, '.') + 1;
}


char const *RemoveExtension(char const *_fullFileName)
{
    strcpy( s_filePathBuffer, _fullFileName );

    char *dot = strrchr(s_filePathBuffer, '.');
    if( dot ) *dot = '\x0';

    return s_filePathBuffer;
}


bool AreFilesIdentical(char const *_name1, char const *_name2)
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


bool CreateDirectory(char const *_directory)
{
#ifdef TARGET_MSVC
    int result = _mkdir ( _directory );
    if( result == 0 ) return true;                              // Directory was created
    if( result == -1 && errno == 17 /* EEXIST */ ) return true;              // Directory already exists
    else return false;
#else
	if (access(_directory, W_OK|X_OK|R_OK) == 0)
		return true;
	return mkdir(_directory, 0777) == 0;
#endif
}


void DeleteThisFile(char const *_filename)
{
#ifdef TARGET_MSVC
    bool result = DeleteFile( _filename );
#else
	unlink( _filename );
#endif
}

bool IsDirectory(const char *_fullPath)
{
#ifdef TARGET_MSVC
	// To do
	return false;
#else
	struct stat s;
	int rc = stat(_fullPath, &s);
	if (rc != 0)
		return false;
	return (s.st_mode & S_IFDIR);
#endif
}



// ***************************************************************************
//  Class EncryptedFileWriter
// ***************************************************************************

static unsigned int s_offsets[] = {
	31, 7, 9, 1,
	11, 2, 5, 5,
	3, 17, 40, 12,
	35, 22, 27, 2
};


EncryptedFileWriter::EncryptedFileWriter(char const *_name)
:	m_offsetIndex(0)
{
	m_out = fopen(_name, "w");
}


EncryptedFileWriter::~EncryptedFileWriter()
{
	fclose(m_out);
}


void EncryptedFileWriter::WriteLine(char *_line)
{
	int len = strlen(_line);

	for (int i = 0; i < len; ++i)
	{
		if (_line[i] > 32) {
			m_offsetIndex++;
			m_offsetIndex %= 16;
			int j = _line[i] + s_offsets[m_offsetIndex];
			if (j >= 128) j -= 95;
			_line[i] = j;
		}
	}

	fprintf(m_out, "%s", _line);
}
