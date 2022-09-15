#include "file_dialog.hpp"

#include <algorithm>
#include <string_view>
#include <stdexcept>
#include <system_error>

#include <objbase.h>
#include <shlobj_core.h>
#include <windows.h>

auto rez::fdg::open( ) -> std::vector<std::filesystem::path>
{
	constexpr const char* filter{ "Rez File (*.rez)\0*.rez\0" };

	OPENFILENAMEA ofn{ sizeof( OPENFILENAMEA ) };

	char file_path[ 4096 ]{};

	ofn.hwndOwner    = ::GetConsoleWindow( );
	ofn.lpstrFile    = file_path;
	ofn.nMaxFile     = sizeof( file_path );
	ofn.lpstrFilter  = filter;
	ofn.nFilterIndex = 1;
	ofn.Flags        = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER | OFN_ALLOWMULTISELECT;

	if ( ::GetOpenFileNameA( &ofn ) )
	{
		// all files selected
		std::vector<std::string> f{};

		// get directory
		std::string dir{ ofn.lpstrFile };

		// get first entry
		char* str{ ofn.lpstrFile + dir.size( ) + 1u };

		// run through entries
		while ( *str )
		{
			// add entry
			f.emplace_back( str );

			// go to next entry
			str += f.back( ).size( ) + 1u;
		}

		// has one entry
		if ( f.empty( ) )
		{
			f.emplace_back( std::move( dir ) );
		}

		// has multiple entries
		else
		{
			// add directory path
			std::for_each( f.begin( ), f.end( ), [&] ( auto& s )
			{
				s.insert( 0u, dir + "\\" );
			} );
		}

		return std::vector<std::filesystem::path>{ f.begin( ), f.end( ) };
	}

	else
	{
		std::string error{};

		if ( ::GetLastError( ) == ERROR_SUCCESS )
		{
			error = "select any file";
		}

		else
		{
			error = std::system_category( ).message( ::GetLastError( ) );
		}

		throw std::runtime_error{
			std::string{ "[file dialog open] " } + error
		};
	}
}

auto rez::fdg::save( ) -> std::wstring
{
    std::wstring result{};

	if ( ::CoInitialize( nullptr ) == S_OK )
	{
		IFileOpenDialog* p_fd{};

		if ( ::CoCreateInstance( CLSID_FileOpenDialog, nullptr, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast< void** >( &p_fd ) ) == S_OK )
		{
			FILEOPENDIALOGOPTIONS curr_opts{};
			if ( p_fd->GetOptions( &curr_opts ) == S_OK )
			{
				p_fd->SetOptions( curr_opts | FOS_PATHMUSTEXIST | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM );
			}

			if ( p_fd->Show( ::GetConsoleWindow( ) ) == S_OK )
			{
				IShellItem* p_si{};
				if ( p_fd->GetResult( &p_si ) == S_OK )
				{
					wchar_t* wpath{};
					if ( p_si->GetDisplayName( SIGDN_FILESYSPATH, &wpath ) == S_OK )
					{
						result = wpath;

						::CoTaskMemFree( std::exchange( wpath, nullptr ) );
					}

					p_si->Release( );
					p_si = nullptr;
				}
			}

			p_fd->Release( );
			p_fd = nullptr;
		}

		::CoUninitialize( );
	}

	if ( result.empty( ) )
	{
		std::string error{};

		if ( ::GetLastError( ) == ERROR_SUCCESS )
		{
			error = "select a path to save files";
		}
		
		else
		{
			error = std::system_category( ).message( ::GetLastError( ) );
		}

		throw std::runtime_error{
			std::string{ "[file dialog save] " } + error
		};
	}

	else
	{
		return result;
	}
}