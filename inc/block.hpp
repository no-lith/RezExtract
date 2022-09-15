#ifndef BLOCK_HPP
#define BLOCK_HPP

#pragma once

#include "reader.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace rez{

/**
 * @brief 
 */
struct block_iterator_t {
	using iterator = std::vector< char >::iterator;

	block_iterator_t(
		iterator begin,
		iterator end
	) :
		m_begin{ begin != end ? &( *begin ) : nullptr },
		m_end{ begin != end ? &( *( end - 1u ) ) : nullptr },
		m_current{ m_begin },
		m_distance{ 0u }
	{}

	template< typename T = std::uint32_t >
	T read( )
	{
		const T value{ this->peek< T >( ) };

		this->advance( sizeof( T ) );

		return value;
	}
	std::string read_string_pointer( )
	{
		std::string value{};

		auto ptr{ this->current( ) };

		if ( ptr && *ptr != '\0' )
		{
			value = ptr;
		}

		this->advance( 4u ); // sizeof( char* ) x86

		return value;
	}
	std::string read_string( )
	{
		std::string value{};

		auto ptr{ this->current( ) };

		if ( ptr && *ptr != '\0' )
		{
			value = ptr;
		}

		this->advance(
			static_cast< std::uint32_t >( value.size( ) ) + 1u
		);

		return value;
	}
	template< typename T = std::uint32_t >
	T peek( )
	{
		return *reinterpret_cast< T* >( this->current( ) );
	}
	void advance( const std::uint32_t size )
	{
		m_current  += size;
		m_distance += size;
	}

	template< typename T = char* >
	auto current( )
	{
		return reinterpret_cast< T >( m_current );
	}
	template< typename T = char* >
	auto begin( )
	{
		return reinterpret_cast< T >( m_begin );
	}
	template< typename T = char* >
	auto end( )
	{
		return reinterpret_cast< T >( m_end );
	}

	char*         m_begin;
	char*         m_end;
	char*         m_current;
	std::uint32_t m_distance;
};

/**
 * @brief
 */
struct block_header_t {
	std::uint32_t m_type{};
	std::uint32_t m_pos{};
	std::uint32_t m_size{};
	std::uint32_t m_time{};
};

/**
 * @brief
 */
struct block_resource_t {
	block_header_t m_header{};

	std::uint32_t  m_id{};
	std::string    m_type{};
	std::uint32_t  m_num_keys{};
	std::string    m_name{};
	std::string    m_description{};

	auto read_resource( block_iterator_t& itr ) -> block_resource_t&;
};

/**
 * @brief
 */
struct directory_t {
	using block_t = std::vector< block_resource_t >;

	block_header_t m_header{};
	block_t        m_resource{};

	std::int32_t   m_owner_index{ -1 };

	std::string    m_name{};
};

/**
 * @brief
 */
struct rez_t {
	std::vector< directory_t > m_directories;

	void read(
		c_reader& reader,
		const std::uint32_t pos,
		const std::uint32_t size
	);
};

/**
 * @brief FileDirectoryEntryType
 */
enum file_directory_entry_type_ {
	file_directory_entry_type_resource  = 0,
	file_directory_entry_type_directory = 1
};

}

#endif