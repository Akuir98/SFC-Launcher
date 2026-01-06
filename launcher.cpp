#define UNICODE
#define _UNICODE
#include <windows.h>
#include <shlwapi.h>
#include <string>
#include <vector>
#include <filesystem>
#include <iostream>
#include <fstream>

#pragma comment(lib, "shlwapi.lib")
namespace fs = std::filesystem;

#include "plugin_interface.h"

fs::path plugins_dir;

std::string ini_read_value(const fs::path& p, const std::string& key) {
    std::ifstream f(p);
    if(!f) return {};
    std::string line;
    while(std::getline(f, line)) {
        auto pos = line.find('=');
        if(pos==std::string::npos) continue;
        std::string k = line.substr(0,pos);
        if(k==key) return line.substr(pos+1);
    }
    return {};
}

void list_plugins() {
    if(!fs::exists(plugins_dir)) { std::cout<<"No plugins folder\n"; return; }
    for(auto &d: fs::directory_iterator(plugins_dir)) {
        if(d.is_directory()){
            auto ini = d.path()/ "plugin.ini";
            std::string name = d.path().filename().string();
            if(fs::exists(ini)) {
                auto n = ini_read_value(ini, "name");
                if(!n.empty()) name = n;
            }
            std::cout<<name<<"\n";
        }
    }
}

bool load_and_run_plugin(const std::string& pluginFolder) {
    fs::path dll = plugins_dir / pluginFolder / "plugin.dll";
    if(!fs::exists(dll)) {
        std::cout<<"plugin.dll not found\n"; return false;
    }
    HMODULE h = LoadLibraryW(dll.wstring().c_str());
    if(!h) { std::cout<<"LoadLibrary failed\n"; return false; }
    auto q = (bool(*)())GetProcAddress(h, "Plugin_Query");
    if(q && !q()) { std::cout<<"Plugin query failed\n"; FreeLibrary(h); return false; }
    auto run = (void(*)())GetProcAddress(h, "Plugin_Run");
    auto init = (void(*)(void*))GetProcAddress(h, "Plugin_Init");
    if(init) init(nullptr);
    if(run) run();
    if(auto sd = (void(*)())GetProcAddress(h, "Plugin_Shutdown")) sd();
    FreeLibrary(h);
    return true;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp){
    switch(msg){
    case WM_PAINT: {
        PAINTSTRUCT ps; HDC hdc = BeginPaint(hwnd,&ps);
        RECT r; GetClientRect(hwnd, &r);
        FillRect(hdc, &r, (HBRUSH)(COLOR_WINDOW+1));
        EndPaint(hwnd,&ps);
        break;
    }
    case WM_KEYDOWN:
        if(wp == 'L') {
            list_plugins();
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0); break;
    default: return DefWindowProc(hwnd,msg,wp,lp);
    }
    return 0;
}

int wmain(int argc, wchar_t** argv) {
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    fs::path exe = fs::path(path).parent_path();
    plugins_dir = exe / "plugins";

    if(argc>1){
        std::wstring cmd = argv[1];
        if(cmd == L"list") { list_plugins(); return 0; }
        if(cmd == L"addplug" && argc>2) {
            std::string name;
            int len = WideCharToMultiByte(CP_UTF8,0,argv[2],-1,nullptr,0,nullptr,nullptr);
            name.assign(len,'\0'); WideCharToMultiByte(CP_UTF8,0,argv[2],-1,name.data(),len,nullptr,nullptr);
            fs::create_directories(plugins_dir / name);
            std::ofstream((plugins_dir / name / "plugin.ini").string())<<"name="<<name<<"\nauthor=unknown\ndesc=sample\n";
            std::cout<<"Plugin folder created\n";
            return 0;
        }
        if(cmd == L"delplug" && argc>2){
            std::string name;
            int len = WideCharToMultiByte(CP_UTF8,0,argv[2],-1,nullptr,0,nullptr,nullptr);
            name.assign(len,'\0'); WideCharToMultiByte(CP_UTF8,0,argv[2],-1,name.data(),len,nullptr,nullptr);
            fs::remove_all(plugins_dir / name);
            std::cout<<"Removed: "<<name<<"\n";
            return 0;
        }
        if(cmd == L"startplug" && argc>2){
            std::string name;
            int len = WideCharToMultiByte(CP_UTF8,0,argv[2],-1,nullptr,0,nullptr,nullptr);
            name.assign(len,'\0'); WideCharToMultiByte(CP_UTF8,0,argv[2],-1,name.data(),len,nullptr,nullptr);
            load_and_run_plugin(name);
            return 0;
        }
    }

    HINSTANCE hInst = GetModuleHandle(nullptr);
    const wchar_t cls[] = L"SFCLauncherClass";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = cls;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, cls, L"SFC Launcher - White UI", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, nullptr, nullptr, hInst, nullptr);
    ShowWindow(hwnd, SW_SHOW);

    MSG msg;
    while(GetMessage(&msg,nullptr,0,0) > 0) {
        TranslateMessage(&msg); DispatchMessage(&msg);
    }
    return 0;
}
