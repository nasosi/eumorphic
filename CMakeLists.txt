cmake_minimum_required( VERSION 3.11 )

option(HUNTER_ENABLED "Enable Hunter package manager" OFF) 

if(  HUNTER_ENABLED )
	cmake_policy(  SET CMP0074 NEW )
endif()

include("cmake/HunterGate.cmake")
HunterGate(
    URL "https://github.com/cpp-pm/hunter/archive/v0.23.221.tar.gz"
    SHA1 "8baada344e09c6a882934b9378bd253f2faa0ce3"
    LOCAL
)

project( eumorphic VERSION 0.1.0 )
hunter_add_package( Boost )

if ( HUNTER_ENABLED )
	find_package( Boost CONFIG  REQUIRED)
else( )
	find_package(Boost )
	if ( NOT Boost_FOUND )
		message( FATAL_ERROR "Boost was not found. Modify 'HUNTER_ENABLED' to ON to download using Hunter. It will take some time to download boost.")
	endif()
endif()


add_subdirectory( lib )
add_subdirectory( example )
add_subdirectory( bench )