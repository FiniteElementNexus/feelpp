/* -*- mode: c++; coding: utf-8; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; show-trailing-whitespace: t -*- vim:fenc=utf-8:ft=cpp:et:sw=4:ts=4:sts=4

  This file is part of the Feel library

  Author(s): Christophe Prud'homme <prudhomm@zion>
       Date: 2005-08-27

  Copyright (C) 2005,2006 EPFL

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 3.0 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
/**
   \file traits.hpp
   \author Christophe Prud'homme <prudhomm@zion>
   \date 2005-08-27
 */
#ifndef __FEELPP_MESH_TRAITS_HPP
#define __FEELPP_MESH_TRAITS_HPP 1

#include <feel/feelcore/traits.hpp>
#include <feel/feelmesh/simplex.hpp>
#include <feel/feelmesh/hypercube.hpp>


namespace Feel
{
/**
 * \class MeshTraits
 * \brief Traits for meshes
 *
 * @author Christophe Prud'homme
 */
template<typename MeshType>
struct MeshTraits
{
    /** @name Typedefs
     */
    //@{

    typedef MeshTraits<MeshType> self_type;
    typedef typename boost::remove_pointer<typename remove_shared_ptr<MeshType>::type >::type mesh_type;

    typedef typename mesh_type::shape_type element_shape_type;

    typedef typename mesh_type::element_type element_type;
    typedef typename mesh_type::face_type face_type;
    typedef typename mesh_type::edge_type edge_type;
    typedef typename mesh_type::point_type point_type;

    // element iterators
    typedef typename mesh_type::element_iterator element_iterator;
    typedef typename mesh_type::element_const_iterator element_const_iterator;

    typedef typename mesh_type::elements_reference_wrapper_type elements_reference_wrapper_type;
    typedef typename mesh_type::elements_reference_wrapper_ptrtype elements_reference_wrapper_ptrtype;
    typedef typename mesh_type::element_reference_wrapper_iterator element_reference_wrapper_iterator;
    typedef typename mesh_type::element_reference_wrapper_const_iterator element_reference_wrapper_const_iterator;

    // face iterators
    typedef typename mesh_type::face_iterator face_iterator;
    typedef typename mesh_type::face_const_iterator face_const_iterator;

    typedef typename mesh_type::faces_reference_wrapper_type faces_reference_wrapper_type;
    typedef typename mesh_type::faces_reference_wrapper_ptrtype faces_reference_wrapper_ptrtype;
    typedef typename mesh_type::face_reference_wrapper_iterator face_reference_wrapper_iterator;
    typedef typename mesh_type::face_reference_wrapper_const_iterator face_reference_wrapper_const_iterator;

    // edge iterators
    typedef typename mesh_type::edge_iterator edge_iterator;
    typedef typename mesh_type::edge_const_iterator edge_const_iterator;

    typedef typename mesh_type::edges_reference_wrapper_type edges_reference_wrapper_type;
    typedef typename mesh_type::edges_reference_wrapper_ptrtype edges_reference_wrapper_ptrtype;
    typedef typename mesh_type::edge_reference_wrapper_iterator edge_reference_wrapper_iterator;
    typedef typename mesh_type::edge_reference_wrapper_const_iterator edge_reference_wrapper_const_iterator;

    // point iterators
    typedef typename mesh_type::point_iterator point_iterator;
    typedef typename mesh_type::point_const_iterator point_const_iterator;

    typedef typename mesh_type::points_reference_wrapper_type points_reference_wrapper_type;
    typedef typename mesh_type::points_reference_wrapper_ptrtype points_reference_wrapper_ptrtype;
    typedef typename mesh_type::point_reference_wrapper_iterator point_reference_wrapper_iterator;
    typedef typename mesh_type::point_reference_wrapper_const_iterator point_reference_wrapper_const_iterator;

    //@}
};

template<typename T>
struct dimension_t : mpl::int_<decay_type<T>::nDim> {};
template<typename T>
constexpr uint16_type dimension_v = dimension_t<T>::value;
/**
 * @return topological dimension of a type T
 */
template<typename T>
inline constexpr int dimension( T const& t )
{
  return dimension_v<T>;
}

template<typename T>
struct real_dimension_t : mpl::int_<decay_type<T>::nRealDim> {};
template<typename T>
constexpr uint16_type real_dimension_v = real_dimension_t<T>::value;

/**
 * @return real dimension of a type T
 */
template<typename T>
inline constexpr int real_dimension( T const& t )
{
  return real_dimension_v<T>;
}



template<typename T>
struct is_3d : mpl::bool_<decay_type<T>::nDim == 3 /*|| decay_type<T>::nRealDim ==3*/> {};
template<typename T>
struct is_2d : mpl::bool_<decay_type<T>::nDim == 2 /*|| decay_type<T>::nRealDim ==2*/> {};
template<typename T>
struct is_1d : mpl::bool_<decay_type<T>::nDim == 1 /*|| decay_type<T>::nRealDim ==1*/> {};
template<typename T>
struct is_0d : mpl::bool_<decay_type<T>::nDim == 0 /*|| decay_type<T>::nRealDim ==0*/> {};

template<typename T>
struct is_3d_real : mpl::bool_< decay_type<T>::nRealDim ==3 > {};
template<typename T>
struct is_2d_real : mpl::bool_< decay_type<T>::nRealDim ==2 > {};
template<typename T>
struct is_1d_real : mpl::bool_< decay_type<T>::nRealDim ==1 > {};
template<typename T>
struct is_0d_real : mpl::bool_< decay_type<T>::nRealDim ==0 > {};


template<typename T>
struct is_topological_face : mpl::bool_<(decay_type<T>::nDim==decay_type<T>::nRealDim-1)> {};
template<typename T>
struct is_face : mpl::bool_<(decay_type<T>::nDim == 2 && decay_type<T>::nRealDim == 3)> {};
template<typename T>
struct is_edge : mpl::bool_<(decay_type<T>::nDim==1 && decay_type<T>::nRealDim == 3)||(decay_type<T>::nDim==1 && decay_type<T>::nRealDim == 2)> {};
template<typename T>
struct is_point : mpl::bool_<(decay_type<T>::nDim == 0)> {};

template<typename T>
struct is_convex : std::is_convertible<T,ConvexBase>::type {};

template<typename T>
struct is_simplex : std::is_base_of<SimplexBase, T>::type {};
template<typename T>
inline constexpr bool is_simplex_v = is_simplex<T>::value;

template<typename T>
struct is_triangle : mpl::and_<is_simplex<T>,is_2d<T>> {};
template<typename T>
struct is_tetrahedron : mpl::and_<is_simplex<T>,is_3d<T>> {};

template<typename T>
struct is_hypercube : std::is_base_of<HypercubeBase, T>::type {};
template <typename T>
inline constexpr bool is_hypercube_v = is_hypercube<T>::value;


template<typename T>
struct is_square : mpl::and_<is_hypercube<T>,is_2d<T>> {};
template<typename T>
struct is_cube : mpl::and_<is_hypercube<T>,is_3d<T>> {};

template<typename T>
struct is_segment : mpl::and_<is_convex<T>,is_1d<T>> {};

/**
 * Checks whether T is a GeoElement<n>D type. 
 * Provides the member constant value that is equal to true, if T is the type GeoElement<n>D. 
 * Otherwise, value is equal to false.
 */
template <typename T> struct is_geoelement: std::false_type {};
/**
 * Helper variable template
 * \return true is T is a GeoElement<n>D at compilation time, false otherwise
 */
template <typename... T>
inline constexpr bool is_geoelement_v = is_geoelement<T...>::value;


} // Feel
#endif /* __Traits_H */
