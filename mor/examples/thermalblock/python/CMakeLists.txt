###  TEMPLATE.txt.tpl; coding: utf-8 ---

#  Author(s): Christophe Prud'homme <christophe.prudhomme@feelpp.org>
#       Date: 2011-04-23
#
#  Copyright (C) 2011 Universit� Joseph Fourier (Grenoble I)
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

mor_add_python_module( opusthermalblockpfem opusthermalblockpfem_wrapper.cpp LINK_LIBRARIES feel++_crb_crb SCRIPTS opusthermalblockpfemtest.py )
#mor_add_python_module( opusthermalblockcrb opusthermalblockcrb_wrapper.cpp LINK_LIBRARIES feel++_crb_crb SCRIPTS opusthermalblockcrbtest.py )


FILE(GLOB pyprogs "${CMAKE_CURRENT_SOURCE_DIR}/*.py" )
install(FILES ${pyprogs} DESTINATION share/doc/feel++-opus/thermalblock/python COMPONENT Bin)
