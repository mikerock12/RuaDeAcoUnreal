#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <shlobj.h>
#include <wrl.h>
#include <WebView2.h>
#include <WebView2EnvironmentOptions.h>
#include <shellapi.h>
#include <atomic>
#include <filesystem>
#include <string>
#include <thread>
#include <sstream>
#include "OnlineWindow.h"

namespace RDAOnline {
using Microsoft::WRL::ComPtr;
using Microsoft::WRL::Callback;
namespace {
std::atomic<HWND> onlineWindow=nullptr;
std::atomic<bool> starting=false;
std::atomic<bool> stopping=false;
int qaPort=0;
std::thread uiThread;
thread_local ComPtr<ICoreWebView2Controller> controller;
thread_local ComPtr<ICoreWebView2> web;
thread_local HWND statusLabel=nullptr;
std::wstring profileSettings;
constexpr wchar_t Origin[]=L"https://mikerock12.github.io/RuaDeAco/";
void Resize(HWND window) {
    RECT bounds{};GetClientRect(window,&bounds);
    MoveWindow(GetDlgItem(window,100),8,7,205,32,TRUE);
    MoveWindow(statusLabel,225,14,std::max(10L,bounds.right-235),22,TRUE);
    bounds.top=46;if(controller)controller->put_Bounds(bounds);
}
void Error(const wchar_t* text) { if(statusLabel)SetWindowTextW(statusLabel,text); }
LRESULT CALLBACK Procedure(HWND window,UINT message,WPARAM wp,LPARAM lp) {
    switch(message) {
        case WM_SIZE:Resize(window);return 0;
        case WM_COMMAND:
            if(LOWORD(wp)==100){DestroyWindow(window);return 0;}
            if(LOWORD(wp)==101){
                wchar_t executable[MAX_PATH]{};GetModuleFileNameW(nullptr,executable,MAX_PATH);
                auto installer=std::filesystem::path(executable).parent_path()/L"MicrosoftEdgeWebview2Setup.exe";
                if(std::filesystem::exists(installer))ShellExecuteW(window,L"open",installer.c_str(),L"/install",nullptr,SW_SHOWNORMAL);
                else ShellExecuteW(window,L"open",L"https://developer.microsoft.com/microsoft-edge/webview2",nullptr,nullptr,SW_SHOWNORMAL);
                return 0;
            }
            break;
        case WM_TIMER:DestroyWindow(window);return 0;
        case WM_CLOSE:DestroyWindow(window);return 0;
        case WM_DESTROY:if(controller)controller->Close();web.Reset();controller.Reset();onlineWindow=nullptr;PostQuitMessage(0);return 0;
    }
    return DefWindowProcW(window,message,wp,lp);
}
void Run(HWND parent) {
    HRESULT apartment=CoInitializeEx(nullptr,COINIT_APARTMENTTHREADED);
    if(FAILED(apartment)){starting=false;return;}
    WNDCLASSEXW type{sizeof(type)};type.lpfnWndProc=Procedure;type.hInstance=GetModuleHandleW(nullptr);
    type.lpszClassName=L"SLTRuaDeAcoOnline";type.hCursor=LoadCursor(nullptr,IDC_ARROW);
    type.hIcon=LoadIconW(type.hInstance,MAKEINTRESOURCEW(101));type.hIconSm=type.hIcon;
    type.hbrBackground=static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));RegisterClassExW(&type);
    HWND window=CreateWindowExW(0,type.lpszClassName,L"Rua de Aco - Online",WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,CW_USEDEFAULT,1280,800,nullptr,nullptr,type.hInstance,nullptr);
    if(!window){starting=false;CoUninitialize();return;}
    onlineWindow=window;starting=false;
    if(stopping){DestroyWindow(window);CoUninitialize();return;}
    if(qaPort)SetTimer(window,1,180000,nullptr);
    CreateWindowW(L"BUTTON",L"Voltar ao jogo offline",WS_CHILD|WS_VISIBLE,8,7,205,32,window,reinterpret_cast<HMENU>(100),type.hInstance,nullptr);
    statusLabel=CreateWindowW(L"STATIC",L"Carregando as salas oficiais...",WS_CHILD|WS_VISIBLE,225,14,800,22,window,nullptr,type.hInstance,nullptr);
    ShowWindow(window,SW_SHOW);UpdateWindow(window);ShowWindow(parent,SW_HIDE);
    PWSTR local=nullptr;SHGetKnownFolderPath(FOLDERID_LocalAppData,0,nullptr,&local);
    std::filesystem::path profile=local?local:L".";CoTaskMemFree(local);
    profile/=L"SmellsLikeTech/RuaDeAco/Online";
    if(qaPort)profile+=L"-QA-"+std::to_wstring(qaPort);
    auto options=Microsoft::WRL::Make<CoreWebView2EnvironmentOptions>();
    if(qaPort){std::wstring args=L"--remote-debugging-port="+std::to_wstring(qaPort);options->put_AdditionalBrowserArguments(args.c_str());}
    LPWSTR version=nullptr;
    if(FAILED(GetAvailableCoreWebView2BrowserVersionString(nullptr,&version))||!version){
        HWND prepare=CreateWindowW(L"BUTTON",L"Instalar componente online",WS_CHILD|WS_VISIBLE,20,90,300,44,window,reinterpret_cast<HMENU>(101),type.hInstance,nullptr);
        (void)prepare;
        Error(L"O modo online precisa do Microsoft WebView2. Instale o componente e abra Online novamente.");
    }
    CoTaskMemFree(version);
    const HRESULT setup=CreateCoreWebView2EnvironmentWithOptions(nullptr,profile.c_str(),options.Get(),
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>([window](HRESULT error,ICoreWebView2Environment* environment)->HRESULT {
            if(onlineWindow.load()!=window)return S_OK;
            if(FAILED(error)||!environment){Error(L"Online indisponivel. Verifique o Microsoft Edge WebView2 Runtime e a conexao.");return S_OK;}
            return environment->CreateCoreWebView2Controller(window,Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>([window](HRESULT result,ICoreWebView2Controller* created)->HRESULT {
                if(onlineWindow.load()!=window){if(created)created->Close();return S_OK;}
                if(FAILED(result)||!created){Error(L"Nao foi possivel abrir o online. Volte e tente novamente.");return S_OK;}
                controller=created;controller->get_CoreWebView2(&web);
                ComPtr<ICoreWebView2Settings> settings;web->get_Settings(&settings);
                settings->put_AreDefaultContextMenusEnabled(FALSE);settings->put_AreDevToolsEnabled(FALSE);settings->put_IsStatusBarEnabled(FALSE);
                EventRegistrationToken token{};
                web->add_NavigationStarting(Callback<ICoreWebView2NavigationStartingEventHandler>([](ICoreWebView2*,ICoreWebView2NavigationStartingEventArgs* args)->HRESULT {
                    LPWSTR uri=nullptr;args->get_Uri(&uri);std::wstring target=uri?uri:L"";CoTaskMemFree(uri);
                    if(target.rfind(Origin,0)!=0&&target!=L"about:blank")args->put_Cancel(TRUE);
                    return S_OK;
                }).Get(),&token);
                web->add_NewWindowRequested(Callback<ICoreWebView2NewWindowRequestedEventHandler>([](ICoreWebView2*,ICoreWebView2NewWindowRequestedEventArgs* args)->HRESULT {args->put_Handled(TRUE);return S_OK;}).Get(),&token);
                web->add_NavigationCompleted(Callback<ICoreWebView2NavigationCompletedEventHandler>([](ICoreWebView2*,ICoreWebView2NavigationCompletedEventArgs* args)->HRESULT {
                    BOOL success=FALSE;args->get_IsSuccess(&success);Error(success?L"Online oficial - escolha Online para criar ou entrar em uma sala":L"Falha de conexao. Confira sua internet e volte para tentar novamente.");return S_OK;
                }).Get(),&token);
                if(!profileSettings.empty())web->AddScriptToExecuteOnDocumentCreated(profileSettings.c_str(),nullptr);
                Resize(window);web->Navigate(Origin);return S_OK;
            }).Get());
        }).Get());
    if(FAILED(setup))Error(L"Online requer Microsoft Edge WebView2 Runtime. Instale o componente oficial da Microsoft.");
    MSG msg{};while(GetMessageW(&msg,nullptr,0,0)>0){TranslateMessage(&msg);DispatchMessageW(&msg);}
    statusLabel=nullptr;ShowWindow(parent,SW_SHOW);SetForegroundWindow(parent);CoUninitialize();
}
}
bool Active(){return starting.load()||onlineWindow.load()!=nullptr;}
bool Open(void* parent,float master,float music,float effects,float touchOpacity) {
    if(Active()){if(auto window=onlineWindow.load())SetForegroundWindow(window);return true;}
    if(uiThread.joinable())uiThread.join();
    std::wostringstream config;config<<L"(()=>{try{if(location.origin!=='https://mikerock12.github.io')return;let s=JSON.parse(localStorage.getItem('rua-de-aco:settings:v1')||'{}');Object.assign(s,{masterVolume:"<<master<<L",musicVolume:"<<music<<L",effectsVolume:"<<effects<<L",touchOpacity:"<<touchOpacity<<L",muted:"<<(master==0?L"true":L"false")<<L"});localStorage.setItem('rua-de-aco:settings:v1',JSON.stringify(s));}catch(e){}})();";
    profileSettings=config.str();stopping=false;starting=true;uiThread=std::thread(Run,static_cast<HWND>(parent));return true;
}
void Close(){stopping=true;if(auto window=onlineWindow.load())PostMessageW(window,WM_CLOSE,0,0);if(uiThread.joinable())uiThread.join();}
int Smoke(int port){if(port!=9223&&port!=9224)return 2;qaPort=port;Run(nullptr);return 0;}
}
