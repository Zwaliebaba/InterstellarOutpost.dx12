#pragma once

namespace Utility
{
  inline void Print(const char *msg)
  {
    OutputDebugStringA(msg);
  }

  inline void Print(const wchar_t *msg)
  {
    OutputDebugString(msg);
  }

  inline void Printf(const char *format, ...)
  {
    char buffer[256];
    va_list ap;
    va_start(ap, format);
    vsprintf_s(buffer, 256, format, ap);
    va_end(ap);
    Print(buffer);
  }

  inline void Printf(const wchar_t *format, ...)
  {
    wchar_t buffer[256];
    va_list ap;
    va_start(ap, format);
    vswprintf(buffer, 256, format, ap);
    va_end(ap);
    Print(buffer);
  }

#ifndef RELEASE
  inline void PrintSubMessage(const char *format, ...)
  {
    Print("--> ");
    char buffer[256];
    va_list ap;
    va_start(ap, format);
    vsprintf_s(buffer, 256, format, ap);
    va_end(ap);
    Print(buffer);
    Print("\n");
  }

  inline void PrintSubMessage(const wchar_t *format, ...)
  {
    Print("--> ");
    wchar_t buffer[256];
    va_list ap;
    va_start(ap, format);
    vswprintf(buffer, 256, format, ap);
    va_end(ap);
    Print(buffer);
    Print("\n");
  }

  inline void PrintSubMessage(void)
  {
  }
#endif

  std::wstring UTF8ToWideString(const std::string &str);
  std::string WideStringToUTF8(const std::wstring &wstr);
  std::string ToLower(const std::string &str);
  std::wstring ToLower(const std::wstring &str);
  std::string GetBasePath(const std::string &str);
  std::wstring GetBasePath(const std::wstring &str);
  std::string RemoveBasePath(const std::string &str);
  std::wstring RemoveBasePath(const std::wstring &str);
  std::string GetFileExtension(const std::string &str);
  std::wstring GetFileExtension(const std::wstring &str);
  std::string RemoveExtension(const std::string &str);
  std::wstring RemoveExtension(const std::wstring &str);

}// namespace Utility

#ifdef HALT
#undef HALT
#endif

#ifdef RELEASE

#define WARN_ONCE_IF(isTrue, ...) (void) (isTrue)
#define WARN_ONCE_IF_NOT(isTrue, ...) (void) (isTrue)

#else// !RELEASE

#define STRINGIFY(x) #x
#define STRINGIFY_BUILTIN(x) STRINGIFY(x)

#define WARN_ONCE_IF(isTrue, ...)                                                                                                                    \
  {                                                                                                                                                  \
    static bool s_TriggeredWarning = false;                                                                                                          \
    if ((bool) (isTrue) && !s_TriggeredWarning)                                                                                                      \
    {                                                                                                                                                \
      s_TriggeredWarning = true;                                                                                                                     \
      Utility::Print("\nWarning issued in " STRINGIFY_BUILTIN(__FILE__) " @ " STRINGIFY_BUILTIN(__LINE__) "\n");                                     \
      Utility::PrintSubMessage("\'" #isTrue "\' is true");                                                                                           \
      Utility::PrintSubMessage(__VA_ARGS__);                                                                                                         \
      Utility::Print("\n");                                                                                                                          \
    }                                                                                                                                                \
  }

#define WARN_ONCE_IF_NOT(isTrue, ...) WARN_ONCE_IF(!(isTrue), __VA_ARGS__)

#endif

#define BreakIfFailed(hr)                                                                                                                            \
  if (FAILED(hr)) __debugbreak()

void SIMDMemCopy(void *__restrict Dest, const void *__restrict Source, size_t NumQuadwords);
void SIMDMemFill(void *__restrict Dest, __m128 FillVector, size_t NumQuadwords);
