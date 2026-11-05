#define NOMINMAX
#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>
#include <objbase.h>

#include "FileDialog.h"

#include <array>
#include <string>
#include <vector>

namespace
{
    std::wstring ToWide(const std::string& value)
    {
        if (value.empty()) return {};
        const int size = MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, nullptr, 0);
        if (size <= 0) return {};
        std::wstring result(static_cast<std::size_t>(size), L'\0');
        MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, result.data(), size);
        result.resize(static_cast<std::size_t>(size - 1));
        return result;
    }

    std::string ToUtf8(const wchar_t* value)
    {
        if (value == nullptr || *value == L'\0') return {};
        const int size = WideCharToMultiByte(CP_UTF8, 0, value, -1, nullptr, 0, nullptr, nullptr);
        if (size <= 0) return {};
        std::string result(static_cast<std::size_t>(size), '\0');
        WideCharToMultiByte(CP_UTF8, 0, value, -1, result.data(), size, nullptr, nullptr);
        result.resize(static_cast<std::size_t>(size - 1));
        return result;
    }

    int CALLBACK InitializeFolderDialog(HWND dialog, UINT message, LPARAM, LPARAM data)
    {
        if (message == BFFM_INITIALIZED && data != 0)
            SendMessageW(dialog, BFFM_SETSELECTIONW, TRUE, data);
        return 0;
    }
}

std::string FileDialog::OpenFile(const char* title, const char* filter,
    const std::string& initialDirectory, void* owner)
{
    std::array<wchar_t, 32768> path{};
    const std::wstring wideTitle = ToWide(title == nullptr ? "Select File" : title);
    const std::wstring wideDirectory = ToWide(initialDirectory);
    std::wstring wideFilter = ToWide(filter == nullptr ? "All Files (*.*)|*.*|" : filter);
    for (wchar_t& character : wideFilter) if (character == L'|') character = L'\0';
    wideFilter.push_back(L'\0');
    OPENFILENAMEW dialog{};
    dialog.lStructSize = sizeof(dialog);
    dialog.hwndOwner = static_cast<HWND>(owner);
    dialog.lpstrFilter = wideFilter.c_str();
    dialog.lpstrFile = path.data();
    dialog.nMaxFile = static_cast<DWORD>(path.size());
    dialog.lpstrTitle = wideTitle.c_str();
    dialog.lpstrInitialDir = wideDirectory.empty() ? nullptr : wideDirectory.c_str();
    dialog.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    return GetOpenFileNameW(&dialog) ? ToUtf8(path.data()) : std::string{};
}

std::string FileDialog::SelectFolder(const char* title, const std::string& initialDirectory, void* owner)
{
    const std::wstring wideTitle = ToWide(title == nullptr ? "Select Folder" : title);
    const std::wstring wideInitialDirectory = ToWide(initialDirectory);
    BROWSEINFOW dialog{};
    dialog.hwndOwner = static_cast<HWND>(owner);
    dialog.lpszTitle = wideTitle.c_str();
    dialog.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_EDITBOX;
    dialog.lpfn = InitializeFolderDialog;
    dialog.lParam = reinterpret_cast<LPARAM>(wideInitialDirectory.c_str());
    PIDLIST_ABSOLUTE item = SHBrowseForFolderW(&dialog);
    if (item == nullptr) return {};
    std::array<wchar_t, 32768> path{};
    const bool found = SHGetPathFromIDListW(item, path.data()) != FALSE;
    CoTaskMemFree(item);
    return found ? ToUtf8(path.data()) : std::string{};
}
