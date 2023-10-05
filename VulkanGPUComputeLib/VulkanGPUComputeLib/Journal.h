#pragma once
#include <fstream>
#include <chrono>
#include <ctime>
#include <string>
#include "json.hpp"
#include <filesystem>

//https://stackoverflow.com/questions/4804298/how-to-convert-wstring-into-string

// WINDOWS
#if (_WIN32)
#include <Windows.h>
#include <conio.h>
#define WINDOWS_PLATFORM 1
#define DLLCALL STDCALL
#define DLLIMPORT _declspec(dllimport)
#define DLLEXPORT _declspec(dllexport)
#define DLLPRIVATE
#define NOMINMAX

//EMSCRIPTEN
#elif defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>
#include <emscripten/bind.h>
#include <unistd.h>
#include <termios.h>
#define EMSCRIPTEN_PLATFORM 1
#define DLLCALL
#define DLLIMPORT
#define DLLEXPORT __attribute__((visibility("default")))
#define DLLPRIVATE __attribute__((visibility("hidden")))

// LINUX - Ubuntu, Fedora, , Centos, Debian, RedHat
#elif (__LINUX__ || __gnu_linux__ || __linux__ || __linux || linux)
#define LINUX_PLATFORM 1
#include <unistd.h>
#include <termios.h>
#define DLLCALL CDECL
#define DLLIMPORT
#define DLLEXPORT __attribute__((visibility("default")))
#define DLLPRIVATE __attribute__((visibility("hidden")))
#define CoTaskMemAlloc(p) malloc(p)
#define CoTaskMemFree(p) free(p)

//ANDROID
#elif (__ANDROID__ || ANDROID)
#define ANDROID_PLATFORM 1
#define DLLCALL
#define DLLIMPORT
#define DLLEXPORT __attribute__((visibility("default")))
#define DLLPRIVATE __attribute__((visibility("hidden")))

//MACOS
#elif defined(__APPLE__)
#include <unistd.h>
#include <termios.h>
#define DLLCALL
#define DLLIMPORT
#define DLLEXPORT __attribute__((visibility("default")))
#define DLLPRIVATE __attribute__((visibility("hidden")))
#include "TargetConditionals.h"
#if TARGET_OS_IPHONE && TARGET_IPHONE_SIMULATOR
#define IOS_SIMULATOR_PLATFORM 1
#elif TARGET_OS_IPHONE
#define IOS_PLATFORM 1
#elif TARGET_OS_MAC
#define MACOS_PLATFORM 1
#else

#endif

#endif


class Journal
{
public:
	
    static void AddWarningEntry(std::wstring fileName, std::wstring warningCode, std::wstring warningMessage);
    static void AddWarningEntry(std::wstring fileLocation, std::wstring fileName, std::wstring warningCode, std::wstring warningMessage);
    static void AddWarningEntry(std::string fileName, std::string warningCode, std::string warningMessage);
    static void AddWarningEntry(std::string fileLocation, std::string fileName, std::string warningCode, std::string warningMessage);

	static void AddErrorEntry(std::wstring fileName, std::wstring errorCode, std::wstring errorMessage);
    static void AddErrorEntry(std::wstring fileLocation, std::wstring fileName, std::wstring errorCode, std::wstring errorMessage);
    static void AddErrorEntry(std::string fileName, std::string errorCode, std::string errorMessage);
    static void AddErrorEntry(std::string fileLocation, std::string fileName, std::string errorCode, std::string errorMessage);

	static void AddMessage(std::wstring fileName, std::wstring msg);
	static void AddMessage(std::wstring fileLocation, std::wstring fileName, std::wstring msg);
	static void AddMessage(std::string fileName, std::string msg);
	static void AddMessage(std::string fileLocation, std::string fileName, std::string msg);

    static void AddPerformaceEntry(std::wstring fileName, std::wstring testName, int testNumber, double timeTaken);
    static void AddPerformaceEntry(std::wstring fileLocation, std::wstring fileName, std::wstring testName, int testNumber, double timeTaken);


    static void CopyToFile(nlohmann::json& outData, std::wstring fileLocation, std::wstring fileName);

private:

    static nlohmann::json CreateNewJournalFile(std::filesystem::path& path);

   static nlohmann::json OpenFile(std::wstring& fileLocation, std::wstring& fileName);


    static std::string WideStringToString(const std::wstring& wstr)
    {
        if (wstr.empty())
        {
            return std::string();
        }
        size_t pos;
        size_t begin = 0;
        std::string ret;

#if WINDOWS_PLATFORM
        int size;
        pos = wstr.find(static_cast<wchar_t>(0), begin);
        while (pos != std::wstring::npos && begin < wstr.length())
        {   
            std::wstring segment = std::wstring(&wstr[begin], pos - begin);
            size = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, &segment[0], segment.size(), NULL, 0, NULL, NULL);
            std::string converted = std::string(size, 0);
            WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, &segment[0], segment.size(), &converted[0], converted.size(), NULL, NULL);
            ret.append(converted);
            ret.append({ 0 });
            begin = pos + 1;
            pos = wstr.find(static_cast<wchar_t>(0), begin);
        }
        if (begin <= wstr.length())
        {
            std::wstring segment = std::wstring(&wstr[begin], wstr.length() - begin);
            size = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, &segment[0], segment.size(), NULL, 0, NULL, NULL);
            std::string converted = std::string(size, 0);
            WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, &segment[0], segment.size(), &converted[0], converted.size(), NULL, NULL);
            ret.append(converted);
        }
#elif LINUX_PLATFORM || MACOS_PLATFORM || EMSCRIPTEN_PLATFORM
        size_t size;
        pos = wstr.find(static_cast<wchar_t>(0), begin);
        while (pos != std::wstring::npos && begin < wstr.length())
        {
            std::wstring segment = std::wstring(&wstr[begin], pos - begin);
            size = wcstombs(nullptr, segment.c_str(), 0);
            std::string converted = std::string(size, 0);
            wcstombs(&converted[0], segment.c_str(), converted.size());
            ret.append(converted);
            ret.append({ 0 });
            begin = pos + 1;
            pos = wstr.find(static_cast<wchar_t>(0), begin);
        }
        if (begin <= wstr.length())
        {
            std::wstring segment = std::wstring(&wstr[begin], wstr.length() - begin);
            size = wcstombs(nullptr, segment.c_str(), 0);
            std::string converted = std::string(size, 0);
            wcstombs(&converted[0], segment.c_str(), converted.size());
            ret.append(converted);
        }
#else
        static_assert(false, "Unknown Platform");
#endif
        return ret;
    }

    static  std::wstring StringToWideString(const std::string& str)
    {
        if (str.empty())
        {
            return  std::wstring();
        }

        size_t pos;
        size_t begin = 0;
        std::wstring ret;
#ifdef WINDOWS_PLATFORM
        int size = 0;
        pos = str.find(static_cast<char>(0), begin);
        while (pos != std::string::npos) {
            std::string segment = std::string(&str[begin], pos - begin);
            std::wstring converted = std::wstring(segment.size() + 1, 0);
            size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, &segment[0], segment.size(), &converted[0], converted.length());
            converted.resize(size);
            ret.append(converted);
            ret.append({ 0 });
            begin = pos + 1;
            pos = str.find(static_cast<char>(0), begin);
        }
        if (begin < str.length()) {
            std::string segment = std::string(&str[begin], str.length() - begin);
            std::wstring converted = std::wstring(segment.size() + 1, 0);
            size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, segment.c_str(), segment.size(), &converted[0], converted.length());
            converted.resize(size);
            ret.append(converted);
        }

#elif LINUX_PLATFORM || MACOS_PLATFORM || EMSCRIPTEN_PLATFORM
        size_t size;
        pos = str.find(static_cast<char>(0), begin);
        while (pos != std::string::npos)
        {
            std::string segment = std::string(&str[begin], pos - begin);
            std::wstring converted = std::wstring(segment.size(), 0);
            size = mbstowcs(&converted[0], &segment[0], converted.size());
            converted.resize(size);
            ret.append(converted);
            ret.append({ 0 });
            begin = pos + 1;
            pos = str.find(static_cast<char>(0), begin);
        }
        if (begin < str.length())
        {
            std::string segment = std::string(&str[begin], str.length() - begin);
            std::wstring converted = std::wstring(segment.size(), 0);
            size = mbstowcs(&converted[0], &segment[0], converted.size());
            converted.resize(size);
            ret.append(converted);
        }
#else
        static_assert(false, "Unknown Platform");
#endif
        return ret;
    }


};

