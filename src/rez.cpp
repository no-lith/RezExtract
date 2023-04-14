#include "rez.hpp"

#include "block.hpp"
#include "reader.hpp"

#include <array>
#include <charconv>
#include <chrono>
#include <iostream>
#include <memory>
#include <string>

namespace rez {

/**
 * @brief rez header
 */
struct rez_header_t {
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
class c_rez_file {
public:
	using path_t = std::filesystem::path;

	c_rez_file( const path_t& input ) :
		m_reader{ input },
		m_path{ input }
	{}
public:
	void extract_to_file( const path_t& output );
private:
	c_reader     m_reader;
	path_t       m_path;

	rez_header_t m_header;
	rez_t        m_rez;
};

}

void rez::c_rez_file::extract_to_file( const path_t& output )
{
	std::cout 
		<< "extracting '"
		<< m_path.filename( ).string( )
		<< "'..."
		<< std::endl;

	/*
	 * Header
	 */
	{
		// remove the extra spaces at the end of the array
		auto remove_spaces{ []( const auto& arr ) -> std::string
		{
			std::string str{ arr.begin( ), arr.end( ) };

			auto pos{ std::find_if( str.rbegin( ), str.rend( ), []( char ch )
			{
				return ch != ' ';
			} ) };

			if ( pos != str.rend( ) )
			{
				str.erase(
					std::remove(
						str.begin( ) + std::distance( pos, str.rend( ) ),
						str.end( ),
						' '
					),
					str.end( )
				);
			}

			return str;
		} };

		// cr version (1 or 2)
		enum cr_ : char {
			cr1_v1 = 0x26,
			cr2_v1 = 0x21,
			cr3_v1 = 0x25,

			cr1_v2 = 0xD,
			cr2_v2 = cr1_v2,
			cr3_v2 = cr2_v2,
		};

		// lf version (1 or 2)
		enum lf_ : char {
			lf1_v1 = 0x23,
			lf2_v1 = 0x22,
			lf3_v1 = 0x27,

			lf1_v2 = 0xA,
			lf2_v2 = lf1_v2,
			lf3_v2 = lf2_v2,
		};

		// rez version
		enum class rez {
			unknown   = 0,

			version_1 = 1,
			version_2 = 2 
		};

		// use cr1 and lf1 to determine the version
		m_header.m_cr1                   = m_reader.read< char >( );
		m_header.m_lf1                   = m_reader.read< char >( );

		auto rez_version{ rez::unknown };

		if ( m_header.m_cr1 == cr1_v1 &&
			 m_header.m_lf1 == lf1_v1 )
		{
			rez_version = rez::version_1;
		}

		if ( m_header.m_cr1 == cr1_v2 &&
			 m_header.m_lf1 == lf1_v2 )
		{
			rez_version = rez::version_2;
		}

		if ( rez_version == rez::unknown )
		{
			throw std::runtime_error{ "invalid rez version" };
		}

		std::array< char, 60u > file_type{};
		std::array< char, 60u > user_title{};
		std::array< char, 32u + 1u > encode{};
		std::array< char, 32u + 1u > detect_encode{};

		m_reader.read(
			file_type[ 0 ],
			file_type.size( )
		);

		m_header.m_file_type             = remove_spaces( file_type );

		m_header.m_cr2                   = m_reader.read< char >( );
		m_header.m_lf2                   = m_reader.read< char >( );

		m_reader.read(
			user_title[ 0 ],
			user_title.size( )
		);

		m_header.m_user_title            = remove_spaces( user_title );

		m_header.m_cr3                   = m_reader.read< char >( );
		m_header.m_lf3                   = m_reader.read< char >( );

		if ( rez_version == rez::version_1 )
		{
			m_header.m_eof1                  = m_reader.read< char >( );
			m_header.m_head                  = m_reader.read< char >( );

			m_reader.read(
				encode[ 0 ],
				encode.size( ) - 1u
			);

			m_header.m_encode                = encode.data( );

			m_header.m_tail                  = m_reader.read< char >( );
			m_header.m_detect_head           = m_reader.read< char >( );

			m_reader.read(
				detect_encode[ 0 ],
				detect_encode.size( ) - 1u
			);

			m_header.m_detect_encode         = detect_encode.data( );

			m_header.m_detect_tail           = m_reader.read< char >( );

			m_header.m_file_format_version   = m_reader.read( );

			m_header.m_root_dir_pos          = m_reader.read( );
			m_header.m_root_dir_size         = m_reader.read( );
			m_header.m_root_dir_time         = m_reader.read( );
			m_header.m_next_write_pos        = m_reader.read( );
			m_header.m_time                  = m_reader.read( );
			m_header.m_largest_key_ary       = m_reader.read( );
			m_header.m_largest_dir_name_size = m_reader.read( );
			m_header.m_largest_rez_name_size = m_reader.read( );
			m_header.m_largest_comment_size  = m_reader.read( );

			m_header.m_is_sorted             = m_reader.read< char >( );
		}

		else
		{
			m_reader.seek(
				m_reader.tell() + 0x8
			); // skip 8 bytes (unknown)

			m_header.m_file_format_version = m_reader.read();

			m_header.m_root_dir_pos        = m_reader.read();
			m_header.m_root_dir_size       = m_reader.read();

			// the remaining bytes of the header are unknown...
		}

		try
		{
			if ( ( m_header.m_cr2 != cr2_v1 ) && ( m_header.m_cr2 != cr2_v2 ) )
			{
				throw std::runtime_error{ "invalid cr2" };
			}

			if ( ( m_header.m_cr3 != cr3_v1 ) && ( m_header.m_cr3 != cr3_v2 ) )
			{
				throw std::runtime_error{ "invalid cr3" };
			}

			if ( ( m_header.m_lf2 != lf2_v1 ) && ( m_header.m_lf2 != lf2_v2 ) )
			{
				throw std::runtime_error{ "invalid lf2" };
			}

			if ( ( m_header.m_lf3 != lf3_v1 ) && ( m_header.m_lf3 != lf3_v2 ) )
			{
				throw std::runtime_error{ "invalid lf3" };
			}

			if ( rez_version == rez::version_1 )
			{
				if ( m_header.m_detect_head != ( m_header.m_head ^ 0x11 ) )
				{
					throw std::runtime_error{ "(detect head) differs from (head ^ 0x11)" };
				}

				std::string cenc( m_header.m_detect_encode.size(), '\0' );
				std::to_chars(
					cenc.data(),
					cenc.data() + cenc.size(),
					std::atol( m_header.m_encode.data() ) ^ 0x16B4423 // magic number
				);

				if ( m_header.m_detect_encode != cenc )
				{
					throw std::runtime_error{ "(detect encode) differs from (encode)" };
				}

				if ( m_header.m_detect_tail != ( m_header.m_tail ^ 0x11 ) )
				{
					throw std::runtime_error{ "(detect tail) differs from (tail ^ 0x11)" };
				}
			}

			if ( ( m_header.m_file_format_version != 0x1 ) &&
				 ( m_header.m_file_format_version != 0x2 ) )
			{
				throw std::runtime_error{ "invalid file format version" };
			}
		}
		catch ( const std::exception& e )
		{
			std::string error{};

			error.append(
				rez_version == rez::version_1 ? "( version 1 )" : "( version 2 )"
			);

			error.append( " header failed with '" );

			error.append( e.what() );

			error.append( "'." );

			throw std::runtime_error{ error };
		}
	}

	/*
	 * Recursive read
	 */
	{
		m_rez.read(
			m_reader,
			m_header.m_root_dir_pos,
			m_header.m_root_dir_size
		);
	}

	/*
	 * Extract Rez
	 */
	{
		// 1MB
		constexpr std::uint32_t max_data_size{ 1'048'576u };

		auto data{ std::make_unique< char[] >( max_data_size ) };

		for ( const auto& dir : m_rez.m_directories )
		{
			std::filesystem::path path{};

			/*
			 * path to extract
			 */
			{
				std::vector< std::string > v{ dir.m_name };

				auto index{ dir.m_owner_index };
				while ( index >= 0 )
				{
					const auto& dir_owner{ m_rez.m_directories.at( index ) };

					v.emplace_back( dir_owner.m_name );

					index = dir_owner.m_owner_index;
				}

				for ( auto itr{ v.rbegin( ) }; itr != v.rend( ); ++itr )
				{
					path.append( *itr );

					if ( !std::filesystem::exists( output / path ) )
					{
						std::error_code ec{};
						if ( !std::filesystem::create_directory( output / path, ec ) )
						{
							throw std::runtime_error{ std::string{}.
								append( "create directory at '" ).
								append( ( output / path ).string( ) ).
								append( "' failed with '" ).
								append( ec.message( ) ).
								append( "'." )
							};
						}
					}
				}
			}

			for ( const auto& res : dir.m_resource )
			{
				auto filename{ res.m_name };
				if ( filename.empty( ) )
				{
					std::cout
						<< '['
						<< dir.m_name
						<< "] warning: empty resource filename detected!"
						<< std::endl;
				}

				if ( res.m_type.size( ) )
				{
					filename.append( "." ).append( res.m_type );
				}

				try
				{
					std::ofstream out{};
					out.exceptions( std::ios::badbit | std::ios::failbit );
					out.open( output / path / filename, std::ios::binary );

					if ( out && out.is_open( ) )
					{
						const std::uint32_t pos{ res.m_header.m_pos };
						const std::uint32_t size{ res.m_header.m_size };

						std::uint32_t step{};
						while ( step < size )
						{
							auto step_size{ std::min( size - step, max_data_size ) };

							m_reader.seek( pos + static_cast< std::streamoff >( step ) );
							m_reader.read( data[ 0u ], step_size );

							if ( step == 0u )
							{
								/**
								 * dtx file extension
								 */
								static constexpr auto DTX_EXT_A{ "dtx" };
								static constexpr auto DTX_EXT_B{ "DTX" };

								/**
								 * dtx header version
								 */
								static constexpr auto DTX_VER_LT1{ -2 };
								static constexpr auto DTX_VER_LT15{ -3 };
								static constexpr auto DTX_VER_LT2{ -5 };

								if ( ( res.m_type == DTX_EXT_A ) ||
									 ( res.m_type == DTX_EXT_B ) )
								{
									/**
									 * the dtx version in the header starts at offset 8, in some files it starts at offset 4
									 * so we need to swap these bytes, this way the file doesn't
									 * need to go through the dtx convert
									 */
									if ( data[ 0x4 ] != DTX_VER_LT1 &&
										 data[ 0x4 ] != DTX_VER_LT15 &&
										 data[ 0x4 ] != DTX_VER_LT2 )
									{
										std::array< char, 4u > lhs_bytes{};
										std::array< char, 4u > rhs_bytes{};

										std::memcpy( lhs_bytes.data(), &data[ 0x4 ], lhs_bytes.size() );
										std::memcpy( rhs_bytes.data(), &data[ 0x8 ], rhs_bytes.size() );

										std::memcpy( &data[ 0x4 ], rhs_bytes.data(), rhs_bytes.size() );
										std::memcpy( &data[ 0x8 ], lhs_bytes.data(), lhs_bytes.size() );
									}
								}
							}

							out.write( data.get( ), step_size );

							step += step_size;
						}
					}
				}
				catch ( const std::exception& e )
				{
					std::cout
						<< '['
						<< filename
						<< "] fatal error: "
						<< e.what( )
						<< std::endl;
				}
			}
		}
	}
}

void rez::extract(
	const std::vector<std::filesystem::path>& file_path,
	const std::wstring& save_path
)
{
	for ( const auto& file : file_path )
	{
		try
		{
			// open stream to read
			c_rez_file rez{ file };

			// extract rez info to save path
			rez.extract_to_file( save_path );
		}
		catch ( const std::exception& e )
		{
			std::cout
				<< "["
				<< file.filename( ).string( )
				<< "] fatal error: "
				<< e.what( )
				<< std::endl;
		}
	}
}