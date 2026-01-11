#include "pch.h"

#include "string_utils.h"

#include <string.h>
#include <ctype.h>
#include <wchar.h>
#include <wctype.h>
#include <algorithm>

#ifdef TARGET_MSVC
#pragma warning( disable : 4996 )
#endif

void StrToLower(char *_string)
{
	while (*_string != '\0')
	{
		*_string = tolower(*_string);
		_string++;
	}
}

void StrToLower		( std::string &_string )
{
	std::transform( _string.begin(), _string.end(), _string.begin(), tolower );
}

void    StrToUpper      ( wchar_t *_string )
{
	while (*_string != '\0')
	{
		*_string = towupper(*_string);
		_string++;
	}
}

void    StrToLower      ( wchar_t *_string )
{
	while (*_string != '\0')
	{
		*_string = towlower(*_string);
		_string++;
	}
}


#ifdef TRACK_MEMORY_LEAKS
#undef newStr
#endif

// Ordinary
char *newStr( const char *s)
{
    if ( !s )
        return NULL;
	char *d = new char [strlen(s) + 1];
	strcpy(d, s);
	return d;
}	

wchar_t *newStr( const wchar_t *s)
{
    if ( !s )
        return NULL;
	wchar_t *d = new wchar_t [wcslen(s) + 1];
	wcscpy(d, s);
	return d;
}	


#ifdef TRACK_MEMORY_LEAKS
#undef new

// Tracks the parent
char *newStr( const char *_filename, int _line, const char *s)
{
    if ( !s )
        return NULL;
	char *d = new (_NORMAL_BLOCK, _filename, _line) char [strlen(s) + 1];
	strcpy(d, s);
	return d;
}	

// Tracks the parent
wchar_t *newStr( const char *_filename, int _line, const wchar_t *s)
{
    if ( !s )
        return NULL;
	wchar_t *d = new (_NORMAL_BLOCK, _filename, _line) wchar_t [wcslen(s) + 1];
	wcscpy(d, s);
	return d;
}	
#endif

std::string Join( std::vector< std::string > _strings, const std::string &_separator )
{
	std::ostringstream s;

	int secondLast = _strings.size() - 1;
	for( int i = 0; i < secondLast; i++ )
	{
		s << _strings[i] << _separator;		
	}
	if( secondLast >= 0 )
		s << _strings[secondLast];

	return s.str();
}

