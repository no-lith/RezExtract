#include "pch.hpp"
#include "file_dialog.hpp"
#include "rez.hpp"

static constexpr std::string_view SELECT_FILES_TITLE  = "Select files . . .";
static constexpr std::string_view SELECT_FILES_FILTER = "REZ File (*.rez)\0*.rez\0";

static constexpr std::wstring_view SAVE_PATH_TITLE    = L"Extract files into . . .";

int main( )
{
	try
	{
		if ( !std::setlocale( LC_ALL, "" ) )
			REZ_THROW( "Failed to copy locale" );

		if ( !::SetConsoleTitleA( "RezExtract" ) )
			REZ_THROW( "Failed to set console title: {}", std::system_category().message( ::GetLastError() ) );

		std::vector<std::filesystem::path> file_path = {};
		std::filesystem::path save_path = {};

		while ( file_path.empty() )
		{
			try
			{
				file_path = rez::fdg::open_file( SELECT_FILES_FILTER, SELECT_FILES_FILTER, ::GetConsoleWindow() );
			}
			catch ( const std::exception& e )
			{
				REZ_LOG( "{:s}\n", e.what() );
				REZ_PAUSE();
			}
		}

		while ( save_path.empty() )
		{
			try
			{
				save_path = rez::fdg::open_folder( SAVE_PATH_TITLE, ::GetConsoleWindow() );
			}
			catch ( const std::exception& e )
			{
				REZ_LOG( "{:s}\n", e.what() );
				REZ_PAUSE();
			}
		}

		rez::extract( file_path, save_path );
	}
	catch ( const std::exception& e )
	{
		std::cout << e.what() << std::endl;
	}

	REZ_PAUSE();
}