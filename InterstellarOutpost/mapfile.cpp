#include "pch.h"
#include "mapfile.h"
#include <fstream>
#include "app.h"
#include "bitmap.h"
#include "directory.h"
#include "location.h"
#include "network_stream.h"
#include "resource.h"
#include "text_stream_readers.h"

MapFile::MapFile()
  : m_levelDataLength(0),
    m_levelData(nullptr),
    m_filename(nullptr),
    m_landscapeTextureFilename(nullptr),
    m_thumbnailTextureFilename(nullptr),
    m_wavesTextureFilename(nullptr),
    m_waterTextureFilename(nullptr) {}

MapFile::MapFile(char* _filename, bool _loadTextures)
  : m_levelDataLength(0),
    m_levelData(nullptr),
    m_filename(nullptr),
    m_landscapeTextureFilename(nullptr),
    m_thumbnailTextureFilename(nullptr),
    m_wavesTextureFilename(nullptr),
    m_waterTextureFilename(nullptr)
{

  m_filename = _strdup(_filename);

  std::ifstream input(m_filename, std::ios::in | std::ios::binary);

  int length = 0;

  input.seekg(0, std::ios::end);
  length = input.tellg();
  input.seekg(0, std::ios::beg);

  int ucLen = 0;
  ReadNetworkValue<int>(input, ucLen);

  auto data = new char[length];
  input.read(data, length);

  auto directory = new Directory();
  directory->Read(data, length);

  input.close();

  auto dirData = static_cast<char*>(directory->GetDataVoid(MAPFILE_MAPDATA, &m_levelDataLength));
  m_levelData = new char[m_levelDataLength];
  memcpy(m_levelData, dirData, m_levelDataLength);

  if (_loadTextures)
    LoadTextures(directory);

  delete data;
  delete directory;
}

MapFile::~MapFile()
{
  if (m_filename) SAFE_DELETE(m_filename);
  if (m_levelData) SAFE_DELETE(m_levelData);
  if (m_landscapeTextureFilename) SAFE_DELETE(m_landscapeTextureFilename);
  if (m_wavesTextureFilename) SAFE_DELETE(m_wavesTextureFilename);
  if (m_waterTextureFilename) SAFE_DELETE(m_waterTextureFilename);
  if (m_thumbnailTextureFilename) SAFE_DELETE(m_thumbnailTextureFilename);
}

TextReader* MapFile::GetTextReader() { return new TextDataReader(m_levelData, m_levelDataLength, m_filename); }

bool MapFile::IsSaveRequired(char* _filename) { return true; }

void MapFile::LoadTextures(Directory* _directory)
{
  int num = 1;
  if (LoadTexture(_directory, MAPFILE_LANDSCAPETEX, num, &m_landscapeTextureFilename))
    num++;
  if (LoadTexture(_directory, MAPFILE_WAVESTEX, num, &m_wavesTextureFilename))
    num++;
  if (LoadTexture(_directory, MAPFILE_WATERTEX, num, &m_waterTextureFilename))
    num++;
  if (LoadTexture(_directory, MAPFILE_THUMBNAIL, num, &m_thumbnailTextureFilename))
    num++;
}

bool MapFile::LoadTexture(Directory* _directory, char* _texName, int _texNum, char** _newFilename)
{
  if (_directory->HasData(_texName))
  {
    char textureinfo[256];
    sprintf(textureinfo, "%s%d", MAPFILE_TEXTUREHEIGHT, _texNum);
    int height = _directory->GetDataInt(textureinfo);
    sprintf(textureinfo, "%s%d", MAPFILE_TEXTUREWIDTH, _texNum);
    int width = _directory->GetDataInt(textureinfo);

    if (width < 0 || height < 0)
      return true;

    auto bitmap = new BitmapRGBA(width, height);
    int size = 0;

    auto pixels = static_cast<RGBAColour*>(_directory->GetDataVoid(_texName, &size));
    memcpy(bitmap->m_pixels, pixels, size);
    // for( int i = 0; i < height * width; ++i )
    {
      // bitmap->m_pixels[i] = pixels[i];
    }

    char filenameIdentifier[512];
    sprintf(filenameIdentifier, "%sFilename", _texName);

    *_newFilename = _strdup(_directory->GetDataString(filenameIdentifier));
    g_app->m_resource->AddBitmap(*_newFilename, *bitmap);

    return true;
  }

  return false;
}
