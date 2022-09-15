#include "block.hpp"

void rez::rez_t::read(
	c_reader& reader,
	const std::uint32_t pos,
	const std::uint32_t size
)
{
	if ( size == 0u )
	{
		throw std::runtime_error{ "invalid block size" };
	}

	std::vector< char > block_data( size, '\0' );

	reader.seek( pos );
	reader.read( block_data[ 0u ], block_data.size( ) );

	/**
	 * directory iterator
	 */
	block_iterator_t itr{
		block_data.begin( ),
		block_data.end( )
	};

	/**
	 * current directory owner
	 */
	auto directory_owner{ -1 };

	/**
	 * has another directory?
	 */
	if ( m_directories.size( ) )
	{
		/**
		 * save last directory index (owner directory)
		 */
		directory_owner = static_cast< std::int32_t >( m_directories.size( ) ) - 1;
	}

	//
	// go through all directories/files
	//
	while ( itr.current( ) < itr.end( ) )
	{
		auto header{ block_header_t{ itr.read( ) } };

		if ( header.m_type != file_directory_entry_type_resource &&
			 header.m_type != file_directory_entry_type_directory )
		{
			throw std::runtime_error{ "invalid block type" };
		}

		header.m_pos  = itr.read( );
		header.m_size = itr.read( );
		header.m_time = itr.read( );

		switch ( header.m_type )
		{
		case file_directory_entry_type_resource:
		{
			if ( m_directories.empty( ) )
			{
				throw std::runtime_error{ "can't get files in empty directory" };
			}

			m_directories.back( ).m_resource.emplace_back(
				block_resource_t{ header }.read_resource( itr )
			);

			break;
		}
		case file_directory_entry_type_directory:
		{
			auto& directory{ m_directories.emplace_back( directory_t{ header } ) };

			directory.m_name        = itr.read_string( );
			directory.m_owner_index = directory_owner;

			if ( directory.m_header.m_size )
			{
				/**
				 * save reader pos
				 */
				const auto pos{ reader.tell( ) };

				/**
				 * recursive read
				 */
				this->read(
					reader,
					directory.m_header.m_pos,
					directory.m_header.m_size
				);

				/**
				 * restore reader pos
				 */
				reader.seek( pos );
			}

			break;
		}
		}
	}
}

auto rez::block_resource_t::read_resource( block_iterator_t& itr ) -> block_resource_t&
{
	m_id   = itr.read( );
	m_type = itr.read_string_pointer( );

	/**
	 * reverse extension (i.e LMX to XML)
	 */
	std::reverse(
		m_type.begin( ),
		m_type.end( )
	);

	m_num_keys    = itr.read( );
	m_name        = itr.read_string( );
	m_description = itr.read_string( );

	if ( m_num_keys )
	{
		itr.advance(
			sizeof( m_num_keys ) * m_num_keys
		);
	}

	return *this;
}
