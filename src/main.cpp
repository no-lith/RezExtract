#include "console.hpp"
#include "file_dialog.hpp"
#include "rez.hpp"

#include <iostream>

int main( )
{
	try
	{
		rez::con::attach( "rez extract 1.0" );

		rez::extract(
			rez::fdg::open( ),
			rez::fdg::save( )
		);
	}
	catch ( const std::exception& e )
	{
		std::cout << e.what( ) << std::endl;
	}

	std::cout << std::endl << "press 'enter/return' key to exit...";

	std::cin.clear( );
	std::cin.get( );
}