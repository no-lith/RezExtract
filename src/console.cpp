#include "console.hpp"

#include <iostream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <windows.h>

void rez::con::attach( const std::string_view& title )
{
	auto throw_error{ []( const std::string& msg )
	{
		const auto error{
			std::system_category( ).message( ::GetLastError( ) )
		};

		throw std::runtime_error{
			msg + error
		};
	} };

	if ( !::AttachConsole( ATTACH_PARENT_PROCESS ) )
	{
		if ( ::GetLastError( ) != ERROR_ACCESS_DENIED )
		{
			if ( ::GetLastError( ) == ERROR_INVALID_PARAMETER )
			{
				throw_error( "AttachConsole: " );
			}

			else
			{
				if ( !::AllocConsole( ) )
				{
					throw_error( "AllocConsole: " );
				}
			}
		}
	}

	if ( !title.empty( ) )
	{
		::SetConsoleTitleA( title.data( ) );
	}
}

