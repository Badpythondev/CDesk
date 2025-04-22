#include "serverspeak.h"
#include <filesystem>
#include <windows.h>
#include <winhttp.h>
#include <iostream>

std::wstring GetExecutableDir() {
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(NULL, path, MAX_PATH);
    std::wstring fullPath(path);
    return fullPath.substr(0, fullPath.find_last_of(L"\\/"));
}

std::wstring GetSiteFilePath(const std::wstring& relativeFile) {
    return GetExecutableDir() + L"\\CantariApp\\" + relativeFile;
}

bool DoesSiteExist() {
    std::vector<std::wstring> requiredFiles = {
        L"index.html",
        L"favicon.ico",
        L"script.js",
        L"styles.css"
    };

    for (const auto& file : requiredFiles) {
        if (!std::filesystem::exists(GetSiteFilePath(file))) {
            return false;
        }
    }

    return true;
}

std::vector<std::wstring> GetMissingFiles() {
    std::vector<std::wstring> requiredFiles = {
        L"index.html",
        L"favicon.ico",
        L"script.js",
        L"styles.css"
    };
    std::vector<std::wstring> missing;

    for (const auto& file : requiredFiles) {
        if (!std::filesystem::exists(GetSiteFilePath(file))) {
            missing.push_back(file);
        }
    }

    return missing;
}


bool DownloadFileFromGitHub(const std::wstring& relativePath, const std::wstring& outputPath) {
    
    return true;
}

void DownloadMissingFilesFromGitHub() {
    auto missing = GetMissingFiles();
    for (const auto& file : missing) {
        DownloadFileFromGitHub(file, GetSiteFilePath(file));
    }
}
