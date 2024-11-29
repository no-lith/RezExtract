#ifndef PCH_HPP
#define PCH_HPP

#pragma once

#define NOMINMAX
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include <objbase.h>
#include <shlobj_core.h>
#include <wrl/client.h>

#include <algorithm>
#include <array>
#include <charconv>
#include <chrono>
#include <clocale>
#include <cstdint>
#include <iostream>
#include <filesystem>
#include <format>
#include <fstream>
#include <locale>
#include <memory>
#include <source_location>
#include <string>
#include <string_view>
#include <stdexcept>
#include <system_error>
#include <vector>

#include "utilities.hpp"

#endif // !PCH_HPP
