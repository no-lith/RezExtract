#ifndef READER_HPP
#define READER_HPP

#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

namespace rez {

/**
 * @brief 
 */
class c_reader {
public:
	c_reader(
		const std::filesystem::path& path
	) :
		m_stream{}
	{
		try
		{
			m_stream.exceptions(
				std::ifstream::eofbit |
				std::ifstream::failbit |
				std::ifstream::badbit
			);

			m_stream.open(
				path.wstring( ).data( ),
				std::ios::binary
			);
		}
		catch ( const std::ios_base::failure& e )
		{
			throw std::runtime_error{
				std::string{ "stream.open(): " } + e.what( )
			};
		}
	}
	~c_reader( ) = default;
public:
	template< typename T >
	void read( T& v, const std::size_t& size )
	{
		m_stream.read(
			reinterpret_cast< char* >( &v ),
			size
		);

		if ( m_stream.gcount( ) != size )
		{
			throw std::runtime_error{
				"stream.gcount() differs from v.size()"
			};
		}
	}

	template< typename T = std::uint32_t >
	T read( )
	{
		T v{};

		this->read( v, sizeof( T ) );

		return v;
	}

	char peek( )
	{
		return static_cast< char >( m_stream.peek( ) );
	}

	template< typename T = std::uint16_t >
	std::string read_string( )
	{
		std::string str( this->read< T >( ), '\0' );

		if ( !str.empty( ) )
		{
			this->read(
				str[ 0u ],
				str.size( )
			);

			str.erase(
				std::remove(
					str.begin( ),
					str.end( ),
					'\0'
				),
				str.end( )
			);
		}

		return str;
	}

	void seek( const std::streamoff& pos, const std::ios::seekdir dir = std::ios::beg )
	{
		m_stream.seekg( pos, dir );
	}
	std::streamoff tell( )
	{
		return m_stream.tellg( );
	}
private:
	std::ifstream m_stream;
};

}

#endif