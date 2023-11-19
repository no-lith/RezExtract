#include "pch.hpp"
#include "file_dialog.hpp"

static constexpr auto MAX_BUFFER_SIZE = 0x1000;

auto rez::fdg::open_file(
	const std::string_view title,
	const std::string_view filter,
	const HWND hwnd
) -> std::vector<std::filesystem::path>
{
	if ( title.empty() )
		REZ_THROW( "File dialog: Empty title" );

	if ( filter.empty() )
		REZ_THROW( "File dialog: Empty filter" );

	OPENFILENAMEA ofn = { sizeof( OPENFILENAMEA ) };

	auto buffer = std::make_unique< char[] >( MAX_BUFFER_SIZE );

	ofn.hwndOwner    = hwnd;
	ofn.lpstrFile    = buffer.get();
	ofn.nMaxFile     = MAX_BUFFER_SIZE;
	ofn.lpstrFilter  = filter.data();
	ofn.nFilterIndex = 1;
	ofn.lpstrTitle   = title.data();
	ofn.Flags        = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER | OFN_ALLOWMULTISELECT;

	if ( ::GetOpenFileNameA( &ofn ) )
	{
		std::string_view file     = ofn.lpstrFile;
		std::string_view filename = ofn.lpstrFile + file.size() + 1u;

		// multiple entries
		if ( !filename.empty() )
		{
			std::vector<std::filesystem::path> entries = {};

			do
			{
				// directory / filename
				entries.emplace_back( file ) /= filename;

				// next entry
				filename = filename.data() + filename.size() + 1u;
			}
			while ( !filename.empty() );

			return entries;
		}
		// single entry
		else
		{
			return std::vector<std::filesystem::path>( { file } );
		}
	}

	else
	{
		if ( ::GetLastError() == ERROR_SUCCESS )
			REZ_THROW( "File dialog: Select any file" );
		else
			REZ_THROW( "File dialog: {:s}", std::system_category().message( ::GetLastError() ) );
	}
}

auto rez::fdg::open_folder(
	const std::wstring_view title,
	const HWND hwnd
) -> std::filesystem::path
{
	using Microsoft::WRL::ComPtr;

	struct co_scoped
	{
		co_scoped()  { ::CoInitialize( nullptr );  }
		~co_scoped() { ::CoUninitialize(); }
	} co;

	HRESULT result = S_OK;

	ComPtr<IFileOpenDialog> fd{};

	if ( FAILED( result = ::CoCreateInstance( CLSID_FileOpenDialog, nullptr, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast< void** >( fd.GetAddressOf() ) ) ) )
		REZ_THROW( "File dialog (CoCreateInstance): {:#x}", ( ULONG ) result );

	FILEOPENDIALOGOPTIONS curr_opts{};
	if ( FAILED( result = fd->GetOptions( &curr_opts ) ) )
		REZ_THROW( "File dialog (GetOptions): {:#x}", ( ULONG ) result );

	if ( FAILED( result = fd->SetOptions( curr_opts | FOS_PATHMUSTEXIST | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM ) ) )
		REZ_THROW( "File dialog (SetOptions): {:#x}", ( ULONG ) result );

	if ( FAILED( result = fd->SetTitle( title.data() ) ) )
		REZ_THROW( "File dialog (SetTitle): {:#x}", ( ULONG ) result );

	if ( FAILED( result = fd->Show( hwnd ) ) )
	{
		if ( result == HRESULT_FROM_WIN32( ERROR_CANCELLED ) )
			REZ_THROW( "File dialog: Select any folder" );
		else
			REZ_THROW( "File dialog (Show): {:#x}", ( ULONG ) result );
	}

	ComPtr<IShellItem> si{};
	if ( FAILED( result = fd->GetResult( si.GetAddressOf() ) ) )
		REZ_THROW( "File dialog (GetResult): {:#x}", ( ULONG ) result );

	struct mem_scoped
	{
		mem_scoped() {}
		~mem_scoped() { if ( this->data ) ::CoTaskMemFree( this->data ); }

		wchar_t* data;
	} mem ;

	if ( FAILED( result = si->GetDisplayName( SIGDN_FILESYSPATH, &mem.data ) ) )
		REZ_THROW( "File dialog (GetDisplayName): {:#x}", ( ULONG ) result );

	return std::filesystem::path( mem.data );
}
