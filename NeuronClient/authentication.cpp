#include "pch.h"
#include "authentication.h"
#include "HashData.h"
#include "directory.h"
#include "llist.h"
#include "net_mutex.h"

static char s_authKey[256];

class AuthenticationResult
{
  public:
    char m_ip[256];
    char m_authKey[256];
    int m_keyId;
    int m_authResult;
    int m_numTries;

    AuthenticationResult()
      : m_keyId(-1),
        m_authResult(AuthenticationUnknown),
        m_numTries(0) {}
};

static LList<AuthenticationResult*> s_authResults;
static NetMutex s_authResultsMutex;

static bool s_authThreadRunning = false;
static int s_hashToken = 0;

void Authentication_GenerateKey(char* _key, int _gameType /* 2 = Defcon, 4 = Multiwinia, ... */, bool _demoKey, bool _steamKey)
{
  strcpy(_key, "DEMOTP-AXYVLW-SGIZLU-DNLXPK-OJS");
}

void Authentication_SaveKey(const char* _key, const char* _filename)
{
  FILE* file = fopen(_filename, "wt");

  if (!file)
  {
    DebugTrace("Failed to save AuthKey");
    return;
  }

  fprintf(file, "%s\n", _key);
  fclose(file);
}

void Authentication_LoadKey(char* _key, const char* _filename)
{
  FILE* file = fopen(_filename, "rt");
  if (file)
  {
    fscanf(file, "%s", _key);
    fclose(file);
  }
  else
  {
    DebugTrace("Failed to load AuthKey : '%s'\n", _filename);
    sprintf(_key, "authkey not found");
  }
}

void Authentication_SetKey(const char* _key)
{
  strcpy(s_authKey, _key);

  // Trim end whitespace
  // In fact trim anything not valid from the end

  int keyLen = strlen(s_authKey);
  for (int i = keyLen - 1; i >= 0; --i)
  {
    if ((s_authKey[i] >= 'a' && s_authKey[i] <= 'z') || (s_authKey[i] >= 'A' && s_authKey[i] <= 'Z') || s_authKey[i] == '-')
      break;
    s_authKey[i] = '\x0';
  }

  DebugTrace("Authentication key set : '{}'\n", s_authKey);
}

bool Authentication_IsKeyFound()
{
  return true;
}

void Authentication_GetKey(char* _key)
{
  if (Authentication_IsKeyFound())
    strcpy(_key, s_authKey);
  else
    strcpy(_key, "authkey not found");
}

void Authentication_SetHashToken(int _hashToken)
{
  if (_hashToken != s_hashToken)
    DebugTrace("Authentication HashToken set to %d\n", _hashToken);

  s_hashToken = _hashToken;
}

int Authentication_GetHashToken() { return s_hashToken; }

void Authentication_GetKeyHash(const char* _sourceKey, char* _destKey, int _hashToken) { HashData(_sourceKey, _hashToken, _destKey); }

bool Authentication_IsHashKey(const char* _key) { return (strncmp(_key, "hsh", 3) == 0); }

void Authentication_GetKeyHash(char* _key)
{
  char keyTemp[256];
  Authentication_GetKey(keyTemp);
  Authentication_GetKeyHash(keyTemp, _key, s_hashToken);
}

static NetCallBackRetType AuthenticationThread(void* _args)
{
  return 0;
}

int Authentication_GetKeyId(const char* _key)
{
  int result = 0;

  s_authResultsMutex.Lock();

  for (int i = 0; i < s_authResults.Size(); ++i)
  {
    AuthenticationResult* authResult = s_authResults[i];
    if (strcmp(authResult->m_authKey, _key) == 0)
    {
      result = authResult->m_keyId;
      break;
    }
  }

  s_authResultsMutex.Unlock();

  return result;
}

void Authentication_RequestStatus(const char* _key, int _gameType, int _keyId, const char* _ip)
{
  //
  // Does the key already exist in our list?

  s_authResultsMutex.Lock();

  AuthenticationResult* result = nullptr;

  for (int i = 0; i < s_authResults.Size(); ++i)
  {
    AuthenticationResult* authResult = s_authResults[i];
    if (strcmp(authResult->m_authKey, _key) == 0)
    {
      result = authResult;
      break;
    }
  }

  //
  // Add the key to our list if required

  if (!result)
  {
    result = new AuthenticationResult();
    strcpy(result->m_authKey, _key);
    result->m_keyId = _keyId;
    if (_ip)
      strcpy(result->m_ip, _ip);
    else
      strcpy(result->m_ip, "unknown");

    s_authResults.PutData(result);
  }

  s_authResultsMutex.Unlock();

  //
  // Start the authorisation thread if required

  if (!s_authThreadRunning)
  {
    s_authThreadRunning = true;
    NetStartThread(AuthenticationThread, (void*)_gameType);
  }
}

int Authentication_GetStatus(const char* _key)
{
  int result = AuthenticationUnknown;

  s_authResultsMutex.Lock();

  for (int i = 0; i < s_authResults.Size(); ++i)
  {
    AuthenticationResult* authResult = s_authResults[i];
    if (strcmp(authResult->m_authKey, _key) == 0)
    {
      result = authResult->m_authResult;
      break;
    }
  }

  s_authResultsMutex.Unlock();

  return result;
}

void Authentication_SetStatus(const char* _key, int _keyId, int _status)
{
  s_authResultsMutex.Lock();

  AuthenticationResult* result = nullptr;

  for (int i = 0; i < s_authResults.Size(); ++i)
  {
    AuthenticationResult* authResult = s_authResults[i];
    if (strcmp(authResult->m_authKey, _key) == 0)
    {
      result = authResult;
      break;
    }
  }

  if (!result)
  {
    result = new AuthenticationResult();
    strcpy(result->m_authKey, _key);
    s_authResults.PutData(result);
  }

  if (result->m_authResult != _status)
  {
    const char* authString = Authentication_GetStatusString(_status);
    DebugTrace("Received Authentication : %s : (keyID %d) : %s\n", _key, _keyId, authString);
  }

  result->m_authResult = _status;
  result->m_keyId = _keyId;

  s_authResultsMutex.Unlock();
}

const char* Authentication_GetStatusString(int _status)
{
  switch (_status)
  {
  case AuthenticationUnknownGame:
    return "Unknown Game";
  case AuthenticationWrongPlatform:
    return "Wrong platform";
  case AuthenticationKeyDead:
    return "Key dead";
  case AuthenticationVersionNotFound:
    return "Version not found";
  case AuthenticationKeyInactive:
    return "Key inactive";
  case AuthenticationVersionRevoked:
    return "Version revoked";
  case AuthenticationKeyNotFound:
    return "Key not found";
  case AuthenticationKeyRevoked:
    return "Key revoked";
  case AuthenticationKeyBanned:
    return "Key banned";
  case AuthenticationKeyInvalid:
    return "Key invalid";
  case AuthenticationUnknown:
    return "Unknown";
  case AuthenticationAccepted:
    return "Accepted";
  default:
    return "Unknown";
  }
}

const char* Authentication_GetStatusStringLanguagePhrase(int _status)
{
  switch (_status)
  {
  case AuthenticationUnknownGame:
    return "dialog_auth_unknown_game";
  case AuthenticationWrongPlatform:
    return "dialog_auth_wrong_platform";
  case AuthenticationKeyDead:
    return "dialog_auth_key_dead";
  case AuthenticationVersionNotFound:
    return "dialog_auth_version_not_found";
  case AuthenticationKeyInactive:
    return "dialog_auth_key_inactive";
  case AuthenticationVersionRevoked:
    return "dialog_auth_version_revoked";
  case AuthenticationKeyNotFound:
    return "dialog_auth_key_not_found";
  case AuthenticationKeyRevoked:
    return "dialog_auth_key_revoked";
  case AuthenticationKeyBanned:
    return "dialog_auth_key_banned";
  case AuthenticationKeyInvalid:
    return "dialog_auth_key_invalid";
  case AuthenticationUnknown:
    return "dialog_auth_unknown";
  case AuthenticationAccepted:
    return "dialog_auth_accepted";
  default:
    return "dialog_auth_unknown";
  }
}
