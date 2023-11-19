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
#include <string>
#include <string_view>
#include <stdexcept>
#include <system_error>
#include <vector>

template<typename ... args_t>
inline auto REZ_LOG( const std::string_view fmt, args_t&& ... args ) -> void
{
	std::cout << std::vformat( fmt, std::make_format_args( std::forward<args_t>( args )... ) );
}

inline auto REZ_PAUSE() -> void
{
	try
	{
		REZ_LOG( "Press the enter key to continue . . ." );

		std::cin.clear();
		std::cin.get();
	}
	catch ( const std::exception& e )
	{
		std::cout << e.what() << std::endl;
	}
}

template<typename ... args_t>
[[noreturn]] inline auto REZ_THROW( const std::string_view fmt, args_t&& ... args ) -> void
{
	throw std::runtime_error( std::vformat( fmt, std::make_format_args( std::forward<args_t>( args )... ) ) );
}

#endif // !PCH_HPP
