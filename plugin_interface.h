#pragma once
#ifdef _WIN32
  #ifdef PLUGIN_EXPORTS
    #define PLUGIN_API extern "C" __declspec(dllexport)
  #else
    #define PLUGIN_API extern "C" __declspec(dllimport)
  #endif
#else
  #define PLUGIN_API extern "C"
#endif

PLUGIN_API bool Plugin_Query();
PLUGIN_API const char* Plugin_Name();
PLUGIN_API void Plugin_Init(void* host);
PLUGIN_API void Plugin_Run();
PLUGIN_API void Plugin_Shutdown();
