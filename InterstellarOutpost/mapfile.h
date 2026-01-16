#pragma once

#include "game_menu.h"

#define MAPFILE_MAPDATA                 "MapData"
#define MAPFILE_LANDSCAPETEX            "MapLandscapeTexture"
#define MAPFILE_WAVESTEX                "MapWavesTexture"
#define MAPFILE_WATERTEX                "MapWaterTexture"
#define MAPFILE_TEXTUREWIDTH            "TextureWidth"
#define MAPFILE_TEXTUREHEIGHT           "TextureHeight"
#define MAPFILE_THUMBNAIL               "MapThumbnail"

class TextReader;

class MapFile
{
  public:
    int m_levelDataLength;
    char* m_levelData;
    char* m_filename;
    char* m_landscapeTextureFilename;
    char* m_thumbnailTextureFilename;
    char* m_wavesTextureFilename;
    char* m_waterTextureFilename;

    MapFile();
    MapFile(char* filename, bool _loadTextures = false); /* Filename must include the full path to the file */
    ~MapFile();

    TextReader* GetTextReader();

    void LoadTextures(Directory* directory); /* Pull out any textures from the Directory and stick them in the resource system */
    bool LoadTexture(Directory* _directory, char* _texName, int _texNum, char** _newFilename);

    bool IsSaveRequired(char* _filename);
};
