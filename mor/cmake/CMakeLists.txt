###  CMakeLists.txt; coding: utf-8 ---

#  Author(s): Christophe Prud'homme <christophe.prudhomme@feelpp.org>
#       Date: 2022-02-05
#
#  Copyright (C) 2022 Université de Strasbourg
#
# Distributed under the GPL(GNU Public License):
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#

set( FEELPP_MOR_CONFIG_FILE  ${CMAKE_CURRENT_BINARY_DIR}/Feel++-MORConfig.cmake)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Feel++-MORConfig.cmake.in ${FEELPP_MOR_CONFIG_FILE} @ONLY)
install(FILES ${FEELPP_MOR_CONFIG_FILE} DESTINATION share/feelpp/mor/cmake COMPONENT Devel)


set( modules
  ${CMAKE_CURRENT_SOURCE_DIR}/feelpp.macros.mor.cmake
  )

INSTALL(FILES ${modules} DESTINATION share/feelpp/mor/cmake/ COMPONENT Devel)
