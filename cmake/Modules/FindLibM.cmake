## FindLibM.cmake:
## Check for the presence of the LibM headers and libraries
## MvdS, 26/10/2011

# This CMake module defines the following variables:
#  LibM_FOUND        =  Libraries and headers found; TRUE/FALSE
#  LibM_INCLUDE_DIR     =  Path to the LibM header files
#  LibM_LIBRARIES    =  Path to all parts of the LibM libraries
#  LibM_LIBRARY_DIR  =  Path to the directory containing the LibM libraries




# Check for the header files:
find_path( LibM_INCLUDE_DIR
  NAMES math.h
  HINTS /usr/include /usr/local/include /usr/local/bic/include
  PATH_SUFFIXES
  )
#set( LibM_INCLUDE_DIR TRUE )



# Check for the libraries:
set( LibM_LIBRARIES "" )

find_library( LibM_LIBRARY
  NAMES m
  HINTS /usr/lib /usr/local/lib /usr/lib/x86_64-linux-gnu /usr/lib/i386-linux-gnu
  #PATH_SUFFIXES 
  NO_DEFAULT_PATH
  )

# Libraries found?
if( LibM_LIBRARY )
  
  list( APPEND LibM_LIBRARIES ${LibM_LIBRARY} )
  get_filename_component( LibM_LIBRARY_DIR ${LibM_LIBRARY} PATH )
  
endif( LibM_LIBRARY )




# Headers AND libraries found?
if( LibM_INCLUDE_DIR AND LibM_LIBRARIES )
  
  # yes!
  set( LibM_FOUND TRUE )
  
else( LibM_INCLUDE_DIR AND LibM_LIBRARIES )
  
  # no!
  set( LibM_FOUND FALSE )
  
  if( NOT LibM_FIND_QUIETLY )
    if( NOT LibM_INCLUDE_DIR )
      message( WARNING "Unable to find LibM header files!" )
    endif( NOT LibM_INCLUDE_DIR )
    if( NOT LibM_LIBRARIES )
      message( WARNING "Unable to find LibM library files!" )
    endif( NOT LibM_LIBRARIES )
  endif( NOT LibM_FIND_QUIETLY )
  
endif( LibM_INCLUDE_DIR AND LibM_LIBRARIES )




# Headers AND libraries found!
if( LibM_FOUND )
  
  if( NOT LibM_FIND_QUIETLY )
    message( STATUS "Found LibM: ${LibM_LIBRARIES}" )
  endif( NOT LibM_FIND_QUIETLY )
  
else( LibM_FOUND )
  
  if( LibM_FIND_REQUIRED )
    message( FATAL_ERROR "Could not find LibM headers or libraries!" )
  endif( LibM_FIND_REQUIRED )
  
endif( LibM_FOUND )



# Mark as advanced options in ccmake:
mark_as_advanced( 
  LibM_INCLUDE_DIR
  LibM_LIBRARIES
  LibM_LIBRARY
  LibM_LIBRARY_DIR
  )

