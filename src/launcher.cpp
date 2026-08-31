#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

static bool launchProcess(const std::wstring& exePath, const std::wstring& workDir) {
    STARTUPINFOW si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);

    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));

    std::wstring cmdLine = L"\"" + exePath + L"\"";

    BOOL success = CreateProcessW(
        NULL,
        &cmdLine[0],
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        workDir.c_str(),
        &si,
        &pi
    );

    if (success) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return true;
    }
    return false;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    // 1. Get launcher path
    wchar_t exePathBuf[MAX_PATH];
    GetModuleFileNameW(NULL, exePathBuf, MAX_PATH);

    fs::path launcherPath(exePathBuf);
    fs::path rootDir = launcherPath.parent_path();

    // 2. Check direct development paths first
    fs::path devExe = rootDir / L"build" / L"Release" / L"ACModOrganize.exe";
    if (fs::exists(devExe)) {
        if (launchProcess(devExe.wstring(), devExe.parent_path().wstring())) {
            return 0;
        }
    }

    fs::path localExe = rootDir / L"ACModOrganize.exe";
    if (fs::exists(localExe) && localExe != launcherPath) {
        if (launchProcess(localExe.wstring(), rootDir.wstring())) {
            return 0;
        }
    }

    // 3. Standalone single-file bundle mode
    wchar_t localAppPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, localAppPath))) {
        fs::path appDataDir = fs::path(localAppPath) / L"ACBO" / L"app";
        fs::path targetExe = appDataDir / L"ACModOrganize.exe";
        fs::path stampFile = appDataDir / L".version_stamp";

        // Read current launcher size and write time
        auto launcherSize = fs::file_size(launcherPath);
        auto launcherTime = fs::last_write_time(launcherPath).time_since_epoch().count();
        std::string currentStamp = std::to_string(launcherSize) + "_" + std::to_string(launcherTime);

        bool needsExtract = true;
        if (fs::exists(targetExe) && fs::exists(stampFile)) {
            std::ifstream sf(stampFile);
            std::string savedStamp;
            sf >> savedStamp;
            if (savedStamp == currentStamp) {
                needsExtract = false;
            }
        }

        if (needsExtract) {
            HANDLE hFile = CreateFileW(
                exePathBuf,
                GENERIC_READ,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL
            );

            if (hFile != INVALID_HANDLE_VALUE) {
                DWORD fileSize = GetFileSize(hFile, NULL);
                if (fileSize > 1024) {
                    std::vector<unsigned char> buffer(fileSize);
                    DWORD bytesRead = 0;
                    if (ReadFile(hFile, buffer.data(), fileSize, &bytesRead, NULL) && bytesRead == fileSize) {
                        size_t zipOffset = std::string::npos;
                        for (size_t i = 1024; i + 4 <= buffer.size(); ++i) {
                            if (buffer[i] == 'P' && buffer[i+1] == 'K' && buffer[i+2] == 0x03 && buffer[i+3] == 0x04) {
                                zipOffset = i;
                                break;
                            }
                        }

                        if (zipOffset != std::string::npos) {
                            wchar_t tempPathBuf[MAX_PATH];
                            GetTempPathW(MAX_PATH, tempPathBuf);
                            fs::path tempZip = fs::path(tempPathBuf) / L"acbo_payload.zip";

                            HANDLE hZipOut = CreateFileW(
                                tempZip.c_str(),
                                GENERIC_WRITE,
                                0,
                                NULL,
                                CREATE_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL
                            );

                            if (hZipOut != INVALID_HANDLE_VALUE) {
                                DWORD written = 0;
                                WriteFile(hZipOut, &buffer[zipOffset], static_cast<DWORD>(buffer.size() - zipOffset), &written, NULL);
                                CloseHandle(hZipOut);

                                fs::create_directories(appDataDir);

                                // Extract using Windows built-in tar.exe
                                std::wstring tarCmd = L"tar.exe -xf \"" + tempZip.wstring() + L"\" -C \"" + appDataDir.wstring() + L"\"";
                                STARTUPINFOW si;
                                ZeroMemory(&si, sizeof(si));
                                si.cb = sizeof(si);
                                si.dwFlags |= STARTF_USESHOWWINDOW;
                                si.wShowWindow = SW_HIDE;

                                PROCESS_INFORMATION pi;
                                ZeroMemory(&pi, sizeof(pi));

                                if (CreateProcessW(NULL, &tarCmd[0], NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
                                    WaitForSingleObject(pi.hProcess, 30000);
                                    CloseHandle(pi.hProcess);
                                    CloseHandle(pi.hThread);
                                }

                                DeleteFileW(tempZip.c_str());

                                std::ofstream sf(stampFile);
                                sf << currentStamp;
                                sf.close();
                            }
                        }
                    }
                }
                CloseHandle(hFile);
            }
        }

        if (fs::exists(targetExe)) {
            if (launchProcess(targetExe.wstring(), appDataDir.wstring())) {
                return 0;
            }
        }
    }

    MessageBoxW(
        NULL,
        L"Failed to launch Assetto Corsa Mod Organizer (ACBO).\nPlease ensure build files or payload exist.",
        L"ACBO Launch Error",
        MB_OK | MB_ICONERROR
    );

    return 1;
}
