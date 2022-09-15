#include "console.hpp"

#include <iostream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <windows.h>

void rez::con::attach( const std::string_view& title )
{
	auto throw_error{ [] ( )
	{
		const auto error{
			std::system_category( ).message( ::GetLastError( ) )
		};

		throw std::runtime_error{
			std::string{ "[rez::con::attach] " } + error
		};
	} };

	if ( !::AttachConsole( ATTACH_PARENT_PROCESS ) )
	{
		if ( ::GetLastError( ) != ERROR_ACCESS_DENIED )
		{
			if ( ::GetLastError( ) == ERROR_INVALID_PARAMETER )
			{
				throw_error( );
			}

			else
			{
				if ( !::AllocConsole( ) )
				{
					throw_error( );
				}
			}
		}
	}

	auto handle{ ::GetStdHandle( STD_OUTPUT_HANDLE ) };

	if ( handle == INVALID_HANDLE_VALUE )
	{
		throw_error( );
	}

	auto mode{ 0ul };

	if ( !::GetConsoleMode( handle, &mode ) )
	{
		throw_error( );
	}

	if ( !( mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING ) )
	{
		if ( !::SetConsoleMode( handle, mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING ) )
		{
			throw_error( );
		}
	}

	if ( !title.empty( ) )
	{
		::SetConsoleTitleA( title.data( ) );
	}
}

