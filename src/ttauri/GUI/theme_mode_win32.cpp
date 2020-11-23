// Copyright 2020 Pokitec
// All rights reserved.

#include "theme_mode.hpp"
#include "../strings.hpp"
#include "../logger.hpp"
#include <Windows.h>
#undef WIN32_NO_STATUS
#include <ntstatus.h>
#include <winreg.h>
#include <Uxtheme.h>


namespace tt {

[[nodiscard]] theme_mode read_os_theme_mode() noexcept
{
    ttlet subkey = tt::to_wstring("Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize");
    ttlet name = tt::to_wstring("AppsUseLightTheme");
    DWORD result;
    DWORD result_length = sizeof(result);
    auto status = RegGetValueW(
        HKEY_CURRENT_USER,
        subkey.c_str(),
        name.c_str(),
        RRF_RT_DWORD,
        NULL,
        &result,
        &result_length
    );

    switch (status) {
    case ERROR_SUCCESS:
        if (result) {
            return theme_mode::light;
        } else {
            return theme_mode::dark;
        }

    case ERROR_BAD_PATHNAME:
    case ERROR_FILE_NOT_FOUND: {
        auto reg_path = "HKEY_CURRENT_USER\\" + tt::to_string(subkey) + "\\" + tt::to_string(name);

        LOG_ERROR("Missing {} registry entry: 0x{:08x}", reg_path, status);
        } return theme_mode::light;

    default:
        LOG_FATAL("Could get AppsUseLightTheme registry value. {:08x}", status);
    }

}

}