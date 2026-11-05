#pragma once

#include <string>

namespace FileDialog
{
    // Return an empty string when the dialog is cancelled.
    std::string OpenFile(const char* title, const char* filter, const std::string& initialDirectory = {}, void* owner = nullptr);
    std::string SelectFolder(const char* title, const std::string& initialDirectory = {}, void* owner = nullptr);
}
