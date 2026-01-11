#include "pch.h"
#include "binary_stream_readers.h"
#include "bitmap.h"
#include "filesys_utils.h"
#include "text_file_writer.h"
#include "resource.h"
#include "shape.h"
#include "text_renderer.h"
#include "text_stream_readers.h"
#include "preferences.h"
#include "runnable.h"
#include "unicode_text_stream_reader.h"
#include "sound_stream_decoder.h"
#include "soundsystem.h"
#include "app.h"
#include "location.h"
#include "renderer.h"
#include "loading_screen.h"

Resource::Resource()
  : m_nameSeed(1),
    m_modName(nullptr) {}

Resource::~Resource()
{
  FlushOpenGlState();
  m_bitmaps.EmptyAndDelete();
  m_shapes.EmptyAndDelete();
#ifdef USE_SEPULVEDA_HELP_TUTORIAL
  m_gestureDemos.EmptyAndDelete();
#endif
}

void Resource::AddBitmap(const char* _name, const BitmapRGBA& _bmp)
{
  // Only insert if a bitmap with no other bitmap is already using that name
  if (m_bitmaps.GetIndex(_name) == -1)
  {
    auto bmpCopy = new BitmapRGBA(_bmp);
    m_bitmaps.PutData(_name, bmpCopy);
  }
}

void Resource::DeleteBitmap(const char* _name)
{
  int index = m_bitmaps.GetIndex(_name);
  if (index >= 0)
  {
    BitmapRGBA* bmp = m_bitmaps.GetData(index);
    delete bmp;
    m_bitmaps.RemoveData(index);
  }
}

const BitmapRGBA* Resource::GetBitmap(const char* _name)
{
  BitmapRGBA* bmp = m_bitmaps.GetData(_name);
  if (bmp)
    return bmp;

  // If we still didn't find it, try to load it from a file on the disk
  char fullPath[512];
  sprintf(fullPath, "%s", _name);
  strlwr(fullPath);
  BinaryReader* reader = GetBinaryReader(fullPath);

  if (reader)
  {
    const char* extension = GetExtensionPart(fullPath);
    auto bmp = new BitmapRGBA(reader, extension);
    m_bitmaps.PutData(_name, bmp);
    delete reader;
    return bmp;
  }

  return nullptr;
}

TextReader* Resource::GetTextReader(const std::string& _filename) { return GetTextReader(_filename.c_str()); }

TextReader* Resource::GetTextReader(const char* _filename)
{
  TextReader* reader = nullptr;
  char fullFilename[256];

  if (!reader)
  {
    sprintf(fullFilename, "%s%s", FileSys::GetHomeDirectoryA().c_str(), _filename);
    if (DoesFileExist(fullFilename))
    {
      reader = new UnicodeTextFileReader(fullFilename);

      if (reader && !reader->IsUnicode())
      {
        delete reader;
        reader = new TextFileReader(fullFilename);
      }
    }
  }

  if (!reader)
  {
    // LEANDER : Uncomment this. Well, not this line, the next line. Then remove this line.
    DebugTrace("Failed to find text resource: {}\n", fullFilename);
  }

  return reader;
}

BinaryReader* Resource::GetBinaryReader(const char* _filename)
{
  BinaryReader* reader = nullptr;
  char fullFilename[256];

  if (!reader)
  {
    sprintf(fullFilename, "%s%s", FileSys::GetHomeDirectoryA().c_str(), _filename);
    if (DoesFileExist(fullFilename))
      reader = new BinaryFileReader(fullFilename);
  }

  if (!reader)
  {
    DebugTrace("Failed to find binary resource: {}\n", fullFilename);
  }

  return reader;
}

Resource::TextureInfo::TextureInfo(int _id, int _width, int _height)
  : m_id(_id),
    m_width(_width),
    m_height(_height) {}

Resource::TextureInfo* Resource::ProcessBmp(BitmapRGBA& _bmp, int _flags)
{
  if (_flags & WithMask)
    _bmp.ConvertPinkToTransparent();

  if (_flags & WithAlpha)
    _bmp.ConvertColourToAlpha();

  bool withMipmaps = (_flags & WithMipmaps) != 0;
  bool withCompression = (_flags & WithCompression) != 0;

  return new TextureInfo(_bmp.ConvertToTextureAsync(withMipmaps, withCompression), _bmp.m_width, _bmp.m_height);
}

int Resource::GetTextureWithFlags(const char* _name, int _flags)
{
  bool loadingScreenRendering = g_loadingScreen->IsRendering();
  TextureInfo* texInfo = nullptr;

  if (loadingScreenRendering)
    m_mutex.Lock();

  // First lookup this name in the BTree of existing textures
  auto it = m_textures.find(_name);
  if (it != m_textures.end())
    texInfo = it->second;

  if (loadingScreenRendering)
    m_mutex.Unlock();

  // If the texture wasn't there, then look in our bitmap store
  if (!texInfo)
  {
    BitmapRGBA* bmp = m_bitmaps.GetData(_name);
    if (bmp)
      texInfo = ProcessBmp(*bmp, _flags);
  }

  // If we still didn't find it, try to load it from a file on the disk
  if (!texInfo)
  {
    char fullPath[512];
    sprintf(fullPath, "%s", _name);
    strlwr(fullPath);

    const char* extension = GetExtensionPart(fullPath);

    BinaryReader* reader = GetBinaryReader(fullPath);

    if (reader)
    {
      BitmapRGBA bmp(reader, extension);
      delete reader;

      texInfo = ProcessBmp(bmp, _flags);
    }
  }

  if (!texInfo)
  {
    char errorString[512];
    sprintf(errorString, "Failed to load texture {}", _name);
    ASSERT_TEXT(false, errorString);
  }

  // We're potentially leaking a texture here, but making this fully threadsafe is a bit of work,
  // due to ProcessBmp() pushing work to the main thread.
  if (loadingScreenRendering)
    m_mutex.Lock();
  m_textures[_name] = texInfo;
  if (loadingScreenRendering)
    m_mutex.Unlock();

  return texInfo->m_id;
}

int Resource::GetTexture(const char* _name, bool _mipMapping, bool _masked, bool _compressed)
{
  return GetTextureWithFlags(_name, (_mipMapping ? WithMipmaps : 0) | (_masked ? WithMask : 0) | (_compressed ? WithCompression : 0));
}

int Resource::GetTextureWithAlpha(const char* _name) { return GetTextureWithFlags(_name, WithAlpha | WithMipmaps); }

bool Resource::GetTextureInfo(const char* _name, int& _width, int& _height)
{
  auto i = m_textures.find(_name);

  if (i == m_textures.end())
    return false;

  TextureInfo* texInfo = i->second;
  _width = texInfo->m_width;
  _height = texInfo->m_height;

  return true;
}

bool Resource::DoesTextureExist(const char* _name)
{
  NetLockMutex lock(m_mutex);

  // First lookup this name in the BTree of existing textures
  auto i = m_textures.find(_name);
  if (i != m_textures.end())
    return true;

  // If the texture wasn't there, then look in our bitmap store
  BitmapRGBA* bmp = m_bitmaps.GetData(_name);
  if (bmp)
    return true;

  // If we still didn't find it, try to load it from a file on the disk
  char fullPath[512];
  sprintf(fullPath, "%s", _name);
  strlwr(fullPath);
  BinaryReader* reader = GetBinaryReader(fullPath);
  bool success = false;
  if (reader)
    success = true;
  delete reader;

  return success;
}

void Resource::DeleteTexture(const char* _name)
{
  auto i = m_textures.find(_name);
  if (i != m_textures.end())
  {
    TextureInfo* texInfo = i->second;
    GLuint id = texInfo->m_id;
    m_textures.erase(i);
    glDeleteTextures(1, &id);

    delete texInfo;
  }
}

class DeleteTextureByName : public Job
{
  public:
    DeleteTextureByName(Resource* _resource, const char* _name)
      : m_resource(_resource),
        m_name(_name) {}

  protected:
    void Run() override { m_resource->DeleteTexture(m_name); }

  private:
    Resource* m_resource;
    const char* m_name;
};

void Resource::DeleteTextureAsync(const char* _name)
{
  if (NetGetCurrentThreadId() == g_app->m_mainThreadId)
    DeleteTexture(_name);
  else
  {
    DeleteTextureByName t(this, _name);
    g_loadingScreen->QueueJob(&t);
    t.Wait();
  }
}

bool Resource::DoesShapeExist(const char* _filename)
{
  if (m_shapes.GetData(_filename))
    return true;

  char fullPath[512];
  sprintf(fullPath, "shapes/%s", _filename);
  strlwr(fullPath);
  if (DoesFileExist(fullPath))
    return true;

  return false;
}

void Resource::AddShape(Shape* _shape, const char* _name)
{
  if (!m_shapes.GetData(_name))
    m_shapes.PutData(_name, _shape);
}

Shape* Resource::GetShape(const char* _name, bool _makeNew)
{
  Shape* theShape = m_shapes.GetData(_name);

  // If we haven't loaded the shape before, or _makeNew is true, then
  // try to load it from the disk
  if (!theShape && _makeNew)
  {
    theShape = GetShapeCopy(_name, false);
    m_shapes.PutData(_name, theShape);
  }

  return theShape;
}

Shape* Resource::GetShapeCopy(const char* _name, bool _animating, bool _buildDisplayList)
{
  char fullPath[512];
  Shape* theShape = nullptr;

  sprintf(fullPath, "%sshapes\\%s", FileSys::GetHomeDirectoryA().c_str(), _name);
  strlwr(fullPath);
  if (DoesFileExist(fullPath))
    theShape = new Shape(fullPath, _animating, _buildDisplayList);

  ASSERT_TEXT(theShape, "Couldn't create shape file {}", _name);
  return theShape;
}

SoundStreamDecoder* Resource::GetSoundStreamDecoder(const char* _filename)
{
  char buf[256];
  sprintf(buf, "%s.wav", _filename);
  BinaryReader* binReader = GetBinaryReader(buf);

  if (!binReader || !binReader->IsOpen())
  {
    if (binReader)
      delete binReader;
    return nullptr;
  }

  auto ssd = new SoundStreamDecoder(binReader);
  if (!ssd)
    return nullptr;

  return ssd;
}

int Resource::CreateDisplayList(const char* _name)
{
  unsigned int id = glGenLists(1);

  // NULL _name indicates an anonymous display list
  if (_name)
  {
    // Make sure name isn't NULL and isn't too long
    DEBUG_ASSERT(strlen(_name) < 20);

    m_displayLists.PutData(_name, id);
  }

  return id;
}

int Resource::CreateDisplayList(const char* _name, const Runnable& _render, const Runnable& _before, const Runnable& _after)
{
  unsigned int id = CreateDisplayList(_name);

  _before.Run();

  glNewList(id, GL_COMPILE);
  _render.Run();
  glEndList();

  _after.Run();

  return id;
}

class DisplayListToCreate : public Job
{
  public:
    DisplayListToCreate(const char* _name, const Runnable& _render, const Runnable& _before, const Runnable& _after)
      : m_name(_name),
        m_render(_render),
        m_before(_before),
        m_after(_after),
        m_displayListId(-1) {}

    int WaitId()
    {
      Wait();
      return m_displayListId;
    }

  protected:
    void Run() override
    {
      //DebugTrace( "Creating Display List in Loading Thread\n" );
      m_displayListId = g_app->m_resource->CreateDisplayList(m_name, m_render, m_before, m_after);
    }

  private:
    const char* m_name;
    const Runnable& m_render;
    const Runnable& m_before;
    const Runnable& m_after;
    int m_displayListId;
};

int Resource::CreateDisplayListAsync(const char* _name, const Runnable& _render, const Runnable& _before, const Runnable& _after)
{
#ifdef USE_DIRECT3D_NOTTRUEANYMORE // JAKHACK
  // Our Direct3D implementation of display lists works nicely from any thread
  return CreateDisplayList(_name, _render, _before, _after);
#else
  if (NetGetCurrentThreadId() == g_app->m_mainThreadId)
    return CreateDisplayList(_name, _render, _before, _after);
  DisplayListToCreate t(_name, _render, _before, _after);
  g_loadingScreen->QueueJob(&t);
  return t.WaitId();
#endif
}

int Resource::GetDisplayList(const char* _name)
{
  // Make sure name isn't NULL and isn't too long
  DEBUG_ASSERT(_name && strlen(_name) < 20);

  return m_displayLists.GetData(_name, -1);
}

class DeleteDisplayListByName : public Job
{
  public:
    DeleteDisplayListByName(const char* _name)
      : m_name(_name) {}

  protected:
    void Run() override
    {
      //DebugTrace( "Deleting Display List in Loading Thread\n" );
      g_app->m_resource->DeleteDisplayList(m_name);
    }

  private:
    const char* m_name;
};

class DeleteDisplayListById : public Job
{
  public:
    DeleteDisplayListById(int _id)
      : m_id(_id) {}

  protected:
    void Run() override
    {
      //DebugTrace( "Deleting Display List in Loading Thread\n" );
      glDeleteLists(m_id, 1);
    }

  private:
    int m_id;
};

void Resource::DeleteDisplayListAsync(int _id)
{
#ifdef USE_DIRECT3D
  // Our Direct3D implementation of display lists works nicely from any thread
  glDeleteLists(_id, 1);
#else
  if (NetGetCurrentThreadId() == g_app->m_mainThreadId) { glDeleteLists(_id, 1); }
  else
  {
    DeleteDisplayListById t(_id);
    g_loadingScreen->QueueJob(&t);
    t.Wait();
  }
#endif
}

void Resource::DeleteDisplayListAsync(const char* _name)
{
#ifdef USE_DIRECT3D
  // Our Direct3D implementation of display lists works nicely from any thread
  DeleteDisplayList(_name);
#else
  if (NetGetCurrentThreadId() == g_app->m_mainThreadId) { DeleteDisplayList(_name); }
  else
  {
    DeleteDisplayListByName t(_name);
    g_loadingScreen->QueueJob(&t);
    t.Wait();
  }
#endif
}

void Resource::DeleteDisplayList(const char* _name)
{
  if (!_name)
    return;

  // Make sure name isn't too long
  DEBUG_ASSERT(strlen(_name) < 20);

  int id = m_displayLists.GetData(_name, -1);
  if (id >= 0)
  {
    glDeleteLists(id, 1);
    m_displayLists.RemoveData(_name);
  }
}

void Resource::FlushOpenGlState()
{
#if 1 // Try to catch crash on shutdown bug
  // Tell OpenGL to delete the textures
  for (auto i = m_textures.begin(); i != m_textures.end(); ++i)
  {
    TextureInfo* texInfo = i->second;
    GLuint id = texInfo->m_id;
    glDeleteTextures(1, &id);
    delete texInfo;
  }

  m_textures.clear();

#endif

  // Tell all the shapes to delete the display list
  for (int i = 0; i < m_shapes.Size(); ++i)
  {
    if (!m_shapes.ValidIndex(i))
      continue;

    m_shapes[i]->FlushDisplayList();
  }

  // Tell OpenGL to delete the display lists
  for (int i = 0; i < m_displayLists.Size(); ++i)
  {
    if (m_displayLists.ValidIndex(i))
      glDeleteLists(m_displayLists[i], 1);
  }

  // Forget all the display lists
  m_displayLists.Empty();

  if (g_app->m_location)
    g_app->m_location->FlushOpenGlState();
}

void Resource::RegenerateOpenGlState()
{
  // Tell the text renderers to reload their font
  if (g_editorFont.IsUnicode())
    g_editorFont.BuildUnicodeArray();
  else
    g_editorFont.BuildOpenGlState();

  if (g_gameFont.IsUnicode())
    g_gameFont.BuildUnicodeArray();
  else
    g_gameFont.BuildOpenGlState();

  g_titleFont.BuildUnicodeArray();

  // Tell all the shapes to generate a new display list
  for (int i = 0; i < m_shapes.Size(); ++i)
  {
    if (!m_shapes.ValidIndex(i))
      continue;

    m_shapes[i]->BuildDisplayList();
  }

  // Tell the renderer (for the pixel effect texture)
  g_app->m_renderer->BuildOpenGlState();

  // Tell the location
  if (g_app->m_location)
    g_app->m_location->RegenerateOpenGlState();

  // Get the loading screen Darwinian sprite texture id
  g_loadingScreen->m_texId = GetTexture("sprites/darwinian.bmp");

  Sphere::s_regenerateDisplayList = true;
}

char* Resource::GenerateName()
{
  int digits = log10f(m_nameSeed) + 1;
  auto name = new char [digits + 1];
  snprintf(name, digits + 1, "%d", m_nameSeed);
  m_nameSeed++;

  return name;
}

TextFileWriter* Resource::GetTextFileWriter(const char* _filename, bool _encrypt)
{
  char fullFilename[256];

  if (m_modName)
  {
    sprintf(fullFilename, "%smods/%s/%s", g_app->GetProfileDirectory(), m_modName, _filename);

    char* nextSlash = fullFilename;
    while (nextSlash = strchr(nextSlash, '/'))
    {
      *nextSlash = 0;
      bool result = CreateDirectory(fullFilename);
      ASSERT_TEXT(result, "Failed to write to %s", fullFilename);
      *nextSlash = '/';
      ++nextSlash;
    }

    return new TextFileWriter(fullFilename, _encrypt);
  }

  sprintf(fullFilename, "%s", _filename);
  return new TextFileWriter(fullFilename, _encrypt);
}

bool Resource::FileExists(const char* _file)
{
  BinaryReader* r = GetBinaryReader(_file);

  if (!r)
    return false;

  delete r;
  return true;
}

std::vector<std::string> Resource::ListResources(const char* _dir, const char* _filter, bool _longResults /* = true */)
{
  std::string fullDir = FileSys::GetHomeDirectoryA() + _dir;
  return ListDirectory(fullDir.c_str(), _filter, _longResults);
}
