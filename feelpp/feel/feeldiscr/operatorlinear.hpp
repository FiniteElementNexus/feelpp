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
   \file operatorlinear.hpp
   \author Christoph Winkelmann <christoph.winkelmann@epfl.ch>
   \date 2006-11-16
 */

#ifndef FEELPP_DISCR_OPERATORLINEAR_H
#define FEELPP_DISCR_OPERATORLINEAR_H

#include <feel/feeldiscr/operator.hpp>

#include <feel/feelvf/expr.hpp>
#include <feel/feelvf/integrate.hpp>
#include <feel/feelvf/operators.hpp>
#include <feel/feelvf/inner.hpp>


namespace Feel
{
/**
 * \class OperatorLinear
 * \brief Linear Operator between function spaces, represented by a matrix
 */
template<class DomainSpace, class DualImageSpace>
class OperatorLinear : public Operator<DomainSpace, DualImageSpace>
{
    typedef Operator<DomainSpace,DualImageSpace> super;
public:

    // -- TYPEDEFS --
    typedef OperatorLinear<DomainSpace, DualImageSpace> this_type;
    typedef Operator<DomainSpace, DualImageSpace> super_type;
    typedef OperatorLinear<DualImageSpace,DomainSpace> adjoint_type;
    typedef std::shared_ptr<adjoint_type> adjoint_ptrtype;

    typedef typename super::domain_space_type domain_space_type;
    typedef typename super::dual_image_space_type  dual_image_space_type;
    typedef typename super::domain_space_ptrtype domain_space_ptrtype;
    typedef typename super::dual_image_space_ptrtype  dual_image_space_ptrtype;
    typedef typename domain_space_type::element_type domain_element_type;
    typedef typename dual_image_space_type::element_type dual_image_element_type;

    typedef typename super::backend_type backend_type;
    typedef typename super::backend_ptrtype backend_ptrtype;
    typedef typename backend_type::sparse_matrix_type matrix_type;
    typedef typename backend_type::vector_type vector_type;
    typedef typename backend_type::vector_ptrtype vector_ptrtype;
    typedef std::shared_ptr<matrix_type> matrix_ptrtype;

    template<typename T, typename Storage>
    struct domain_element: public super::domain_space_type::template Element<T,Storage> {};

    //template<typename T, typename Storage>
    //struct dual_image_element: public super::dual_image_space_type::template Element<T,Storage> {};

    typedef FsFunctionalLinear<DualImageSpace> image_element_type;

    OperatorLinear()
        :
        super_type(),
        M_backend( backend_type::build( soption( _name="backend" ) ) ),
        M_matrix(),
        M_pattern( Pattern::COUPLED ),
        M_name("operatorlinear")
    {}
    OperatorLinear( OperatorLinear const& ol ) = default;
    OperatorLinear( OperatorLinear && ol ) = default;

    OperatorLinear( domain_space_ptrtype     domainSpace,
                    dual_image_space_ptrtype dualImageSpace,
                    backend_ptrtype          backend,
                    bool buildMatrix = true ,
                    size_type pattern=Pattern::COUPLED)
        :
        super_type( domainSpace, dualImageSpace ),
        M_backend( backend ),
        M_pattern( pattern ),
        M_name( "operatorlinear" )
    {
        if ( buildMatrix ) M_matrix = M_backend->newMatrix( _trial=domainSpace, _test=dualImageSpace , _pattern=M_pattern );
    }

    ~OperatorLinear() override {}

    virtual void
    init( domain_space_ptrtype     domainSpace,
          dual_image_space_ptrtype dualImageSpace,
          backend_ptrtype          backend,
          bool buildMatrix = true ,
          size_type pattern = Pattern::COUPLED )
    {
        this->setDomainSpace( domainSpace );
        this->setDualImageSpace( dualImageSpace );
        M_backend = backend;
        M_pattern = pattern;
        if ( buildMatrix ) M_matrix = M_backend->newMatrix( _trial=domainSpace, _test=dualImageSpace , _pattern=M_pattern );
    }

    void setName( std::string name ) { M_name = name; }
    std::string name() const { return M_name ; }

    void setMatrix( matrix_ptrtype m ) { M_matrix = m; }

    // apply the operator: ie := Op de
    template<typename Storage>
    void
    apply( const domain_element<typename domain_element_type::value_type, Storage>& de,
           image_element_type&        ie ) const
    {
        if ( ! M_matrix->closed() )
        {
            M_matrix->close();
        }

        vector_ptrtype _v1( M_backend->newVector( _test=de.functionSpace() ) );
        *_v1 = de;_v1->close();
        vector_ptrtype _v2( M_backend->newVector( _test=ie.space() ) );
        M_backend->prod( M_matrix, _v1, _v2 );
        ie.container() = *_v2;
    }

    void
    apply( const domain_element_type& de,
           image_element_type&        ie ) const override
    {
        if ( ! M_matrix->closed() )
        {
            M_matrix->close();
        }

        vector_ptrtype _v1( M_backend->newVector( _test=de.functionSpace() ) );
        *_v1 = de;_v1->close();
        vector_ptrtype _v2( M_backend->newVector( _test=ie.space() ) );
        M_backend->prod( M_matrix, _v1, _v2 );
        ie.container() = *_v2;
    }

    virtual double
    energy( const typename domain_space_type::element_type & de,
            const typename dual_image_space_type::element_type & ie ) const
    {
        if ( ! M_matrix->closed() )
        {
            M_matrix->close();
        }
        vector_ptrtype _v1( M_backend->newVector( _test=de.functionSpace() ) );
        *_v1 = de;_v1->close();
        vector_ptrtype _v2( M_backend->newVector( _test=ie.functionSpace() ) );
        *_v2 = ie;_v2->close();
        vector_ptrtype _v3( M_backend->newVector( _test=ie.functionSpace() ) );
        M_backend->prod( M_matrix, _v1, _v3 );
        return inner_product( _v2, _v3 );
    }


    virtual void
    apply( const typename domain_space_type::element_type & de,
           typename dual_image_space_type::element_type & ie )
    {
        if ( ! M_matrix->closed() )
        {
            M_matrix->close();
        }

        vector_ptrtype _v1( M_backend->newVector( _test=de.functionSpace() ) );
        *_v1 = de;_v1->close();
        vector_ptrtype _v2( M_backend->newVector( _test=ie.functionSpace() ) );
        M_backend->prod( M_matrix, _v1, _v2 );
        ie.container() = *_v2;
    }

    template <typename T1 = typename domain_space_type::element_type,
             typename T2 = typename dual_image_space_type::element_type >
    T2
    operator()( T1 & de )
    {
        T2 elt_image( this->dualImageSpace(),"oio" );
        this->apply( de,elt_image );

        return elt_image;
    }

    template <typename T1 = typename domain_space_type::element_type,
             typename T2 = typename dual_image_space_type::element_type >
    T2
    operator()( std::shared_ptr<T1> de )
    {
        T2 elt_image( this->dualImageSpace(),"oio" );
        this->apply( *de,elt_image );

        return elt_image;
    }


    //! apply the inverse of the operator: \f$de = O^{-1} ie\f$
    virtual void
    applyInverse( domain_element_type&      de,
                  const image_element_type& ie )
    {
        if ( ! M_matrix->closed() )
        {
            M_matrix->close();
        }

        vector_ptrtype _v1( M_backend->newVector( _test=de.functionSpace() ) );
        vector_ptrtype _v2( M_backend->newVector( _test=ie.space() ) );
        *_v2 = ie.container();
        M_backend->solve( M_matrix, M_matrix, _v1, _v2 );
        de = *_v1;

#if 0

        if ( !M_backend->converged() )
        {
            std::cerr << "[OperatorLinear::applyInverse] "
                      << "solver failed to converge" << std::endl;
        }

#endif
    }


    // "improved version of applyInverse" :
    // take directly the expression of the rhs
    // return the result of inversion
    template<typename RhsExpr>
    domain_element_type
    inv( RhsExpr const& rhs_expr )
    {
        if ( ! M_matrix->closed() )
        {
            M_matrix->close();
        }

        domain_element_type de = this->domainSpace()->element();

        auto ie = M_backend->newVector( _test=this->dualImageSpace() );
        form1( _test=this->dualImageSpace(), _vector=ie ) =
            integrate( _range=elements( this->domainSpace()->mesh() ),
                       _expr=inner( rhs_expr , id( this->dualImageSpace()->element() ) ) );

        M_backend->solve( M_matrix, de, ie );

        return de;
    }


    // fill underlying matrix
    template<class ExprT>
    this_type& operator=( ExprT const& e )
    {
        //         M_matrix->clear();
        form2( _trial=this->domainSpace(),
               _test=this->dualImageSpace(),
               _matrix=M_matrix ) = e;
        return *this;
    }

    this_type& operator=( this_type const& m ) = default;
    this_type& operator=( this_type && m ) = default;

    // add to underlying matrix
    template<class ExprT>
    this_type& operator+=( ExprT const& e )
    {
        form2( _trial=this->domainSpace(),
               _test=this->dualImageSpace(),
               _matrix=M_matrix ) += e;
        return *this;
    }

    void close()
    {
        if ( ! M_matrix->closed() )
        {
            M_matrix->close();
        }

    }
    // retrieve underlying matrix
    matrix_type& mat()
    {
        return *M_matrix;
    }

    // retrieve underlying matrix
    matrix_type const& mat() const
    {
        return *M_matrix;
    }

    // retrieve underlying matrix
    matrix_ptrtype const& matPtr() const
    {
        return M_matrix;
    }

    // retrieve underlying matrix
    matrix_ptrtype& matPtr()
    {
        return M_matrix;
    }

    virtual void matPtr( matrix_ptrtype & matrix )
    {
        matrix = M_matrix;
    }

    template<typename T>
    OperatorLinear& add( T const& scalar, OperatorLinear const& ol )
    {
        this->close();
        M_matrix->addMatrix( scalar, *ol.M_matrix );
        return *this;
    }

    backend_ptrtype& backend()
    {
        return M_backend;
    }

    size_type pattern()
    {
        return M_pattern;
    }

    template<typename T>
    OperatorLinear& add( T const& scalar, std::shared_ptr<OperatorLinear> ol )
    {
        this->close();
        this->add( scalar, *ol );
        return *this;
    }

    OperatorLinear& operator+( std::shared_ptr<OperatorLinear> ol )
    {
        this->close();
        this->add( 1.0, *ol );
        return *this;
    }
    OperatorLinear& operator+( OperatorLinear const& ol )
    {
        this->close();
        this->add( 1.0, ol );
        return *this;
    }

    adjoint_ptrtype adjoint( size_type options = MATRIX_TRANSPOSE_ASSEMBLED ) const
    {
        auto opT = adjoint_ptrtype( new adjoint_type( this->dualImageSpace(), this->domainSpace(), M_backend, false ) );
        //opT->matPtr() = M_backend->newMatrix(_test=this->domainSpace(), _trial=this->dualImageSpace());
        if ( 1 ) //Context( options ). test( MATRIX_TRANSPOSE_ASSEMBLED ) )
            opT->matPtr() = M_backend->newMatrix( this->dualImageSpace()->dofOnOff(),
                                                  this->domainSpace()->dofOn(), (size_type) NON_HERMITIAN,false);
        else
            opT->matPtr() = M_backend->newMatrix();
        M_matrix->transpose(opT->matPtr(),options);
        return opT;
    }

private:

    backend_ptrtype M_backend;
    matrix_ptrtype M_matrix;
    size_type M_pattern;
    std::string M_name;
}; // class Operator


template <typename ... Ts>
auto opLinear( Ts && ... v )
{
    auto args = NA::make_arguments( std::forward<Ts>(v)... );
    auto && domainSpace = args.get(_domainSpace);
    auto && imageSpace = args.get(_imageSpace);
    using domain_space_type = Feel::remove_shared_ptr_type<std::remove_pointer_t<std::decay_t<decltype(domainSpace)>>>;
    using image_space_type = Feel::remove_shared_ptr_type<std::remove_pointer_t<std::decay_t<decltype(imageSpace)>>>;
    auto && backend = args.get_else_invocable(_backend,[](){ return Backend<typename domain_space_type::value_type>::build( soption( _name="backend" ) ); } );
    size_type pattern = args.get_else(_pattern,Pattern::COUPLED );

    return std::make_shared<OperatorLinear<domain_space_type,image_space_type>>( domainSpace,imageSpace,backend,pattern );
}




} // Feel

#endif /* _OPERATORLINEAR_HPP_ */
