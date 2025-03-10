/* -*- mode: c++; coding: utf-8; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; show-trailing-whitespace: t -*- vim:fenc=utf-8:ft=cpp:et:sw=4:ts=4:sts=4

  This file is part of the Feel library

  Author(s): Christoph Winkelmann <christoph.winkelmann@epfl.ch>
       Date: 2006-11-16

  Copyright (C) 2006 EPFL

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
   \file fsfunctionallinear.hpp
   \author Christoph Winkelmann <christoph.winkelmann@epfl.ch>
   \date 2006-11-16
 */

#ifndef FEELPP_DISCR_FSFUNCTIONALLINEAR_H
#define FEELPP_DISCR_FSFUNCTIONALLINEAR_H

#include <feel/feelalg/backend.hpp>
#include <feel/feeldiscr/fsfunctional.hpp>

#include <feel/feelvf/form.hpp>

namespace Feel
{

// Linear functional on function space, represented by M_vector
template<class Space>
class FsFunctionalLinear : public FsFunctional<Space>
{
public:

    // -- TYPEDEFS --
    typedef FsFunctionalLinear<Space> this_type;
    typedef FsFunctional<Space> super_type;

    typedef Space space_type;

    typedef std::shared_ptr<space_type> space_ptrtype;
    typedef typename space_type::element_type element_type;

    typedef typename space_type::value_type value_type;

    typedef Backend<value_type> backend_type;
    typedef std::shared_ptr<backend_type> backend_ptrtype;

    typedef typename backend_type::vector_type vector_type;
    typedef typename backend_type::vector_ptrtype vector_ptrtype;

    FsFunctionalLinear() :
        super_type(),
        M_backend( backend_type::build( soption( _name="backend" ) ) ),
        M_name("functionallinear")
    {
    }

    FsFunctionalLinear( space_ptrtype space, backend_ptrtype backend = backend_type::build( soption( _name="backend" ) ) )
        :
        super_type( space ),
        M_backend( backend ),
        M_vector( M_backend->newVector( space ) ),
        M_name( "functionallinear" )
    {
    }


    ~FsFunctionalLinear() override {}

    virtual void setSpace( space_ptrtype const& space ) override
    {
        super_type::setSpace(space);
        M_vector = M_backend->newVector(space);
    }

    void setName( std::string name ) { M_name = name; }
    std::string name() const { return M_name ; }

    // apply the functional
    virtual value_type
    operator()( const element_type& x ) const override
    {
        DCHECK( M_vector ) << "vector not init";
        return M_vector->dot( x.container() );
    }

    // get the representation vector
    vector_type const& container() const
    {
        return *M_vector;
    }

    vector_type& container()
    {
        return *M_vector;
    }

    // get the representation vector
    virtual vector_ptrtype const& containerPtr() const
    {
        return M_vector;
    }

    virtual vector_ptrtype& containerPtr()
    {
        return M_vector;
    }

    virtual void containerPtr( vector_ptrtype & vector )
    {
        vector = M_vector;
    }

    virtual void container( vector_type & vector )
    {
        vector = *M_vector;
    }

    // fill linear functional from linear form
    template<typename ExprT>
    this_type& operator=( ExprT const& e )
    {
        form1( _test=this->space(), _vector=M_vector ) = e;
        return *this;
    }

    // fill linear functional from linear form
    template<typename ExprT>
    this_type& operator+=( ExprT const& e )
    {
        form1( _test=this->space(), _vector=M_vector ) += e;
        return *this;
    }

    void close()
    {
        if ( ! M_vector->closed() )
        {
            M_vector->close();
        }
    }

    void
    add( this_type const& f )
    {
        M_vector->add( f.container() );
    }
private:

    backend_ptrtype M_backend;
    vector_ptrtype M_vector;
    std::string M_name;
}; // class FsFunctionalLinear


template <typename ... Ts>
auto functionalLinear( Ts && ... v )
{
    auto args = NA::make_arguments( std::forward<Ts>(v)... );
    auto && space = args.template get<NA::constraint::is_convertible<std::shared_ptr<FunctionSpaceBase>>::apply>(_space);
    using space_type = Feel::remove_shared_ptr_type<std::remove_pointer_t<std::decay_t<decltype(space)>>>;
    //auto && backend = args.get_else(_backend, Backend<typename space_type::value_type>::build( soption( _name="backend" ) ) );
    auto && backend = args.get_else_invocable( _backend, [](){ return Feel::backend(); } );

    return std::make_shared<FsFunctionalLinear<space_type>>( space , backend );
}



} // Feel

#endif /* _FSFUNCTIONALLINEAR_HPP_ */
