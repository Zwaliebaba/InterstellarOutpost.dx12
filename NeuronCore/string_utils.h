#ifndef __STRING_UTILS_H
#define __STRING_UTILS_H

#include <string>

void	StrToLower		( char *_string );											// Lowercase the string
void	StrToLower		( std::string &_string );

void    StrToLower      ( wchar_t *_string );
void    StrToUpper      ( wchar_t *_string );

#include <sstream>

std::string Join( std::vector< std::string > _strings, const std::string &_separator );

char    *newStr         ( const char *s );											// Make a copy of s, use delete[] to reclaim storage
wchar_t	*newStr			( const wchar_t *s );

#endif // __STRING_UTILS_H
