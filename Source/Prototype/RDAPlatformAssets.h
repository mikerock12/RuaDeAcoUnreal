#pragma once
#include "raylib.h"
#include <cstring>
#include <filesystem>
#include <vector>
#if defined(PLATFORM_ANDROID)
#include <android_native_app_glue.h>
#include <android/asset_manager.h>
extern "C" struct android_app* GetAndroidApp(void);
#endif

namespace RDAAssets {
#if defined(PLATFORM_ANDROID)
inline unsigned char* LoadBinary(const char* name, int* size) {
    *size=0;
    auto* asset=AAssetManager_open(GetAndroidApp()->activity->assetManager,name,AASSET_MODE_STREAMING);
    if(!asset) { TraceLog(LOG_ERROR,"RDA_ASSET_MISSING: %s",name); return nullptr; }
    const auto length=AAsset_getLength64(asset);
    if(length<=0 || length>0x7ffffffe) { AAsset_close(asset); return nullptr; }
    auto* data=static_cast<unsigned char*>(MemAlloc(static_cast<unsigned int>(length)));
    int read=0;
    while(data && read<length) {
        const int count=AAsset_read(asset,data+read,static_cast<size_t>(length-read));
        if(count<=0) break;
        read+=count;
    }
    AAsset_close(asset);
    if(read!=length) { if(data) MemFree(data); return nullptr; }
    *size=read; return data;
}
inline char* LoadText(const char* name) {
    int size=0; auto* bytes=LoadBinary(name,&size); if(!bytes) return nullptr;
    auto* text=static_cast<char*>(MemAlloc(static_cast<unsigned int>(size+1)));
    if(text) { std::memcpy(text,bytes,size); text[size]='\0'; }
    MemFree(bytes); return text;
}
#endif
inline void Initialize() {
#if defined(PLATFORM_ANDROID)
    SetLoadFileDataCallback(LoadBinary); SetLoadFileTextCallback(LoadText);
#endif
}
inline bool Exists(const std::filesystem::path& path) {
#if defined(PLATFORM_ANDROID)
    auto* asset=AAssetManager_open(GetAndroidApp()->activity->assetManager,path.generic_string().c_str(),AASSET_MODE_UNKNOWN);
    if(!asset) return false;
    AAsset_close(asset); return true;
#else
    return FileExists(path.generic_string().c_str());
#endif
}
inline Music LoadMusic(const std::filesystem::path& path, std::vector<unsigned char>& backing) {
#if defined(PLATFORM_ANDROID)
    int size=0; auto* data=LoadBinary(path.generic_string().c_str(),&size);
    if(!data) return Music{};
    backing.assign(data,data+size); MemFree(data);
    return LoadMusicStreamFromMemory(".ogg",backing.data(),static_cast<int>(backing.size()));
#else
    (void)backing;
    return LoadMusicStream(path.generic_string().c_str());
#endif
}
}
