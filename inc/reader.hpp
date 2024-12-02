#ifndef READER_HPP
#define READER_HPP

#pragma once

namespace rez
{

class c_reader
{
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
				path.wstring().data(),
				std::ios::binary
			);
		}
		catch ( const std::ios_base::failure& e )
		{
			REZ_THROW( " - ifstream: {:s}", e.what() );
		}
	}
	~c_reader() = default;
public:
	template< typename T >
	auto read( T& v, const std::size_t& size ) -> void
	{
		m_stream.read(
			reinterpret_cast< char* >( &v ),
			size
		);

		if ( m_stream.gcount() != static_cast<std::streamsize>(size) )
			REZ_THROW( " - ifstream: invalid read (Expected: {:d} | Current: {:d}", size, m_stream.gcount() );
	}

	template< typename T = std::uint32_t >
	auto read() -> T
	{
		T v{};

		this->read( v, sizeof( T ) );

		return v;
	}

	auto peek() -> char
	{
		return static_cast< char >( m_stream.peek() );
	}

	template< typename T = std::uint16_t >
	auto read_string() -> std::string
	{
		std::string str( this->read< T >(), '\0' );

		if ( !str.empty() )
		{
			this->read(
				str[ 0u ],
				str.size()
			);

			str.erase(
				std::remove(
					str.begin(),
					str.end(),
					'\0'
				),
				str.end()
			);
		}

		return str;
	}

	auto seek( const std::streamoff& pos, const std::ios::seekdir dir = std::ios::beg ) -> void
	{
		m_stream.seekg( pos, dir );
	}

	auto tell() -> std::streamoff
	{
		return m_stream.tellg();
	}
private:
	std::ifstream m_stream;
};

}

#endif
