###  TEMPLATE.txt.tpl; coding: utf-8 ---

#  Author(s): Christophe Prud'homme <christophe.prudhomme@feelpp.org>
#       Date: 2012-05-23
#
#  Copyright (C) 2012 Universit� Joseph Fourier (Grenoble I)
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
set_directory_properties(PROPERTIES LABEL heat )

feelpp_add_test( heat NO_SEQ_TEST LINK_LIBRARIES Feelpp::feelpp_toolbox_heat_lib CLI "--config-file ${CMAKE_CURRENT_SOURCE_DIR}/cases/Building/ThermalBridgesENISO10211/case2.cfg " )

feelpp_add_test( cavity_triangular NO_MPI_TEST LINK_LIBRARIES Feelpp::feelpp CLI "--config-file ${CMAKE_CURRENT_SOURCE_DIR}/cases/triangular_cavity/triangular_cavity.cfg " DEFS FEELPP_DIM=2 FEELPP_ORDER=1  )
feelpp_add_test( cavity_rectangular NO_MPI_TEST LINK_LIBRARIES Feelpp::feelpp CLI "--config-file ${CMAKE_CURRENT_SOURCE_DIR}/cases/rectangular_cavity/rectangular_cavity.cfg " DEFS FEELPP_DIM=2 FEELPP_ORDER=1  )

if( LSB_RELEASE_CODENAME_SHORT MATCHES "noble" )
    feelpp_add_test(cavity_cylindrical NO_MPI_TEST SKIP_TEST LINK_LIBRARIES Feelpp::feelpp CLI "--config-file ${CMAKE_CURRENT_SOURCE_DIR}/cases/cylindrical_cavity/cylindrical_cavity.cfg " DEFS FEELPP_DIM=3 FEELPP_ORDER=1)
else()
    feelpp_add_test(cavity_cylindrical NO_MPI_TEST LINK_LIBRARIES Feelpp::feelpp CLI "--config-file ${CMAKE_CURRENT_SOURCE_DIR}/cases/cylindrical_cavity/cylindrical_cavity.cfg " DEFS FEELPP_DIM=3 FEELPP_ORDER=1)
endif()
