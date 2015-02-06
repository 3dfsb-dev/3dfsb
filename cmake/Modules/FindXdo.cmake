# - Find XDO on the development system.
# This module finds if XDO is installed and determines where the
# include files and libraries are. It also determines what the name of
# the library is. This code sets the following variables:
#
#  XDO_LIBRARIES           - path to the XDO library
#  XDO_INCLUDE_DIRS        - path to where xdo.h is found
#
#=============================================================================
# Copyright (c) 2012 Jacky Alcine <jacky.alcine@thesii.org>
#
# This module is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

find_package(PkgConfig QUIET)
pkg_check_modules(_XDO xdo QUIET)

find_path(XDO_INCLUDE_DIRS "xdo.h"
          HINTS ${_XDO_INCLUDEDIR} ${_XDO_INCLUDE_DIRS})
find_library(XDO_LIBRARIES "xdo"
    HINTS ${_XDO_LIBRARY_DIRS} ${_XDO_LIBDIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(XDO DEFAULT_MSG
    XDO_LIBRARIES XDO_INCLUDE_DIRS)

list(APPEND XDO_INCLUDE_DIRS ${_XDO_INCLUDE_DIRS})

mark_as_advanced(XDO_INCLUDE_DIRS XDO_LIBRARIES)