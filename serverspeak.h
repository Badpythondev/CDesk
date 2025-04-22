#pragma once
#include <string>
#include <vector>

// Declare your functions here
bool DoesSiteExist();
std::vector<std::wstring> GetMissingFiles();
void DownloadMissingFilesFromGitHub();
bool DownloadFileFromGitHub(const std::wstring& relativePath, const std::wstring& outputPath);
std::wstring GetExecutableDir();
std::wstring GetSiteFilePath(const std::wstring& relativeFile);
