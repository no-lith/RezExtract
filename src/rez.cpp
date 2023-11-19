#include "pch.hpp"
#include "rez.hpp"

#include "block.hpp"
#include "reader.hpp"

namespace rez
{

/**
 * @brief rez header
 */
struct rez_header_t
{
	char          m_cr1;
	char          m_lf1;
	std::string   m_file_type;
	char          m_cr2;
	char          m_lf2;
	std::string   m_user_title;
	char          m_cr3;
	char          m_lf3;
	char          m_eof1;
	char          m_head;
	std::string   m_encode;
	char          m_tail;
	char          m_detect_head;
	std::string   m_detect_encode;
	char          m_detect_tail;
	std::uint32_t m_file_format_version;
	std::uint32_t m_root_dir_pos;
	std::uint32_t m_root_dir_size;
	std::uint32_t m_root_dir_time;
	std::uint32_t m_next_write_pos;
	std::uint32_t m_time;
	std::uint32_t m_largest_key_ary;
	std::uint32_t m_largest_dir_name_size;
	std::uint32_t m_largest_rez_name_size;
	std::uint32_t m_largest_comment_size;
	char          m_is_sorted;
};

/**
 * @brief represents a rez file
 */
class c_rez_file
{
public:
	c_rez_file( const std::filesystem::path& input ) :
		m_reader{ input },
		m_path{ input }
	{
	}
public:
	void load();
	void extract( const std::filesystem::path& output );
private:
	c_reader              m_reader;
	std::filesystem::path m_path;

	rez_header_t          m_header;
	rez_t                 m_rez;
};

}

void rez::c_rez_file::load()
{
	const std::string stem = m_path.stem().string();

	// remove the extra spaces at the end of the array
	auto remove_spaces{ []( const auto& arr ) -> std::string
	{
		std::string str{ arr.begin(), arr.end() };

		auto pos{ std::find_if( str.rbegin(), str.rend(), [] ( char ch )
		{
			return ch != ' ';
		} ) };

		if ( pos != str.rend() )
		{
			str.erase(
				std::remove(
					str.begin() + std::distance( pos, str.rend() ),
					str.end(),
					' '
				),
				str.end()
			);
		}

		return str;
	} };

	m_header.m_cr1 = m_reader.read< char >();

	if ( m_header.m_cr1 != '\r' && m_header.m_cr1 != '&' )
		REZ_THROW( " - Invalid cr1" );

	m_header.m_lf1 = m_reader.read< char >();

	if ( m_header.m_lf1 != '\n' && m_header.m_lf1 != '#' )
		REZ_THROW( " - Invalid lf1" );

	std::array< char, 60u > file_type{};

	m_reader.read( file_type[ 0 ], file_type.size() );

	m_header.m_file_type = remove_spaces( file_type );

	m_header.m_cr2 = m_reader.read< char >();

	if ( m_header.m_cr2 != '\r' && m_header.m_cr2 != '!' )
		REZ_THROW( " - Invalid cr2" );

	m_header.m_lf2 = m_reader.read< char >();

	if ( m_header.m_lf2 != '\n' && m_header.m_lf2 != '\"' )
		REZ_THROW( " - Invalid lf2" );

	std::array< char, 60u > user_title{};

	m_reader.read( user_title[ 0 ], user_title.size() );

	m_header.m_user_title = remove_spaces( user_title );

	m_header.m_cr3 = m_reader.read< char >();

	if ( m_header.m_cr3 != '\r' && m_header.m_cr3 != '%' )
		REZ_THROW( " - Invalid cr3" );

	m_header.m_lf3 = m_reader.read< char >();

	if ( m_header.m_lf3 != '\n' && m_header.m_lf3 != '\'' )
		REZ_THROW( " - Invalid lf3" );

	m_header.m_eof1 = m_reader.read< char >();

	if ( m_header.m_eof1 != 0x1A && m_header.m_eof1 != '*' )
		REZ_THROW( " - Invalid eof1" );

	// eof values
	static constexpr auto EOF1 = 0x1A;
	static constexpr auto EOF2 = 0x2A;

	// file format version
	static constexpr auto FFV1 = 0x1;
	static constexpr auto FFV2 = 0x2;

	// use eof1 to determine the version
	switch ( m_header.m_eof1 )
	{
	case EOF1:
	{
		const auto offset = m_reader.tell();

		m_header.m_file_format_version = m_reader.read();

		if ( m_header.m_file_format_version != FFV1 )
		{
			// fallback to rez format 2
			m_reader.seek( offset + 0x7 /*unknown bytes*/ );

			m_header.m_file_format_version = m_reader.read();

			if ( m_header.m_file_format_version != FFV2 )
				REZ_THROW( " - Invalid file format version (Expected: {:d} | Current: {:d})", FFV2, m_header.m_file_format_version );
		}

		break;
	}
	case EOF2:
	{
		m_header.m_head = m_reader.read< char >();

		std::array< char, 32u + 1u > encode{};

		m_reader.read( encode[ 0 ], encode.size() - 1 );

		m_header.m_encode = encode.data();

		m_header.m_tail = m_reader.read< char >();

		m_header.m_detect_head = m_reader.read< char >();

		// head = 1, detect_head = 16
		// 1 ^ 17 = 16
		if ( m_header.m_detect_head != ( m_header.m_head ^ 0x11 ) )
		{
			REZ_THROW(
				" - Invalid head (Head: {:d} | Detect: {:d} | Xor: {:d})",
				m_header.m_head,
				m_header.m_detect_head,
				( m_header.m_head ^ 0x11 )
			);
		}

		std::array< char, 32u + 1u > detect_encode{};

		m_reader.read( detect_encode[ 0 ], detect_encode.size() - 1u );

		m_header.m_detect_encode = detect_encode.data();

		std::to_chars(
			detect_encode.data(),
			detect_encode.data() + m_header.m_detect_encode.size(),
			std::atol( m_header.m_encode.data() ) ^ 0x16B4423 /*magic number*/
		);

		if ( m_header.m_detect_encode != detect_encode.data() )
		{
			REZ_THROW(
				" - Invalid encode (Encode: {:s} | Detect: {:s} | Xor: {:s})",
				m_header.m_encode,
				m_header.m_detect_encode,
				detect_encode.data()
			);
		}

		m_header.m_detect_tail = m_reader.read< char >();

		// tail = 16, m_detect_tail = 1
		// 16 ^ 17 = 1
		if ( m_header.m_detect_tail != ( m_header.m_tail ^ 0x11 ) )
		{
			REZ_THROW(
				" - Invalid tail (Tail: {:d} | Detect: {:d} | Xor: {:d})",
				m_header.m_tail,
				m_header.m_detect_tail, ( m_header.m_tail ^ 0x11 )
			);
		}

		m_header.m_file_format_version = m_reader.read();

		if ( m_header.m_file_format_version != FFV1 )
			REZ_THROW( " - Invalid file format version (Expected: {:d} | Current: {:d})", FFV1, m_header.m_file_format_version );

		break;
	}
	default:
	break;
	}

	m_header.m_root_dir_pos = m_reader.read();
	m_header.m_root_dir_size = m_reader.read();
	m_header.m_root_dir_time = m_reader.read();
	m_header.m_next_write_pos = m_reader.read();
	m_header.m_time = m_reader.read();
	m_header.m_largest_key_ary = m_reader.read();
	m_header.m_largest_dir_name_size = m_reader.read();
	m_header.m_largest_rez_name_size = m_reader.read();
	m_header.m_largest_comment_size = m_reader.read();

	m_header.m_is_sorted = m_reader.read< char >();
}

void rez::c_rez_file::extract( const std::filesystem::path& output )
{
	//
	// Recursive read
	//
	m_rez.read( m_reader, m_header.m_root_dir_pos, m_header.m_root_dir_size );

	auto& directories = m_rez.m_directories;

	if ( directories.empty() )
	{
		REZ_LOG( " - No directory found\n" );

		return;
	}

	auto create_dirs = [] ( const std::filesystem::path& path ) -> void
	{
		std::error_code ec = {};

		if ( !std::filesystem::create_directory( path, ec ) && ec )
			REZ_THROW( " - {:s}: {:s}", path.string(), ec.message() );
	};

	//
	// Extract 10MB per step
	//
	static constexpr std::uint32_t STEP_DATA_SIZE = 1'048'576u * 10u;

	auto data = std::make_unique< char[] >( STEP_DATA_SIZE );

	//
	// Extract Rez
	//
	for ( const auto& dir : directories )
	{
		REZ_LOG( " - Directory: {:s}\n", dir.m_name );

		std::filesystem::path path = {};

		//
		// generate recursive path to extract
		//
		std::vector< std::string_view > v{ dir.m_name };

		if ( dir.m_owner_index != std::numeric_limits<std::size_t>::max() )
		{
			v.reserve( dir.m_owner_index );

			auto i = dir.m_owner_index;
			do
			{
				const auto& dir_owner = directories.at( i );

				v.emplace_back( dir_owner.m_name );

				i = dir_owner.m_owner_index;
			}
			while ( i != std::numeric_limits<std::size_t>::max() );
		}

		for ( auto it = v.rbegin(); it != v.rend(); ++it )
		{
			path.append( *it );

			// create the directory if necessary
			create_dirs( output / path );
		}

		for ( const auto& res : dir.m_resource )
		{
			REZ_LOG( "  - File: {:s}\n", res.m_name );

			auto filename{ res.m_name };
			if ( filename.empty() )
				REZ_LOG( "  - Empty resource filename detected\n" );

			if ( res.m_type.size() )
				filename.append( "." ).append( res.m_type );

			try
			{
				std::ofstream out{};
				out.exceptions( std::ios::badbit | std::ios::failbit );
				out.open( output / path / filename, std::ios::binary );

				if ( out )
				{
					const std::uint32_t pos  = res.m_header.m_pos;
					const std::uint32_t size = res.m_header.m_size;

					std::uint32_t step = 0;

					while ( step < size )
					{
						const std::uint32_t step_size = std::min( size - step, STEP_DATA_SIZE );

						m_reader.seek( pos + static_cast< std::streamoff >( step ) );
						m_reader.read( data[ 0u ], step_size );

						if ( g_dtx_to_lithtech && step == 0u )
						{
							/**
							 * DTX file extension
							 */
							static constexpr auto DTX_EXT_A{ "dtx" };
							static constexpr auto DTX_EXT_B{ "DTX" };

							/**
							 * DTX header version
							 */
							static constexpr auto DTX_VER_LT1  = -2;
							static constexpr auto DTX_VER_LT15 = -3;
							static constexpr auto DTX_VER_LT2  = -5;

							if ( ( res.m_type == DTX_EXT_A ) || ( res.m_type == DTX_EXT_B ) )
							{
								/**
								 * The DTX version in the header starts at offset 8, in some files it starts at offset 4
								 * so we need to swap these bytes
								 */
								auto&  ver = data[ 0x4 ];

								if ( ( ver != DTX_VER_LT1 ) && ( ver != DTX_VER_LT15 ) && ( ver != DTX_VER_LT2 ) )
								{
									std::array< char, 4u > lhs_bytes = {};
									std::array< char, 4u > rhs_bytes = {};

									std::memcpy( lhs_bytes.data(), &data[ 0x4 ], lhs_bytes.size() );
									std::memcpy( rhs_bytes.data(), &data[ 0x8 ], rhs_bytes.size() );

									std::memcpy( &data[ 0x4 ], rhs_bytes.data(), rhs_bytes.size() );
									std::memcpy( &data[ 0x8 ], lhs_bytes.data(), lhs_bytes.size() );
								}
							}
						}

						//
						// write to file
						//
						out.write( data.get(), step_size );

						step += step_size;
					}
				}
			}
			catch ( const std::exception& e )
			{
				REZ_LOG( "{:s}\n", e.what() );
				REZ_PAUSE();
			}
		}
	}
}

void rez::extract( const std::vector<std::filesystem::path>& file_path, const std::filesystem::path& save_path )
{
	for ( const auto& file : file_path )
	{
		try
		{
			REZ_LOG( "Extracting: {}\n", file.stem().string() );

			// open stream to read
			auto rez = c_rez_file{ file };

			// load info
			rez.load();

			// extract to save path
			rez.extract( save_path );
		}
		catch ( const std::exception& e )
		{
			REZ_LOG( "[FATAL] {:s}\n", e.what() );
			REZ_PAUSE();
		}
	}
}