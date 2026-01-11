#include "pch.h"

// TODO: Consider changing the API so that Read*() functions return error codes
// some other way, unless you are sure, for instance, that ReadU8() will never
// read an actual byte with the value ((unsigned char) -1) == EOF

#ifdef WIN32
#include <io.h>
#endif

#include <stdio.h>
#include <string.h>

#include "binary_stream_readers.h"
#include "filesys_utils.h"


// ****************************************************************************
// BinaryReader
// ****************************************************************************

BinaryReader::BinaryReader()
:	m_eof(false)
{
	m_filename[0] = '\0';
}


BinaryReader::~BinaryReader()
{
}

const char *BinaryReader::GetFileType()
{
	using std::string;

	string::size_type dotIndex = m_filename.find_last_of( '.' );
	if( dotIndex == string::npos )
		return "";
	else
		return m_filename.c_str() + dotIndex + 1;
}

// ****************************************************************************
// BinaryFileReader
// ****************************************************************************

BinaryFileReader::BinaryFileReader(char const *_filename)
:	BinaryReader()
{
	if (_filename)
	{
		m_filename = FindCaseInsensitive( _filename );
        m_file = fopen(m_filename.c_str(), "rb");
	}
}


BinaryFileReader::~BinaryFileReader()
{
	if( m_file ) fclose(m_file);
}


bool BinaryFileReader::IsOpen()
{
	if (m_file) return true;
	return false;
}


signed char BinaryFileReader::ReadS8()
{
	int c = fgetc(m_file);
	if (c == EOF)
	{
		m_eof = true;
	}
	return c;
}


unsigned char BinaryFileReader::ReadU8()
{
	int c = fgetc(m_file);
	if (c == EOF)
	{
		m_eof = true;
	}
	return c;
}


short BinaryFileReader::ReadS16()
{
	int b1 = fgetc(m_file);
	int b2 = fgetc(m_file);
	
	if (b1 == EOF || b2 == EOF)
	{
		m_eof = true;
	}

	return ((b2 << 8) | b1);
}


int BinaryFileReader::ReadS32()
{
	int b1 = fgetc(m_file);
	int b2 = fgetc(m_file);
	int b3 = fgetc(m_file);
	int b4 = fgetc(m_file);

	if (b1 == EOF || b2 == EOF || b3 == EOF || b4 == EOF)
	{
		m_eof = true;
	}

	return ((b4 << 24) | (b3 << 16) | (b2 << 8) | b1);
}


unsigned int BinaryFileReader::ReadBytes(unsigned int _count, unsigned char *_buffer)
{
	size_t bytesRead = fread(_buffer, 1, _count, m_file);
	if (bytesRead < _count)
	{
		m_eof = true;
	}

	return (unsigned int) bytesRead;
}


int BinaryFileReader::Seek(int _offset, int _origin)
{
	return fseek(m_file, _offset, _origin);
}


int BinaryFileReader::Tell()
{
	return ftell(m_file);
}

int BinaryReader::GetSize()
{
	int now = Tell();
	Seek(0,SEEK_END);
	int size = Tell();
	Seek(now,SEEK_SET);
	return size;
}
