#define PLUGIN_EXPORTS
#include <windows.h>
#include <iostream>
#include "plugin_interface.h"

PLUGIN_API bool Plugin_Query() { return true; }
PLUGIN_API const char* Plugin_Name() { return "SamplePlugin"; }
PLUGIN_API void Plugin_Init(void* host) {
    MessageBoxA(nullptr, "SamplePlugin initialized","SFC", MB_OK);
}
PLUGIN_API void Plugin_Run() {
    MessageBoxA(nullptr, "SamplePlugin Run","SFC", MB_OK);
}
PLUGIN_API void Plugin_Shutdown() { }
