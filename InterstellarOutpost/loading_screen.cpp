#include "pch.h"
#include "resource.h"
#include "window_manager.h"
#include "text_renderer.h"
#include "preferences.h"
#include "input.h"
#include "soundsystem.h"
#include "loading_screen.h"
#include "app.h"
#include "renderer.h"
#include "user_input.h"
#include "game_menu.h"
#include "main.h"
#include "clienttoserver.h"
#include "server.h"

LoadingScreen* g_loadingScreen = NULL;
extern NetMutex g_openGLMutex;

static void SetWorkingThreadProcessor()
{
#if defined( WIN32 )
  SetThreadAffinityMask(GetCurrentThread(), 0x01 | 0x4 | 0x10 | 0x40); // Odd processors
#endif
}

LoadingScreen::LoadingScreen()
  : m_texId(-1),
    m_initialLoad(true),
    m_platformLogoShown(false),
    m_startTime(0.0),
    m_startFadeOutTime(0.0),
    m_fadeOutLogo(false)
{
  m_workQueue = new WorkQueue();

  // Set the loading thread going on a different processor
  m_workQueue->Add(SetWorkingThreadProcessor);
}

#ifdef USE_DIRECT3D
bool GeneratingMipmaps();
#endif

void LoadingScreen::RunJobs() { while (Job* j = TryDequeueJob()) { j->Process(); } }

bool LoadingScreen::IsRendering() const { return m_rendering; }

void LoadingScreen::RenderSpinningDarwinian()
{
  float w = 100;
  float h = 100;
  float x = -(w / 2);
  float y = -(h / 2);

  int screenW = g_app->m_renderer->ScreenW();
  int screenH = g_app->m_renderer->ScreenH();

  glColor4f(0.2f, 0.8f, 0.2f, 1.0f);
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, m_texId);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glLoadIdentity();
  glTranslatef(screenW - w, screenH - h, 0.0);

  // float angle = 18 * int( GetHighResTime() * 5 + 0.5 ) ;
  float angle = fmod(GetHighResTime() * 90.0, 360.0);

  glRotatef(angle, 0.0f, 0.0f, 1.0f);

  glBegin(GL_QUADS);
  glTexCoord2i(0, 1);
  glVertex2f(x, y);
  glTexCoord2i(1, 1);
  glVertex2f(x + w, y);
  glTexCoord2i(1, 0);
  glVertex2f(x + w, y + h);
  glTexCoord2i(0, 0);
  glVertex2f(x, y + h);
  glEnd();

  glDisable(GL_TEXTURE_2D);
  //glEnable( GL_DEPTH_TEST ); 
  //glDepthMask ( true );
  glEnable(GL_CULL_FACE);
  glDisable(GL_BLEND);
}

void LoadingScreen::RenderIVLogo(float _alpha)
{
  glEnable(GL_TEXTURE_2D);
  glColor4f(1.0f, 1.0f, 1.0f, _alpha);
  glBindTexture(GL_TEXTURE_2D, g_app->m_resource->GetTexture("textures\\ivlogo." TEXTURE_EXTENSION));
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  int screenW = g_app->m_renderer->ScreenW();
  int screenH = g_app->m_renderer->ScreenH();

  float scale = static_cast<float>(screenH) / 1024.0f;

  float w = 500 * scale;
  float h = 500 * scale;
  float x = (screenW / 2.0f) - (w / 2.0f);
  float y = (screenH / 2.0f) - (h / 2.0f);

  glBegin(GL_QUADS);
  glTexCoord2i(0, 1);
  glVertex2f(x, y);
  glTexCoord2i(1, 1);
  glVertex2f(x + w, y);
  glTexCoord2i(1, 0);
  glVertex2f(x + w, y + h);
  glTexCoord2i(0, 0);
  glVertex2f(x, y + h);
  glEnd();

  glDisable(GL_TEXTURE_2D);
}

void LoadingScreen::RenderFrame()
{
  NetLockMutex lock(g_openGLMutex);

  if (m_texId != -1)
  {
    if (m_startTime == 0.0)
      m_startTime = GetHighResTime();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    if (!m_initialLoad && !g_app->m_loadingLocation && g_app->m_atMainMenu && g_app->m_renderer->IsFadeComplete())
    {
      g_app->m_gameMenu->Render();
      g_app->m_userInput->Render();
    }

    g_editorFont.BeginText2D();
    glEnable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    //glDisable   ( GL_DEPTH_TEST );
    // glDepthMask ( false );

    constexpr float platformLogoFadeOutDuration = 1.0f;

#if defined(TARGET_OS_MACOSX)
    const float platformLogoDuration = 5.0f; const double ivLogoStartTime = platformLogoDuration;
#else
    constexpr float platformLogoDuration = 0.0f;
    constexpr double ivLogoStartTime = 0.0f;
#endif
    constexpr float ivLogoFadeInDuration = 1.0f;
    constexpr float platformLogoFadeInDuration = 1.0f;

    double elapsedTime = GetHighResTime() - m_startTime;

    float alpha = elapsedTime - platformLogoDuration;
    alpha = min(alpha, 1.0f);
    if (m_initialLoad && alpha > 0.0f)
    {
      if (elapsedTime > ivLogoStartTime)
      {
        alpha = static_cast<float>(elapsedTime - ivLogoStartTime) / ivLogoFadeInDuration;
        alpha = min(alpha, 1.0f);
      }

      if (m_workQueue->Empty() && !JobsToDo())
      {
        if (m_startFadeOutTime == 0.0)
          m_startFadeOutTime = GetHighResTime();
        alpha = 1.0f - (GetHighResTime() - m_startFadeOutTime);

        if (alpha <= 0.0f)
          m_fadeOutLogo = false;
      }

      RenderIVLogo(alpha);
    }

#if defined(TARGET_OS_MACOSX)
    if (elapsedTime < platformLogoDuration && m_initialLoad && !m_platformLogoShown)
    {
      const float platformLogoFadeOutStartTime = platformLogoDuration - platformLogoFadeOutDuration;

      if (elapsedTime > platformLogoFadeOutStartTime)
        alpha = 1.0f - (elapsedTime - platformLogoFadeOutStartTime) / platformLogoFadeOutDuration;
      else
      {
        alpha = elapsedTime / platformLogoFadeInDuration;
        alpha = min(alpha, 1.0f);
      }
    } if (elapsedTime > platformLogoDuration)
      m_platformLogoShown = true;
#endif

#if defined(TARGET_OS_MACOSX)
    if (!m_initialLoad || elapsedTime > platformLogoDuration) { RenderSpinningDarwinian(); }
#else
    RenderSpinningDarwinian();
#endif

    g_editorFont.EndText2D();

    g_windowManager->Flip();
  }
  else
  {
    char* filename = "sprites/darwinian.bmp";

    if (g_app->m_resource->DoesTextureExist(filename))
      m_texId = g_app->m_resource->GetTexture(filename);
  }
}

void LoadingScreen::Render()
{
  m_rendering = true;

  bool transition = false;
  //m_startTime = GetHighResTime();

  m_fadeOutLogo = false;
  if (m_initialLoad)
    m_fadeOutLogo = true;

  m_startFadeOutTime = 0.0;

  while (!m_workQueue->Empty() || JobsToDo() || (g_app->m_requireSoundsLoaded && !g_app->m_soundsLoaded) || m_fadeOutLogo
#ifdef USE_DIRECT3D
        || GeneratingMipmaps()
#endif
  )
  {
    UpdateAdvanceTime();
    double frameStartTime = GetHighResTime();

    if (g_inputManager)
      g_inputManager->PollForEvents();
    if (g_app && g_app->m_resource && g_app->m_renderer)
    {
      RenderFrame();

      // Process the sound system
      if (g_app->m_soundSystem && g_app->m_soundSystem->IsInitialized())
        g_app->m_soundSystem->Advance();

      // Process the network
      if (g_app->m_networkMutex.TryLock())
      {
        if (g_app->m_clientToServer)
          g_app->m_clientToServer->KeepAlive();

        if (g_app->m_server)
          g_app->m_server->AdvanceIfNecessary(GetHighResTime());

        g_app->m_networkMutex.Unlock();
      }

      // Do a job
      Job* t = TryDequeueJob();
      if (t != NULL)
        t->Process();

      double frameEndTime = frameStartTime + 1.0 / 60.0;

      // Or two

      while (GetHighResTime() < frameEndTime)
      {
        Job* t = TryDequeueJob();
        if (t != NULL)
          t->Process();
        else
          break;
      }

      double timeRemaining = frameEndTime - GetHighResTime();

      if (timeRemaining > 0.0)
        Sleep(timeRemaining * 1000.0 /* Milliseconds */);
    }
  }
  m_rendering = false;
  m_startTime = 0.0;
}

Job* LoadingScreen::TryDequeueJob()
{
  NetLockMutex lock(m_lock);
  if (m_jobs.Size() > 0)
  {
    Job* result = m_jobs[0];
    m_jobs.RemoveData(0);
    return result;
  }
  return NULL;
}

bool LoadingScreen::JobsToDo()
{
  NetLockMutex lock(m_lock);
  return m_jobs.Size() > 0;
}

void LoadingScreen::QueueJob(Job* _t)
{
  NetLockMutex lock(m_lock);
  m_jobs.PutData(_t);
}
