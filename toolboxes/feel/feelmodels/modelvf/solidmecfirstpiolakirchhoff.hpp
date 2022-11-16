/* -*- mode: c++; coding: utf-8; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; show-trailing-whitespace: t -*- vim:fenc=utf-8:ft=cpp:et:sw=4:ts=4:sts=4

  This file is part of the Feel library

  Author(s): Vincent Chabannes
       Date: 2014-08-19

  Copyright (C) 2014 Universite Joseph Fourier (Grenoble I)

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
   \file solidmecfirstpiolakirchhoff.hpp
   \author Vincent Chabannes
   \date 2014-08-19
 */
#ifndef __FEELPP_MODELS_VF_SOLIDMEC_FIRSTPIOLAKIRCHHOFF_H
#define __FEELPP_MODELS_VF_SOLIDMEC_FIRSTPIOLAKIRCHHOFF_H 1

#include <feel/feelmodels/modelvf/exprtensorbase.hpp>
#include <feel/feelmodels/modelvf/exprevaluatefieldoperators.hpp>

namespace Feel
{
namespace FeelModels
{

enum class ExprApplyTypeSolidMecFirstPiolaKirchhoff { EVAL=0,JACOBIAN_TRIAL_DISP=1,JACOBIAN_TRIAL_PRES=2 };

// fwd declarations
template< typename SolidMecFirstPiolaKirchhoffBaseType,bool UseDispPresForm>
class SolidMecFirstPiolaKirchhoffLinearElasticity;
template< typename SolidMecFirstPiolaKirchhoffBaseType,bool UseDispPresForm>
class SolidMecFirstPiolaKirchhoffStVenantKirchhoff;

template<typename ExprEvaluateFieldOperatorsDispType,typename FiniteElementDisplacementType,
         typename ExprEvaluateFieldOperatorsPressureType,typename FiniteElementPressureType,
         typename ModelPhysicSolidType, typename MaterialPropertiesType, typename SymbolsExprType,
         ExprApplyTypeSolidMecFirstPiolaKirchhoff ExprApplied, int QuadOrder>
struct SolidMecFirstPiolaKirchhoffBase
{
    using this_type = SolidMecFirstPiolaKirchhoffBase<ExprEvaluateFieldOperatorsDispType,FiniteElementDisplacementType,ExprEvaluateFieldOperatorsPressureType,FiniteElementPressureType,ModelPhysicSolidType,MaterialPropertiesType,SymbolsExprType,ExprApplied,QuadOrder>;
    using model_physic_solid_type = ModelPhysicSolidType;
    using material_properties_type = MaterialPropertiesType;
    using symbols_expr_type = SymbolsExprType;
    using expr_evaluate_displacement_opertors_type = ExprEvaluateFieldOperatorsDispType;
    using expr_evaluate_pressure_opertors_type = ExprEvaluateFieldOperatorsPressureType;

    using value_type = typename expr_evaluate_displacement_opertors_type::value_type;

    static constexpr bool is_applied_as_eval = (ExprApplied == ExprApplyTypeSolidMecFirstPiolaKirchhoff::EVAL);
    static constexpr bool is_applied_as_jacobian_displacement = (ExprApplied == ExprApplyTypeSolidMecFirstPiolaKirchhoff::JACOBIAN_TRIAL_DISP);
    static constexpr bool is_applied_as_jacobian_pressure = (ExprApplied == ExprApplyTypeSolidMecFirstPiolaKirchhoff::JACOBIAN_TRIAL_PRES);

    static const size_type context = is_applied_as_eval || is_applied_as_jacobian_displacement?
        ExprEvaluateFieldOperatorsDispType::expr_grad_type::context|vm::DYNAMIC :
        ExprEvaluateFieldOperatorsPressureType::expr_id_type::context|vm::DYNAMIC ;

    static constexpr uint16_type nDim = model_physic_solid_type::nDim;
    static constexpr uint16_type nRealDim = nDim;
    typedef Shape<nDim, Tensor2, false, false> shape_type;

    static const bool is_terminal = true;

    template<typename Func>
    struct HasTestFunction
    {
        static const bool result = false;
    };

    template<typename Func>
    struct HasTrialFunction
    {
        static const bool result =
            ( is_applied_as_jacobian_displacement && std::is_same_v<Func,FiniteElementDisplacementType/*fe_displacement_type*/ /*FiniteElementVelocityType*/> ) ||
            ( is_applied_as_jacobian_pressure &&  std::is_same_v<Func,FiniteElementPressureType/*fe_pressure_type*/> );
    };
    using test_basis = std::nullptr_t;
    using trial_basis = std::nullptr_t;


    template<typename Geo_t, typename Basis_i_t, typename Basis_j_t>
    using tensor_base_type = tensorBase<Geo_t,Basis_i_t,Basis_j_t,shape_type,value_type>;

    template <int M,int N>
    using material_property_expr_type = std::decay_t<decltype( expr( ModelExpression{}.template expr<M,N>(),symbols_expr_type{} ) )>;

    static constexpr auto tuple_specialization_type_object = hana::to_tuple(hana::tuple_t<
                                                                            SolidMecFirstPiolaKirchhoffLinearElasticity<this_type,false>,
                                                                            SolidMecFirstPiolaKirchhoffLinearElasticity<this_type,true>,
                                                                            SolidMecFirstPiolaKirchhoffStVenantKirchhoff<this_type,false>,
                                                                            SolidMecFirstPiolaKirchhoffStVenantKirchhoff<this_type,true>
                                                                            >);

    SolidMecFirstPiolaKirchhoffBase( std::shared_ptr<expr_evaluate_displacement_opertors_type> exprEvaluateDisplacementOperators,
                                     std::shared_ptr<expr_evaluate_pressure_opertors_type> exprEvaluatePressureOperators,
                                     std::shared_ptr<model_physic_solid_type> const& physicSolidData, material_properties_type const& matProperties,
                                     symbols_expr_type const& se, double timeSteppingScaling = 1.0 )
        :
        M_exprEvaluateDisplacementOperators( exprEvaluateDisplacementOperators ),
        M_exprEvaluatePressureOperators( exprEvaluatePressureOperators ),
        M_physicSolidData( physicSolidData ),
        M_matProperties( matProperties ),
        M_se( se ),
        M_timeSteppingScaling( timeSteppingScaling )
        {}

    SolidMecFirstPiolaKirchhoffBase( SolidMecFirstPiolaKirchhoffBase const& ) = default;
    SolidMecFirstPiolaKirchhoffBase( SolidMecFirstPiolaKirchhoffBase && ) = default;


    static std::shared_ptr<this_type> New( std::shared_ptr<expr_evaluate_displacement_opertors_type> exprEvaluateDisplacementOperators,
                                           std::shared_ptr<expr_evaluate_pressure_opertors_type> exprEvaluatePressureOperators,
                                           std::shared_ptr<model_physic_solid_type> const& physicSolidData, material_properties_type const& matProperties,
                                           symbols_expr_type const& se, double timeSteppingScaling = 1.0 );

    std::shared_ptr<expr_evaluate_displacement_opertors_type> exprEvaluateDisplacementOperatorsPtr() const { return M_exprEvaluateDisplacementOperators; }

    ModelPhysicSolidType const& physicSolidData() const { CHECK(M_physicSolidData) << "ooooooiuiu"; return *M_physicSolidData; }
    MaterialPropertiesType const& matProperties() const { return M_matProperties; }
    SymbolsExprType const& symbolsExpr() const { return M_se; }
    double timeSteppingScaling() const { return M_timeSteppingScaling; }


    MaterialProperties const& materialProperties() const { return M_matProperties; }

    template <int M,int N>
    material_property_expr_type<M,N> materialPropertyExpr( std::string const& prop ) const { return expr( this->materialProperties().property( prop ).template expr<M,N>(), M_se ); }



    virtual size_type dynamicContext() const = 0;

    virtual uint16_type polynomialOrder() const = 0;

    template <typename TheSymbolExprType>
    bool hasSymbolDependency( std::string const& symb, TheSymbolExprType const& se ) const;

    template <typename OtherSymbolsExprType>
    auto applySymbolsExpr( OtherSymbolsExprType const& se ) const
        {
            // TODO concat se with M_se
            auto newse = se;//Feel::vf::symbolsExpr( this->symbolsExpressionWithoutTensorContext(), M_se );
            using new_se_type = std::decay_t<decltype(newse)>;
            using new_type = SolidMecFirstPiolaKirchhoffBase<ExprEvaluateFieldOperatorsDispType,FiniteElementDisplacementType,
                                                             ExprEvaluateFieldOperatorsPressureType,FiniteElementPressureType,
                                                             ModelPhysicSolidType,MaterialPropertiesType, new_se_type,
                                                             ExprApplied, QuadOrder>;
            return new_type::New( M_exprEvaluateDisplacementOperators,M_exprEvaluatePressureOperators,
                                  M_physicSolidData, M_matProperties,
                                  newse, M_timeSteppingScaling );
        }


    template<typename Geo_t, typename Basis_i_t, typename Basis_j_t>
    struct tensor : public this_type::template tensor_base_type<Geo_t,Basis_i_t,Basis_j_t>
    {
    protected :
        using super_type = typename this_type::template tensor_base_type<Geo_t,Basis_i_t,Basis_j_t>;
        using tensor_expr_evaluate_displacement_opertors_type = typename expr_evaluate_displacement_opertors_type::template tensor<Geo_t,Basis_i_t,Basis_j_t>;
        using tensor_expr_evaluate_pressure_opertors_type = typename expr_evaluate_pressure_opertors_type::template tensor<Geo_t,Basis_i_t,Basis_j_t>;
    public :

        typedef typename super_type::gmc_type gmc_type;
        typedef typename super_type::gm_type gm_type;
        typedef typename super_type::value_type value_type;

        using shape = typename super_type::shape_type;
        using matrix_shape_type = typename super_type::matrix_shape_type;

        using array_shape_type = typename super_type::new_array_shape_type;
        using ret_type = typename super_type::ret_type;

        template<typename... TheArgsType>
        tensor( this_type const& expr, Geo_t const& geom, const TheArgsType&... theInitArgs )
            :
            super_type( geom, theInitArgs... )
            {
                if ( expr.M_exprEvaluateDisplacementOperators )
                    M_tensorExprEvaluateDisplacementOperators = std::make_shared<tensor_expr_evaluate_displacement_opertors_type>( *(expr.M_exprEvaluateDisplacementOperators), geom, theInitArgs... );
                if ( expr.M_exprEvaluatePressureOperators )
                    M_tensorExprEvaluatePressureOperators = std::make_shared<tensor_expr_evaluate_pressure_opertors_type>( *(expr.M_exprEvaluatePressureOperators), geom, theInitArgs... );
            }

        void update( Geo_t const& geom ) override { CHECK( false ) << "should be override"; };
        void update( Geo_t const& geom, Basis_i_t const& fev, Basis_j_t const& feu ) override { CHECK( false ) << "should be override"; }
        void update( Geo_t const& geom, Basis_i_t const& fev ) override { CHECK( false ) << "should be override"; }

        template<typename TheExprExpandedType,typename TupleTensorSymbolsExprType, typename... TheArgsType>
        void update( std::true_type /**/, TheExprExpandedType const& exprExpanded, TupleTensorSymbolsExprType & ttse,
                     Geo_t const& geom, const TheArgsType&... theUpdateArgs );


        value_type
        evaliq( uint16_type i, uint16_type c1, uint16_type c2, uint16_type q ) const override
            {
                if constexpr ( this_type::is_applied_as_eval )
                    return this->evalq( c1,c2,q );
                else
                {
                    CHECK( false) << "not allow";
                    return value_type(0);
                }
            }
        ret_type
        evaliq( uint16_type i, uint16_type q ) const override
            {
                if constexpr ( this_type::is_applied_as_eval )
                    return this->evalq( q );
                else
                {
                    CHECK( false) << "not allow";
                    return ret_type(this->locMatrixShape().data());
               }
            }

        value_type
        evalq( uint16_type c1, uint16_type c2, uint16_type q ) const override
            {
                return M_localEval[q](c1,c2);
            }
        ret_type
        evalq( uint16_type q ) const override
            {
                return ret_type(M_localEval[q].data());
            }

    protected :
        std::shared_ptr<tensor_expr_evaluate_displacement_opertors_type> M_tensorExprEvaluateDisplacementOperators;
        std::shared_ptr<tensor_expr_evaluate_pressure_opertors_type> M_tensorExprEvaluatePressureOperators;
        std::vector<matrix_shape_type> M_localEval;
    };


    template<typename Geo_t, typename Basis_i_t, typename Basis_j_t,typename... TheArgsType>
    std::shared_ptr<tensor<Geo_t,Basis_i_t,Basis_j_t> >
    evaluator( const TheArgsType&... theInitArgs ) const;

    template<typename Geo_t, typename Basis_i_t, typename Basis_j_t,typename TheExprExpandedType,typename TupleTensorSymbolsExprType,typename... TheArgsType>
    std::shared_ptr<tensor<Geo_t,Basis_i_t,Basis_j_t> >
    evaluator( std::true_type/**/, TheExprExpandedType const& exprExpanded, TupleTensorSymbolsExprType & ttse, const TheArgsType&... theInitArgs ) const;


    template<typename Geo_t, typename Basis_i_t, typename Basis_j_t>
    friend struct tensor;

protected :
    std::shared_ptr<expr_evaluate_displacement_opertors_type> M_exprEvaluateDisplacementOperators;
    std::shared_ptr<expr_evaluate_pressure_opertors_type> M_exprEvaluatePressureOperators;
    std::shared_ptr<ModelPhysicSolidType> M_physicSolidData;
    MaterialPropertiesType const& M_matProperties;
    SymbolsExprType const& M_se;
    double M_timeSteppingScaling;
};

template< typename SolidMecFirstPiolaKirchhoffBaseType,bool UseDispPresForm>
class SolidMecFirstPiolaKirchhoffLinearElasticity : public SolidMecFirstPiolaKirchhoffBaseType
{
    using super_type = SolidMecFirstPiolaKirchhoffBaseType;
public :
    using this_type = SolidMecFirstPiolaKirchhoffLinearElasticity<SolidMecFirstPiolaKirchhoffBaseType,UseDispPresForm>;
    using material_property_scalar_expr_type = typename this_type::template material_property_expr_type<1,1>;

    static constexpr bool useDispPresForm = UseDispPresForm;

    SolidMecFirstPiolaKirchhoffLinearElasticity( std::shared_ptr<typename super_type::expr_evaluate_displacement_opertors_type> exprEvaluateDisplacementOperators,
                                                 std::shared_ptr<typename super_type::expr_evaluate_pressure_opertors_type> exprEvaluatePressureOperators,
                                                 std::shared_ptr<typename super_type::model_physic_solid_type> const& physicSolidData,
                                                 typename super_type::material_properties_type const& matProperties,
                                                 typename super_type::symbols_expr_type const& se, double timeSteppingScaling = 1.0 )
        :
        super_type( exprEvaluateDisplacementOperators,exprEvaluatePressureOperators,physicSolidData,matProperties,se,timeSteppingScaling ),
        M_exprLameSecondParameter( this->template materialPropertyExpr<1,1>("Lame-second-parameter") )
        {
            if constexpr ( !useDispPresForm )
               M_exprLameFirstParameter.emplace( this->template materialPropertyExpr<1,1>("Lame-first-parameter") );

            if constexpr ( this_type::is_applied_as_eval )
            {
                this->exprEvaluateDisplacementOperatorsPtr()->setEnableGrad( true );
                if constexpr ( useDispPresForm )
                       this->M_exprEvaluatePressureOperators->setEnableId( true );
            }
        }

    SolidMecFirstPiolaKirchhoffLinearElasticity( SolidMecFirstPiolaKirchhoffLinearElasticity const& ) = default;
    SolidMecFirstPiolaKirchhoffLinearElasticity( SolidMecFirstPiolaKirchhoffLinearElasticity &&  ) = default;

    material_property_scalar_expr_type const& exprLameFirstParameter() const { return *M_exprLameFirstParameter; }
    material_property_scalar_expr_type const& exprLameSecondParameter() const { return M_exprLameSecondParameter; }

    template <typename BB>
    static auto const& cast( BB const& exprExpanded  )
        {
            return static_cast<SolidMecFirstPiolaKirchhoffLinearElasticity<BB,UseDispPresForm> const&>( exprExpanded );
        }

    size_type dynamicContext() const override
        {
            size_type res = Feel::vf::dynamicContext( M_exprLameSecondParameter );
            if constexpr ( !useDispPresForm )
                res = res | Feel::vf::dynamicContext( *M_exprLameFirstParameter );
            if constexpr ( this_type::is_applied_as_eval )
            {
                res = res | this->exprEvaluateDisplacementOperatorsPtr()->dynamicContext();
                if constexpr ( useDispPresForm )
                    res = res | this->M_exprEvaluatePressureOperators->dynamicContext();
            }
            return res;
        }

    uint16_type polynomialOrder() const override
        {
            if constexpr ( this_type::is_applied_as_eval )
            {
                uint16_type orderGradDisp = this->exprEvaluateDisplacementOperatorsPtr()->exprGrad().polynomialOrder();
                if constexpr ( useDispPresForm )
                {
                    uint16_type orderIdPressure = this->M_exprEvaluatePressureOperators->exprId().polynomialOrder();
                    return std::max( orderIdPressure, (uint16_type) (M_exprLameSecondParameter.polynomialOrder() + orderGradDisp) );
                }
                else
                    return std::max( M_exprLameFirstParameter->polynomialOrder(),M_exprLameSecondParameter.polynomialOrder() ) + orderGradDisp;
            }
            else if constexpr ( this_type::is_applied_as_jacobian_displacement )
            {
                uint16_type orderGradDisp = super_type::expr_evaluate_displacement_opertors_type::polynomialOrderGrad();
                if constexpr ( useDispPresForm )
                    return M_exprLameSecondParameter.polynomialOrder() + orderGradDisp;
                else
                    return std::max( M_exprLameFirstParameter->polynomialOrder(),M_exprLameSecondParameter.polynomialOrder() ) + orderGradDisp;
            }
            else if constexpr ( this_type::is_applied_as_jacobian_pressure )
                return super_type::expr_evaluate_pressure_opertors_type::polynomialOrderId();

            return 0;
        }

    template <typename TheSymbolExprType>
    bool hasSymbolDependency( std::string const& symb, TheSymbolExprType const& se ) const
        {
             if constexpr ( useDispPresForm )
                 return M_exprLameSecondParameter.hasSymbolDependency( symb, se );
             else
                 return M_exprLameFirstParameter->M_exprLameSecondParameter.hasSymbolDependency( symb,se) || M_exprLameSecondParameter.hasSymbolDependency( symb, se );
        }


    template<typename Geo_t, typename Basis_i_t, typename Basis_j_t>
    struct tensor : public this_type::super_type::template tensor<Geo_t,Basis_i_t,Basis_j_t>
    {
        using super_type = typename this_type::super_type::template tensor<Geo_t,Basis_i_t,Basis_j_t>;
    public :
        typedef typename super_type::gmc_type gmc_type;
        typedef typename super_type::gm_type gm_type;
        typedef typename super_type::value_type value_type;

        //using shape = typename super_type::shape_type;
        using matrix_shape_type = typename super_type::matrix_shape_type;
        using array_shape_type = typename super_type::new_array_shape_type;
        using ret_type = typename super_type::ret_type;

        template<typename... TheArgsType>
        tensor( this_type const& expr, Geo_t const& geom, const TheArgsType&... theInitArgs )
            :
            super_type( expr, geom, theInitArgs... ),
            M_expr( expr ),
            M_tensorLameSecondParameter( M_expr.M_exprLameSecondParameter.evaluator( geom ) ),
            M_mId( matrix_shape_type::Identity() )
            {
                if constexpr ( !useDispPresForm )
                    M_tensorLameFirstParameter.emplace( M_expr.M_exprLameFirstParameter->evaluator( geom ) );
            }

        template<typename TheExprExpandedType,typename TupleTensorSymbolsExprType, typename... TheArgsType>
        tensor( std::true_type /**/, TheExprExpandedType const& exprExpanded, TupleTensorSymbolsExprType & ttse,
                this_type const& expr, Geo_t const& geom, const TheArgsType&... theInitArgs )
            :
            super_type( expr, geom, theInitArgs... ),
            M_expr( expr ),
            M_tensorLameSecondParameter( std::true_type{}, exprExpanded.exprLameSecondParameter(), ttse, expr.M_exprLameSecondParameter, geom/*, theInitArgs...*/ ),
            M_mId( matrix_shape_type::Identity() )
            {
                if constexpr ( !useDispPresForm )
                                 M_tensorLameFirstParameter.emplace( std::true_type{}, exprExpanded.exprLameFirstParameter(), ttse, *expr.M_exprLameFirstParameter, geom/*, theInitArgs...*/ );
            }

        void update( Geo_t const& geom ) override
            {
                M_tensorLameSecondParameter.update( geom );
                if constexpr ( !useDispPresForm )
                    M_tensorLameFirstParameter->update( geom );

                if constexpr ( this_type::is_applied_as_eval )
                {
                    this->M_tensorExprEvaluateDisplacementOperators->update( geom );
                    if constexpr ( useDispPresForm )
                          this->M_tensorExprEvaluatePressureOperators->update( geom );
                }

                this->updateImpl();
            }

        template<typename TheExprExpandedType,typename TupleTensorSymbolsExprType, typename... TheArgsType>
        void update( std::true_type /**/, TheExprExpandedType const& exprExpanded, TupleTensorSymbolsExprType & ttse,
                     Geo_t const& geom, const TheArgsType&... theUpdateArgs )
            {
                M_tensorLameSecondParameter.update( std::true_type{}, exprExpanded.exprLameSecondParameter(), ttse, geom/*, theUpdateArgs...*/ );
                if constexpr ( !useDispPresForm )
                                 M_tensorLameFirstParameter->update( std::true_type{}, exprExpanded.exprLameFirstParameter(), ttse, geom/*, theUpdateArgs...*/ );

                if constexpr ( this_type::is_applied_as_eval )
                {
                    this->M_tensorExprEvaluateDisplacementOperators->update( geom );
                    if constexpr ( useDispPresForm )
                         this->M_tensorExprEvaluatePressureOperators->update( geom );
                }

                this->updateImpl();
            }

        ret_type
        evalijq( uint16_type i, uint16_type j, uint16_type q ) const override
            {
                return evalijqImpl<this_type>( i,j,q );
            }


    private :

        void updateImpl()
        {
            if constexpr ( !this_type::is_applied_as_eval )
                 return;

            double timeSteppingScaling = M_expr.timeSteppingScaling();
            uint16_type nPoint = this->gmc()->nPoints();
            this->M_localEval.resize( nPoint, matrix_shape_type::Zero() );
            matrix_shape_type E;
            for ( uint16_type q = 0; q < nPoint; ++q )
            {
                matrix_shape_type & theLocRes = this->M_localEval[q];

                auto const& gradDisplacementEval = this->M_tensorExprEvaluateDisplacementOperators->localEvalGrad( q );
                //auto const& gradDisplacementEval = this->locGradDisplacement(q);
                const value_type du1vdx = gradDisplacementEval(0,0), du1vdy = gradDisplacementEval(0,1);
                const value_type du2vdx = gradDisplacementEval(1,0), du2vdy = gradDisplacementEval(1,1);

                if constexpr ( this_type::nRealDim == 2 )
                {
                    E(0,0) = du1vdx;
                    E(0,1) = 0.5*(du1vdy+du2vdx);
                    E(1,0) = E(0,1);
                    E(1,1) = du2vdy;
                }
                else if constexpr ( this_type::nRealDim == 3 )
                {
                    const value_type du1vdz = gradDisplacementEval(0,2);
                    const value_type du2vdz = gradDisplacementEval(1,2);
                    const value_type du3vdx = gradDisplacementEval(2,0), du3vdy = gradDisplacementEval(2,1), du3vdz = gradDisplacementEval(2,2);
                    E(0,0) = du1vdx;
                    E(0,1) = 0.5*(du1vdy+du2vdx);
                    E(0,2) = 0.5*(du1vdz+du3vdx);
                    E(1,0) = E(0,1);
                    E(1,1) = du2vdy;
                    E(1,2) = 0.5*(du2vdz+du3vdy);
                    E(2,0) = E(0,2);
                    E(2,1) = E(1,2);
                    E(2,2) = du3vdz;
                }

                if constexpr ( useDispPresForm )
                {
                    // p*Id+S_bis
                    const value_type coefflame2 = M_tensorLameSecondParameter.evalq(0,0,q);
                    const value_type pEval = this->M_tensorExprEvaluatePressureOperators->localEvalId( q )(0,0);
                    //const value_type pEval = this->locIdPressure()[q](0,0);
                    const value_type traceE = E.trace();
                    theLocRes = (pEval-(2.0/3.0)*coefflame2*traceE)*M_mId + (2*coefflame2*timeSteppingScaling)*E;
                }
                else
                {
                    const value_type coefflame1 = M_tensorLameFirstParameter->evalq(0,0,q);
                    const value_type coefflame2 = M_tensorLameSecondParameter.evalq(0,0,q);
                    theLocRes = coefflame1*E.trace()*M_mId + 2*coefflame2*E;
                    theLocRes *= timeSteppingScaling;
                }

            }

        }

        template <typename TT,std::enable_if_t< TT::is_applied_as_eval, bool> = true>
        ret_type
        evalijqImpl( uint16_type i, uint16_type j, uint16_type q ) const
        {
            //if constexpr ( expr_type::is_applied_as_eval )
            CHECK( false ) << "not allow";
            return ret_type(this->locMatrixShape().data());
        }

        template <typename TT,std::enable_if_t< TT::is_applied_as_jacobian_displacement, bool> = true>
        ret_type
        evalijqImpl( uint16_type i, uint16_type j, uint16_type q ) const
        {
            auto const& gradTrial = this->fecTrial()->grad( j, q );
            const value_type du1tdx = gradTrial( 0, 0, 0 ), du1tdy = gradTrial( 0, 1, 0 );
            const value_type du2tdx = gradTrial( 1, 0, 0 ), du2tdy = gradTrial( 1, 1, 0 );

            typename super_type::matrix_shape_type E;
            if constexpr ( this_type::nRealDim == 2 )
            {
                E(0,0) = du1tdx;
                E(0,1) = 0.5*(du1tdy+du2tdx);
                E(1,0) = E(0,1);
                E(1,1) = du2tdy;
            }
            else if constexpr ( this_type::nRealDim == 3 )
            {
                const value_type du1tdz = gradTrial( 0, 2, 0 );
                const value_type du2tdz = gradTrial( 1, 2, 0 );
                const value_type du3tdx = gradTrial( 2, 0, 0 ), du3tdy = gradTrial( 2, 1, 0 ), du3tdz = gradTrial( 2, 2, 0 );
                E(0,0) = du1tdx;
                E(0,1) = 0.5*(du1tdy+du2tdx);
                E(0,2) = 0.5*(du1tdz+du3tdx);
                E(1,0) = E(0,1);
                E(1,1) = du2tdy;
                E(1,2) = 0.5*(du2tdz+du3tdy);
                E(2,0) = E(0,2);
                E(2,1) = E(1,2);
                E(2,2) = du3tdz;
            }

            typename super_type::matrix_shape_type & thelocMat = this->locMatrixShape();
            if constexpr ( useDispPresForm )
            {
                const value_type coefflame2 = M_tensorLameSecondParameter.evalq(0,0,q);
                const value_type traceE = E.trace();
                thelocMat = 2*coefflame2*E - (2.0/3.0)*coefflame2*traceE*M_mId;
            }
            else
            {
                const value_type coefflame1 = M_tensorLameFirstParameter->evalq(0,0,q);
                const value_type coefflame2 = M_tensorLameSecondParameter.evalq(0,0,q);
                thelocMat = coefflame1*E.trace()*M_mId + 2*coefflame2*E;
            }
            return ret_type(thelocMat.data());
        }

        template <typename TT,std::enable_if_t< TT::is_applied_as_jacobian_pressure, bool> = true>
        ret_type
        evalijqImpl( uint16_type i, uint16_type j, uint16_type q ) const
        {
            // compute idt(p)*Id
            const value_type idTrialPressure = this->fecTrial()->id( j, 0, 0, q );
            typename super_type::matrix_shape_type & thelocMat = this->locMatrixShape();
            thelocMat = /*-*/idTrialPressure*M_mId;
            return ret_type(thelocMat.data());
        }

    private :
        this_type const& M_expr;
        using material_property_scalar_tensor_type = typename material_property_scalar_expr_type::template tensor<Geo_t/*, Basis_i_t, Basis_j_t*/>;
        std::optional<material_property_scalar_tensor_type> M_tensorLameFirstParameter;
        material_property_scalar_tensor_type M_tensorLameSecondParameter;
        matrix_shape_type M_mId;
    };

    template<typename Geo_t, typename Basis_i_t, typename Basis_j_t>
    friend struct tensor;

private :

    std::optional<material_property_scalar_expr_type> M_exprLameFirstParameter;
    material_property_scalar_expr_type M_exprLameSecondParameter;
};


/**
 * -------------------------------------------------------------------------------------------------  *
 * -------------------------------------------------------------------------------------------------  *
 * StVenantKirchhoff :
 *   S = \lambda trace(E)*Id + 2*\mu*E
 *   S = 2*\mu*E ( + pressure part : p*Id )
 * -------------------------------------------------------------------------------------------------  *
 * -------------------------------------------------------------------------------------------------  *
 */

template< typename SolidMecFirstPiolaKirchhoffBaseType,bool UseDispPresForm>
class SolidMecFirstPiolaKirchhoffStVenantKirchhoff : public SolidMecFirstPiolaKirchhoffBaseType
{
    using super_type = SolidMecFirstPiolaKirchhoffBaseType;
public :
    using this_type = SolidMecFirstPiolaKirchhoffStVenantKirchhoff<SolidMecFirstPiolaKirchhoffBaseType,UseDispPresForm>;
    using material_property_scalar_expr_type = typename this_type::template material_property_expr_type<1,1>;

    static constexpr bool useDispPresForm = UseDispPresForm;

    SolidMecFirstPiolaKirchhoffStVenantKirchhoff( std::shared_ptr<typename super_type::expr_evaluate_displacement_opertors_type> exprEvaluateDisplacementOperators,
                                                  std::shared_ptr<typename super_type::expr_evaluate_pressure_opertors_type> exprEvaluatePressureOperators,
                                                  std::shared_ptr<typename super_type::model_physic_solid_type> const& physicSolidData,
                                                  typename super_type::material_properties_type const& matProperties,
                                                  typename super_type::symbols_expr_type const& se, double timeSteppingScaling = 1.0 )
        :
        super_type( exprEvaluateDisplacementOperators,exprEvaluatePressureOperators,physicSolidData,matProperties,se,timeSteppingScaling ),
        M_exprLameSecondParameter( this->template materialPropertyExpr<1,1>("Lame-second-parameter") )
        {
            if constexpr ( !useDispPresForm )
               M_exprLameFirstParameter.emplace( this->template materialPropertyExpr<1,1>("Lame-first-parameter") );

            this->exprEvaluateDisplacementOperatorsPtr()->setEnableGrad( true );
            if constexpr ( this_type::is_applied_as_eval || this_type::is_applied_as_jacobian_displacement )
            {
                if constexpr ( useDispPresForm )
                       this->M_exprEvaluatePressureOperators->setEnableId( true );
            }
        }

    SolidMecFirstPiolaKirchhoffStVenantKirchhoff( SolidMecFirstPiolaKirchhoffStVenantKirchhoff const& ) = default;
    SolidMecFirstPiolaKirchhoffStVenantKirchhoff( SolidMecFirstPiolaKirchhoffStVenantKirchhoff &&  ) = default;

    material_property_scalar_expr_type const& exprLameFirstParameter() const { return *M_exprLameFirstParameter; }
    material_property_scalar_expr_type const& exprLameSecondParameter() const { return M_exprLameSecondParameter; }

    template <typename BB>
    static auto const& cast( BB const& exprExpanded  )
        {
            return static_cast<SolidMecFirstPiolaKirchhoffStVenantKirchhoff<BB,UseDispPresForm> const&>( exprExpanded );
        }

    size_type dynamicContext() const override
        {
            size_type res = Feel::vf::dynamicContext( M_exprLameSecondParameter );
            if constexpr ( !useDispPresForm )
                res = res | Feel::vf::dynamicContext( *M_exprLameFirstParameter );
            res = res | this->exprEvaluateDisplacementOperatorsPtr()->dynamicContext();
            if constexpr ( this_type::is_applied_as_eval || this_type::is_applied_as_jacobian_displacement )
            {
                if constexpr ( useDispPresForm )
                    res = res | this->M_exprEvaluatePressureOperators->dynamicContext();
            }
            return res;
        }

    uint16_type polynomialOrder() const override
        {
            uint16_type orderGradDisp = this->exprEvaluateDisplacementOperatorsPtr()->exprGrad().polynomialOrder();
            if constexpr ( this_type::is_applied_as_eval || this_type::is_applied_as_jacobian_displacement )
            {
                if constexpr ( useDispPresForm  )
                {
                    uint16_type orderIdPressure = this->M_exprEvaluatePressureOperators->exprId().polynomialOrder();
                    return std::max( orderIdPressure + orderGradDisp, M_exprLameSecondParameter.polynomialOrder() + 3*orderGradDisp );
                }
                else
                    return std::max( M_exprLameFirstParameter->polynomialOrder(),M_exprLameSecondParameter.polynomialOrder() ) + 3*orderGradDisp;
            }
            else if constexpr ( this_type::is_applied_as_jacobian_pressure )
                return super_type::expr_evaluate_pressure_opertors_type::polynomialOrderId() + orderGradDisp;

            return 0;
        }

    template <typename TheSymbolExprType>
    bool hasSymbolDependency( std::string const& symb, TheSymbolExprType const& se ) const
        {
             if constexpr ( useDispPresForm )
                 return M_exprLameSecondParameter.hasSymbolDependency( symb, se );
             else
                 return M_exprLameFirstParameter->M_exprLameSecondParameter.hasSymbolDependency( symb,se) || M_exprLameSecondParameter.hasSymbolDependency( symb, se );
        }


    template<typename Geo_t, typename Basis_i_t, typename Basis_j_t>
    struct tensor : public this_type::super_type::template tensor<Geo_t,Basis_i_t,Basis_j_t>
    {
        using super_type = typename this_type::super_type::template tensor<Geo_t,Basis_i_t,Basis_j_t>;
    public :
        typedef typename super_type::gmc_type gmc_type;
        typedef typename super_type::gm_type gm_type;
        typedef typename super_type::value_type value_type;

        //using shape = typename super_type::shape_type;
        using matrix_shape_type = typename super_type::matrix_shape_type;
        using array_shape_type = typename super_type::new_array_shape_type;
        using ret_type = typename super_type::ret_type;

        template<typename... TheArgsType>
        tensor( this_type const& expr, Geo_t const& geom, const TheArgsType&... theInitArgs )
            :
            super_type( expr, geom, theInitArgs... ),
            M_expr( expr ),
            M_tensorLameSecondParameter( M_expr.M_exprLameSecondParameter.evaluator( geom ) ),
            M_mId( matrix_shape_type::Identity() )
            {
                //M_mId.setIdentity();
                if constexpr ( !useDispPresForm )
                    M_tensorLameFirstParameter.emplace( M_expr.M_exprLameFirstParameter->evaluator( geom ) );
            }

        template<typename TheExprExpandedType,typename TupleTensorSymbolsExprType, typename... TheArgsType>
        tensor( std::true_type /**/, TheExprExpandedType const& exprExpanded, TupleTensorSymbolsExprType & ttse,
                this_type const& expr, Geo_t const& geom, const TheArgsType&... theInitArgs )
            :
            super_type( expr, geom, theInitArgs... ),
            M_expr( expr ),
            M_tensorLameSecondParameter( std::true_type{}, exprExpanded.exprLameSecondParameter(), ttse, expr.M_exprLameSecondParameter, geom/*, theInitArgs...*/ ),
            M_mId( matrix_shape_type::Identity() )
            {
                //M_mId.setIdentity();
                if constexpr ( !useDispPresForm )
                                 M_tensorLameFirstParameter.emplace( std::true_type{}, exprExpanded.exprLameFirstParameter(), ttse, *expr.M_exprLameFirstParameter, geom/*, theInitArgs...*/ );
            }

        void update( Geo_t const& geom ) override
            {
                M_tensorLameSecondParameter.update( geom );
                if constexpr ( !useDispPresForm )
                    M_tensorLameFirstParameter->update( geom );

                this->M_tensorExprEvaluateDisplacementOperators->update( geom );
                if constexpr ( this_type::is_applied_as_eval || this_type::is_applied_as_jacobian_displacement )
                {
                    if constexpr ( useDispPresForm )
                          this->M_tensorExprEvaluatePressureOperators->update( geom );
                }

                this->updateImpl();
            }

        template<typename TheExprExpandedType,typename TupleTensorSymbolsExprType, typename... TheArgsType>
        void update( std::true_type /**/, TheExprExpandedType const& exprExpanded, TupleTensorSymbolsExprType & ttse,
                     Geo_t const& geom, const TheArgsType&... theUpdateArgs )
            {
                M_tensorLameSecondParameter.update( std::true_type{}, exprExpanded.exprLameSecondParameter(), ttse, geom/*, theUpdateArgs...*/ );
                if constexpr ( !useDispPresForm )
                                 M_tensorLameFirstParameter->update( std::true_type{}, exprExpanded.exprLameFirstParameter(), ttse, geom/*, theUpdateArgs...*/ );

                this->M_tensorExprEvaluateDisplacementOperators->update( geom );
                if constexpr ( this_type::is_applied_as_eval || this_type::is_applied_as_jacobian_displacement )
                {
                    if constexpr ( useDispPresForm )
                         this->M_tensorExprEvaluatePressureOperators->update( geom );
                }

                this->updateImpl();
            }

        ret_type
        evalijq( uint16_type i, uint16_type j, uint16_type q ) const override
            {
                return evalijqImpl<this_type>( i,j,q );
            }


    private :

        void updateImpl()
        {
            if constexpr ( !(this_type::is_applied_as_eval || this_type::is_applied_as_jacobian_displacement) )
                 return;

            double timeSteppingScaling = M_expr.timeSteppingScaling();
            uint16_type nPoint = this->gmc()->nPoints();
            this->M_localEval.resize( nPoint, matrix_shape_type::Zero() );
            matrix_shape_type E;
            for ( uint16_type q = 0; q < nPoint; ++q )
            {
                matrix_shape_type & theLocRes = this->M_localEval[q];

                auto const& gradDisplacementEval = this->M_tensorExprEvaluateDisplacementOperators->localEvalGrad( q );

                if constexpr ( this_type::nRealDim == 2 )
                {
                    const value_type du1vdx = gradDisplacementEval(0,0), du1vdy = gradDisplacementEval(0,1);
                    const value_type du2vdx = gradDisplacementEval(1,0), du2vdy = gradDisplacementEval(1,1);
                    const value_type subtraceE1 = 0.5*(std::pow(du1vdx,2)+std::pow(du2vdx,2));
                    const value_type subtraceE2 = 0.5*(std::pow(du1vdy,2)+std::pow(du2vdy,2));
                    E(0,0) = du1vdx + subtraceE1;
                    E(0,1) = 0.5*( du1vdy + du2vdx + du1vdx*du1vdy + du2vdx*du2vdy );
                    E(1,0) = E(0,1);
                    E(1,1) = du2vdy + subtraceE2;
                }
                else if constexpr ( this_type::nRealDim == 3 )
                {
                    const value_type du1vdx = gradDisplacementEval(0,0), du1vdy = gradDisplacementEval(0,1), du1vdz = gradDisplacementEval(0,2);
                    const value_type du2vdx = gradDisplacementEval(1,0), du2vdy = gradDisplacementEval(1,1), du2vdz = gradDisplacementEval(1,2);
                    const value_type du3vdx = gradDisplacementEval(2,0), du3vdy = gradDisplacementEval(2,1), du3vdz = gradDisplacementEval(2,2);
                    const value_type subtraceE1 = 0.5*(std::pow(du1vdx,2)+std::pow(du2vdx,2)+std::pow(du3vdx,2));
                    const value_type subtraceE2 = 0.5*(std::pow(du1vdy,2)+std::pow(du2vdy,2)+std::pow(du3vdy,2));
                    const value_type subtraceE3 = 0.5*(std::pow(du1vdz,2)+std::pow(du2vdz,2)+std::pow(du3vdz,2));

                    E(0,0) = du1vdx + subtraceE1;
                    E(0,1) = 0.5*( du1vdy + du2vdx + du1vdx*du1vdy + du2vdx*du2vdy + du3vdx*du3vdy );
                    E(0,2) = 0.5*( du1vdz + du3vdx + du1vdx*du1vdz + du2vdx*du2vdz + du3vdx*du3vdz );
                    //E(1,0) = 0.5*( du2vdx + du1vdy + du1vdy*du1vdx + du2vdy*du2vdx + du3vdy*du3vdx );
                    E(1,0) = E(0,1);
                    E(1,1) = du2vdy + subtraceE2;
                    E(1,2) = 0.5*( du2vdz + du3vdy + du1vdy*du1vdz + du2vdy*du2vdz + du3vdy*du3vdz );
                    //E(2,0) = 0.5*( du3vdx + du1vdz + du1vdz*du1vdx + du2vdz*du2vdx + du3vdz*du3vdx );
                    //E(2,1) = 0.5*( du3vdy + du2vdz + du1vdz*du1vdy + du2vdz*du2vdy + du3vdz*du3vdy );
                    E(2,0) = E(0,2);
                    E(2,1) = E(1,2);
                    E(2,2) = du3vdz + subtraceE3;
                }

                if constexpr ( useDispPresForm )
                {
                    // p*Id+S_bis
                    const value_type coefflame2 = M_tensorLameSecondParameter.evalq(0,0,q);
                    const value_type pEval = this->M_tensorExprEvaluatePressureOperators->localEvalId( q )(0,0);
                    auto S = pEval*M_mId + 2*coefflame2*timeSteppingScaling*E;
                    if constexpr ( this_type::is_applied_as_jacobian_displacement )
                    {
                        theLocRes = S;
                    }
                    else
                    {
                        auto F = M_mId + gradDisplacementEval;
                        theLocRes = F*S;
                    }
                }
                else
                {
                    const value_type coefflame1 = M_tensorLameFirstParameter->evalq(0,0,q);
                    const value_type coefflame2 = M_tensorLameSecondParameter.evalq(0,0,q);
                    auto S = coefflame1*E.trace()*M_mId + 2*coefflame2*E;
                    if constexpr ( this_type::is_applied_as_jacobian_displacement )
                    {
                        theLocRes = S;
                    }
                    else
                    {
                        auto F = M_mId + gradDisplacementEval;
                        theLocRes = timeSteppingScaling*F*S;
                        //theLocRes *= timeSteppingScaling;
                    }
                }
            }
        }

        template <typename TT,std::enable_if_t< TT::is_applied_as_eval, bool> = true>
        ret_type
        evalijqImpl( uint16_type i, uint16_type j, uint16_type q ) const
        {
            CHECK( false ) << "not allow";
            return ret_type(this->locMatrixShape().data());
        }

        template <typename TT,std::enable_if_t< TT::is_applied_as_jacobian_displacement, bool> = true>
        ret_type
        evalijqImpl( uint16_type i, uint16_type j, uint16_type q ) const noexcept
        {
            auto const& gradTrial = this->fecTrial()->grad( j, q );
            auto const& gradDisplacementEval = this->M_tensorExprEvaluateDisplacementOperators->localEvalGrad( q );
            auto const& localEval = this->M_localEval[q];

            Eigen::Map< const Eigen::Matrix<typename super_type::value_type,super_type::shape_type::M,super_type::shape_type::N/*,Eigen::ColMajor*/ > >
                gradTrial_matrix( gradTrial.data() );
            // dFS = dF*S
            typename super_type::matrix_shape_type dFS = gradTrial_matrix*localEval;
            typename super_type::matrix_shape_type dE;
            //typename super_type::matrix_shape_type & thelocMat = this->locMatrixShape();
            typename super_type::matrix_shape_type thelocMat; // seems faster if thelocMat is a local variable (mayber cache access?)

#if 0
            dE = 0.5*(gradTrial_matrix + gradTrial_matrix.transpose() + gradDisplacementEval.transpose()*gradTrial_matrix + gradTrial_matrix.transpose()*gradDisplacementEval );
            auto F = M_mId + gradDisplacementEval;
#if 1
            auto dS = coefflame1*dE.trace()*M_mId + 2*coefflame2*dE;
#else
            typename super_type::matrix_shape_type dS = coeffMult*dE;
            dS.diagonal().array() += coeffOnDiag;
#endif
            thelocMat = F*dS + dFS;
#endif
            if constexpr ( this_type::nRealDim == 2 )
            {
                const value_type du1tdx = gradTrial( 0, 0, 0 ), du1tdy = gradTrial( 0, 1, 0 );
                const value_type du2tdx = gradTrial( 1, 0, 0 ), du2tdy = gradTrial( 1, 1, 0 );
                const value_type du1vdx = gradDisplacementEval(0,0), du1vdy = gradDisplacementEval(0,1);
                const value_type du2vdx = gradDisplacementEval(1,0), du2vdy = gradDisplacementEval(1,1);

                //auto dE = sym(gradt(u)) + 0.5*(trans(gradv(u))*gradt(u) + trans(gradt(u))*gradv(u));
                dE(0,0) = du1tdx + /*0.5**/(du1vdx*du1tdx + du2vdx*du2tdx);
                dE(0,1) = 0.5*( du1tdy + du2tdx + du1tdx*du1vdy + du2tdx*du2vdy + du1vdx*du1tdy + du2vdx*du2tdy );
                dE(1,0) = dE(0,1);
                dE(1,1) = du2tdy + du1vdy*du1tdy + du2vdy*du2tdy;

                const value_type coefflame2 = M_tensorLameSecondParameter.evalq(0,0,q);
                const value_type coeffMult = 2*coefflame2/**timeSteppingScaling*/;

                value_type coeffOnDiag = 0;
                if constexpr ( !useDispPresForm )
                {
                    const value_type tracedE = dE.trace();
                    const value_type coefflame1 = M_tensorLameFirstParameter->evalq(0,0,q);
                    coeffOnDiag = coefflame1*tracedE/**timeSteppingScaling*/;
                }

                // auto dS = idv(CoeffLame1)*trace(dE)*Id + 2*idv(CoeffLame2)*dE;
                const value_type dS11 = coeffOnDiag + coeffMult*dE(0,0);
                const value_type dS12 = coeffMult*dE(0,1);
                const value_type dS21 = dS12;//coeffMult*dE(1,0);
                const value_type dS22 = coeffOnDiag + coeffMult*dE(1,1);

                const value_type F11 = 1 + du1vdx, F22 = 1 + du2vdy;
                thelocMat(0,0) = F11*dS11 + du1vdy*dS21;
                thelocMat(0,1) = F11*dS12 + du1vdy*dS22;
                thelocMat(1,0) = du2vdx*dS11 + F22*dS21;
                thelocMat(1,1) = du2vdx*dS12 + F22*dS22;
                thelocMat += dFS;
            }
            else if constexpr ( this_type::nRealDim == 3 )
            {
                const value_type du1tdx = gradTrial( 0, 0, 0 ), du1tdy = gradTrial( 0, 1, 0 ), du1tdz=gradTrial( 0, 2, 0 );
                const value_type du2tdx = gradTrial( 1, 0, 0 ), du2tdy = gradTrial( 1, 1, 0 ), du2tdz=gradTrial( 1, 2, 0 );
                const value_type du3tdx = gradTrial( 2, 0, 0 ), du3tdy = gradTrial( 2, 1, 0 ), du3tdz=gradTrial( 2, 2, 0 );
                const value_type du1vdx = gradDisplacementEval(0,0), du1vdy = gradDisplacementEval(0,1), du1vdz = gradDisplacementEval(0,2);
                const value_type du2vdx = gradDisplacementEval(1,0), du2vdy = gradDisplacementEval(1,1), du2vdz = gradDisplacementEval(1,2);
                const value_type du3vdx = gradDisplacementEval(2,0), du3vdy = gradDisplacementEval(2,1), du3vdz = gradDisplacementEval(2,2);

                //auto dE = sym(gradt(u)) + 0.5*(trans(gradv(u))*gradt(u) + trans(gradt(u))*gradv(u));
                dE(0,0) = du1tdx + /*0.5**/(du1vdx*du1tdx + du2vdx*du2tdx + du3vdx*du3tdx);
                dE(0,1) = 0.5*( du1tdy + du2tdx + du1tdx*du1vdy + du2tdx*du2vdy + du3tdx*du3vdy + du1vdx*du1tdy + du2vdx*du2tdy + du3vdx*du3tdy );
                dE(0,2) = 0.5*( du1tdz + du3tdx + du1tdx*du1vdz + du2tdx*du2vdz + du3tdx*du3vdz + du1vdx*du1tdz + du2vdx*du2tdz + du3vdx*du3tdz );
                //dE(1,0) = 0.5*( du2tdx + du1tdy + du1tdy*du1vdx + du2tdy*du2vdx + du3tdy*du3vdx + du1vdy*du1tdx + du2vdy*du2tdx + du3vdy*du3tdx );
                dE(1,0) = dE(0,1);
                dE(1,1) = du2tdy + du1vdy*du1tdy + du2vdy*du2tdy + du3vdy*du3tdy;
                dE(1,2) = 0.5*( du2tdz + du3tdy + du1tdy*du1vdz + du2tdy*du2vdz + du3tdy*du3vdz + du1vdy*du1tdz + du2vdy*du2tdz + du3vdy*du3tdz );
                //dE(2,0) = 0.5*( du3tdx + du1tdz + du1tdz*du1vdx + du2tdz*du2vdx + du3tdz*du3vdx + du1vdz*du1tdx + du2vdz*du2tdx + du3vdz*du3tdx );
                //dE(2,1) = 0.5*( du3tdy + du2tdz + du1tdz*du1vdy + du2tdz*du2vdy + du3tdz*du3vdy + du1vdz*du1tdy + du2vdz*du2tdy + du3vdz*du3tdy );
                dE(2,0) = dE(0,2);
                dE(2,1) = dE(1,2);
                dE(2,2) = du3tdz + du1vdz*du1tdz + du2vdz*du2tdz + du3vdz*du3tdz;

                const value_type coefflame2 = M_tensorLameSecondParameter.evalq(0,0,q);
                const value_type coeffMult = 2*coefflame2/**timeSteppingScaling*/;

                value_type coeffOnDiag = 0;
                if constexpr ( !useDispPresForm )
                {
                    const value_type tracedE = dE.trace();
                    const value_type coefflame1 = M_tensorLameFirstParameter->evalq(0,0,q);
                    coeffOnDiag = coefflame1*tracedE/**timeSteppingScaling*/;
                }

                // auto dS = idv(CoeffLame1)*trace(dE)*Id + 2*idv(CoeffLame2)*dE;
                const value_type dS11 = coeffOnDiag + coeffMult*dE(0,0);
                const value_type dS12 = coeffMult*dE(0,1);
                const value_type dS13 = coeffMult*dE(0,2);
                const value_type dS21 = dS12;//coeffMult*dE(1,0);
                const value_type dS22 = coeffOnDiag + coeffMult*dE(1,1);
                const value_type dS23 = coeffMult*dE(1,2);
                const value_type dS31 = dS13;//coeffMult*dE(2,0);
                const value_type dS32 = dS23;//coeffMult*dE(2,1);
                const value_type dS33 = coeffOnDiag + coeffMult*dE(2,2);

                const value_type F11 = 1 + du1vdx, F22 = 1 + du2vdy, F33 = 1 + du3vdz;
                thelocMat(0,0) = F11*dS11 + du1vdy*dS21 + du1vdz*dS31;
                thelocMat(0,1) = F11*dS12 + du1vdy*dS22 + du1vdz*dS32;
                thelocMat(0,2) = F11*dS13 + du1vdy*dS23 + du1vdz*dS33;
                thelocMat(1,0) = du2vdx*dS11 + F22*dS21 + du2vdz*dS31;
                thelocMat(1,1) = du2vdx*dS12 + F22*dS22 + du2vdz*dS32;
                thelocMat(1,2) = du2vdx*dS13 + F22*dS23 + du2vdz*dS33;
                thelocMat(2,0) = du3vdx*dS11 + du3vdy*dS21 + F33*dS31;
                thelocMat(2,1) = du3vdx*dS12 + du3vdy*dS22 + F33*dS32;
                thelocMat(2,2) = du3vdx*dS13 + du3vdy*dS23 + F33*dS33;
                thelocMat += dFS;
            }

            this->locMatrixShape() = thelocMat;
            //return ret_type(thelocMat.data());
            return ret_type(this->locMatrixShape().data());
        }

        template <typename TT,std::enable_if_t< TT::is_applied_as_jacobian_pressure, bool> = true>
        ret_type
        evalijqImpl( uint16_type i, uint16_type j, uint16_type q ) const
        {
            // compute idt(p)*F
            const value_type idTrialPressure = this->fecTrial()->id( j, 0, 0, q );
            auto const& gradDisplacementEval = this->M_tensorExprEvaluateDisplacementOperators->localEvalGrad( q );
            typename super_type::matrix_shape_type & thelocMat = this->locMatrixShape();
            thelocMat = /*-*/idTrialPressure*(M_mId+gradDisplacementEval);
            return ret_type(thelocMat.data());
        }

    private :
        this_type const& M_expr;
        using material_property_scalar_tensor_type = typename material_property_scalar_expr_type::template tensor<Geo_t/*, Basis_i_t, Basis_j_t*/>;
        std::optional<material_property_scalar_tensor_type> M_tensorLameFirstParameter;
        material_property_scalar_tensor_type M_tensorLameSecondParameter;
        matrix_shape_type M_mId;
    };

    template<typename Geo_t, typename Basis_i_t, typename Basis_j_t>
    friend struct tensor;

private :

    std::optional<material_property_scalar_expr_type> M_exprLameFirstParameter;
    material_property_scalar_expr_type M_exprLameSecondParameter;
};



















template<typename ExprEvaluateFieldOperatorsDispType,typename FiniteElementDisplacementType,
         typename ExprEvaluateFieldOperatorsPressureType,typename FiniteElementPressureType,
         typename ModelPhysicSolidType, typename MaterialPropertiesType, typename SymbolsExprType,
         ExprApplyTypeSolidMecFirstPiolaKirchhoff ExprApplied, int QuadOrder>
std::shared_ptr<SolidMecFirstPiolaKirchhoffBase<ExprEvaluateFieldOperatorsDispType,FiniteElementDisplacementType,ExprEvaluateFieldOperatorsPressureType,FiniteElementPressureType,ModelPhysicSolidType,MaterialPropertiesType,SymbolsExprType,ExprApplied,QuadOrder>>
SolidMecFirstPiolaKirchhoffBase<ExprEvaluateFieldOperatorsDispType,FiniteElementDisplacementType,ExprEvaluateFieldOperatorsPressureType,FiniteElementPressureType,
                                ModelPhysicSolidType,MaterialPropertiesType,SymbolsExprType,ExprApplied,QuadOrder>::New( std::shared_ptr<expr_evaluate_displacement_opertors_type> exprEvaluateDisplacementOperators,
                                                                                                                         std::shared_ptr<expr_evaluate_pressure_opertors_type> exprEvaluatePressureOperators,
                                                                                                                         std::shared_ptr<model_physic_solid_type> const& physicSolidData, material_properties_type const& matProperties,
                                                                                                                         symbols_expr_type const& se, double timeSteppingScaling )
{
    bool useDispPresForm = physicSolidData->useDisplacementPressureFormulation();
    if ( physicSolidData->equation() == "Elasticity" )
    {
        if ( useDispPresForm )
            return std::make_shared<SolidMecFirstPiolaKirchhoffLinearElasticity<this_type,true>>( exprEvaluateDisplacementOperators,exprEvaluatePressureOperators,physicSolidData,matProperties,se,timeSteppingScaling );
        else
            return std::make_shared<SolidMecFirstPiolaKirchhoffLinearElasticity<this_type,false>>( exprEvaluateDisplacementOperators,exprEvaluatePressureOperators,physicSolidData,matProperties,se,timeSteppingScaling );
    }
    else if ( physicSolidData->equation() == "Hyper-Elasticity" )
    {
        if ( physicSolidData->materialModel() == "StVenantKirchhoff" )
        {
            if ( useDispPresForm )
                return std::make_shared<SolidMecFirstPiolaKirchhoffStVenantKirchhoff<this_type,true>>( exprEvaluateDisplacementOperators,exprEvaluatePressureOperators,physicSolidData,matProperties,se,timeSteppingScaling );
            else
                return std::make_shared<SolidMecFirstPiolaKirchhoffStVenantKirchhoff<this_type,false>>( exprEvaluateDisplacementOperators,exprEvaluatePressureOperators,physicSolidData,matProperties,se,timeSteppingScaling );
        }
        else
            CHECK ( false ) << "invalid materialLaw : "<< physicSolidData->materialModel();
    }
    else
        CHECK( false ) << "equation not implemented : " << physicSolidData->equation();
    return std::shared_ptr<this_type>{};
}



template<typename ExprEvaluateFieldOperatorsDispType,typename FiniteElementDisplacementType,
         typename ExprEvaluateFieldOperatorsPressureType,typename FiniteElementPressureType,
         typename ModelPhysicSolidType, typename MaterialPropertiesType, typename SymbolsExprType,
         ExprApplyTypeSolidMecFirstPiolaKirchhoff ExprApplied, int QuadOrder>
template<typename Geo_t, typename Basis_i_t, typename Basis_j_t,typename... TheArgsType>
std::shared_ptr<typename SolidMecFirstPiolaKirchhoffBase<ExprEvaluateFieldOperatorsDispType,FiniteElementDisplacementType,
                                                         ExprEvaluateFieldOperatorsPressureType,FiniteElementPressureType,
                                                         ModelPhysicSolidType,MaterialPropertiesType,SymbolsExprType,ExprApplied,QuadOrder>::template tensor<Geo_t,Basis_i_t,Basis_j_t> >
SolidMecFirstPiolaKirchhoffBase<ExprEvaluateFieldOperatorsDispType,FiniteElementDisplacementType,ExprEvaluateFieldOperatorsPressureType,FiniteElementPressureType,
                                ModelPhysicSolidType,MaterialPropertiesType,SymbolsExprType,ExprApplied,QuadOrder>::evaluator( const TheArgsType&... theInitArgs ) const
{
    std::shared_ptr<tensor<Geo_t,Basis_i_t,Basis_j_t>> ret;
    hana::for_each( tuple_specialization_type_object, [this,&ret,&theInitArgs...]( auto e ) {
                                                          if ( ret )
                                                              return;
                                                          using the_expr_type = typename std::decay_t<decltype( e ) >::type;
                                                          if ( auto objptr = dynamic_cast<the_expr_type const*>(this) )
                                                              ret = std::make_shared< typename the_expr_type::template tensor<Geo_t,Basis_i_t,Basis_j_t> >( *objptr, theInitArgs... );
                                                      });
    CHECK( ret ) << "cast failed : something wrong";
    return ret;
}


template<typename ExprEvaluateFieldOperatorsDispType,typename FiniteElementDisplacementType,
         typename ExprEvaluateFieldOperatorsPressureType,typename FiniteElementPressureType,
         typename ModelPhysicSolidType, typename MaterialPropertiesType, typename SymbolsExprType,
         ExprApplyTypeSolidMecFirstPiolaKirchhoff ExprApplied, int QuadOrder>
template<typename Geo_t, typename Basis_i_t, typename Basis_j_t,typename TheExprExpandedType,typename TupleTensorSymbolsExprType,typename... TheArgsType>
std::shared_ptr<typename SolidMecFirstPiolaKirchhoffBase<ExprEvaluateFieldOperatorsDispType,FiniteElementDisplacementType,
                                                         ExprEvaluateFieldOperatorsPressureType,FiniteElementPressureType,
                                                         ModelPhysicSolidType,MaterialPropertiesType,SymbolsExprType,ExprApplied,QuadOrder>::template tensor<Geo_t,Basis_i_t,Basis_j_t> >
SolidMecFirstPiolaKirchhoffBase<ExprEvaluateFieldOperatorsDispType,FiniteElementDisplacementType,ExprEvaluateFieldOperatorsPressureType,FiniteElementPressureType,
                                ModelPhysicSolidType,MaterialPropertiesType,SymbolsExprType,ExprApplied,QuadOrder>::evaluator( std::true_type/**/, TheExprExpandedType const& exprExpanded, TupleTensorSymbolsExprType & ttse,
                                                                                                                               const TheArgsType&... theInitArgs ) const
{
    std::shared_ptr<tensor<Geo_t,Basis_i_t,Basis_j_t>> ret;
    hana::for_each( tuple_specialization_type_object, [this,&ret,&exprExpanded,&ttse,&theInitArgs...]( auto e ) {
                                                          if ( ret )
                                                              return;
                                                          using the_expr_type = typename std::decay_t<decltype( e ) >::type;
                                                          if ( auto objptr = dynamic_cast<the_expr_type const*>(this) )
                                                              ret = std::make_shared< typename the_expr_type::template tensor<Geo_t,Basis_i_t,Basis_j_t> >( std::true_type{}, the_expr_type::cast( exprExpanded ), ttse, *objptr, theInitArgs... );
                                                      });
    CHECK( ret ) << "cast failed : something wrong";
    return ret;
}


template<typename ExprEvaluateFieldOperatorsDispType,typename FiniteElementDisplacementType,
         typename ExprEvaluateFieldOperatorsPressureType,typename FiniteElementPressureType,
         typename ModelPhysicSolidType, typename MaterialPropertiesType, typename SymbolsExprType,
         ExprApplyTypeSolidMecFirstPiolaKirchhoff ExprApplied, int QuadOrder>
template <typename TheSymbolExprType>
bool
SolidMecFirstPiolaKirchhoffBase<ExprEvaluateFieldOperatorsDispType,FiniteElementDisplacementType,ExprEvaluateFieldOperatorsPressureType,FiniteElementPressureType,ModelPhysicSolidType,MaterialPropertiesType,SymbolsExprType,ExprApplied,QuadOrder>::hasSymbolDependency( std::string const& symb, TheSymbolExprType const& se ) const
{
    return false; // TODO
}




template<typename ExprEvaluateFieldOperatorsDispType,typename FiniteElementDisplacementType,
         typename ExprEvaluateFieldOperatorsPressureType,typename FiniteElementPressureType,
         typename ModelPhysicSolidType, typename MaterialPropertiesType, typename SymbolsExprType,
         ExprApplyTypeSolidMecFirstPiolaKirchhoff ExprApplied, int QuadOrder>
template<typename Geo_t, typename Basis_i_t, typename Basis_j_t>
template<typename TheExprExpandedType,typename TupleTensorSymbolsExprType, typename... TheArgsType>
void
SolidMecFirstPiolaKirchhoffBase<ExprEvaluateFieldOperatorsDispType,FiniteElementDisplacementType,
                                ExprEvaluateFieldOperatorsPressureType,FiniteElementPressureType,
                                ModelPhysicSolidType,MaterialPropertiesType,SymbolsExprType,ExprApplied,QuadOrder>::tensor<Geo_t,Basis_i_t,Basis_j_t>::update( std::true_type /**/, TheExprExpandedType const& exprExpanded, TupleTensorSymbolsExprType & ttse,
                                                                                                                                                               Geo_t const& geom, const TheArgsType&... theUpdateArgs )
{
    bool isUp = false;
    hana::for_each( tuple_specialization_type_object, [this,&isUp,&exprExpanded,&ttse,&geom,&theUpdateArgs...]( auto e ) {
                                                          if ( isUp )
                                                              return;
                                                          using the_expr_type = typename std::decay_t<decltype( e ) >::type;
                                                          using the_expr_tensor_type = typename the_expr_type::template tensor<Geo_t,Basis_i_t,Basis_j_t>;
                                                          if ( auto objptr = dynamic_cast<the_expr_tensor_type*>(this) )
                                                          {
                                                              objptr->update( std::true_type{}, the_expr_type::cast( exprExpanded ), ttse, geom, theUpdateArgs... );
                                                              isUp = true;
                                                          }
                                                      });
    CHECK( isUp ) << "cast failed : something wrong";
}







     
#if 0

template< typename ExprType>
template<typename Geo_t, typename Basis_i_t, typename Basis_j_t,typename... TheArgsType>
std::shared_ptr<typename SolidMecFirstPiolaKirchhoffBase<ExprType>::template tensor_base_type<Geo_t,Basis_i_t,Basis_j_t> >
SolidMecFirstPiolaKirchhoffBase<ExprType>::evaluator( tensor_main_type<Geo_t,Basis_i_t,Basis_j_t> const& tensorExprMain, const TheArgsType&... theInitArgs ) const
{
    auto const& physicSolidData = this->expr().physicSolidData();
    if ( physicSolidData.equation() == "Elasticity" )
    {
        //try{ auto const& hhh = dynamic_cast<SolidMecFirstPiolaKirchhoffLinearElasticity<expr_type> const&>(*this); } catch (...) { CHECK( false ) << "iiii"; }
        //if ( physicSolidData.useDisplacementPressureFormulation() )
        return std::make_shared< typename SolidMecFirstPiolaKirchhoffLinearElasticity<expr_type>::template tensor<Geo_t,Basis_i_t,Basis_j_t>>( static_cast<SolidMecFirstPiolaKirchhoffLinearElasticity<expr_type> const&>(*this), tensorExprMain, theInitArgs... );
    }
    else
        CHECK ( false ) << "invalid equation : "<< physicSolidData.equation() <<"\n";

#if 0
    auto const& dynamicViscosity = this->expr().dynamicViscosity();
    if ( dynamicViscosity.isNewtonianLaw() )
        return std::make_shared< typename FluidMecDynamicViscosityNewtonian<expr_type>::template tensor<Geo_t,Basis_i_t,Basis_j_t>>( static_cast<FluidMecDynamicViscosityNewtonian<expr_type> const&>(*this), tensorExprMain, theInitArgs... );
    else if ( dynamicViscosity.isPowerLaw() )
        return std::make_shared< typename FluidMecDynamicViscosityPowerLaw<expr_type>::template tensor<Geo_t,Basis_i_t,Basis_j_t>>( static_cast<FluidMecDynamicViscosityPowerLaw<expr_type> const&>(*this), tensorExprMain, theInitArgs... );
    else if ( dynamicViscosity.isCarreauLaw() )
        return std::make_shared< typename FluidMecDynamicViscosityCarreau<expr_type>::template tensor<Geo_t,Basis_i_t,Basis_j_t>>( static_cast<FluidMecDynamicViscosityCarreau<expr_type> const&>(*this), tensorExprMain, theInitArgs... );
    else if ( dynamicViscosity.isCarreauYasudaLaw() )
        return std::make_shared< typename FluidMecDynamicViscosityCarreauYasuda<expr_type>::template tensor<Geo_t,Basis_i_t,Basis_j_t>>( static_cast<FluidMecDynamicViscosityCarreauYasuda<expr_type> const&>(*this), tensorExprMain, theInitArgs... );
    else
        CHECK ( false ) << "invalid viscosity model : "<< dynamicViscosity.lawName() <<"\n";
#endif
    return std::shared_ptr<tensor_base_type<Geo_t,Basis_i_t,Basis_j_t> >{};
}

template< typename ExprType>
template<typename Geo_t, typename Basis_i_t, typename Basis_j_t,typename TheExprExpandedType,typename TupleTensorSymbolsExprType,typename... TheArgsType>
std::shared_ptr<typename SolidMecFirstPiolaKirchhoffBase<ExprType>::template tensor_base_type<Geo_t,Basis_i_t,Basis_j_t> >
SolidMecFirstPiolaKirchhoffBase<ExprType>::evaluator( std::true_type/**/, tensor_main_type<Geo_t,Basis_i_t,Basis_j_t> const& tensorExprMain,
                                                      TheExprExpandedType const& exprExpanded, TupleTensorSymbolsExprType & ttse, const TheArgsType&... theInitArgs ) const
{
    CHECK(false)<<"TODO";
#if 0
    using expr_expanded_type = typename TheExprExpandedType::expr_type;

    auto const& dynamicViscosity = this->expr().dynamicViscosity();
    if ( dynamicViscosity.isNewtonianLaw() )
        return std::make_shared< typename FluidMecDynamicViscosityNewtonian<expr_type>::template tensor<Geo_t,Basis_i_t,Basis_j_t>>( std::true_type{},
                                                                                                                                     static_cast<FluidMecDynamicViscosityNewtonian<expr_expanded_type> const&>(exprExpanded) ,
                                                                                                                                     ttse,
                                                                                                                                     static_cast<FluidMecDynamicViscosityNewtonian<expr_type> const&>(*this),
                                                                                                                                     tensorExprMain, theInitArgs... );
    else if ( dynamicViscosity.isPowerLaw() )
        return std::make_shared< typename FluidMecDynamicViscosityPowerLaw<expr_type>::template tensor<Geo_t,Basis_i_t,Basis_j_t>>( std::true_type{},
                                                                                                                                    static_cast<FluidMecDynamicViscosityPowerLaw<expr_expanded_type> const&>(exprExpanded) ,
                                                                                                                                    ttse,
                                                                                                                                    static_cast<FluidMecDynamicViscosityPowerLaw<expr_type> const&>(*this),
                                                                                                                                    tensorExprMain, theInitArgs... );
    else if ( dynamicViscosity.isCarreauLaw() )
        return std::make_shared< typename FluidMecDynamicViscosityCarreau<expr_type>::template tensor<Geo_t,Basis_i_t,Basis_j_t>>( std::true_type{},
                                                                                                                                   static_cast<FluidMecDynamicViscosityCarreau<expr_expanded_type> const&>(exprExpanded) ,
                                                                                                                                   ttse,
                                                                                                                                   static_cast<FluidMecDynamicViscosityCarreau<expr_type> const&>(*this),
                                                                                                                                   tensorExprMain, theInitArgs... );
    else if ( dynamicViscosity.isCarreauYasudaLaw() )
        return std::make_shared< typename FluidMecDynamicViscosityCarreauYasuda<expr_type>::template tensor<Geo_t,Basis_i_t,Basis_j_t>>( std::true_type{},
                                                                                                                                         static_cast<FluidMecDynamicViscosityCarreauYasuda<expr_expanded_type> const&>(exprExpanded) ,
                                                                                                                                         ttse,
                                                                                                                                         static_cast<FluidMecDynamicViscosityCarreauYasuda<expr_type> const&>(*this),
                                                                                                                                         tensorExprMain, theInitArgs... );
    else
        CHECK ( false ) << "invalid viscosity model : "<< dynamicViscosity.lawName() <<"\n";
#endif
    return std::shared_ptr<tensor_base_type<Geo_t,Basis_i_t,Basis_j_t> >{};
}


template< typename ExprType>
template<typename Geo_t, typename Basis_i_t, typename Basis_j_t,typename TheExprExpandedType,typename TupleTensorSymbolsExprType, typename... TheArgsType>
void
SolidMecFirstPiolaKirchhoffBase<ExprType>::updateEvaluator( std::true_type /**/, std::shared_ptr<tensor_base_type<Geo_t,Basis_i_t,Basis_j_t> > & tensorToUpdate,
                                                            TheExprExpandedType const& exprExpanded, TupleTensorSymbolsExprType & ttse,
                                                            Geo_t const& geom, const TheArgsType&... theUpdateArgs )
{
    CHECK(false)<<"TODO";
    using expr_expanded_type = typename TheExprExpandedType::expr_type;
#if 0
    auto const& dynamicViscosity = this->expr().dynamicViscosity();
    if ( dynamicViscosity.isNewtonianLaw() )
        std::static_pointer_cast<typename FluidMecDynamicViscosityNewtonian<expr_type>::template tensor<Geo_t,Basis_i_t,Basis_j_t>>( tensorToUpdate )->update( std::true_type{},
                                                                                                                                                               static_cast<FluidMecDynamicViscosityNewtonian<expr_expanded_type> const&>(exprExpanded),
                                                                                                                                                               ttse, geom, theUpdateArgs... );
    else if ( dynamicViscosity.isPowerLaw() )
        std::static_pointer_cast<typename FluidMecDynamicViscosityPowerLaw<expr_type>::template tensor<Geo_t,Basis_i_t,Basis_j_t>>( tensorToUpdate )->update( std::true_type{},
                                                                                                                                                               static_cast<FluidMecDynamicViscosityPowerLaw<expr_expanded_type> const&>(exprExpanded),
                                                                                                                                                               ttse, geom, theUpdateArgs... );
    else if ( dynamicViscosity.isCarreauLaw() )
        std::static_pointer_cast<typename FluidMecDynamicViscosityCarreau<expr_type>::template tensor<Geo_t,Basis_i_t,Basis_j_t>>( tensorToUpdate )->update( std::true_type{},
                                                                                                                                                             static_cast<FluidMecDynamicViscosityCarreau<expr_expanded_type> const&>(exprExpanded),
                                                                                                                                                             ttse, geom, theUpdateArgs... );

    else if ( dynamicViscosity.isCarreauYasudaLaw() )
        std::static_pointer_cast<typename FluidMecDynamicViscosityCarreauYasuda<expr_type>::template tensor<Geo_t,Basis_i_t,Basis_j_t>>( tensorToUpdate )->update( std::true_type{},
                                                                                                                                                                   static_cast<FluidMecDynamicViscosityCarreauYasuda<expr_expanded_type> const&>(exprExpanded),
                                                                                                                                                                   ttse, geom, theUpdateArgs... );
    else
        CHECK ( false ) << "invalid viscosity model : "<< dynamicViscosity.lawName() <<"\n";
#endif
}



#endif

         

















          
          


#if 0
    template<typename Geo_t, typename Basis_i_t, typename Basis_j_t,typename ExprType>
    struct tensorFirstPiolaKirchhoffBase;
    template<typename Geo_t, typename Basis_i_t, typename Basis_j_t,typename ExprType>
    struct tensorFirstPiolaKirchhoffCompressibleVolumicPartClassic;
    template<typename Geo_t, typename Basis_i_t, typename Basis_j_t,typename ExprType>
    struct tensorFirstPiolaKirchhoffCompressibleVolumicPartSimo1985;



template<typename Geo_t, typename Basis_i_t, typename Basis_j_t, typename ExprType >
struct tensorSolidMecPressureFormulationMultiplierClassicBIS  : public tensorBase<Geo_t,Basis_i_t,Basis_j_t,typename ExprType::shape_type,typename ExprType::value_type >
{
    typedef tensorBase<Geo_t,Basis_i_t,Basis_j_t,typename ExprType::shape_type,typename ExprType::value_type > super_type;
    typedef ExprType expr_type;

    typedef typename expr_type::value_type value_type;
    using matrix_shape_type = typename super_type::matrix_shape_type;
    using ret_type = typename super_type::ret_type;

    tensorSolidMecPressureFormulationMultiplierClassicBIS( expr_type const& expr, Geo_t const& geom, Basis_i_t const& fev, Basis_j_t const& feu )
        :
        super_type( geom, fev, feu)
        {}
    tensorSolidMecPressureFormulationMultiplierClassicBIS( expr_type const& expr, Geo_t const& geom, Basis_i_t const& fev )
        :
        super_type( geom, fev )
        {}
    tensorSolidMecPressureFormulationMultiplierClassicBIS( expr_type const& expr, Geo_t const& geom )
        :
        super_type( geom )
        {}

    typename super_type::matrix_shape_type const& localAssembly( uint16_type q ) const { return M_locRes[q]; }

    void update( Geo_t const& geom ) override { CHECK(false) << "not allow"; }
    void update( Geo_t const& geom, uint16_type face ) override { CHECK(false) << "not allow"; }

    void update( tensorFirstPiolaKirchhoffBase<Geo_t,Basis_i_t,Basis_j_t,ExprType> const& tFPK, Geo_t const& geom );

    ret_type
    evalijq( tensorFirstPiolaKirchhoffBase<Geo_t,Basis_i_t,Basis_j_t,ExprType> const& tFPK,
             uint16_type i, uint16_type j, uint16_type q ) const
        {
            return this->evalijq( tFPK, i, j, q, mpl::int_<expr_type::specific_expr_type::value>() );
        }
private :

    ret_type
    evalijq( tensorFirstPiolaKirchhoffBase<Geo_t,Basis_i_t,Basis_j_t,ExprType> const& tFPK,
             uint16_type i, uint16_type j, uint16_type q,
             mpl::int_<ExprApplySolidMecFirstPiolaKirchhoff::JACOBIAN_TRIAL_DISP> ) const;

    ret_type
    evalijq( tensorFirstPiolaKirchhoffBase<Geo_t,Basis_i_t,Basis_j_t,ExprType> const& tFPK,
             uint16_type i, uint16_type j, uint16_type q,
             mpl::int_<ExprApplySolidMecFirstPiolaKirchhoff::JACOBIAN_TRIAL_PRES> ) const;

private :
    std::vector<typename super_type::matrix_shape_type> M_locRes;

};



    template<typename Geo_t, typename Basis_i_t, typename Basis_j_t,typename ExprType>
    struct tensorFirstPiolaKirchhoffCompressibleVolumicPartBase : public tensorBase<Geo_t,Basis_i_t,Basis_j_t,typename ExprType::shape_type,typename ExprType::value_type >
    {
        typedef tensorBase<Geo_t,Basis_i_t,Basis_j_t,typename ExprType::shape_type,typename ExprType::value_type> super_type;
        typedef tensorFirstPiolaKirchhoffCompressibleVolumicPartBase<Geo_t,Basis_i_t,Basis_j_t,ExprType> self_type;
        typedef ExprType expr_type;
        typedef typename expr_type::geoelement_type geoelement_type;
        typedef typename expr_type::value_type value_type;
        typedef typename super_type::gm_type gm_type;
        typedef typename super_type::gmc_type gmc_type;

        typedef tensorFirstPiolaKirchhoffCompressibleVolumicPartClassic<Geo_t,Basis_i_t,Basis_j_t,ExprType> tensor_volumic_part_classic_type;
        typedef tensorFirstPiolaKirchhoffCompressibleVolumicPartSimo1985<Geo_t,Basis_i_t,Basis_j_t,ExprType> tensor_volumic_part_simo1985_type;

        tensorFirstPiolaKirchhoffCompressibleVolumicPartBase( expr_type const& theexpr,Geo_t const& geom, Basis_i_t const& fev, Basis_j_t const& feu )
            :
            super_type( geom,fev,feu ),
            M_expr( theexpr ),
            M_locRes( M_expr.displacement().gradExtents(*this->gmc() ) )
            {}
        tensorFirstPiolaKirchhoffCompressibleVolumicPartBase( expr_type const& theexpr, Geo_t const& geom, Basis_i_t const& fev )
            :
            super_type( geom,fev ),
            M_expr( theexpr ),
            M_locRes( M_expr.displacement().gradExtents(*this->gmc() ) )
            {}
        tensorFirstPiolaKirchhoffCompressibleVolumicPartBase( expr_type const& theexpr, Geo_t const& geom )
            :
            super_type( geom ),
            M_expr( theexpr ),
            M_locRes( M_expr.displacement().gradExtents(*this->gmc() ) )
            {}

        std::set<std::string> propertiesUsed() const { return std::set<std::string>({ "bulk-modulus" }); }

        void update( Geo_t const& geom ) { CHECK( false ) << "TODO"; }
        void update( Geo_t const& geom, uint16_type face ) { CHECK( false ) << "TODO"; }
        virtual void updateImpl( tensorFirstPiolaKirchhoffBase<Geo_t,Basis_i_t,Basis_j_t,ExprType> const& tFPK ) = 0;

        using super_type::evalijq; // fix clang warning
        using ret_type = Eigen::Map<typename super_type::matrix_shape_type const>;

        virtual
        ret_type
        evalijq( uint16_type i, uint16_type j, uint16_type q,
                 tensorFirstPiolaKirchhoffBase<Geo_t,Basis_i_t,Basis_j_t,ExprType> const& tFPK,
                 const value_type detFv, const value_type ddetF,
                 typename super_type::loc_matrix_tensor2_type const& dFmt ) const = 0;

        expr_type const& expr() const { return M_expr; }
        typename super_type::array_shape_type const& locRes() const { return M_locRes; }
        typename super_type::matrix_shape_type const& locRes( uint16_type q ) const { return M_locRes[q]; }

        static
        std::shared_ptr<self_type>
        New( std::string const& name, expr_type const& expr,Geo_t const& geom, Basis_i_t const& fev, Basis_j_t const& feu )
        {
            std::shared_ptr<self_type> res;
            if ( name == "classic" )
                res.reset( new tensor_volumic_part_classic_type( expr,geom,fev,feu ) );
            else if ( name == "simo1985" )
                res.reset( new tensor_volumic_part_simo1985_type( expr,geom,fev,feu ) );
            return res;
        }
        static
        std::shared_ptr<self_type>
        New( std::string const& name, expr_type const& expr,Geo_t const& geom, Basis_i_t const& fev )
        {
            std::shared_ptr<self_type> res;
            if ( name == "classic" )
                res.reset( new tensor_volumic_part_classic_type( expr,geom,fev ) );
            else if ( name == "simo1985" )
                res.reset( new tensor_volumic_part_simo1985_type( expr,geom,fev ) );
            return res;
        }
        static
        std::shared_ptr<self_type>
        New( std::string const& name, expr_type const& expr,Geo_t const& geom )
        {
            std::shared_ptr<self_type> res;
            if ( name == "classic" )
                res.reset( new tensor_volumic_part_classic_type( expr,geom ) );
            else if ( name == "simo1985" )
                res.reset( new tensor_volumic_part_simo1985_type( expr,geom ) );
            return res;
        }

    private :
        expr_type const& M_expr;
    protected :
        typename super_type::array_scalar_type M_locEvalBulkModulus;
        typename super_type::array_shape_type M_locRes;
    };


    template<typename Geo_t, typename Basis_i_t, typename Basis_j_t,typename ExprType>
    struct tensorFirstPiolaKirchhoffBase : public tensorBase<Geo_t,Basis_i_t,Basis_j_t,typename ExprType::shape_type,typename ExprType::value_type >
    {
        typedef tensorBase<Geo_t,Basis_i_t,Basis_j_t,typename ExprType::shape_type,typename ExprType::value_type> super_type;

        typedef ExprType expr_type;
        typedef typename expr_type::fe_displacement_type fe_displacement_type;
        typedef typename expr_type::geoelement_type geoelement_type;
        typedef typename super_type::gm_type gm_type;
        typedef typename super_type::gmc_type gmc_type;
        // fe displacement context
        typedef typename fe_displacement_type::PreCompute pc_displacement_type;
        typedef std::shared_ptr<pc_displacement_type> pc_displacement_ptrtype;
        typedef typename fe_displacement_type::template Context<expr_type::context_displacement, fe_displacement_type, gm_type,geoelement_type, 0, gmc_type::subEntityCoDim> ctx_displacement_type;
        typedef std::shared_ptr<ctx_displacement_type> ctx_displacement_ptrtype;
        typedef typename expr_type::value_type value_type;
        // fe pressure context
        typedef typename expr_type::fe_pressure_type::PreCompute pc_pressure_type;
        typedef std::shared_ptr<pc_pressure_type> pc_pressure_ptrtype;
        typedef typename expr_type::fe_pressure_type::template Context<expr_type::context_pressure, typename expr_type::fe_pressure_type,
                                                                       gm_type,geoelement_type,0, gmc_type::subEntityCoDim> ctx_pressure_type;
        typedef std::shared_ptr<ctx_pressure_type> ctx_pressure_ptrtype;

        typedef typename super_type::shape_type shape;
        typedef typename super_type::matrix_shape_type matrix_shape_type;
        typedef typename super_type::array_shape_type array_shape_type;
        using ret_type = Eigen::Map<const matrix_shape_type>;

        typedef typename super_type::loc_tensor2_type loc_tensor2_type;
        typedef typename super_type::array_tensor2_type array_tensor2_type;
        typedef typename super_type::loc_matrix_tensor2_type loc_matrix_tensor2_type;
        typedef typename super_type::array_matrix_tensor2_type array_matrix_tensor2_type;

        using expr_mat_properity_scalar_type = std::decay_t<decltype(Feel::vf::expr( typename ModelExpression::expr_scalar_type{},typename ExprType::symbols_expr_type{} ) )>;
        using tensor_mat_properity_scalar_type = typename expr_mat_properity_scalar_type::template tensor<Geo_t,Basis_i_t,Basis_j_t>;

        using tensor_fpk_lm_type = tensorSolidMecPressureFormulationMultiplierClassicBIS<Geo_t,Basis_i_t,Basis_j_t,expr_type>;

        tensorFirstPiolaKirchhoffBase( expr_type const& theexpr,
                                     Geo_t const& geom, Basis_i_t const& fev, Basis_j_t const& feu )
            :
            super_type( geom,fev,feu ),
            M_expr( theexpr )
        {}

        tensorFirstPiolaKirchhoffBase( expr_type const& expr, Geo_t const& geom, Basis_i_t const& fev )
            :
            super_type( geom,fev ),
            M_expr( expr )
        {}

        tensorFirstPiolaKirchhoffBase( expr_type const& expr, Geo_t const& geom )
            :
            super_type( geom ),
            M_expr( expr )
        {}

        expr_type const& expr() const { return M_expr; }

        array_matrix_tensor2_type const& locGradDisplacement() const { return M_locMatrixGradDisplacement; }
        loc_matrix_tensor2_type const& locGradDisplacement( uint16_type q ) const { return M_locMatrixGradDisplacement[q]; }

        typename expr_type::element_pressure_type::id_array_type const& locIdPressure() const { return M_locIdPressure; }

        std::vector<value_type> const& localAssemblyBulkModulus() const { return M_localAssemblyBulkModulus; }

        std::vector<matrix_shape_type>/*array_shape_type*/ & locRes() { return M_locRes; }
        std::vector<matrix_shape_type>/*array_shape_type*/ const& locRes() const { return M_locRes; }
        matrix_shape_type & locRes( uint16_type q ) { return M_locRes[q]; }
        matrix_shape_type const& locRes( uint16_type q ) const { return M_locRes[q]; }

        void update( Geo_t const& geom )
        {
            this->setGmc( geom );

            if ( M_ctxDisplacement )
            {
                if ( this->gmc()->faceId() != invalid_uint16_type_value ) /*face case*/
                    M_pcDisplacement->update( this->gmc()->pc()->nodes() );
                M_ctxDisplacement->update( this->gmc(),  (pc_displacement_ptrtype const&) M_pcDisplacement );
                std::fill( M_locGradDisplacement.data(), M_locGradDisplacement.data()+M_locGradDisplacement.num_elements(), this->M_zeroLocTensor2/*matrix_shape_type::Zero()*/ );
                this->expr().displacement().grad( *M_ctxDisplacement, M_locGradDisplacement );
            }

            if ( M_ctxPressure )
            {
                if ( this->gmc()->faceId() != invalid_uint16_type_value ) /*face case*/
                    M_pcPressure->update( this->gmc()->pc()->nodes() );
                M_ctxPressure->update( this->gmc(), (pc_pressure_ptrtype const&) M_pcPressure );
                std::fill( M_locIdPressure.data(), M_locIdPressure.data()+M_locIdPressure.num_elements(), M_mzero_p );
                this->expr().pressure().id( *M_ctxPressure, M_locIdPressure );
            }

            uint16_type nPt = this->gmc()->nPoints();

            if ( M_tensorLameFirst )
            {
                M_tensorLameFirst->update( geom );
                M_localAssemblyLameFirst.resize( nPt );
            }
            if ( M_tensorLameSecond )
            {
                M_tensorLameSecond->update( geom );
                M_localAssemblyLameSecond.resize( nPt );
            }
            if ( M_tensorBulkModulus )
            {
                M_tensorBulkModulus->update( geom );
                M_localAssemblyBulkModulus.resize( nPt );
            }

            for ( uint16_type q=0;q<nPt;++q )
            {
                if ( M_ctxDisplacement )
                {
                    Eigen::Map< Eigen::Matrix<typename super_type::value_type,Eigen::Dynamic,Eigen::Dynamic/*,Eigen::ColMajor*/ > >
                        gradDisplacementEval( M_locGradDisplacement[q].data(), super_type::shape_tensor2::M,super_type::shape_tensor2::N);
                    M_locMatrixGradDisplacement[q] = gradDisplacementEval;
                }

                if ( M_tensorLameFirst )
                    M_localAssemblyLameFirst[q] = M_tensorLameFirst->evalq(0,0,q);
                if ( M_tensorLameSecond )
                    M_localAssemblyLameSecond[q] = M_tensorLameSecond->evalq(0,0,q);
                if ( M_tensorBulkModulus )
                    M_localAssemblyBulkModulus[q] =  M_tensorBulkModulus->evalq(0,0,q);
            }

            if ( M_tensorLagrangeMultiplier )
                M_tensorLagrangeMultiplier->update( *this, geom );
        }
        void update( Geo_t const& geom, uint16_type face )
        {
            this->setGmc( geom );

            if ( M_ctxDisplacement )
            {
                if ( this->gmc()->faceId() != invalid_uint16_type_value ) /*face case*/
                    M_pcDisplacement->update( this->gmc()->pc()->nodes() );
                M_ctxDisplacement->update( this->gmc(),  (pc_displacement_ptrtype const&) M_pcDisplacement );
                std::fill( M_locGradDisplacement.data(), M_locGradDisplacement.data()+M_locGradDisplacement.num_elements(), this->M_zeroLocTensor2/*matrix_shape_type::Zero()*/ );
                this->expr().displacement().grad( *M_ctxDisplacement, M_locGradDisplacement );
            }

            if ( M_ctxPressure )
            {
                if ( this->gmc()->faceId() != invalid_uint16_type_value ) /*face case*/
                    M_pcPressure->update( this->gmc()->pc()->nodes() );
                M_ctxPressure->update( this->gmc(), (pc_pressure_ptrtype const&) M_pcPressure );
                this->expr().pressure().id( *M_ctxPressure, M_locIdPressure );
            }

            uint16_type nPt = this->gmc()->nPoints();

            if ( M_tensorLameFirst )
            {
                M_tensorLameFirst->update( geom,face );
                M_localAssemblyLameFirst.resize( nPt );
            }
            if ( M_tensorLameSecond )
            {
                M_tensorLameSecond->update( geom,face );
                M_localAssemblyLameSecond.resize( nPt );
            }
            if ( M_tensorBulkModulus )
            {
                M_tensorBulkModulus->update( geom,face );
                M_localAssemblyBulkModulus.resize( nPt );
            }

            for ( uint16_type q=0;q<nPt;++q )
            {
                if ( M_ctxDisplacement )
                {
                    Eigen::Map< Eigen::Matrix<typename super_type::value_type,Eigen::Dynamic,Eigen::Dynamic/*,Eigen::ColMajor*/ > >
                        gradDisplacementEval( M_locGradDisplacement[q].data(), super_type::shape_tensor2::M,super_type::shape_tensor2::N);
                    M_locMatrixGradDisplacement[q] = gradDisplacementEval;
                }

                if ( M_tensorLameFirst )
                    M_localAssemblyLameFirst[q] = M_tensorLameFirst->evalq(0,0,q);
                if ( M_tensorLameSecond )
                    M_localAssemblyLameSecond[q] = M_tensorLameSecond->evalq(0,0,q);
                if ( M_tensorBulkModulus )
                    M_localAssemblyBulkModulus[q] =  M_tensorBulkModulus->evalq(0,0,q);
            }


            if ( M_tensorLagrangeMultiplier )
                M_tensorLagrangeMultiplier->update( *this, geom );
        }

        ret_type
        evalijq( uint16_type i, uint16_type j, uint16_type q ) const
        {
            CHECK( false ) << "not allow\n";
            return ret_type(this->M_locMatrixShape.data());
        }

        value_type
        evalijq( uint16_type i, uint16_type j, uint16_type c1, uint16_type c2, uint16_type q ) const
        {
            //CHECK( false ) << "not allow\n";
            //LOG(WARNING) << "evalijq non optimized";
            return this->evalijq(i,j,q)(c1,c2);
            //return 0;
        }

        value_type
        evaliq( uint16_type i, uint16_type c1, uint16_type c2, uint16_type q ) const
        {
            return evalq( c1,c2,q );
        }
        ret_type
        evaliq( uint16_type i, uint16_type q ) const
        {
            return ret_type(M_locRes[q].data());
        }

        value_type
        evalq( uint16_type c1, uint16_type c2, uint16_type q ) const
        {
            return M_locRes[q]( c1,c2 );
        }
        ret_type
        evalq( uint16_type q ) const
        {
            return ret_type(M_locRes[q].data());
        }

    protected :
        template<typename... TheArgsType>
        void initSubTensor( std::set<std::string> const& subexprUsed, const TheArgsType&... theInitArgs )
            {
                if ( subexprUsed.find( "Lame-first-parameter" ) != subexprUsed.end() )
                {
                    M_exprLameFirst.emplace( Feel::vf::expr( M_expr.matProperties().property( "Lame-first-parameter" ).exprScalar(), M_expr.symbolsExpr() ) );
                    M_tensorLameFirst.emplace( *M_exprLameFirst, theInitArgs... );
                }
                if ( subexprUsed.find( "Lame-second-parameter" ) != subexprUsed.end() )
                {
                    M_exprLameSecond.emplace( Feel::vf::expr( M_expr.matProperties().property( "Lame-second-parameter" ).exprScalar(), M_expr.symbolsExpr() ) );
                    M_tensorLameSecond.emplace( *M_exprLameSecond, theInitArgs... );
                }
                if ( subexprUsed.find( "bulk-modulus" ) != subexprUsed.end() )
                {
                    M_exprBulkModulus.emplace( Feel::vf::expr( M_expr.matProperties().property( "bulk-modulus" ).exprScalar(), M_expr.symbolsExpr() ) );
                    M_tensorBulkModulus.emplace( *M_exprBulkModulus, theInitArgs... );
                }

                if ( subexprUsed.find( "displacement" ) != subexprUsed.end() )
                {
                    M_pcDisplacement.reset( new pc_displacement_type( M_expr.displacement().functionSpace()->fe(), this->gmc()->xRefs() ) );
                    M_ctxDisplacement.reset( new ctx_displacement_type( M_expr.displacement().functionSpace()->fe(),this->gmc(),(pc_displacement_ptrtype const&)M_pcDisplacement ) );
                    M_locGradDisplacement.resize( M_expr.displacement().gradExtents(*this->gmc()) );
                    M_locMatrixGradDisplacement.resize( M_expr.displacement().gradExtents(*this->gmc()) );
                }

                if ( subexprUsed.find( "pressure" ) != subexprUsed.end() )
                {
                    M_pcPressure.reset( new pc_pressure_type( M_expr.pressure().functionSpace()->fe(), this->gmc()->xRefs() ) );
                    M_ctxPressure.reset( new ctx_pressure_type( M_expr.pressure().functionSpace()->fe(), this->gmc(),(pc_pressure_ptrtype const&)M_pcPressure ) );
                    M_locIdPressure.resize( M_expr.pressure().idExtents(*this->gmc()) );
                    M_mzero_p.setZero();
                }

                if ( subexprUsed.find( "lagange_multiplier" ) != subexprUsed.end() )
                {
                    M_tensorLagrangeMultiplier.emplace( M_expr, theInitArgs... );
                }
            }

    private :
        expr_type const& M_expr;

        pc_displacement_ptrtype M_pcDisplacement;
        ctx_displacement_ptrtype M_ctxDisplacement;
        pc_pressure_ptrtype M_pcPressure;
        ctx_pressure_ptrtype M_ctxPressure;

        //array_shape_type M_locRes;
        std::vector<matrix_shape_type> M_locRes;
        array_tensor2_type M_locGradDisplacement;
        typename super_type::array_matrix_tensor2_type M_locMatrixGradDisplacement;
        //typename super_type::array_scalar_type M_locIdPressure;
        typename expr_type::element_pressure_type::id_array_type M_locIdPressure;
        typename expr_type::element_pressure_type::_id_type M_mzero_p;

    protected :
        std::optional<expr_mat_properity_scalar_type> M_exprLameFirst, M_exprLameSecond, M_exprBulkModulus;
        std::optional<tensor_mat_properity_scalar_type> M_tensorLameFirst, M_tensorLameSecond, M_tensorBulkModulus;
        std::vector<value_type> M_localAssemblyLameFirst, M_localAssemblyLameSecond, M_localAssemblyBulkModulus;
        std::optional<tensor_fpk_lm_type> M_tensorLagrangeMultiplier;
    };



template<typename Geo_t, typename Basis_i_t, typename Basis_j_t,typename ExprType>
void
tensorSolidMecPressureFormulationMultiplierClassicBIS<Geo_t,Basis_i_t,Basis_j_t,ExprType>::update( tensorFirstPiolaKirchhoffBase<Geo_t,Basis_i_t,Basis_j_t,ExprType> const& tFPK, Geo_t const& geom )
{
    this->setGmc( geom );
    uint16_type nPts = this->gmc()->nPoints();
    M_locRes.resize( nPts );
    for ( uint16_type q = 0; q < nPts; ++q )
    {
        // compute : Fv*J*Cv^{-1}*idv(p) = Fv*J*(Fv^T*Fv)^{-1}*idv(p) = J*Fv^{-T}*idv(p) (with J=det(F))
        auto const& gradDisplacementEval = tFPK.locGradDisplacement(q);
        typename super_type::matrix_shape_type/*loc_tensor2_type*/ & theLocRes = this->M_locRes[q];
        if constexpr( super_type::gmc_type::nDim == 2 )
        {
            const value_type du1vdx = gradDisplacementEval(0,0), du1vdy = gradDisplacementEval(0,1);
            const value_type du2vdx = gradDisplacementEval(1,0), du2vdy = gradDisplacementEval(1,1);
            theLocRes(0,0) = 1+du2vdy;
            theLocRes(0,1) = -du2vdx;
            theLocRes(1,0) = -du1vdy;
            theLocRes(1,1) = 1+du1vdx;
        }
        else if constexpr( super_type::gmc_type::nDim == 3 )
        {
            const value_type du1vdx = gradDisplacementEval(0,0), du1vdy = gradDisplacementEval(0,1), du1vdz = gradDisplacementEval(0,2);
            const value_type du2vdx = gradDisplacementEval(1,0), du2vdy = gradDisplacementEval(1,1), du2vdz = gradDisplacementEval(1,2);
            const value_type du3vdx = gradDisplacementEval(2,0), du3vdy = gradDisplacementEval(2,1), du3vdz = gradDisplacementEval(2,2);
            theLocRes(0,0) = (1+du2vdy)*(1+du3vdz) - du2vdz*du3vdy;
            theLocRes(0,1) = du2vdz*du3vdx - du2vdx*(1+du3vdz);
            theLocRes(0,2) = du2vdx*du3vdy - (1+du2vdy)*du3vdx;
            theLocRes(1,0) = du1vdz*du3vdy - du1vdy*(1+du3vdz);
            theLocRes(1,1) = (1+du1vdx)*(1+du3vdz) - du1vdz*du3vdx;
            theLocRes(1,2) = du1vdy*du3vdx - (1+du1vdx)*du3vdy;
            theLocRes(2,0) = du1vdy*du2vdz - du1vdz*(1+du2vdy);
            theLocRes(2,1) = du1vdz*du2vdx - (1+du1vdx)*du2vdz;
            theLocRes(2,2) = (1+du1vdx)*(1+du2vdy) - du1vdy*du2vdx;
        }
        if constexpr ( expr_type::specific_expr_type::value == ExprApplySolidMecFirstPiolaKirchhoff::EVAL )
        {
            const value_type idPressureEval = tFPK.locIdPressure()[q](0,0);
            theLocRes *= /*-*/idPressureEval;
        }
    }
}

template<typename Geo_t, typename Basis_i_t, typename Basis_j_t,typename ExprType>
typename tensorSolidMecPressureFormulationMultiplierClassicBIS<Geo_t,Basis_i_t,Basis_j_t,ExprType>::ret_type
tensorSolidMecPressureFormulationMultiplierClassicBIS<Geo_t,Basis_i_t,Basis_j_t,ExprType>::evalijq( tensorFirstPiolaKirchhoffBase<Geo_t,Basis_i_t,Basis_j_t,ExprType> const& tFPK,
                                                                                                    uint16_type i, uint16_type j, uint16_type q,
                                                                                                    mpl::int_<ExprApplySolidMecFirstPiolaKirchhoff::JACOBIAN_TRIAL_DISP> ) const
{
    matrix_shape_type & thelocRes = this->locMatrixShape();
    auto const& gradTrial = this->fecTrial()->grad( j, q );
    if constexpr( super_type::gmc_type::nDim == 2 )
    {
        const value_type dF11 = gradTrial( 0, 0, 0 ), dF12 = gradTrial( 0, 1, 0 );
        const value_type dF21 = gradTrial( 1, 0, 0 ), dF22 = gradTrial( 1, 1, 0 );
        thelocRes(0,0) =  dF22;
        thelocRes(0,1) = -dF21;
        thelocRes(1,0) = -dF12;
        thelocRes(1,1) =  dF11;
    }
    else if constexpr( super_type::gmc_type::nDim == 3 )
    {
        const value_type dF11 = gradTrial( 0, 0, 0 ), dF12 = gradTrial( 0, 1, 0 ), dF13 = gradTrial( 0, 2, 0 );
        const value_type dF21 = gradTrial( 1, 0, 0 ), dF22 = gradTrial( 1, 1, 0 ), dF23 = gradTrial( 1, 2, 0 );
        const value_type dF31 = gradTrial( 2, 0, 0 ), dF32 = gradTrial( 2, 1, 0 ), dF33 = gradTrial( 2, 2, 0 );
        auto const& gradDisplacementEval = tFPK.locGradDisplacement(q);
        const value_type Fv11 = 1+gradDisplacementEval(0,0), Fv12 =   gradDisplacementEval(0,1), Fv13 =   gradDisplacementEval(0,2);
        const value_type Fv21 =   gradDisplacementEval(1,0), Fv22 = 1+gradDisplacementEval(1,1), Fv23 =   gradDisplacementEval(1,2);
        const value_type Fv31 =   gradDisplacementEval(2,0), Fv32 =   gradDisplacementEval(2,1), Fv33 = 1+gradDisplacementEval(2,2);

        thelocRes(0,0) = dF22+dF33+dF22*Fv33+Fv22*dF33-dF23*Fv32-Fv23*dF32;
        thelocRes(0,1) = dF23*Fv31+Fv23*dF31-dF21-dF21*Fv33-Fv21*dF33;
        thelocRes(0,2) = dF21*Fv32+Fv21*dF32-dF31-dF31*Fv22-Fv31*dF22;
        thelocRes(1,0) = dF13*Fv32+Fv13*dF32-dF12-dF12*Fv33-Fv12*dF33;
        thelocRes(1,1) = dF11+dF33+dF11*Fv33+Fv11*dF33-dF13*Fv31-Fv13*dF31;
        thelocRes(1,2) = dF12*Fv31+Fv12*dF31-dF32-dF32*Fv11-Fv32*dF11;
        thelocRes(2,0) = dF12*Fv23+Fv12*dF23-dF13-dF13*Fv22-Fv13*dF22;
        thelocRes(2,1) = dF13*Fv21+Fv13*dF21-dF23-dF23*Fv11-Fv23*dF11;
        thelocRes(2,2) = dF11+dF22+dF11*Fv22+Fv11*dF22-dF12*Fv21-Fv12*dF21;
    }
    const value_type idPressureEval = tFPK.locIdPressure()[q](0,0);
    thelocRes *= /*-*/idPressureEval;

    return ret_type(this->locMatrixShape().data());
}

template<typename Geo_t, typename Basis_i_t, typename Basis_j_t,typename ExprType>
typename tensorSolidMecPressureFormulationMultiplierClassicBIS<Geo_t,Basis_i_t,Basis_j_t,ExprType>::ret_type
tensorSolidMecPressureFormulationMultiplierClassicBIS<Geo_t,Basis_i_t,Basis_j_t,ExprType>::evalijq( tensorFirstPiolaKirchhoffBase<Geo_t,Basis_i_t,Basis_j_t,ExprType> const& tFPK,
                                                                                                    uint16_type i, uint16_type j, uint16_type q,
                                                                                                    mpl::int_<ExprApplySolidMecFirstPiolaKirchhoff::JACOBIAN_TRIAL_PRES> ) const
{
    // compute -idt(p)*(Fv*Cv^{-1}) and Fv*Cv^{-1} is computed in update
    const value_type idTrialPressure = this->fecTrial()->id( j, 0, 0, q );
    matrix_shape_type & thelocMat = this->locMatrixShape();
    thelocMat = /*-*/idTrialPressure*this->localAssembly(q);

    return ret_type(this->locMatrixShape().data());
}


    template<typename Geo_t, typename Basis_i_t, typename Basis_j_t,typename ExprType>
    struct tensorFirstPiolaKirchhoffCompressibleVolumicPartClassic : public tensorFirstPiolaKirchhoffCompressibleVolumicPartBase<Geo_t,Basis_i_t,Basis_j_t,ExprType >
    {
        typedef tensorFirstPiolaKirchhoffCompressibleVolumicPartBase<Geo_t,Basis_i_t,Basis_j_t,ExprType > super_type;
        typedef ExprType expr_type;
        typedef typename super_type::value_type value_type;

        tensorFirstPiolaKirchhoffCompressibleVolumicPartClassic( expr_type const& expr,Geo_t const& geom, Basis_i_t const& fev, Basis_j_t const& feu )
            :
            super_type( expr,geom,fev,feu ),
            M_locEvalFmt( boost::extents[ this->gmc()->xRefs().size2()] )
        {}
        tensorFirstPiolaKirchhoffCompressibleVolumicPartClassic( expr_type const& expr, Geo_t const& geom, Basis_i_t const& fev )
            :
            super_type( expr,geom,fev ),
            M_locEvalFmt( boost::extents[ this->gmc()->xRefs().size2()] )
        {}
        tensorFirstPiolaKirchhoffCompressibleVolumicPartClassic( expr_type const& expr, Geo_t const& geom )
            :
            super_type( expr,geom ),
            M_locEvalFmt( boost::extents[ this->gmc()->xRefs().size2()] )
            {}

        void updateImpl( tensorFirstPiolaKirchhoffBase<Geo_t,Basis_i_t,Basis_j_t,ExprType> const& tFPK ) override
        {
            auto const& locGradEval = tFPK.locGradDisplacement();
            auto const& locBulkModulus = tFPK.localAssemblyBulkModulus();
            for ( uint16_type q = 0; q < this->gmc()->nPoints(); ++q )
            {
                // F*\kappa*(J-1)*J*C^{-1} = \kappa*(J-1)*J*F^{-T}
                const value_type bulkModulus = locBulkModulus[q];
                auto const& gradDisplacementEval = locGradEval[q];
                auto Id = super_type::loc_matrix_tensor2_type::Identity();
                auto F = Id + gradDisplacementEval;
                const value_type detFv = F.determinant();

                if ( expr_type::specific_expr_type::value == ExprApplySolidMecFirstPiolaKirchhoff::EVAL )
                {
                    auto InvFvmt = F.inverse().transpose();
                    this->M_locRes[q] = bulkModulus*detFv*(detFv-1)*InvFvmt;
                }
                else
                {
                    M_locEvalFmt[q] = F.inverse().transpose();
                }
            }
        }
        using super_type::evalijq; // fix clang warning
        using ret_type = typename super_type::ret_type;
        ret_type
        evalijq( uint16_type i, uint16_type j, uint16_type q,
                 tensorFirstPiolaKirchhoffBase<Geo_t,Basis_i_t,Basis_j_t,ExprType> const& tFPK,
                 const value_type detFv, const value_type ddetF,
                 typename super_type::loc_matrix_tensor2_type const& dFmt ) const override
        {
            const value_type bulkModulus = tFPK.localAssemblyBulkModulus()[q];
            this->locMatrixShape() = bulkModulus*(detFv*(detFv-1)*dFmt + ddetF*(2*detFv-1)*M_locEvalFmt[q]);
            return ret_type(this->locMatrixShape().data());
        }

    private :
        typename super_type::array_matrix_tensor2_type M_locEvalFmt;
    };

    template<typename Geo_t, typename Basis_i_t, typename Basis_j_t,typename ExprType>
    struct tensorFirstPiolaKirchhoffCompressibleVolumicPartSimo1985 : public tensorFirstPiolaKirchhoffCompressibleVolumicPartBase<Geo_t,Basis_i_t,Basis_j_t,ExprType >
    {
        typedef tensorFirstPiolaKirchhoffCompressibleVolumicPartBase<Geo_t,Basis_i_t,Basis_j_t,ExprType > super_type;
        typedef ExprType expr_type;
        typedef typename super_type::value_type value_type;
        using ret_type = typename super_type::ret_type;
        tensorFirstPiolaKirchhoffCompressibleVolumicPartSimo1985( expr_type const& expr,Geo_t const& geom, Basis_i_t const& fev, Basis_j_t const& feu )
            :
            super_type( expr,geom,fev,feu ),
            M_locEvalPrecomputeLogDetF( boost::extents[ this->gmc()->xRefs().size2()] ),
            M_locEvalFmt( boost::extents[ this->gmc()->xRefs().size2()] )
        {}
        tensorFirstPiolaKirchhoffCompressibleVolumicPartSimo1985( expr_type const& expr, Geo_t const& geom, Basis_i_t const& fev )
            :
            super_type( expr,geom,fev ),
            M_locEvalPrecomputeLogDetF( boost::extents[ this->gmc()->xRefs().size2()] ),
            M_locEvalFmt( boost::extents[ this->gmc()->xRefs().size2()] )
        {}
        tensorFirstPiolaKirchhoffCompressibleVolumicPartSimo1985( expr_type const& expr, Geo_t const& geom )
            :
            super_type( expr,geom ),
            M_locEvalPrecomputeLogDetF( boost::extents[ this->gmc()->xRefs().size2()] ),
            M_locEvalFmt( boost::extents[ this->gmc()->xRefs().size2()] )
            {}

        void updateImpl( tensorFirstPiolaKirchhoffBase<Geo_t,Basis_i_t,Basis_j_t,ExprType> const& tFPK ) override
        {
            auto const& locGradEval = tFPK.locGradDisplacement();
            auto const& locBulkModulus = tFPK.localAssemblyBulkModulus();
            for ( uint16_type q = 0; q < this->gmc()->nPoints(); ++q )
            {
                // F*\kappa*ln(J)*C^{-1} = \kappa*ln(J)*F^{-T}
                const value_type bulkModulus = locBulkModulus[q];
                auto const& gradDisplacementEval = locGradEval[q];
                auto Id = super_type::loc_matrix_tensor2_type::Identity();
                auto F = Id + gradDisplacementEval;
                const value_type detFv = F.determinant();

                if ( expr_type::specific_expr_type::value == ExprApplySolidMecFirstPiolaKirchhoff::EVAL )
                {
                    auto InvFvmt = F.inverse().transpose();
                    this->M_locRes[q] = bulkModulus*math::log(detFv)*InvFvmt;
                }
                else
                {
                    M_locEvalFmt[q] = F.inverse().transpose();
                    M_locEvalPrecomputeLogDetF[q] = bulkModulus*math::log(detFv);
                }
            }
        }
        using super_type::evalijq; // fix clang warning
        ret_type
        evalijq( uint16_type i, uint16_type j, uint16_type q,
                 tensorFirstPiolaKirchhoffBase<Geo_t,Basis_i_t,Basis_j_t,ExprType> const& tFPK,
                 const value_type detFv, const value_type ddetF,
                 typename super_type::loc_matrix_tensor2_type const& dFmt ) const override
        {
            typename super_type::matrix_shape_type& locMat = this->locMatrixShape();
            const value_type bulkModulus = tFPK.localAssemblyBulkModulus()[q];
            const value_type precomputeLog = M_locEvalPrecomputeLogDetF[q];
            locMat = precomputeLog*dFmt;
            const value_type factorOther2 = bulkModulus*ddetF/detFv;
            typename super_type::loc_matrix_tensor2_type const& Fmt = M_locEvalFmt[q];
            locMat += factorOther2*Fmt;
            return ret_type(this->locMatrixShape().data());
        }

    private :
        typename super_type::array_value_type M_locEvalPrecomputeLogDetF;
        typename super_type::array_matrix_tensor2_type M_locEvalFmt;
    };


    /**
     * -------------------------------------------------------------------------------------------------  *
     * -------------------------------------------------------------------------------------------------  *
     * Linear elasticity
     *   sigma = P = FS  = \lambda trace(E)*Id + 2*\mu*E    ( E is linear part)
     *   sigma = 2*\mu*E ( + pressure part : p*Id )
     * -------------------------------------------------------------------------------------------------  *
     * -------------------------------------------------------------------------------------------------  *
     */
    template<typename Geo_t, typename Basis_i_t, typename Basis_j_t,typename ExprType,bool useDispPresForm>
    struct tensorSolidMecFirstPiolaKirchhoffLinearElasticity : public tensorFirstPiolaKirchhoffBase<Geo_t,Basis_i_t,Basis_j_t,ExprType>
    {
        typedef tensorFirstPiolaKirchhoffBase<Geo_t,Basis_i_t,Basis_j_t,ExprType> super_type;
    public :
        typedef ExprType expr_type;
        typedef typename super_type::geoelement_type geoelement_type;
        typedef typename super_type::gmc_type gmc_type;
        typedef typename super_type::gm_type gm_type;
        typedef typename super_type::value_type value_type;

        using ret_type = typename super_type::ret_type;

        tensorSolidMecFirstPiolaKirchhoffLinearElasticity( expr_type const& _expr,
                                                           Geo_t const& geom, Basis_i_t const& fev, Basis_j_t const& feu )
            :
            super_type( _expr,geom,fev,feu ),
            M_mId( super_type::matrix_shape_type::Identity() )
            {
                this->updateForUse( geom, fev, feu );
            }
        tensorSolidMecFirstPiolaKirchhoffLinearElasticity( expr_type const& _expr, Geo_t const& geom, Basis_i_t const& fev )
            :
            super_type( _expr,geom,fev ),
            M_mId( super_type::matrix_shape_type::Identity() )
            {
                this->updateForUse( geom, fev );
            }
        tensorSolidMecFirstPiolaKirchhoffLinearElasticity( expr_type const& _expr,Geo_t const& geom )
            :
            super_type( _expr,geom ),
            M_mId( super_type::matrix_shape_type::Identity() )
            {
                this->updateForUse( geom );
            }

        void update( Geo_t const& geom ) override
        {
            super_type::update( geom );
            updateImpl( geom );
        }
        void update( Geo_t const& geom, uint16_type face ) override
        {
            super_type::update( geom,face );
            updateImpl( geom );
        }

        ret_type
        evalijq( uint16_type i, uint16_type j, uint16_type q ) const override
            {
                return evalijq( i,j,q, mpl::int_<expr_type::specific_expr_type::value>() );
            }

    private :

        void updateImpl( Geo_t const& geom )
        {
            if constexpr ( expr_type::specific_expr_type::value != ExprApplySolidMecFirstPiolaKirchhoff::EVAL )
                 return;

            double timeSteppingScaling = this->expr().timeSteppingScaling();
            this->locRes().resize( this->gmc()->nPoints(), super_type::matrix_shape_type/*loc_res_type*/::Zero() );
            typename super_type::matrix_shape_type E;
            for ( uint16_type q = 0; q < this->gmc()->nPoints(); ++q )
            {
                typename super_type::matrix_shape_type & theLocRes = this->locRes(q);

                auto const& gradDisplacementEval = this->locGradDisplacement(q);
                const value_type du1vdx = gradDisplacementEval(0,0), du1vdy = gradDisplacementEval(0,1);
                const value_type du2vdx = gradDisplacementEval(1,0), du2vdy = gradDisplacementEval(1,1);

                if constexpr ( expr_type::nRealDim == 2 )
                {
                    E(0,0) = du1vdx;
                    E(0,1) = 0.5*(du1vdy+du2vdx);
                    E(1,0) = E(0,1);
                    E(1,1) = du2vdy;
                }
                else if constexpr ( expr_type::nRealDim == 3 )
                {
                    const value_type du1vdz = gradDisplacementEval(0,2);
                    const value_type du2vdz = gradDisplacementEval(1,2);
                    const value_type du3vdx = gradDisplacementEval(2,0), du3vdy = gradDisplacementEval(2,1), du3vdz = gradDisplacementEval(2,2);
                    E(0,0) = du1vdx;
                    E(0,1) = 0.5*(du1vdy+du2vdx);
                    E(0,2) = 0.5*(du1vdz+du3vdx);
                    E(1,0) = E(0,1);
                    E(1,1) = du2vdy;
                    E(1,2) = 0.5*(du2vdz+du3vdy);
                    E(2,0) = E(0,2);
                    E(2,1) = E(1,2);
                    E(2,2) = du3vdz;
                }

                if constexpr ( useDispPresForm )
                {
                    // p*Id+S_bis
                    const value_type coefflame2 = this->M_localAssemblyLameSecond[q];
                    const value_type pEval = this->locIdPressure()[q](0,0);
                    const value_type traceE = E.trace();
                    theLocRes = (pEval-(2.0/3.0)*coefflame2*traceE)*M_mId + (2*coefflame2*timeSteppingScaling)*E;
                }
                else
                {
                    const value_type coefflame1 = this->M_localAssemblyLameFirst[q];
                    const value_type coefflame2 = this->M_localAssemblyLameSecond[q];
                    theLocRes = coefflame1*E.trace()*M_mId + 2*coefflame2*E;
                    theLocRes *= timeSteppingScaling;
                }

            }
        }

        ret_type
        evalijq( uint16_type i, uint16_type j, uint16_type q, mpl::int_<ExprApplySolidMecFirstPiolaKirchhoff::EVAL> /**/ ) const
        {
            CHECK( false ) << "not allow";
            return ret_type(this->locMatrixShape().data());
        }

        ret_type
        evalijq( uint16_type i, uint16_type j, uint16_type q, mpl::int_<ExprApplySolidMecFirstPiolaKirchhoff::JACOBIAN_TRIAL_DISP> /**/ ) const
        {
            auto const& gradTrial = this->fecTrial()->grad( j, q );
            const value_type du1tdx = gradTrial( 0, 0, 0 ), du1tdy = gradTrial( 0, 1, 0 );
            const value_type du2tdx = gradTrial( 1, 0, 0 ), du2tdy = gradTrial( 1, 1, 0 );

            typename super_type::matrix_shape_type E;
            if constexpr ( expr_type::nRealDim == 2 )
            {
                E(0,0) = du1tdx;
                E(0,1) = 0.5*(du1tdy+du2tdx);
                E(1,0) = E(0,1);
                E(1,1) = du2tdy;
            }
            else if constexpr ( expr_type::nRealDim == 3 )
            {
                const value_type du1tdz = gradTrial( 0, 2, 0 );
                const value_type du2tdz = gradTrial( 1, 2, 0 );
                const value_type du3tdx = gradTrial( 2, 0, 0 ), du3tdy = gradTrial( 2, 1, 0 ), du3tdz = gradTrial( 2, 2, 0 );
                E(0,0) = du1tdx;
                E(0,1) = 0.5*(du1tdy+du2tdx);
                E(0,2) = 0.5*(du1tdz+du3tdx);
                E(1,0) = E(0,1);
                E(1,1) = du2tdy;
                E(1,2) = 0.5*(du2tdz+du3tdy);
                E(2,0) = E(0,2);
                E(2,1) = E(1,2);
                E(2,2) = du3tdz;
            }

            typename super_type::matrix_shape_type & thelocMat = this->locMatrixShape();
            if constexpr ( useDispPresForm )
            {
                const value_type coefflame2 = this->M_localAssemblyLameSecond[q];
                const value_type traceE = E.trace();
                thelocMat = 2*coefflame2*E - (2.0/3.0)*coefflame2*traceE*M_mId;
            }
            else
            {
                const value_type coefflame1 = this->M_localAssemblyLameFirst[q];
                const value_type coefflame2 = this->M_localAssemblyLameSecond[q];
                thelocMat = coefflame1*E.trace()*M_mId + 2*coefflame2*E;
            }

            return ret_type(thelocMat.data());
        }
        ret_type
        evalijq( uint16_type i, uint16_type j, uint16_type q, mpl::int_<ExprApplySolidMecFirstPiolaKirchhoff::JACOBIAN_TRIAL_PRES> /**/ ) const
        {
            // compute idt(p)*Id
            const value_type idTrialPressure = this->fecTrial()->id( j, 0, 0, q );
            typename super_type::matrix_shape_type & thelocMat = this->locMatrixShape();
            thelocMat = /*-*/idTrialPressure*M_mId;
            return ret_type(thelocMat.data());
        }


        template<typename... TheArgsType>
        void updateForUse( const TheArgsType&... theInitArgs )
            {
                std::set<std::string> subexprUsed = { "displacement","Lame-second-parameter" };
                if constexpr ( !useDispPresForm )
                    subexprUsed.insert( "Lame-first-parameter" );
                if constexpr ( useDispPresForm && expr_type::specific_expr_type::value != ExprApplySolidMecFirstPiolaKirchhoff::JACOBIAN_TRIAL_PRES )
                    subexprUsed.insert( "pressure" );
                if constexpr ( useDispPresForm && expr_type::specific_expr_type::value == ExprApplySolidMecFirstPiolaKirchhoff::JACOBIAN_TRIAL_PRES )
                    subexprUsed.clear();
                this->initSubTensor(subexprUsed,theInitArgs...);
            }
    private :
        typename super_type::matrix_shape_type M_mId;
    };


    /**
     * -------------------------------------------------------------------------------------------------  *
     * -------------------------------------------------------------------------------------------------  *
     * StVenantKirchhoff :
     *   S = \lambda trace(E)*Id + 2*\mu*E
     *   S = 2*\mu*E ( + pressure part : p*Id )
     * -------------------------------------------------------------------------------------------------  *
     * -------------------------------------------------------------------------------------------------  *
     */
    template<typename Geo_t, typename Basis_i_t, typename Basis_j_t,typename ExprType,bool useDispPresForm>
    struct tensorSolidMecFirstPiolaKirchhoffStVenantKirchhoff : public tensorFirstPiolaKirchhoffBase<Geo_t,Basis_i_t,Basis_j_t,ExprType>
    {
        typedef tensorFirstPiolaKirchhoffBase<Geo_t,Basis_i_t,Basis_j_t,ExprType> super_type;
    public :
        typedef ExprType expr_type;
        typedef typename super_type::geoelement_type geoelement_type;
        typedef typename super_type::gmc_type gmc_type;
        typedef typename super_type::gm_type gm_type;
        typedef typename super_type::value_type value_type;

        using ret_type = Eigen::Map<const Eigen::Matrix<value_type, super_type::shape::M, super_type::shape::N>>;

        tensorSolidMecFirstPiolaKirchhoffStVenantKirchhoff( expr_type const& _expr,
                                                            Geo_t const& geom, Basis_i_t const& fev, Basis_j_t const& feu )
            :
            super_type( _expr,geom,fev,feu )
            {
                this->updateForUse( geom, fev, feu );
            }
        tensorSolidMecFirstPiolaKirchhoffStVenantKirchhoff( expr_type const& _expr, Geo_t const& geom, Basis_i_t const& fev )
            :
            super_type( _expr,geom,fev )
            {
                this->updateForUse( geom, fev );
            }
        tensorSolidMecFirstPiolaKirchhoffStVenantKirchhoff( expr_type const& _expr,Geo_t const& geom )
            :
            super_type( _expr,geom )
            {
                this->updateForUse( geom );
            }

        void update( Geo_t const& geom ) override
        {
            super_type::update( geom );
            updateImpl( geom );
        }
        void update( Geo_t const& geom, uint16_type face ) override
        {
            super_type::update( geom,face );
            updateImpl( geom );
        }

        ret_type
        evalijq( uint16_type i, uint16_type j, uint16_type q ) const override
            {
                return evalijq( i,j,q, mpl::int_<expr_type::specific_expr_type::value>() );
            }

    private :
        void updateImpl( Geo_t const& geom )
        {
            //std::fill( this->locRes().data(), this->locRes().data()+this->locRes().num_elements(), super_type::matrix_shape_type/*loc_res_type*/::Zero() );
            this->locRes().resize( this->gmc()->nPoints(), super_type::matrix_shape_type/*loc_res_type*/::Zero() );

            if constexpr ( expr_type::specific_expr_type::value == ExprApplySolidMecFirstPiolaKirchhoff::JACOBIAN_TRIAL_PRES )
            {
                for ( uint16_type q = 0; q < this->gmc()->nPoints(); ++q )
                    this->locRes(q) = super_type::matrix_shape_type::Identity()+this->locGradDisplacement(q);
            }
            else
            {
                this->updateImpl( mpl::int_<expr_type::nRealDim>() );
            }
        }
        void updateImpl( mpl::int_<2> /**/ )
        {
            double timeSteppingScaling = this->expr().timeSteppingScaling();
            for ( uint16_type q = 0; q < this->gmc()->nPoints(); ++q )
            {
                typename super_type::matrix_shape_type/*loc_tensor2_type*/ & theLocRes = this->locRes(q);

                auto const& gradDisplacementEval = this->locGradDisplacement(q);
                const value_type du1vdx = gradDisplacementEval(0,0), du1vdy = gradDisplacementEval(0,1);
                const value_type du2vdx = gradDisplacementEval(1,0), du2vdy = gradDisplacementEval(1,1);

                const value_type subtraceE1 = 0.5*(std::pow(du1vdx,2)+std::pow(du2vdx,2));
                const value_type subtraceE2 = 0.5*(std::pow(du1vdy,2)+std::pow(du2vdy,2));

                const value_type E11 = du1vdx + subtraceE1;
                const value_type E12 = 0.5*( du1vdy + du2vdx + du1vdx*du1vdy + du2vdx*du2vdy );
                const value_type E21 = 0.5*( du2vdx + du1vdy + du1vdy*du1vdx + du2vdy*du2vdx );
                const value_type E22 = du2vdy + subtraceE2;

                if constexpr ( useDispPresForm )
                {
                    const value_type coefflame2 = this->M_localAssemblyLameSecond[q];
                    const value_type coeffMult = 2*coefflame2*timeSteppingScaling;
                    const value_type S11 = coeffMult*E11;
                    const value_type S12 = coeffMult*E12;
                    const value_type S21 = coeffMult*E21;
                    const value_type S22 = coeffMult*E22;
                    if constexpr ( expr_type::specific_expr_type::value == ExprApplySolidMecFirstPiolaKirchhoff::EVAL )
                    {
#if 0
                        // F*S
                        theLocRes(0,0) = (1+du1vdx)*S11 + du1vdy*S21;
                        theLocRes(0,1) = (1+du1vdx)*S12 + du1vdy*S22;
                        theLocRes(1,0) = du2vdx*S11 + (1+du2vdy)*S21;
                        theLocRes(1,1) = du2vdx*S12 + (1+du2vdy)*S22;
#else
                        //F*(p*Id+S)
                        const value_type pEval = this->locIdPressure()[q](0,0);
                        theLocRes(0,0) = (1+du1vdx)*(S11+pEval) + du1vdy*S21;
                        theLocRes(0,1) = (1+du1vdx)*S12 + du1vdy*(S22+pEval);
                        theLocRes(1,0) = du2vdx*(S11+pEval) + (1+du2vdy)*S21;
                        theLocRes(1,1) = du2vdx*S12 + (1+du2vdy)*(S22+pEval);
#endif
                    }
                    else if constexpr ( expr_type::specific_expr_type::value == ExprApplySolidMecFirstPiolaKirchhoff::JACOBIAN_TRIAL_DISP )
                    {
                        theLocRes(0,0) = S11;
                        theLocRes(0,1) = S12;
                        theLocRes(1,0) = S21;
                        theLocRes(1,1) = S22;
                    }
                }
                else
                {
                    const value_type coefflame1 = this->M_localAssemblyLameFirst[q];
                    const value_type coefflame2 = this->M_localAssemblyLameSecond[q];
                    const value_type traceE = E11 + E22;

                    const value_type coeffOnDiag = coefflame1*traceE*timeSteppingScaling;
                    const value_type coeffMult = 2*coefflame2*timeSteppingScaling;

                    const value_type S11 = coeffOnDiag + coeffMult*E11;
                    const value_type S12 = coeffMult*E12;
                    const value_type S21 = coeffMult*E21;
                    const value_type S22 = coeffOnDiag + coeffMult*E22;

                    if constexpr ( expr_type::specific_expr_type::value == ExprApplySolidMecFirstPiolaKirchhoff::EVAL )
                    {
                        // F*S
                        theLocRes(0,0) = (1+du1vdx)*S11 + du1vdy*S21;
                        theLocRes(0,1) = (1+du1vdx)*S12 + du1vdy*S22;
                        theLocRes(1,0) = du2vdx*S11 + (1+du2vdy)*S21;
                        theLocRes(1,1) = du2vdx*S12 + (1+du2vdy)*S22;
                    }
                    else if constexpr ( expr_type::specific_expr_type::value == ExprApplySolidMecFirstPiolaKirchhoff::JACOBIAN_TRIAL_DISP )
                    {
                        theLocRes(0,0) = S11;
                        theLocRes(0,1) = S12;
                        theLocRes(1,0) = S21;
                        theLocRes(1,1) = S22;
                    }
                }
            }
        }

        void updateImpl( mpl::int_<3> /**/ )
        {
            double timeSteppingScaling = this->expr().timeSteppingScaling();
            for ( uint16_type q = 0; q < this->gmc()->nPoints(); ++q )
            {
                auto const& gradDisplacementEval = this->locGradDisplacement(q);
                const value_type du1vdx = gradDisplacementEval(0,0), du1vdy = gradDisplacementEval(0,1), du1vdz = gradDisplacementEval(0,2);
                const value_type du2vdx = gradDisplacementEval(1,0), du2vdy = gradDisplacementEval(1,1), du2vdz = gradDisplacementEval(1,2);
                const value_type du3vdx = gradDisplacementEval(2,0), du3vdy = gradDisplacementEval(2,1), du3vdz = gradDisplacementEval(2,2);

                const value_type subtraceE1 = 0.5*(std::pow(du1vdx,2)+std::pow(du2vdx,2)+std::pow(du3vdx,2));
                const value_type subtraceE2 = 0.5*(std::pow(du1vdy,2)+std::pow(du2vdy,2)+std::pow(du3vdy,2));
                const value_type subtraceE3 = 0.5*(std::pow(du1vdz,2)+std::pow(du2vdz,2)+std::pow(du3vdz,2));

                const value_type E11 = du1vdx + subtraceE1;
                const value_type E12 = 0.5*( du1vdy + du2vdx + du1vdx*du1vdy + du2vdx*du2vdy + du3vdx*du3vdy );
                const value_type E13 = 0.5*( du1vdz + du3vdx + du1vdx*du1vdz + du2vdx*du2vdz + du3vdx*du3vdz );
                const value_type E21 = 0.5*( du2vdx + du1vdy + du1vdy*du1vdx + du2vdy*du2vdx + du3vdy*du3vdx );
                const value_type E22 = du2vdy + subtraceE2;
                const value_type E23 = 0.5*( du2vdz + du3vdy + du1vdy*du1vdz + du2vdy*du2vdz + du3vdy*du3vdz );
                const value_type E31 = 0.5*( du3vdx + du1vdz + du1vdz*du1vdx + du2vdz*du2vdx + du3vdz*du3vdx );
                const value_type E32 = 0.5*( du3vdy + du2vdz + du1vdz*du1vdy + du2vdz*du2vdy + du3vdz*du3vdy );
                const value_type E33 = du3vdz + subtraceE3;

                typename super_type::matrix_shape_type/*loc_tensor2_type*/ & theLocRes = this->locRes(q);
                if constexpr ( useDispPresForm )
                {
                    const value_type coefflame2 = this->M_localAssemblyLameSecond[q];
                    const value_type coeffMult = 2*coefflame2*timeSteppingScaling;
                    const value_type S11 = coeffMult*E11;
                    const value_type S12 = coeffMult*E12;
                    const value_type S13 = coeffMult*E13;
                    const value_type S21 = coeffMult*E21;
                    const value_type S22 = coeffMult*E22;
                    const value_type S23 = coeffMult*E23;
                    const value_type S31 = coeffMult*E31;
                    const value_type S32 = coeffMult*E32;
                    const value_type S33 = coeffMult*E33;

                    if constexpr ( expr_type::specific_expr_type::value == ExprApplySolidMecFirstPiolaKirchhoff::EVAL )
                    {
#if 0
                        // F*S
                        theLocRes(0,0) = (1+du1vdx)*S11 + du1vdy*S21 + du1vdz*S31;
                        theLocRes(0,1) = (1+du1vdx)*S12 + du1vdy*S22 + du1vdz*S32;
                        theLocRes(0,2) = (1+du1vdx)*S13 + du1vdy*S23 + du1vdz*S33;
                        theLocRes(1,0) = du2vdx*S11 + (1+du2vdy)*S21 + du2vdz*S31;
                        theLocRes(1,1) = du2vdx*S12 + (1+du2vdy)*S22 + du2vdz*S32;
                        theLocRes(1,2) = du2vdx*S13 + (1+du2vdy)*S23 + du2vdz*S33;
                        theLocRes(2,0) = du3vdx*S11 + du3vdy*S21 + (1+du3vdz)*S31;
                        theLocRes(2,1) = du3vdx*S12 + du3vdy*S22 + (1+du3vdz)*S32;
                        theLocRes(2,2) = du3vdx*S13 + du3vdy*S23 + (1+du3vdz)*S33;
#else
                        //F*(p*Id+S)
                        const value_type pEval = this->locIdPressure()[q](0,0);
                        theLocRes(0,0) = (1+du1vdx)*(S11+pEval) + du1vdy*S21 + du1vdz*S31;
                        theLocRes(0,1) = (1+du1vdx)*S12 + du1vdy*(S22+pEval) + du1vdz*S32;
                        theLocRes(0,2) = (1+du1vdx)*S13 + du1vdy*S23 + du1vdz*(S33+pEval);
                        theLocRes(1,0) = du2vdx*(S11+pEval) + (1+du2vdy)*S21 + du2vdz*S31;
                        theLocRes(1,1) = du2vdx*S12 + (1+du2vdy)*(S22+pEval) + du2vdz*S32;
                        theLocRes(1,2) = du2vdx*S13 + (1+du2vdy)*S23 + du2vdz*(S33+pEval);
                        theLocRes(2,0) = du3vdx*(S11+pEval) + du3vdy*S21 + (1+du3vdz)*S31;
                        theLocRes(2,1) = du3vdx*S12 + du3vdy*(S22+pEval) + (1+du3vdz)*S32;
                        theLocRes(2,2) = du3vdx*S13 + du3vdy*S23 + (1+du3vdz)*(S33+pEval);
#endif
                    }
                    else if constexpr ( expr_type::specific_expr_type::value == ExprApplySolidMecFirstPiolaKirchhoff::JACOBIAN_TRIAL_DISP )
                    {
                        // S
                        theLocRes(0,0) = S11;
                        theLocRes(0,1) = S12;
                        theLocRes(0,2) = S13;
                        theLocRes(1,0) = S21;
                        theLocRes(1,1) = S22;
                        theLocRes(1,2) = S23;
                        theLocRes(2,0) = S31;
                        theLocRes(2,1) = S32;
                        theLocRes(2,2) = S33;
                    }
                }
                else
                {
                    const value_type coefflame1 = this->M_localAssemblyLameFirst[q];
                    const value_type coefflame2 = this->M_localAssemblyLameSecond[q];
                    const value_type traceE = E11+E22+E33;
                    const value_type coeffOnDiag = coefflame1*traceE*timeSteppingScaling;
                    const value_type coeffMult = 2*coefflame2*timeSteppingScaling;

                    const value_type S11 = coeffOnDiag + coeffMult*E11;
                    const value_type S12 = coeffMult*E12;
                    const value_type S13 = coeffMult*E13;
                    const value_type S21 = coeffMult*E21;
                    const value_type S22 = coeffOnDiag + coeffMult*E22;
                    const value_type S23 = coeffMult*E23;
                    const value_type S31 = coeffMult*E31;
                    const value_type S32 = coeffMult*E32;
                    const value_type S33 = coeffOnDiag + coeffMult*E33;

                    if constexpr ( expr_type::specific_expr_type::value == ExprApplySolidMecFirstPiolaKirchhoff::EVAL )
                    {
                        // F*S
                        theLocRes(0,0) = (1+du1vdx)*S11 + du1vdy*S21 + du1vdz*S31;
                        theLocRes(0,1) = (1+du1vdx)*S12 + du1vdy*S22 + du1vdz*S32;
                        theLocRes(0,2) = (1+du1vdx)*S13 + du1vdy*S23 + du1vdz*S33;
                        theLocRes(1,0) = du2vdx*S11 + (1+du2vdy)*S21 + du2vdz*S31;
                        theLocRes(1,1) = du2vdx*S12 + (1+du2vdy)*S22 + du2vdz*S32;
                        theLocRes(1,2) = du2vdx*S13 + (1+du2vdy)*S23 + du2vdz*S33;
                        theLocRes(2,0) = du3vdx*S11 + du3vdy*S21 + (1+du3vdz)*S31;
                        theLocRes(2,1) = du3vdx*S12 + du3vdy*S22 + (1+du3vdz)*S32;
                        theLocRes(2,2) = du3vdx*S13 + du3vdy*S23 + (1+du3vdz)*S33;
                    }
                    else if constexpr ( expr_type::specific_expr_type::value == ExprApplySolidMecFirstPiolaKirchhoff::JACOBIAN_TRIAL_DISP )
                    {
                        // S
                        theLocRes(0,0) = S11;
                        theLocRes(0,1) = S12;
                        theLocRes(0,2) = S13;
                        theLocRes(1,0) = S21;
                        theLocRes(1,1) = S22;
                        theLocRes(1,2) = S23;
                        theLocRes(2,0) = S31;
                        theLocRes(2,1) = S32;
                        theLocRes(2,2) = S33;
                    }
                }
            }
        }

        ret_type
        evalijq( uint16_type i, uint16_type j, uint16_type q, mpl::int_<ExprApplySolidMecFirstPiolaKirchhoff::EVAL> /**/ ) const
        {
            CHECK( false ) << "not allow";
            return ret_type(this->locMatrixShape().data());
        }

        ret_type
        evalijq( uint16_type i, uint16_type j, uint16_type q, mpl::int_<ExprApplySolidMecFirstPiolaKirchhoff::JACOBIAN_TRIAL_DISP> /**/ ) const
        {
            return evalijq( i,j,q, mpl::int_<ExprApplySolidMecFirstPiolaKirchhoff::JACOBIAN_TRIAL_DISP>(),mpl::int_<expr_type::nRealDim>() );
        }
        ret_type
        evalijq( uint16_type i, uint16_type j, uint16_type q, mpl::int_<ExprApplySolidMecFirstPiolaKirchhoff::JACOBIAN_TRIAL_DISP> /**/, mpl::int_<2> /**/ ) const
        {
            // not use time scaling, we put this scaling directly in the bilinearform
            //double timeSteppingScaling = this->expr().timeSteppingScaling();

            auto const& gradDisplacementEval = this->locGradDisplacement(q);
            const value_type du1vdx = gradDisplacementEval(0,0), du1vdy = gradDisplacementEval(0,1);
            const value_type du2vdx = gradDisplacementEval(1,0), du2vdy = gradDisplacementEval(1,1);

            auto const& gradTrial = this->fecTrial()->grad( j, q );
            const value_type du1tdx = gradTrial( 0, 0, 0 ), du1tdy = gradTrial( 0, 1, 0 );
            const value_type du2tdx = gradTrial( 1, 0, 0 ), du2tdy = gradTrial( 1, 1, 0 );

            // dFSv = dF*val(Sv)
            auto const& localEval = this->locRes(q);
            const value_type dFSv11 = du1tdx*localEval(0,0) + du1tdy*localEval(1,0);
            const value_type dFSv12 = du1tdx*localEval(0,1) + du1tdy*localEval(1,1);
            const value_type dFSv21 = du2tdx*localEval(0,0) + du2tdy*localEval(1,0);
            const value_type dFSv22 = du2tdx*localEval(0,1) + du2tdy*localEval(1,1);

            //auto dE = sym(gradt(u)) + 0.5*(trans(gradv(u))*gradt(u) + trans(gradt(u))*gradv(u));
            const value_type dE11 = du1tdx + /*0.5**/(du1vdx*du1tdx + du2vdx*du2tdx);
            const value_type dE12 = 0.5*( du1tdy + du2tdx + du1tdx*du1vdy + du2tdx*du2vdy + du1vdx*du1tdy + du2vdx*du2tdy );
            const value_type dE21 = 0.5*( du2tdx + du1tdy + du1tdy*du1vdx + du2tdy*du2vdx + du1vdy*du1tdx + du2vdy*du2tdx );
            const value_type dE22 = du2tdy + du1vdy*du1tdy + du2vdy*du2tdy;

            typename super_type::matrix_shape_type & thelocMat = this->locMatrixShape();
            if constexpr ( useDispPresForm )
            {
                const value_type coefflame2 = this->M_localAssemblyLameSecond[q];
                const value_type coeffMult = 2*coefflame2/**timeSteppingScaling*/;
                // auto dS = 2*idv(CoeffLame2)*dE;
                const value_type dS11 = coeffMult*dE11;
                const value_type dS12 = coeffMult*dE12;
                const value_type dS21 = coeffMult*dE21;
                const value_type dS22 = coeffMult*dE22;

                // FvdS = val(Fv)*dS
                const value_type FvdS11 = (1.+du1vdx)*dS11 + du1vdy*dS21;
                const value_type FvdS12 = (1.+du1vdx)*dS12 + du1vdy*dS22;
                const value_type FvdS21 = du2vdx*dS11 + (1.+du2vdy)*dS21;
                const value_type FvdS22 = du2vdx*dS12 + (1.+du2vdy)*dS22;

                // dF*val(Sv) + val(Fv)*dS
                thelocMat(0,0) = dFSv11 + FvdS11;
                thelocMat(0,1) = dFSv12 + FvdS12;
                thelocMat(1,0) = dFSv21 + FvdS21;
                thelocMat(1,1) = dFSv22 + FvdS22;

                // add p*dF
                Eigen::Map< const Eigen::Matrix<typename super_type::value_type,Eigen::Dynamic,Eigen::Dynamic/*,Eigen::ColMajor*/ > >
                    gradTrial_matrix( gradTrial.data(), super_type::shape_tensor2::M,super_type::shape_tensor2::N);
                const value_type pEval = this->locIdPressure()[q](0,0);
                thelocMat += /*-*/pEval*gradTrial_matrix;
            }
            else
            {
                const value_type tracedE = dE11 + dE22;
                const value_type coefflame1 = this->M_localAssemblyLameFirst[q];
                const value_type coefflame2 = this->M_localAssemblyLameSecond[q];

                const value_type coeffOnDiag = coefflame1*tracedE/**timeSteppingScaling*/;
                const value_type coeffMult = 2*coefflame2/**timeSteppingScaling*/;

                // auto dS = idv(CoeffLame1)*trace(dE)*Id + 2*idv(CoeffLame2)*dE;
                const value_type dS11 = coeffOnDiag + coeffMult*dE11;
                const value_type dS12 = coeffMult*dE12;
                const value_type dS21 = coeffMult*dE21;
                const value_type dS22 = coeffOnDiag + 2*coefflame2*dE22;

                // FvdS = val(Fv)*dS
                const value_type FvdS11 = (1.+du1vdx)*dS11 + du1vdy*dS21;
                const value_type FvdS12 = (1.+du1vdx)*dS12 + du1vdy*dS22;
                const value_type FvdS21 = du2vdx*dS11 + (1.+du2vdy)*dS21;
                const value_type FvdS22 = du2vdx*dS12 + (1.+du2vdy)*dS22;

                // dF*val(Sv) + val(Fv)*dS
                thelocMat(0,0) = dFSv11 + FvdS11;
                thelocMat(0,1) = dFSv12 + FvdS12;
                thelocMat(1,0) = dFSv21 + FvdS21;
                thelocMat(1,1) = dFSv22 + FvdS22;
            }
            return ret_type(this->locMatrixShape().data());
        }
        ret_type
        evalijq( uint16_type i, uint16_type j, uint16_type q, mpl::int_<ExprApplySolidMecFirstPiolaKirchhoff::JACOBIAN_TRIAL_DISP> /**/, mpl::int_<3> /**/ ) const
        {
            auto const& gradDisplacementEval = this->locGradDisplacement(q);
            const value_type du1vdx = gradDisplacementEval(0,0), du1vdy = gradDisplacementEval(0,1), du1vdz = gradDisplacementEval(0,2);
            const value_type du2vdx = gradDisplacementEval(1,0), du2vdy = gradDisplacementEval(1,1), du2vdz = gradDisplacementEval(1,2);
            const value_type du3vdx = gradDisplacementEval(2,0), du3vdy = gradDisplacementEval(2,1), du3vdz = gradDisplacementEval(2,2);

            auto const& gradTrial = this->fecTrial()->grad( j, q );
            const value_type du1tdx = gradTrial( 0, 0, 0 ), du1tdy = gradTrial( 0, 1, 0 ), du1tdz=gradTrial( 0, 2, 0 );
            const value_type du2tdx = gradTrial( 1, 0, 0 ), du2tdy = gradTrial( 1, 1, 0 ), du2tdz=gradTrial( 1, 2, 0 );
            const value_type du3tdx = gradTrial( 2, 0, 0 ), du3tdy = gradTrial( 2, 1, 0 ), du3tdz=gradTrial( 2, 2, 0 );

            // dFSv = dF*val(Sv)
            auto const& localEval = this->locRes(q);
            const value_type dFSv11 = du1tdx*localEval(0,0) + du1tdy*localEval(1,0) + du1tdz*localEval(2,0);
            const value_type dFSv12 = du1tdx*localEval(0,1) + du1tdy*localEval(1,1) + du1tdz*localEval(2,1);
            const value_type dFSv13 = du1tdx*localEval(0,2) + du1tdy*localEval(1,2) + du1tdz*localEval(2,2);

            const value_type dFSv21 = du2tdx*localEval(0,0) + du2tdy*localEval(1,0) + du2tdz*localEval(2,0);
            const value_type dFSv22 = du2tdx*localEval(0,1) + du2tdy*localEval(1,1) + du2tdz*localEval(2,1);
            const value_type dFSv23 = du2tdx*localEval(0,2) + du2tdy*localEval(1,2) + du2tdz*localEval(2,2);

            const value_type dFSv31 = du3tdx*localEval(0,0) + du3tdy*localEval(1,0) + du3tdz*localEval(2,0);
            const value_type dFSv32 = du3tdx*localEval(0,1) + du3tdy*localEval(1,1) + du3tdz*localEval(2,1);
            const value_type dFSv33 = du3tdx*localEval(0,2) + du3tdy*localEval(1,2) + du3tdz*localEval(2,2);


            //auto dE = sym(gradt(u)) + 0.5*(trans(gradv(u))*gradt(u) + trans(gradt(u))*gradv(u));
            const value_type dE11 = du1tdx + /*0.5**/(du1vdx*du1tdx + du2vdx*du2tdx + du3vdx*du3tdx);
            const value_type dE12 = 0.5*( du1tdy + du2tdx + du1tdx*du1vdy + du2tdx*du2vdy + du3tdx*du3vdy + du1vdx*du1tdy + du2vdx*du2tdy + du3vdx*du3tdy );
            const value_type dE13 = 0.5*( du1tdz + du3tdx + du1tdx*du1vdz + du2tdx*du2vdz + du3tdx*du3vdz + du1vdx*du1tdz + du2vdx*du2tdz + du3vdx*du3tdz );
            const value_type dE21 = 0.5*( du2tdx + du1tdy + du1tdy*du1vdx + du2tdy*du2vdx + du3tdy*du3vdx + du1vdy*du1tdx + du2vdy*du2tdx + du3vdy*du3tdx );
            const value_type dE22 = du2tdy + du1vdy*du1tdy + du2vdy*du2tdy + du3vdy*du3tdy;
            const value_type dE23 = 0.5*( du2tdz + du3tdy + du1tdy*du1vdz + du2tdy*du2vdz + du3tdy*du3vdz + du1vdy*du1tdz + du2vdy*du2tdz + du3vdy*du3tdz );
            const value_type dE31 = 0.5*( du3tdx + du1tdz + du1tdz*du1vdx + du2tdz*du2vdx + du3tdz*du3vdx + du1vdz*du1tdx + du2vdz*du2tdx + du3vdz*du3tdx );
            const value_type dE32 = 0.5*( du3tdy + du2tdz + du1tdz*du1vdy + du2tdz*du2vdy + du3tdz*du3vdy + du1vdz*du1tdy + du2vdz*du2tdy + du3vdz*du3tdy );
            const value_type dE33 = du3tdz + du1vdz*du1tdz + du2vdz*du2tdz + du3vdz*du3tdz;

            typename super_type::matrix_shape_type & thelocMat = this->locMatrixShape();
            if constexpr ( useDispPresForm )
            {
                const value_type coefflame2 = this->M_localAssemblyLameSecond[q];
                const value_type coeffMult = 2*coefflame2/**timeSteppingScaling*/;

                // auto dS = 2*idv(CoeffLame2)*dE;
                const value_type dS11 = coeffMult*dE11;
                const value_type dS12 = coeffMult*dE12;
                const value_type dS13 = coeffMult*dE13;
                const value_type dS21 = coeffMult*dE21;
                const value_type dS22 = coeffMult*dE22;
                const value_type dS23 = coeffMult*dE23;
                const value_type dS31 = coeffMult*dE31;
                const value_type dS32 = coeffMult*dE32;
                const value_type dS33 = coeffMult*dE33;

                // FvdS = val(Fv)*dS
                const value_type FvdS11 = (1.+du1vdx)*dS11 + du1vdy*dS21 + du1vdz*dS31;
                const value_type FvdS12 = (1.+du1vdx)*dS12 + du1vdy*dS22 + du1vdz*dS32;
                const value_type FvdS13 = (1.+du1vdx)*dS13 + du1vdy*dS23 + du1vdz*dS33;
                const value_type FvdS21 = du2vdx*dS11 + (1.+du2vdy)*dS21 + du2vdz*dS31;
                const value_type FvdS22 = du2vdx*dS12 + (1.+du2vdy)*dS22 + du2vdz*dS32;
                const value_type FvdS23 = du2vdx*dS13 + (1.+du2vdy)*dS23 + du2vdz*dS33;
                const value_type FvdS31 = du3vdx*dS11 + du3vdy*dS21 + (1.+du3vdz)*dS31;
                const value_type FvdS32 = du3vdx*dS12 + du3vdy*dS22 + (1.+du3vdz)*dS32;
                const value_type FvdS33 = du3vdx*dS13 + du3vdy*dS23 + (1.+du3vdz)*dS33;

                // dF*val(Sv) + val(Fv)*dS
                thelocMat(0,0) = dFSv11 + FvdS11;
                thelocMat(0,1) = dFSv12 + FvdS12;
                thelocMat(0,2) = dFSv13 + FvdS13;
                thelocMat(1,0) = dFSv21 + FvdS21;
                thelocMat(1,1) = dFSv22 + FvdS22;
                thelocMat(1,2) = dFSv23 + FvdS23;
                thelocMat(2,0) = dFSv31 + FvdS31;
                thelocMat(2,1) = dFSv32 + FvdS32;
                thelocMat(2,2) = dFSv32 + FvdS33;

                // add p*dF
                Eigen::Map< const Eigen::Matrix<typename super_type::value_type,Eigen::Dynamic,Eigen::Dynamic/*,Eigen::ColMajor*/ > >
                    gradTrial_matrix( gradTrial.data(), super_type::shape_tensor2::M,super_type::shape_tensor2::N);
                const value_type pEval = this->locIdPressure()[q](0,0);
                thelocMat += /*-*/pEval*gradTrial_matrix;
            }
            else
            {
                const value_type coefflame1 = this->M_localAssemblyLameFirst[q];
                const value_type coefflame2 = this->M_localAssemblyLameSecond[q];
                const value_type tracedE = dE11 + dE22 + dE33;

                const value_type coeffOnDiag = coefflame1*tracedE/**timeSteppingScaling*/;
                const value_type coeffMult = 2*coefflame2/**timeSteppingScaling*/;

                // auto dS = idv(CoeffLame1)*trace(dE)*Id + 2*idv(CoeffLame2)*dE;
                const value_type dS11 = coeffOnDiag + coeffMult*dE11;
                const value_type dS12 = coeffMult*dE12;
                const value_type dS13 = coeffMult*dE13;
                const value_type dS21 = coeffMult*dE21;
                const value_type dS22 = coeffOnDiag + 2*coefflame2*dE22;
                const value_type dS23 = coeffMult*dE23;
                const value_type dS31 = coeffMult*dE31;
                const value_type dS32 = coeffMult*dE32;
                const value_type dS33 = coeffOnDiag + coeffMult*dE33;

                // FvdS = val(Fv)*dS
                const value_type FvdS11 = (1.+du1vdx)*dS11 + du1vdy*dS21 + du1vdz*dS31;
                const value_type FvdS12 = (1.+du1vdx)*dS12 + du1vdy*dS22 + du1vdz*dS32;
                const value_type FvdS13 = (1.+du1vdx)*dS13 + du1vdy*dS23 + du1vdz*dS33;
                const value_type FvdS21 = du2vdx*dS11 + (1.+du2vdy)*dS21 + du2vdz*dS31;
                const value_type FvdS22 = du2vdx*dS12 + (1.+du2vdy)*dS22 + du2vdz*dS32;
                const value_type FvdS23 = du2vdx*dS13 + (1.+du2vdy)*dS23 + du2vdz*dS33;
                const value_type FvdS31 = du3vdx*dS11 + du3vdy*dS21 + (1.+du3vdz)*dS31;
                const value_type FvdS32 = du3vdx*dS12 + du3vdy*dS22 + (1.+du3vdz)*dS32;
                const value_type FvdS33 = du3vdx*dS13 + du3vdy*dS23 + (1.+du3vdz)*dS33;

                // dF*val(Sv) + val(Fv)*dS
                thelocMat(0,0) = dFSv11 + FvdS11;
                thelocMat(0,1) = dFSv12 + FvdS12;
                thelocMat(0,2) = dFSv13 + FvdS13;
                thelocMat(1,0) = dFSv21 + FvdS21;
                thelocMat(1,1) = dFSv22 + FvdS22;
                thelocMat(1,2) = dFSv23 + FvdS23;
                thelocMat(2,0) = dFSv31 + FvdS31;
                thelocMat(2,1) = dFSv32 + FvdS32;
                thelocMat(2,2) = dFSv32 + FvdS33;
            }
            return ret_type(this->locMatrixShape().data());
        }

        ret_type
        evalijq( uint16_type i, uint16_type j, uint16_type q, mpl::int_<ExprApplySolidMecFirstPiolaKirchhoff::JACOBIAN_TRIAL_PRES> /**/ ) const
        {
            // compute -idt(p)*Fv and Fv is computed in update
            const value_type idTrialPressure = this->fecTrial()->id( j, 0, 0, q );
            typename super_type::matrix_shape_type & thelocMat = this->locMatrixShape();
            auto const& localEval = this->locRes(q);
            thelocMat = /*-*/idTrialPressure*this->locRes(q);
            return ret_type(thelocMat.data());
        }

        template<typename... TheArgsType>
        void updateForUse( const TheArgsType&... theInitArgs )
            {
                std::set<std::string> subexprUsed;
                if constexpr ( expr_type::specific_expr_type::value == ExprApplySolidMecFirstPiolaKirchhoff::JACOBIAN_TRIAL_PRES )
                    subexprUsed.insert("displacement");
                else
                {
                    subexprUsed.insert("displacement");
                    subexprUsed.insert( "Lame-second-parameter" );
                    if constexpr ( !useDispPresForm )
                        subexprUsed.insert( "Lame-first-parameter" );
                    else
                        subexprUsed.insert( "pressure" );
                }
                this->initSubTensor(subexprUsed,theInitArgs...);
            }

    }; // tensorSolidMecFirstPiolaKirchhoffStVenantKirchhoff
















           
             
               
             
           

    /**
     * -------------------------------------------------------------------------------------------------  *
     * -------------------------------------------------------------------------------------------------  *
     * NeoHookeanCompressible :
     *   S = \mu*J^{-2/3}*(I - (1/3)*trace(C)*C^{-1}) (+ pressure part : pJC^{-1})
     * -------------------------------------------------------------------------------------------------  *
     * -------------------------------------------------------------------------------------------------  *
     */
    template<typename Geo_t, typename Basis_i_t, typename Basis_j_t,typename ExprType, bool useDispPresForm>
    struct tensorSolidMecFirstPiolaKirchhoffNeoHookeanCompressible : public tensorFirstPiolaKirchhoffBase<Geo_t,Basis_i_t,Basis_j_t,ExprType>
    {
        typedef tensorFirstPiolaKirchhoffBase<Geo_t,Basis_i_t,Basis_j_t,ExprType> super_type;
    public :
        typedef ExprType expr_type;
        typedef typename super_type::geoelement_type geoelement_type;
        typedef typename super_type::gmc_type gmc_type;
        typedef typename super_type::gm_type gm_type;
        typedef typename super_type::value_type value_type;

        typedef tensorFirstPiolaKirchhoffCompressibleVolumicPartBase<Geo_t,Basis_i_t,Basis_j_t,ExprType> tensor_volumic_part_type;

        tensorSolidMecFirstPiolaKirchhoffNeoHookeanCompressible( expr_type const& expr, Geo_t const& geom, Basis_i_t const& fev, Basis_j_t const& feu )
            :
            super_type( expr,geom,fev,feu ),
            M_locEvalPrecomputeDetF1( boost::extents[ this->gmc()->xRefs().size2()] ),
            //M_locEvalPrecomputeDetF2( boost::extents[ this->gmc()->xRefs().size2()] ),
            M_locEvalPrecomputeDetF3( boost::extents[ this->gmc()->xRefs().size2()] ),
            M_locEvalPrecomputeInvDetF( boost::extents[ this->gmc()->xRefs().size2()] ),
            M_locEvalPrecomputeTraceC( boost::extents[ this->gmc()->xRefs().size2()] ),
            M_locEvalPrecomputeTrialDetF( boost::extents[ this->gmc()->xRefs().size2()] )
            {
                this->updateForUse( geom, fev, feu );
            }
        tensorSolidMecFirstPiolaKirchhoffNeoHookeanCompressible( expr_type const& expr, Geo_t const& geom, Basis_i_t const& fev )
            :
            super_type( expr,geom,fev ),
            M_locEvalPrecomputeDetF1( boost::extents[ this->gmc()->xRefs().size2()] ),
            //M_locEvalPrecomputeDetF2( boost::extents[ this->gmc()->xRefs().size2()] ),
            M_locEvalPrecomputeDetF3( boost::extents[ this->gmc()->xRefs().size2()] ),
            M_locEvalPrecomputeInvDetF( boost::extents[ this->gmc()->xRefs().size2()] ),
            M_locEvalPrecomputeTraceC( boost::extents[ this->gmc()->xRefs().size2()] ),
            M_locEvalPrecomputeTrialDetF( boost::extents[ this->gmc()->xRefs().size2()] )
            {
                this->updateForUse( geom, fev );
            }
        tensorSolidMecFirstPiolaKirchhoffNeoHookeanCompressible( expr_type const& expr,Geo_t const& geom )
            :
            super_type( expr,geom ),
            M_locEvalPrecomputeDetF1( boost::extents[ this->gmc()->xRefs().size2()] ),
            //M_locEvalPrecomputeDetF2( boost::extents[ this->gmc()->xRefs().size2()] ),
            M_locEvalPrecomputeDetF3( boost::extents[ this->gmc()->xRefs().size2()] ),
            M_locEvalPrecomputeInvDetF(boost::extents[ this->gmc()->xRefs().size2()] ),
            M_locEvalPrecomputeTraceC( boost::extents[ this->gmc()->xRefs().size2()] ),
            M_locEvalPrecomputeTrialDetF( boost::extents[ this->gmc()->xRefs().size2()] )
            {
                this->updateForUse( geom );
            }

        void update( Geo_t const& geom ) override
        {
            super_type::update( geom );
            updateImpl( geom );
        }
        void update( Geo_t const& geom, uint16_type face ) override
        {
            super_type::update( geom,face );
            updateImpl( geom );
        }

        // using super_type::evalijq; // fix clang warning
        using ret_type = typename super_type::ret_type;
        ret_type
        evalijq( uint16_type i, uint16_type j, uint16_type q ) const override
        {
            return evalijq( i,j,q, mpl::int_<expr_type::specific_expr_type::value>() );
        }

    private :
#if 1
        void updateImpl( Geo_t const& geom )
        {
            if constexpr ( expr_type::specific_expr_type::value == ExprApplySolidMecFirstPiolaKirchhoff::JACOBIAN_TRIAL_PRES )
                return;

            if constexpr ( !useDispPresForm )
                M_tensorVolumicPart->updateImpl( *this );

            //std::fill( this->locRes().data(), this->locRes().data()+this->locRes().num_elements(), super_type::matrix_shape_type/*loc_res_type*/::Zero() );
            this->locRes().resize( this->gmc()->nPoints(), super_type::matrix_shape_type/*loc_res_type*/::Zero() );
            //updateImpl( /*mpl::int_<expr_type::nRealDim>()*/ );
            updateImpl( mpl::int_<expr_type::nRealDim>() );
        }
#else
        // generic update but seems slower
        void updateImpl( Geo_t const& geom )
        {
            double timeSteppingScaling = this->expr().timeSteppingScaling();
            this->locRes().resize( this->gmc()->nPoints(), super_type::matrix_shape_type/*loc_res_type*/::Zero() );

            for ( uint16_type q = 0; q < this->gmc()->nPoints(); ++q )
            {
                const value_type coefflame2 = M_locEvalFieldCoefflame2[q](0,0);
                auto Id = super_type::loc_matrix_tensor2_type::Identity();
                auto const& gradDisplacementEval = this->locGradDisplacement(q);
                auto F = Id + gradDisplacementEval;
                const value_type detFv = F.determinant();
                //const value_type traceCv = std::pow(Fv11,2) + std::pow(Fv21,2) + std::pow(Fv12,2) + std::pow(Fv22,2);
                const value_type traceCv = F.diagonal().squaredNorm();
                if ( expr_type::specific_expr_type::value == ExprApplySolidMecFirstPiolaKirchhoff::EVAL )
                {
                    auto InvFvmt = F.inverse().transpose();
                    //const value_type A10 = coefflame2/2;
                    this->locRes(q) = coefflame2*std::pow(detFv,-2./expr_type::nRealDim)*(F - (1./expr_type::nRealDim)*traceCv*InvFvmt);
                    if constexpr ( useDispPresForm )
                    {
                        matLoc *= timeSteppingScaling;
                        matLoc += this->M_tensorLagrangeMultiplier->localAssembly(q);
                    }
                    else
                    {
                        this->locRes(q) += M_tensorVolumicPart->locRes(q);
                        matLoc *= timeSteppingScaling;
                    }
                }
                else
                {
                    const value_type factorWithDetFv1 = 1./math::pow(detFv,2);
                    const value_type factorWithDetFv2 = -(2./expr_type::nRealDim)*coefflame2*math::pow(detFv,( (expr_type::nRealDim==2)?-4.:-5.)/expr_type::nRealDim);
                    //const value_type factorWithDetFv2 = -(2./3.)*coefflame2*math::pow(detFv,-5./3.);
                    const value_type factorWithDetFv3 = coefflame2*math::pow(detFv,-2./expr_type::nRealDim);
                    M_locEvalPrecomputeInvDetF[q] = 1./detFv;
                    M_locEvalPrecomputeDetF1[q] = factorWithDetFv1;
                    //M_locEvalPrecomputeDetF2[q](0,0) = factorWithDetFv2;
                    M_locEvalPrecomputeDetF3[q] = factorWithDetFv3;
                    M_locEvalPrecomputeTraceC[q] = traceCv;
                    //TODO here
                    const value_type factorTrialDetF = (1./expr_type::nRealDim)*traceCv;
                    typename super_type::loc_matrix_tensor2_type & matLocTrialDetF = M_locEvalPrecomputeTrialDetF[q];
                    matLocTrialDetF = factorWithDetFv2*(F - factorTrialDetF*F.inverse().transpose() );
                }
            }
        }
#endif
        void updateImpl( mpl::int_<2> /**/ )
        {
            double timeSteppingScaling = this->expr().timeSteppingScaling();
            for ( uint16_type q = 0; q < this->gmc()->nPoints(); ++q )
            {
                const value_type coefflame2 = this->M_localAssemblyLameSecond[q];

                auto const& gradDisplacementEval = this->locGradDisplacement(q);
                const value_type Fv11 = 1.+gradDisplacementEval(0,0), Fv12 = gradDisplacementEval(0,1);
                const value_type Fv21 = gradDisplacementEval(1,0), Fv22 = 1.+gradDisplacementEval(1,1);
                const value_type detFv = Fv11*Fv22-Fv12*Fv21;
                const value_type traceCv = std::pow(Fv11,2) + std::pow(Fv21,2) + std::pow(Fv12,2) + std::pow(Fv22,2);

                if constexpr ( expr_type::specific_expr_type::value == ExprApplySolidMecFirstPiolaKirchhoff::EVAL )
                {
                    const value_type scaleUsedWithInvFv = 1./detFv;
                    const value_type factorWithDetFv = coefflame2*std::pow(detFv,-2./2.);
                    const value_type factorWithTraceCv = (1./2.)*traceCv;
                    const value_type factorOther = factorWithTraceCv*scaleUsedWithInvFv;
                    //auto InvFv = (cst(1.)/detFv)*mat<2,2>( Fv22,-Fv12,-Fv21,Fv11);
                    //auto FSv_neohookean = idv(CoeffLame2)*pow(detFv,-2./2.)*( Fv - (1./2.)*traceCv*trans(InvFv) );
                    typename super_type::matrix_shape_type & matLoc = this->locRes(q);
                    matLoc(0,0) = factorWithDetFv*(Fv11 - factorOther*Fv22);
                    matLoc(1,0) = factorWithDetFv*(Fv21 + factorOther*Fv12);
                    matLoc(0,1) = factorWithDetFv*(Fv12 + factorOther*Fv21);
                    matLoc(1,1) = factorWithDetFv*(Fv22 - factorOther*Fv11);
                    if constexpr ( useDispPresForm )
                    {
                        matLoc *= timeSteppingScaling;
                        matLoc += this->M_tensorLagrangeMultiplier->localAssembly(q);
                    }
                    else
                    {
                        matLoc += M_tensorVolumicPart->locRes(q);
                        matLoc *= timeSteppingScaling;
                    }
                }
                else
                {
                    const value_type factorWithDetFv1 = 1./math::pow(detFv,2);
                    const value_type factorWithDetFv2 = -(2./2.)*coefflame2*math::pow(detFv,-4./2.);
                    const value_type factorWithDetFv3 = coefflame2*math::pow(detFv,-2./2.);
                    M_locEvalPrecomputeInvDetF[q] = 1./detFv;
                    M_locEvalPrecomputeDetF1[q] = factorWithDetFv1;
                    //M_locEvalPrecomputeDetF2[q](0,0) = factorWithDetFv2;
                    M_locEvalPrecomputeDetF3[q] = factorWithDetFv3;
                    M_locEvalPrecomputeTraceC[q] = traceCv;
#if 1 // NEW
                    const value_type factorTrialDetF = (1./2.)*traceCv*(1./detFv);
                    typename super_type::loc_matrix_tensor2_type & matLocTrialDetF = M_locEvalPrecomputeTrialDetF[q];
                    matLocTrialDetF(0,0) = factorWithDetFv2*( Fv11 - factorTrialDetF*Fv22 );
                    matLocTrialDetF(0,1) = factorWithDetFv2*( Fv12 + factorTrialDetF*Fv21 );
                    matLocTrialDetF(1,0) = factorWithDetFv2*( Fv21 + factorTrialDetF*Fv12 );
                    matLocTrialDetF(1,1) = factorWithDetFv2*( Fv22 - factorTrialDetF*Fv11 );
#endif
                }
            }
        }

        void updateImpl( mpl::int_<3> /**/ )
        {
            double timeSteppingScaling = this->expr().timeSteppingScaling();
            for ( uint16_type q = 0; q < this->gmc()->nPoints(); ++q )
            {
                const value_type coefflame2 = this->M_localAssemblyLameSecond[q];

                auto const& gradDisplacementEval = this->locGradDisplacement(q);
                const value_type Fv11 = 1.+gradDisplacementEval(0,0), Fv12 =    gradDisplacementEval(0,1), Fv13 =    gradDisplacementEval(0,2);
                const value_type Fv21 =    gradDisplacementEval(1,0), Fv22 = 1.+gradDisplacementEval(1,1), Fv23 =    gradDisplacementEval(1,2);
                const value_type Fv31 =    gradDisplacementEval(2,0), Fv32 =    gradDisplacementEval(2,1), Fv33 = 1.+gradDisplacementEval(2,2);

                const value_type detFv = Fv11*(Fv22*Fv33-Fv23*Fv32) - Fv21*(Fv12*Fv33-Fv13*Fv32) + Fv31*(Fv12*Fv23 - Fv13*Fv22);
#if 0
                //const value_type detFv = Fv11*(Fv22*Fv33-Fv23*Fv32) - Fv12*(Fv21*Fv33-Fv31*Fv23) + Fv13*(Fv21*Fv32 - Fv31*Fv22);
                if ( detFv < 0 )
                {
                    std::cout << "Negative : " << detFv << "\n"
                              << Fv11 << " " << Fv12 << " " << Fv13 << "\n"
                              << Fv21 << " " << Fv22 << " " << Fv23 << "\n"
                              << Fv31 << " " << Fv32 << " " << Fv33 << "\n";
                }
#endif
                const value_type traceCv = math::pow(Fv11,2) + math::pow(Fv21,2) + math::pow(Fv31,2) + math::pow(Fv12,2) +
                    math::pow(Fv22,2) + math::pow(Fv32,2) + math::pow(Fv13,2) + math::pow(Fv23,2) + math::pow(Fv33,2);


                /*auto InvFv = (cst(1.)/detFv)*mat<3,3>( Fv22*Fv33-Fv23*Fv32 , Fv13*Fv32-Fv12*Fv33 , Fv12*Fv23-Fv13*Fv22,
                 Fv23*Fv31-Fv21*Fv33 , Fv11*Fv33-Fv13*Fv31 , Fv13*Fv21-Fv11*Fv23,
                 Fv21*Fv32-Fv22*Fv31 , Fv12*Fv31-Fv11*Fv32 , Fv11*Fv22-Fv12*Fv21 );*/
                // InvFv ( without 1/detF )
                const value_type InvFv11 = Fv22*Fv33-Fv23*Fv32;
                const value_type InvFv12 = Fv13*Fv32-Fv12*Fv33;
                const value_type InvFv13 = Fv12*Fv23-Fv13*Fv22;
                const value_type InvFv21 = Fv23*Fv31-Fv21*Fv33;
                const value_type InvFv22 = Fv11*Fv33-Fv13*Fv31;
                const value_type InvFv23 = Fv13*Fv21-Fv11*Fv23;
                const value_type InvFv31 = Fv21*Fv32-Fv22*Fv31;
                const value_type InvFv32 = Fv12*Fv31-Fv11*Fv32;
                const value_type InvFv33 = Fv11*Fv22-Fv12*Fv21;

                if constexpr ( expr_type::specific_expr_type::value == ExprApplySolidMecFirstPiolaKirchhoff::EVAL )
                {

                    //auto FSv_neohookean = idv(CoeffLame2)*pow(detFv,-2./3.)*( Fv - (1./3.)*traceCv*trans(InvFv) );
                    const value_type scaleUsedWithInvFv = 1./detFv;
                    const value_type factorWithDetFv = coefflame2*math::pow(detFv,-2./3.);
                    const value_type factorWithTraceCv = (1./3.)*traceCv;
                    const value_type factorOther = factorWithTraceCv*scaleUsedWithInvFv;
                    typename super_type::matrix_shape_type & matLoc = this->locRes(q);
                    matLoc(0,0) = factorWithDetFv*(Fv11 - factorOther*InvFv11);
                    matLoc(1,0) = factorWithDetFv*(Fv21 - factorOther*InvFv12);
                    matLoc(2,0) = factorWithDetFv*(Fv31 - factorOther*InvFv13);
                    matLoc(0,1) = factorWithDetFv*(Fv12 - factorOther*InvFv21);
                    matLoc(1,1) = factorWithDetFv*(Fv22 - factorOther*InvFv22);
                    matLoc(2,1) = factorWithDetFv*(Fv32 - factorOther*InvFv23);
                    matLoc(0,2) = factorWithDetFv*(Fv13 - factorOther*InvFv31);
                    matLoc(1,2) = factorWithDetFv*(Fv23 - factorOther*InvFv32);
                    matLoc(2,2) = factorWithDetFv*(Fv33 - factorOther*InvFv33);
                    if constexpr ( useDispPresForm )
                    {
                        matLoc *= timeSteppingScaling;
                        matLoc += this->M_tensorLagrangeMultiplier->localAssembly(q);
                    }
                    else
                    {
                        matLoc += M_tensorVolumicPart->locRes(q);
                        matLoc *= timeSteppingScaling;
                    }
                }
                else
                {
                    const value_type factorWithDetFv1 = 1./math::pow(detFv,2);
                    const value_type factorWithDetFv2 = -(2./3.)*coefflame2*math::pow(detFv,-5./3.);
                    const value_type factorWithDetFv3 = coefflame2*math::pow(detFv,-2./3.);
                    M_locEvalPrecomputeInvDetF[q] = 1./detFv;
                    M_locEvalPrecomputeDetF1[q] = factorWithDetFv1;
                    //M_locEvalPrecomputeDetF2[q](0,0) = factorWithDetFv2;
                    M_locEvalPrecomputeDetF3[q] = factorWithDetFv3;
                    M_locEvalPrecomputeTraceC[q] = traceCv;
#if 1 // NEW
                    const value_type factorTrialDetF = (1./3.)*traceCv*(1./detFv);
                    typename super_type::loc_matrix_tensor2_type & matLocTrialDetF = M_locEvalPrecomputeTrialDetF[q];
                    matLocTrialDetF(0,0) = factorWithDetFv2*( Fv11 - factorTrialDetF*InvFv11 );
                    matLocTrialDetF(0,1) = factorWithDetFv2*( Fv12 - factorTrialDetF*InvFv21 );
                    matLocTrialDetF(0,2) = factorWithDetFv2*( Fv13 - factorTrialDetF*InvFv31 );
                    matLocTrialDetF(1,0) = factorWithDetFv2*( Fv21 - factorTrialDetF*InvFv12 );
                    matLocTrialDetF(1,1) = factorWithDetFv2*( Fv22 - factorTrialDetF*InvFv22 );
                    matLocTrialDetF(1,2) = factorWithDetFv2*( Fv23 - factorTrialDetF*InvFv32 );
                    matLocTrialDetF(2,0) = factorWithDetFv2*( Fv31 - factorTrialDetF*InvFv13 );
                    matLocTrialDetF(2,1) = factorWithDetFv2*( Fv32 - factorTrialDetF*InvFv23 );
                    matLocTrialDetF(2,2) = factorWithDetFv2*( Fv33 - factorTrialDetF*InvFv33 );
#endif
                }

            }

        }

        ret_type
        evalijq( uint16_type i, uint16_type j, uint16_type q, mpl::int_<ExprApplySolidMecFirstPiolaKirchhoff::EVAL> /**/ ) const
        {
            CHECK( false ) << "not allow";
            return ret_type(this->locMatrixShape().data());
        }

        ret_type
        evalijq( uint16_type i, uint16_type j, uint16_type q, mpl::int_<ExprApplySolidMecFirstPiolaKirchhoff::JACOBIAN_TRIAL_DISP> /**/ ) const
        {
            return evalijq( i,j,q, mpl::int_<ExprApplySolidMecFirstPiolaKirchhoff::JACOBIAN_TRIAL_DISP>(),mpl::int_<expr_type::nRealDim>() );
        }
        ret_type
        evalijq( uint16_type i, uint16_type j, uint16_type q, mpl::int_<ExprApplySolidMecFirstPiolaKirchhoff::JACOBIAN_TRIAL_DISP> /**/, mpl::int_<2> /**/ ) const
        {
            //const value_type coefflame2 = M_locEvalFieldCoefflame2[q](0,0);

            auto const& gradDisplacementEval = this->locGradDisplacement(q);
            const value_type Fv11 = 1.+gradDisplacementEval(0,0), Fv12 = gradDisplacementEval(0,1);
            const value_type Fv21 = gradDisplacementEval(1,0), Fv22 = 1.+gradDisplacementEval(1,1);

            auto const& gradTrial = this->fecTrial()->grad( j, q );
            const value_type dF11/*du1tdx*/ = gradTrial(0,0,0), dF12/*du1tdy*/ = gradTrial(0,1,0);
            const value_type dF21/*du2tdx*/ = gradTrial(1,0,0), dF22/*du2tdy*/ = gradTrial(1,1,0);

            const value_type ddetF = dF11*Fv22 + Fv11*dF22 - Fv21*dF12 - dF21*Fv12;
            const value_type dtraceC = 2*Fv11*dF11 + 2*Fv21*dF21 + 2*Fv12*dF12 + 2*Fv22*dF22;

            const value_type traceCv = M_locEvalPrecomputeTraceC[q];
            // 1./detFv
            const value_type precomputeInvDetF = M_locEvalPrecomputeInvDetF[q];
            // 1./math::pow(detFv,2);
            const value_type precomputeDetF1 = M_locEvalPrecomputeDetF1[q];

            // F^{-T} trial
            const value_type dFmt11 = -ddetF*precomputeDetF1*Fv22+precomputeInvDetF*dF22;
            const value_type dFmt12 =  ddetF*precomputeDetF1*Fv21-precomputeInvDetF*dF21;
            const value_type dFmt21 =  ddetF*precomputeDetF1*Fv12-precomputeInvDetF*dF12;
            const value_type dFmt22 = -ddetF*precomputeDetF1*Fv11+precomputeInvDetF*dF11;
#if 0 // NEW
            const value_type precomputeDetF2 = M_locEvalPrecomputeDetF2[q];
            const value_type dFS_neohookean_a11 = precomputeDetF2*ddetF*( Fv11 - (1./2.)*traceCv*precomputeInvDetF*Fv22 );
            const value_type dFS_neohookean_a12 = precomputeDetF2*ddetF*( Fv12 + (1./2.)*traceCv*precomputeInvDetF*Fv21 );
            const value_type dFS_neohookean_a21 = precomputeDetF2*ddetF*( Fv21 + (1./2.)*traceCv*precomputeInvDetF*Fv12 );
            const value_type dFS_neohookean_a22 = precomputeDetF2*ddetF*( Fv22 - (1./2.)*traceCv*precomputeInvDetF*Fv11 );

            const value_type precomputeDetF3 = M_locEvalPrecomputeDetF3[q];
            const value_type dFS_neohookean_b11 = precomputeDetF3*( dF11 - (1./2.)*(dtraceC*precomputeInvDetF*Fv22+traceCv*dFmt11) );
            const value_type dFS_neohookean_b12 = precomputeDetF3*( dF12 - (1./2.)*(-dtraceC*precomputeInvDetF*Fv21+traceCv*dFmt12) );
            const value_type dFS_neohookean_b21 = precomputeDetF3*( dF21 - (1./2.)*(-dtraceC*precomputeInvDetF*Fv12+traceCv*dFmt21) );
            const value_type dFS_neohookean_b22 = precomputeDetF3*( dF22 - (1./2.)*(dtraceC*precomputeInvDetF*Fv11+traceCv*dFmt22) );

            typename super_type::matrix_shape_type & matLoc = this->locMatrixShape();
            matLoc(0,0) = dFS_neohookean_a11+dFS_neohookean_b11;
            matLoc(1,0) = dFS_neohookean_a21+dFS_neohookean_b21;
            matLoc(0,1) = dFS_neohookean_a12+dFS_neohookean_b12;
            matLoc(1,1) = dFS_neohookean_a22+dFS_neohookean_b22;
#else
            typename super_type::matrix_shape_type & matLoc = this->locMatrixShape();
            matLoc = ddetF*M_locEvalPrecomputeTrialDetF[q];
            const value_type precomputeDetF3 = M_locEvalPrecomputeDetF3[q];
            matLoc(0,0) += precomputeDetF3*( dF11 - (1./2.)*(dtraceC*precomputeInvDetF*Fv22+traceCv*dFmt11) );
            matLoc(0,1) += precomputeDetF3*( dF12 - (1./2.)*(-dtraceC*precomputeInvDetF*Fv21+traceCv*dFmt12) );
            matLoc(1,0) += precomputeDetF3*( dF21 - (1./2.)*(-dtraceC*precomputeInvDetF*Fv12+traceCv*dFmt21) );
            matLoc(1,1) += precomputeDetF3*( dF22 - (1./2.)*(dtraceC*precomputeInvDetF*Fv11+traceCv*dFmt22) );
#endif
            if constexpr ( useDispPresForm )
                matLoc += this->M_tensorLagrangeMultiplier->evalijq( *this,i,j,q );
            else
            {
                typename super_type::loc_matrix_tensor2_type dFmt;
                dFmt(0,0) = dFmt11;dFmt(0,1) = dFmt12;
                dFmt(1,0) = dFmt21;dFmt(1,1) = dFmt22;
                const value_type detFv = 1./precomputeInvDetF;
                matLoc += M_tensorVolumicPart->evalijq( i,j,q, *this, detFv, ddetF, dFmt );
            }
            return ret_type(this->locMatrixShape().data());
        }
        ret_type
        evalijq( uint16_type i, uint16_type j, uint16_type q, mpl::int_<ExprApplySolidMecFirstPiolaKirchhoff::JACOBIAN_TRIAL_DISP> /**/, mpl::int_<3> /**/ ) const
        {
            //const value_type coefflame2 = M_locEvalFieldCoefflame2[q](0,0);

            auto const& gradDisplacementEval = this->locGradDisplacement(q);
            const value_type Fv11 = 1.+gradDisplacementEval(0,0), Fv12 =    gradDisplacementEval(0,1), Fv13 =    gradDisplacementEval(0,2);
            const value_type Fv21 =    gradDisplacementEval(1,0), Fv22 = 1.+gradDisplacementEval(1,1), Fv23 =    gradDisplacementEval(1,2);
            const value_type Fv31 =    gradDisplacementEval(2,0), Fv32 =    gradDisplacementEval(2,1), Fv33 = 1.+gradDisplacementEval(2,2);

            const value_type Fav11 = gradDisplacementEval(0,0), Fav12 = Fv12, Fav13 = Fv13;
            const value_type Fav21 = Fv21, Fav22 = gradDisplacementEval(1,1), Fav23 = Fv23;
            const value_type Fav31 = Fv31, Fav32 = Fv32, Fav33 = gradDisplacementEval(2,2);

            auto const& gradTrial = this->fecTrial()->grad( j, q );
            const value_type dF11/*du1tdx*/ = gradTrial(0,0,0), dF12/*du1tdy*/ = gradTrial(0,1,0), dF13/*du1tdz*/ = gradTrial(0,2,0);
            const value_type dF21/*du2tdx*/ = gradTrial(1,0,0), dF22/*du2tdy*/ = gradTrial(1,1,0), dF23/*du2tdz*/ = gradTrial(1,2,0);
            const value_type dF31/*du3tdx*/ = gradTrial(2,0,0), dF32/*du3tdy*/ = gradTrial(2,1,0), dF33/*du3tdz*/ = gradTrial(2,2,0);

            /*auto InvFvMat = mat<3,3>( Fv22*Fv33-Fv23*Fv32 , Fv13*Fv32-Fv12*Fv33 , Fv12*Fv23-Fv13*Fv22,
              Fv23*Fv31-Fv21*Fv33 , Fv11*Fv33-Fv13*Fv31 , Fv13*Fv21-Fv11*Fv23,
              Fv21*Fv32-Fv22*Fv31 , Fv12*Fv31-Fv11*Fv32 , Fv11*Fv22-Fv12*Fv21 );*/
            // InvFv ( without 1/detF )
            const value_type InvFv11 = Fv22*Fv33-Fv23*Fv32;
            const value_type InvFv12 = Fv13*Fv32-Fv12*Fv33;
            const value_type InvFv13 = Fv12*Fv23-Fv13*Fv22;
            const value_type InvFv21 = Fv23*Fv31-Fv21*Fv33;
            const value_type InvFv22 = Fv11*Fv33-Fv13*Fv31;
            const value_type InvFv23 = Fv13*Fv21-Fv11*Fv23;
            const value_type InvFv31 = Fv21*Fv32-Fv22*Fv31;
            const value_type InvFv32 = Fv12*Fv31-Fv11*Fv32;
            const value_type InvFv33 = Fv11*Fv22-Fv12*Fv21;


            const value_type FmtNL11 = dF22+dF33+dF22*Fav33+Fav22*dF33-dF23*Fav32-Fav23*dF32;
            const value_type FmtNL12 = dF23*Fav31+Fav23*dF31-dF21-dF21*Fav33-Fav21*dF33;
            const value_type FmtNL13 = dF21*Fav32+Fav21*dF32-dF31-dF31*Fav22-Fav31*dF22;
            const value_type FmtNL21 = dF13*Fav32+Fav13*dF32-dF12-dF12*Fav33-Fav12*dF33;
            const value_type FmtNL22 = dF11+dF33+dF11*Fav33+Fav11*dF33-dF13*Fav31-Fav13*dF31;
            const value_type FmtNL23 = dF12*Fav31+Fav12*dF31-dF32-dF32*Fav11-Fav32*dF11;
            const value_type FmtNL31 = dF12*Fav23+Fav12*dF23-dF13-dF13*Fav22-Fav13*dF22;
            const value_type FmtNL32 = dF13*Fav21+Fav13*dF21-dF23-dF23*Fav11-Fav23*dF11;
            const value_type FmtNL33 = dF11+dF22+dF11*Fav22+Fav11*dF22-dF12*Fav21-Fav12*dF21;

            const value_type ddetF = dF11 + dF22 + dF33 + (dF22*Fav33 + Fav22*dF33) + (dF11*Fav22 + Fav11*dF22) + (dF11*Fav33 + Fav11*dF33)
            + dF11*Fav22*Fav33 + Fav11*dF22*Fav33 + Fav11*Fav22*dF33
            + dF12*Fav23*Fav31 + Fav12*dF23*Fav31 + Fav12*Fav23*dF31
            + dF13*Fav21*Fav32 + Fav13*dF21*Fav32 + Fav13*Fav21*dF32
            - (dF23*Fav32 + Fav23*dF32) - (dF12*Fav21 + Fav12*dF21) - (dF13*Fav31 + Fav13*dF31)
            - (dF11*Fav23*Fav32 + Fav11*dF23*Fav32 + Fav11*Fav23*dF32)
            - (dF12*Fav21*Fav33 + Fav12*dF21*Fav33 + Fav12*Fav21*dF33)
            - (dF13*Fav22*Fav31 + Fav13*dF22*Fav31 + Fav13*Fav22*dF31) ;

            const value_type dtraceC = 2*Fv11*dF11 + 2*Fv21*dF21 + 2*Fv31*dF31 + 2*Fv12*dF12 + 2*Fv22*dF22 + 2*Fv32*dF32 + 2*Fv13*dF13 + 2*Fv23*dF23 + 2*Fv33*dF33;



            /*auto InvFtMat =  mat<3,3>(FmtNL11,FmtNL12,FmtNL13,
                                  FmtNL21,FmtNL22,FmtNL23,
                                  FmtNL31,FmtNL32,FmtNL33 );*/
            //auto dFmt = -(ddetF/pow(detFv,2))*trans(InvFvMat)+(cst(1.)/detFv)*/*trans*/(InvFtMat) ;

            //auto dFS_neohookean_a = -(2./3.)*idv(CoeffLame2)*ddetF*pow(detFv,-5./3.) *( Fv - (1./3.)*traceCv*trans(InvFv) );
            //auto dFS_neohookean_b = idv(CoeffLame2)*pow(detFv,-2./3.)*( dF - (1./3.)*(dtraceC*trans(InvFv)+traceCv*dFmt) );
            //auto dFS_neohookean = dFS_neohookean_a+dFS_neohookean_b;

            const value_type traceCv = M_locEvalPrecomputeTraceC[q];
            const value_type precomputeInvDetF = M_locEvalPrecomputeInvDetF[q];
            const value_type precomputeDetF1 = M_locEvalPrecomputeDetF1[q];
            const value_type dFmt11 = -ddetF*precomputeDetF1*InvFv11 + precomputeInvDetF*FmtNL11;
            const value_type dFmt12 = -ddetF*precomputeDetF1*InvFv21 + precomputeInvDetF*FmtNL12;
            const value_type dFmt13 = -ddetF*precomputeDetF1*InvFv31 + precomputeInvDetF*FmtNL13;
            const value_type dFmt21 = -ddetF*precomputeDetF1*InvFv12 + precomputeInvDetF*FmtNL21;
            const value_type dFmt22 = -ddetF*precomputeDetF1*InvFv22 + precomputeInvDetF*FmtNL22;
            const value_type dFmt23 = -ddetF*precomputeDetF1*InvFv32 + precomputeInvDetF*FmtNL23;
            const value_type dFmt31 = -ddetF*precomputeDetF1*InvFv13 + precomputeInvDetF*FmtNL31;
            const value_type dFmt32 = -ddetF*precomputeDetF1*InvFv23 + precomputeInvDetF*FmtNL32;
            const value_type dFmt33 = -ddetF*precomputeDetF1*InvFv33 + precomputeInvDetF*FmtNL33;
#if 0 // NEW
            const value_type precomputeDetF2 = M_locEvalPrecomputeDetF2[q];
            const value_type dFS_neohookean_a11 = precomputeDetF2*ddetF*( Fv11 - (1./3.)*traceCv*precomputeInvDetF*InvFv11 );
            const value_type dFS_neohookean_a12 = precomputeDetF2*ddetF*( Fv12 - (1./3.)*traceCv*precomputeInvDetF*InvFv21 );
            const value_type dFS_neohookean_a13 = precomputeDetF2*ddetF*( Fv13 - (1./3.)*traceCv*precomputeInvDetF*InvFv31 );
            const value_type dFS_neohookean_a21 = precomputeDetF2*ddetF*( Fv21 - (1./3.)*traceCv*precomputeInvDetF*InvFv12 );
            const value_type dFS_neohookean_a22 = precomputeDetF2*ddetF*( Fv22 - (1./3.)*traceCv*precomputeInvDetF*InvFv22 );
            const value_type dFS_neohookean_a23 = precomputeDetF2*ddetF*( Fv23 - (1./3.)*traceCv*precomputeInvDetF*InvFv32 );
            const value_type dFS_neohookean_a31 = precomputeDetF2*ddetF*( Fv31 - (1./3.)*traceCv*precomputeInvDetF*InvFv13 );
            const value_type dFS_neohookean_a32 = precomputeDetF2*ddetF*( Fv32 - (1./3.)*traceCv*precomputeInvDetF*InvFv23 );
            const value_type dFS_neohookean_a33 = precomputeDetF2*ddetF*( Fv33 - (1./3.)*traceCv*precomputeInvDetF*InvFv33 );

            const value_type precomputeDetF3 = M_locEvalPrecomputeDetF3[q];
            const value_type dFS_neohookean_b11 = precomputeDetF3*( dF11 - (1./3.)*(dtraceC*precomputeInvDetF*InvFv11+traceCv*dFmt11) );
            const value_type dFS_neohookean_b12 = precomputeDetF3*( dF12 - (1./3.)*(dtraceC*precomputeInvDetF*InvFv21+traceCv*dFmt12) );
            const value_type dFS_neohookean_b13 = precomputeDetF3*( dF13 - (1./3.)*(dtraceC*precomputeInvDetF*InvFv31+traceCv*dFmt13) );
            const value_type dFS_neohookean_b21 = precomputeDetF3*( dF21 - (1./3.)*(dtraceC*precomputeInvDetF*InvFv12+traceCv*dFmt21) );
            const value_type dFS_neohookean_b22 = precomputeDetF3*( dF22 - (1./3.)*(dtraceC*precomputeInvDetF*InvFv22+traceCv*dFmt22) );
            const value_type dFS_neohookean_b23 = precomputeDetF3*( dF23 - (1./3.)*(dtraceC*precomputeInvDetF*InvFv32+traceCv*dFmt23) );
            const value_type dFS_neohookean_b31 = precomputeDetF3*( dF31 - (1./3.)*(dtraceC*precomputeInvDetF*InvFv13+traceCv*dFmt31) );
            const value_type dFS_neohookean_b32 = precomputeDetF3*( dF32 - (1./3.)*(dtraceC*precomputeInvDetF*InvFv23+traceCv*dFmt32) );
            const value_type dFS_neohookean_b33 = precomputeDetF3*( dF33 - (1./3.)*(dtraceC*precomputeInvDetF*InvFv33+traceCv*dFmt33) );

            typename super_type::matrix_shape_type & matLoc = this->locMatrixShape();
            matLoc(0,0) = dFS_neohookean_a11 + dFS_neohookean_b11;
            matLoc(1,0) = dFS_neohookean_a21 + dFS_neohookean_b21;
            matLoc(2,0) = dFS_neohookean_a31 + dFS_neohookean_b31;
            matLoc(0,1) = dFS_neohookean_a12 + dFS_neohookean_b12;
            matLoc(1,1) = dFS_neohookean_a22 + dFS_neohookean_b22;
            matLoc(2,1) = dFS_neohookean_a32 + dFS_neohookean_b32;
            matLoc(0,2) = dFS_neohookean_a13 + dFS_neohookean_b13;
            matLoc(1,2) = dFS_neohookean_a23 + dFS_neohookean_b23;
            matLoc(2,2) = dFS_neohookean_a33 + dFS_neohookean_b33;
#else
            typename super_type::matrix_shape_type & matLoc = this->locMatrixShape();
            matLoc = ddetF*M_locEvalPrecomputeTrialDetF[q];
            const value_type precomputeDetF3 = M_locEvalPrecomputeDetF3[q];
            matLoc(0,0) += precomputeDetF3*( dF11 - (1./3.)*(dtraceC*precomputeInvDetF*InvFv11+traceCv*dFmt11) );
            matLoc(0,1) += precomputeDetF3*( dF12 - (1./3.)*(dtraceC*precomputeInvDetF*InvFv21+traceCv*dFmt12) );
            matLoc(0,2) += precomputeDetF3*( dF13 - (1./3.)*(dtraceC*precomputeInvDetF*InvFv31+traceCv*dFmt13) );
            matLoc(1,0) += precomputeDetF3*( dF21 - (1./3.)*(dtraceC*precomputeInvDetF*InvFv12+traceCv*dFmt21) );
            matLoc(1,1) += precomputeDetF3*( dF22 - (1./3.)*(dtraceC*precomputeInvDetF*InvFv22+traceCv*dFmt22) );
            matLoc(1,2) += precomputeDetF3*( dF23 - (1./3.)*(dtraceC*precomputeInvDetF*InvFv32+traceCv*dFmt23) );
            matLoc(2,0) += precomputeDetF3*( dF31 - (1./3.)*(dtraceC*precomputeInvDetF*InvFv13+traceCv*dFmt31) );
            matLoc(2,1) += precomputeDetF3*( dF32 - (1./3.)*(dtraceC*precomputeInvDetF*InvFv23+traceCv*dFmt32) );
            matLoc(2,2) += precomputeDetF3*( dF33 - (1./3.)*(dtraceC*precomputeInvDetF*InvFv33+traceCv*dFmt33) );
#endif
            if constexpr ( useDispPresForm )
                matLoc += this->M_tensorLagrangeMultiplier->evalijq( *this,i,j,q );
            else
            {
                typename super_type::loc_matrix_tensor2_type dFmt;
                dFmt(0,0) = dFmt11;dFmt(0,1) = dFmt12;dFmt(0,2) = dFmt13;
                dFmt(1,0) = dFmt21;dFmt(1,1) = dFmt22;dFmt(1,2) = dFmt23;
                dFmt(2,0) = dFmt31;dFmt(2,1) = dFmt32;dFmt(2,2) = dFmt33;
                const value_type detFv = 1./precomputeInvDetF;
                matLoc += M_tensorVolumicPart->evalijq( i,j,q, *this, detFv, ddetF, dFmt );
            }

            return ret_type(this->locMatrixShape().data());
        }

        ret_type
        evalijq( uint16_type i, uint16_type j, uint16_type q, mpl::int_<ExprApplySolidMecFirstPiolaKirchhoff::JACOBIAN_TRIAL_PRES> /**/ ) const
            {
                return this->M_tensorLagrangeMultiplier->evalijq( *this,i,j,q );
            }

        template<typename... TheArgsType>
        void updateForUse( const TheArgsType&... theInitArgs )
            {
                std::set<std::string> propUsed = { "displacement" };
                if constexpr ( useDispPresForm )
                {
                    propUsed.insert( "lagange_multiplier" );
                    if ( expr_type::specific_expr_type::value == ExprApplySolidMecFirstPiolaKirchhoff::EVAL ||
                         expr_type::specific_expr_type::value == ExprApplySolidMecFirstPiolaKirchhoff::JACOBIAN_TRIAL_DISP )
                    {
                        propUsed.insert( "Lame-second-parameter" );
                        propUsed.insert( "pressure");
                    }
                }
                else
                {
                    propUsed.insert( "Lame-second-parameter" );
                    M_tensorVolumicPart = tensor_volumic_part_type::New( this->expr().physicSolidData().decouplingEnergyVolumicLaw(),this->expr(),theInitArgs... );
                    auto propToAdd = M_tensorVolumicPart->propertiesUsed();
                    propUsed.insert( propToAdd.begin(), propToAdd.end() );
                }
                this->initSubTensor(propUsed,theInitArgs...);
            }

    private :
        typename super_type::array_value_type M_locEvalPrecomputeDetF1/*,M_locEvalPrecomputeDetF2*/,M_locEvalPrecomputeDetF3, M_locEvalPrecomputeInvDetF, M_locEvalPrecomputeTraceC;
        typename super_type::array_matrix_tensor2_type M_locEvalPrecomputeTrialDetF;

        std::shared_ptr<tensor_volumic_part_type> M_tensorVolumicPart;
    };

    template<typename Geo_t, typename Basis_i_t, typename Basis_j_t,typename ExprType,bool useDispPresForm>
    struct tensorSolidMecFirstPiolaKirchhoffNeoHookeanCompressibleMolecularTheory : public tensorFirstPiolaKirchhoffBase<Geo_t,Basis_i_t,Basis_j_t,ExprType>
    {
        typedef tensorFirstPiolaKirchhoffBase<Geo_t,Basis_i_t,Basis_j_t,ExprType> super_type;
    public :
        typedef ExprType expr_type;
        typedef typename super_type::geoelement_type geoelement_type;
        typedef typename super_type::gmc_type gmc_type;
        typedef typename super_type::gm_type gm_type;
        typedef typename super_type::value_type value_type;

        using ret_type = typename super_type::ret_type;

        typedef tensorFirstPiolaKirchhoffCompressibleVolumicPartBase<Geo_t,Basis_i_t,Basis_j_t,ExprType> tensor_volumic_part_type;

        tensorSolidMecFirstPiolaKirchhoffNeoHookeanCompressibleMolecularTheory( expr_type const& expr,Geo_t const& geom, Basis_i_t const& fev, Basis_j_t const& feu )
            :
            super_type( expr,geom,fev,feu ),
            M_locEvalPrecomputePowDetF( boost::extents[ this->gmc()->xRefs().size2()] ),
            M_locEvalPrecomputeDetF( boost::extents[ this->gmc()->xRefs().size2()] )
            {
                this->updateForUse( geom, fev, feu );
            }
        tensorSolidMecFirstPiolaKirchhoffNeoHookeanCompressibleMolecularTheory( expr_type const& expr, Geo_t const& geom, Basis_i_t const& fev )
            :
            super_type( expr,geom,fev ),
            M_locEvalPrecomputePowDetF( boost::extents[ this->gmc()->xRefs().size2()] ),
            M_locEvalPrecomputeDetF( boost::extents[ this->gmc()->xRefs().size2()] )
            {
                this->updateForUse( geom, fev );
            }
        tensorSolidMecFirstPiolaKirchhoffNeoHookeanCompressibleMolecularTheory( expr_type const& expr, Geo_t const& geom )
            :
            super_type( expr,geom ),
            M_locEvalPrecomputePowDetF( boost::extents[ this->gmc()->xRefs().size2()] ),
            M_locEvalPrecomputeDetF( boost::extents[ this->gmc()->xRefs().size2()] )
            {
                this->updateForUse( geom );
            }

        void update( Geo_t const& geom ) override
        {
            super_type::update( geom );
            updateImpl( geom );
        }
        void update( Geo_t const& geom, uint16_type face ) override
        {
            super_type::update( geom,face );
            updateImpl( geom );
        }

        // using super_type::evalijq; // fix clang warning

        ret_type
        evalijq( uint16_type i, uint16_type j, uint16_type q ) const override
        {
            return evalijq( i,j,q, mpl::int_<expr_type::specific_expr_type::value>() );
        }

    private :

        void updateImpl( Geo_t const& geom )
        {
            if constexpr ( expr_type::specific_expr_type::value == ExprApplySolidMecFirstPiolaKirchhoff::JACOBIAN_TRIAL_PRES )
                return;

            if constexpr ( !useDispPresForm )
                M_tensorVolumicPart->updateImpl( *this );

            this->locRes().resize( this->gmc()->nPoints(), super_type::matrix_shape_type/*loc_res_type*/::Zero() );
            double timeSteppingScaling = this->expr().timeSteppingScaling();
            for ( uint16_type q = 0; q < this->gmc()->nPoints(); ++q )
            {
                const value_type coefflame2 = this->M_localAssemblyLameSecond[q];
                auto const& gradDisplacementEval = this->locGradDisplacement(q);
                auto Id = super_type::loc_matrix_tensor2_type::Identity();
                auto F = Id + gradDisplacementEval;
                const value_type detFv = F.determinant();
                if constexpr ( expr_type::specific_expr_type::value == ExprApplySolidMecFirstPiolaKirchhoff::EVAL )
                {
                    auto InvFvmt = F.inverse().transpose();
                    this->locRes(q) = coefflame2*(F-InvFvmt);
                    if constexpr ( useDispPresForm )
                    {
                        this->locRes(q) *= timeSteppingScaling;
                        this->locRes(q) += this->M_tensorLagrangeMultiplier->localAssembly(q);
                    }
                    else
                    {
                        this->locRes(q) += M_tensorVolumicPart->locRes(q);
                        this->locRes(q) *= timeSteppingScaling;
                    }
                }
                else
                {
                    M_locEvalPrecomputePowDetF[q] = math::pow(detFv,2);
                    M_locEvalPrecomputeDetF[q] = detFv;
                }
            }
        }

        ret_type
        evalijq( uint16_type i, uint16_type j, uint16_type q, mpl::int_<ExprApplySolidMecFirstPiolaKirchhoff::EVAL> /**/ ) const
        {
            CHECK( false ) << "not allow";
            return ret_type(this->locMatrixShape().data());
        }

        ret_type
        evalijq( uint16_type i, uint16_type j, uint16_type q, mpl::int_<ExprApplySolidMecFirstPiolaKirchhoff::JACOBIAN_TRIAL_DISP> /**/ ) const
        {
            return evalijq( i,j,q, mpl::int_<ExprApplySolidMecFirstPiolaKirchhoff::JACOBIAN_TRIAL_DISP>(),mpl::int_<expr_type::nRealDim>() );
        }
        ret_type
        evalijq( uint16_type i, uint16_type j, uint16_type q, mpl::int_<ExprApplySolidMecFirstPiolaKirchhoff::JACOBIAN_TRIAL_DISP> /**/, mpl::int_<2> /**/ ) const
        {
            const value_type coefflame2 = this->M_localAssemblyLameSecond[q];

            auto const& gradDisplacementEval = this->locGradDisplacement(q);
            const value_type Fv11 = 1.+gradDisplacementEval(0,0), Fv12 = gradDisplacementEval(0,1);
            const value_type Fv21 = gradDisplacementEval(1,0), Fv22 = 1.+gradDisplacementEval(1,1);


            auto const& _gradTrial = this->fecTrial()->grad( j, q );
            const value_type dF11/*du1tdx*/ = _gradTrial(0,0,0), dF12/*du1tdy*/ = _gradTrial(0,1,0);
            const value_type dF21/*du2tdx*/ = _gradTrial(1,0,0), dF22/*du2tdy*/ = _gradTrial(1,1,0);
            Eigen::Map< const Eigen::Matrix<typename super_type::value_type,Eigen::Dynamic,Eigen::Dynamic/*,Eigen::ColMajor*/ > >
                gradTrial( _gradTrial.data(), super_type::shape_tensor2::M,super_type::shape_tensor2::N);

            const value_type ddetF = dF11*Fv22 + Fv11*dF22 - Fv21*dF12 - dF21*Fv12;
            const value_type detFv = M_locEvalPrecomputeDetF[q];

            const value_type scaleUsedWithInvFv = 1./detFv;
            const value_type precomputePowDetF = M_locEvalPrecomputePowDetF[q];

            const value_type factorOther = ddetF/precomputePowDetF;

            typename super_type::loc_matrix_tensor2_type dFmt;
            dFmt(0,0) = -factorOther*Fv22 + scaleUsedWithInvFv*dF22;
            dFmt(0,1) =  factorOther*Fv21 - scaleUsedWithInvFv*dF21;
            dFmt(1,0) =  factorOther*Fv12 - scaleUsedWithInvFv*dF12;
            dFmt(1,1) = -factorOther*Fv11 + scaleUsedWithInvFv*dF11;

            //typename super_type::matrix_shape_type& locMat = this->locMatrixShape();
            this->locMatrixShape() = coefflame2*(gradTrial - dFmt);

            if constexpr ( useDispPresForm )
                 this->locMatrixShape() += this->M_tensorLagrangeMultiplier->evalijq( *this,i,j,q );
            else
                this->locMatrixShape() += M_tensorVolumicPart->evalijq( i,j,q, *this, detFv, ddetF, dFmt );

            return ret_type(this->locMatrixShape().data());
        }
        ret_type
        evalijq( uint16_type i, uint16_type j, uint16_type q, mpl::int_<ExprApplySolidMecFirstPiolaKirchhoff::JACOBIAN_TRIAL_DISP> /**/, mpl::int_<3> /**/ ) const
        {
            const value_type coefflame2 = this->M_localAssemblyLameSecond[q];

            auto const& gradDisplacementEval = this->locGradDisplacement(q);
            const value_type Fv11 = 1.+gradDisplacementEval(0,0), Fv12 =    gradDisplacementEval(0,1), Fv13 =    gradDisplacementEval(0,2);
            const value_type Fv21 =    gradDisplacementEval(1,0), Fv22 = 1.+gradDisplacementEval(1,1), Fv23 =    gradDisplacementEval(1,2);
            const value_type Fv31 =    gradDisplacementEval(2,0), Fv32 =    gradDisplacementEval(2,1), Fv33 = 1.+gradDisplacementEval(2,2);

            const value_type Fav11 = gradDisplacementEval(0,0), Fav12 = Fv12, Fav13 = Fv13;
            const value_type Fav21 = Fv21, Fav22 = gradDisplacementEval(1,1), Fav23 = Fv23;
            const value_type Fav31 = Fv31, Fav32 = Fv32, Fav33 = gradDisplacementEval(2,2);

            auto const& _gradTrial = this->fecTrial()->grad( j, q );
            const value_type dF11/*du1tdx*/ = _gradTrial(0,0,0), dF12/*du1tdy*/ = _gradTrial(0,1,0), dF13/*du1tdz*/ = _gradTrial(0,2,0);
            const value_type dF21/*du2tdx*/ = _gradTrial(1,0,0), dF22/*du2tdy*/ = _gradTrial(1,1,0), dF23/*du2tdz*/ = _gradTrial(1,2,0);
            const value_type dF31/*du3tdx*/ = _gradTrial(2,0,0), dF32/*du3tdy*/ = _gradTrial(2,1,0), dF33/*du3tdz*/ = _gradTrial(2,2,0);
            Eigen::Map< const Eigen::Matrix<typename super_type::value_type,Eigen::Dynamic,Eigen::Dynamic/*,Eigen::ColMajor*/ > >
                gradTrial( _gradTrial.data(), super_type::shape_tensor2::M,super_type::shape_tensor2::N);

            const value_type ddetF = dF11 + dF22 + dF33 + (dF22*Fav33 + Fav22*dF33) + (dF11*Fav22 + Fav11*dF22) + (dF11*Fav33 + Fav11*dF33)
            + dF11*Fav22*Fav33 + Fav11*dF22*Fav33 + Fav11*Fav22*dF33
            + dF12*Fav23*Fav31 + Fav12*dF23*Fav31 + Fav12*Fav23*dF31
            + dF13*Fav21*Fav32 + Fav13*dF21*Fav32 + Fav13*Fav21*dF32
            - (dF23*Fav32 + Fav23*dF32) - (dF12*Fav21 + Fav12*dF21) - (dF13*Fav31 + Fav13*dF31)
            - (dF11*Fav23*Fav32 + Fav11*dF23*Fav32 + Fav11*Fav23*dF32)
            - (dF12*Fav21*Fav33 + Fav12*dF21*Fav33 + Fav12*Fav21*dF33)
            - (dF13*Fav22*Fav31 + Fav13*dF22*Fav31 + Fav13*Fav22*dF31) ;


            /*auto InvFvMat = mat<3,3>( Fv22*Fv33-Fv23*Fv32 , Fv13*Fv32-Fv12*Fv33 , Fv12*Fv23-Fv13*Fv22,
              Fv23*Fv31-Fv21*Fv33 , Fv11*Fv33-Fv13*Fv31 , Fv13*Fv21-Fv11*Fv23,
              Fv21*Fv32-Fv22*Fv31 , Fv12*Fv31-Fv11*Fv32 , Fv11*Fv22-Fv12*Fv21 );*/
            // InvFv ( without 1/detF )
            const value_type InvFv11 = Fv22*Fv33-Fv23*Fv32;
            const value_type InvFv12 = Fv13*Fv32-Fv12*Fv33;
            const value_type InvFv13 = Fv12*Fv23-Fv13*Fv22;
            const value_type InvFv21 = Fv23*Fv31-Fv21*Fv33;
            const value_type InvFv22 = Fv11*Fv33-Fv13*Fv31;
            const value_type InvFv23 = Fv13*Fv21-Fv11*Fv23;
            const value_type InvFv31 = Fv21*Fv32-Fv22*Fv31;
            const value_type InvFv32 = Fv12*Fv31-Fv11*Fv32;
            const value_type InvFv33 = Fv11*Fv22-Fv12*Fv21;

            const value_type FmtNL11 = dF22+dF33+dF22*Fav33+Fav22*dF33-dF23*Fav32-Fav23*dF32;
            const value_type FmtNL12 = dF23*Fav31+Fav23*dF31-dF21-dF21*Fav33-Fav21*dF33;
            const value_type FmtNL13 = dF21*Fav32+Fav21*dF32-dF31-dF31*Fav22-Fav31*dF22;
            const value_type FmtNL21 = dF13*Fav32+Fav13*dF32-dF12-dF12*Fav33-Fav12*dF33;
            const value_type FmtNL22 = dF11+dF33+dF11*Fav33+Fav11*dF33-dF13*Fav31-Fav13*dF31;
            const value_type FmtNL23 = dF12*Fav31+Fav12*dF31-dF32-dF32*Fav11-Fav32*dF11;
            const value_type FmtNL31 = dF12*Fav23+Fav12*dF23-dF13-dF13*Fav22-Fav13*dF22;
            const value_type FmtNL32 = dF13*Fav21+Fav13*dF21-dF23-dF23*Fav11-Fav23*dF11;
            const value_type FmtNL33 = dF11+dF22+dF11*Fav22+Fav11*dF22-dF12*Fav21-Fav12*dF21;

            /*auto InvFtMat =  mat<3,3>(FmtNL11,FmtNL12,FmtNL13,
                                  FmtNL21,FmtNL22,FmtNL23,
                                  FmtNL31,FmtNL32,FmtNL33 );*/
            //auto dFmt = -(ddetF/pow(detFv,2))*trans(InvFvMat)+(cst(1.)/detFv)*/*trans*/(InvFtMat) ;

            const value_type detFv = M_locEvalPrecomputeDetF[q];
            const value_type scaleUsedWithInvFv = 1./detFv;
            const value_type precomputePowDetF = M_locEvalPrecomputePowDetF[q];//pow(detFv,2)  //idv(CoeffLame1)/pow(detFv,2)

            const value_type factorOther = ddetF/precomputePowDetF;//math::pow(detFv,2);

            typename super_type::loc_matrix_tensor2_type dFmt;
            dFmt(0,0) = -factorOther*InvFv11 + scaleUsedWithInvFv*FmtNL11;
            dFmt(0,1) = -factorOther*InvFv21 + scaleUsedWithInvFv*FmtNL12;
            dFmt(0,2) = -factorOther*InvFv31 + scaleUsedWithInvFv*FmtNL13;
            dFmt(1,0) = -factorOther*InvFv12 + scaleUsedWithInvFv*FmtNL21;
            dFmt(1,1) = -factorOther*InvFv22 + scaleUsedWithInvFv*FmtNL22;
            dFmt(1,2) = -factorOther*InvFv32 + scaleUsedWithInvFv*FmtNL23;
            dFmt(2,0) = -factorOther*InvFv13 + scaleUsedWithInvFv*FmtNL31;
            dFmt(2,1) = -factorOther*InvFv23 + scaleUsedWithInvFv*FmtNL32;
            dFmt(2,2) = -factorOther*InvFv33 + scaleUsedWithInvFv*FmtNL33;

            this->locMatrixShape() = coefflame2*(gradTrial - dFmt);

            if constexpr ( useDispPresForm )
                 this->locMatrixShape() += this->M_tensorLagrangeMultiplier->evalijq( *this,i,j,q );
            else
                 this->locMatrixShape() += M_tensorVolumicPart->evalijq( i,j,q, *this, detFv, ddetF, dFmt );

            return ret_type(this->locMatrixShape().data());
        }

        ret_type
        evalijq( uint16_type i, uint16_type j, uint16_type q, mpl::int_<ExprApplySolidMecFirstPiolaKirchhoff::JACOBIAN_TRIAL_PRES> /**/ ) const
            {
                return this->M_tensorLagrangeMultiplier->evalijq( *this,i,j,q );
            }

        template<typename... TheArgsType>
        void updateForUse( const TheArgsType&... theInitArgs )
            {
                std::set<std::string> propUsed = { "displacement" };
                if constexpr ( useDispPresForm )
                {
                    propUsed.insert( "lagange_multiplier" );
                    if ( expr_type::specific_expr_type::value == ExprApplySolidMecFirstPiolaKirchhoff::EVAL ||
                         expr_type::specific_expr_type::value == ExprApplySolidMecFirstPiolaKirchhoff::JACOBIAN_TRIAL_DISP )
                    {
                        propUsed.insert( "Lame-second-parameter" );
                        propUsed.insert( "pressure");
                    }
                }
                else
                {
                    propUsed.insert( "Lame-second-parameter" );
                    M_tensorVolumicPart = tensor_volumic_part_type::New( this->expr().physicSolidData().decouplingEnergyVolumicLaw(),this->expr(),theInitArgs... );
                    auto propToAdd = M_tensorVolumicPart->propertiesUsed();
                    propUsed.insert( propToAdd.begin(), propToAdd.end() );
                }
                this->initSubTensor(propUsed,theInitArgs...);
            }

    private :
        typename super_type::array_value_type M_locEvalPrecomputePowDetF,M_locEvalPrecomputeDetF;
        std::shared_ptr<tensor_volumic_part_type> M_tensorVolumicPart;
    };


    /**
     * -------------------------------------------------------------------------------------------------  *
     * -------------------------------------------------------------------------------------------------  *
     * NeoHookeanCompressible :
     *   S = \mu*(Id-C^{-1}) + \kappa*ln(J)*C^{-1}
     * -------------------------------------------------------------------------------------------------  *
     * -------------------------------------------------------------------------------------------------  *
     */
    template<typename Geo_t, typename Basis_i_t, typename Basis_j_t,typename ExprType>
    struct tensorSolidMecFirstPiolaKirchhoffNeoHookeanCompressibleMolecularTheoryAndSimo1985 : public tensorFirstPiolaKirchhoffBase<Geo_t,Basis_i_t,Basis_j_t,ExprType>
    {
        typedef tensorFirstPiolaKirchhoffBase<Geo_t,Basis_i_t,Basis_j_t,ExprType> super_type;
    public :
        typedef ExprType expr_type;
        typedef typename super_type::geoelement_type geoelement_type;
        typedef typename super_type::gmc_type gmc_type;
        typedef typename super_type::gm_type gm_type;
        typedef typename super_type::value_type value_type;

        using ret_type = typename super_type::ret_type;

        tensorSolidMecFirstPiolaKirchhoffNeoHookeanCompressibleMolecularTheoryAndSimo1985( expr_type const& expr,Geo_t const& geom, Basis_i_t const& fev, Basis_j_t const& feu )
            :
            super_type( expr,geom,fev,feu ),
            M_locEvalPrecomputeLogDetF( boost::extents[ this->gmc()->xRefs().size2()] ),
            M_locEvalPrecomputePowDetF( boost::extents[ this->gmc()->xRefs().size2()] ),
            M_locEvalPrecomputeDetF( boost::extents[ this->gmc()->xRefs().size2()] )
            {
                this->updateForUse( geom, fev, feu );
            }
        tensorSolidMecFirstPiolaKirchhoffNeoHookeanCompressibleMolecularTheoryAndSimo1985( expr_type const& expr, Geo_t const& geom, Basis_i_t const& fev )
            :
            super_type( expr,geom,fev ),
            M_locEvalPrecomputeLogDetF( boost::extents[ this->gmc()->xRefs().size2()] ),
            M_locEvalPrecomputePowDetF( boost::extents[ this->gmc()->xRefs().size2()] ),
            M_locEvalPrecomputeDetF( boost::extents[ this->gmc()->xRefs().size2()] )
            {
                this->updateForUse( geom, fev );
            }
        tensorSolidMecFirstPiolaKirchhoffNeoHookeanCompressibleMolecularTheoryAndSimo1985( expr_type const& expr,Geo_t const& geom )
            :
            super_type( expr,geom ),
            M_locEvalPrecomputeLogDetF( boost::extents[ this->gmc()->xRefs().size2()] ),
            M_locEvalPrecomputePowDetF( boost::extents[ this->gmc()->xRefs().size2()] ),
            M_locEvalPrecomputeDetF( boost::extents[ this->gmc()->xRefs().size2()] )
            {
                this->updateForUse( geom );
            }

        void update( Geo_t const& geom ) override
        {
            super_type::update( geom );
            updateImpl( geom );
        }
        void update( Geo_t const& geom, uint16_type face ) override
        {
            super_type::update( geom,face );
            updateImpl( geom );
        }
        void updateImpl( Geo_t const& geom )
        {
            //std::fill( this->locRes().data(), this->locRes().data()+this->locRes().num_elements(), super_type::matrix_shape_type/*loc_res_type*/::Zero() );
            this->locRes().resize( this->gmc()->nPoints(), super_type::matrix_shape_type/*loc_res_type*/::Zero() );
            updateImpl( mpl::int_<expr_type::nRealDim>() );
        }
        void updateImpl( mpl::int_<2> /**/ )
        {
            double timeSteppingScaling = this->expr().timeSteppingScaling();
            for ( uint16_type q = 0; q < this->gmc()->nPoints(); ++q )
            {
                const value_type bulkModulus = this->M_localAssemblyBulkModulus[q];
                const value_type coefflame2 = this->M_localAssemblyLameSecond[q];

                auto const& gradDisplacementEval = this->locGradDisplacement(q);
                const value_type Fv11 = 1.+gradDisplacementEval(0,0), Fv12 = gradDisplacementEval(0,1);
                const value_type Fv21 = gradDisplacementEval(1,0), Fv22 = 1.+gradDisplacementEval(1,1);
                const value_type detFv = Fv11*Fv22-Fv12*Fv21;
                //const value_type traceCv = std::pow(Fv11,2) + std::pow(Fv21,2) + std::pow(Fv12,2) + std::pow(Fv22,2);

                if constexpr ( expr_type::specific_expr_type::value == ExprApplySolidMecFirstPiolaKirchhoff::EVAL )
                {
                    //auto InvFv = (cst(1.)/detFv)*mat<2,2>( Fv22,-Fv12,-Fv21,Fv11);
                    const value_type scaleUsedWithInvFv = 1./detFv;
                    const value_type InvFv11 =  scaleUsedWithInvFv*Fv22, InvFv12 = -scaleUsedWithInvFv*Fv12;
                    const value_type InvFv21 = -scaleUsedWithInvFv*Fv21, InvFv22 = scaleUsedWithInvFv*Fv11;

                    /*const value_type factorWithDetFv = coefflame2*std::pow(detFv,-2./3.);
                      const value_type factorWithTraceCv = (1./3.)*traceCv;
                      const value_type factorOther = factorWithDetFv*factorWithTraceCv*scaleUsedWithInvFv;*/

                    //auto FSv_neohookean = idv(CoeffLame2)*(Fv-trans(InvFv)) + idv(CoeffLame1)*vf::log(/*vf::abs*/(detFv))*trans(InvFv);
                    const value_type factorWithLogDetFv = bulkModulus*math::log(detFv);
                    this->locRes(q)(0,0) = coefflame2*(Fv11-InvFv11) + factorWithLogDetFv*InvFv11;
                    this->locRes(q)(0,1) = coefflame2*(Fv12-InvFv21) + factorWithLogDetFv*InvFv21;
                    this->locRes(q)(1,0) = coefflame2*(Fv21-InvFv12) + factorWithLogDetFv*InvFv12;
                    this->locRes(q)(1,1) = coefflame2*(Fv22-InvFv22) + factorWithLogDetFv*InvFv22;
                    this->locRes(q) *= timeSteppingScaling;
                }
                else
                {
                    //const value_type precomputeLog = M_locEvalPrecomputeLog[q](0,0); // idv(CoeffLame1)*vf::log(/*vf::abs*/(detFv)) -idv(CoeffLame2)
                    //const value_type precomputePowDetF = M_locEvalPrecomputePowDetF[q](0,0);//idv(CoeffLame1)/pow(detFv,2)
                    M_locEvalPrecomputeLogDetF[q] = bulkModulus*math::log(detFv) - coefflame2;
                    M_locEvalPrecomputePowDetF[q] = /*coefflame1/*/math::pow(detFv,2);
                    M_locEvalPrecomputeDetF[q] = detFv;
                }
            }
        }

        void updateImpl( mpl::int_<3> /**/ )
        {
            double timeSteppingScaling = this->expr().timeSteppingScaling();
            for ( uint16_type q = 0; q < this->gmc()->nPoints(); ++q )
            {
                const value_type bulkModulus = this->M_localAssemblyBulkModulus[q];
                const value_type coefflame2 = this->M_localAssemblyLameSecond[q];

                auto const& gradDisplacementEval = this->locGradDisplacement(q);
                const value_type Fv11 = 1.+gradDisplacementEval(0,0), Fv12 =    gradDisplacementEval(0,1), Fv13 =    gradDisplacementEval(0,2);
                const value_type Fv21 =    gradDisplacementEval(1,0), Fv22 = 1.+gradDisplacementEval(1,1), Fv23 =    gradDisplacementEval(1,2);
                const value_type Fv31 =    gradDisplacementEval(2,0), Fv32 =    gradDisplacementEval(2,1), Fv33 = 1.+gradDisplacementEval(2,2);

                const value_type detFv = Fv11*(Fv22*Fv33-Fv23*Fv32) - Fv21*(Fv12*Fv33-Fv13*Fv32) + Fv31*(Fv12*Fv23 - Fv13*Fv22);
                //const value_type traceCv = math::pow(Fv11,2) + math::pow(Fv21,2) + math::pow(Fv31,2) + math::pow(Fv12,2) +
                //    math::pow(Fv22,2) + math::pow(Fv32,2) + math::pow(Fv13,2) + math::pow(Fv23,2) + math::pow(Fv33,2);

                if constexpr ( expr_type::specific_expr_type::value == ExprApplySolidMecFirstPiolaKirchhoff::EVAL )
                {
                    // InvFv
                    /*auto InvFv = (cst(1.)/detFv)*mat<3,3>( Fv22*Fv33-Fv23*Fv32 , Fv13*Fv32-Fv12*Fv33 , Fv12*Fv23-Fv13*Fv22,
                      Fv23*Fv31-Fv21*Fv33 , Fv11*Fv33-Fv13*Fv31 , Fv13*Fv21-Fv11*Fv23,
                      Fv21*Fv32-Fv22*Fv31 , Fv12*Fv31-Fv11*Fv32 , Fv11*Fv22-Fv12*Fv21 );*/
                    const value_type scaleUsedWithInvFv = 1./detFv;
                    const value_type InvFv11 = scaleUsedWithInvFv*(Fv22*Fv33-Fv23*Fv32);
                    const value_type InvFv12 = scaleUsedWithInvFv*(Fv13*Fv32-Fv12*Fv33);
                    const value_type InvFv13 = scaleUsedWithInvFv*(Fv12*Fv23-Fv13*Fv22);
                    const value_type InvFv21 = scaleUsedWithInvFv*(Fv23*Fv31-Fv21*Fv33);
                    const value_type InvFv22 = scaleUsedWithInvFv*(Fv11*Fv33-Fv13*Fv31);
                    const value_type InvFv23 = scaleUsedWithInvFv*(Fv13*Fv21-Fv11*Fv23);
                    const value_type InvFv31 = scaleUsedWithInvFv*(Fv21*Fv32-Fv22*Fv31);
                    const value_type InvFv32 = scaleUsedWithInvFv*(Fv12*Fv31-Fv11*Fv32);
                    const value_type InvFv33 = scaleUsedWithInvFv*(Fv11*Fv22-Fv12*Fv21);

                    //auto FSv_neohookean = idv(CoeffLame2)*(Fv-trans(InvFv)) + idv(CoeffLame1)*vf::log(/*vf::abs*/(detFv))*trans(InvFv);
                    const value_type factorWithLogDetFv = bulkModulus*math::log(detFv);
                    this->locRes(q)(0,0) = coefflame2*(Fv11-InvFv11) + factorWithLogDetFv*InvFv11;
                    this->locRes(q)(0,1) = coefflame2*(Fv12-InvFv21) + factorWithLogDetFv*InvFv21;
                    this->locRes(q)(0,2) = coefflame2*(Fv13-InvFv31) + factorWithLogDetFv*InvFv31;
                    this->locRes(q)(1,0) = coefflame2*(Fv21-InvFv12) + factorWithLogDetFv*InvFv12;
                    this->locRes(q)(1,1) = coefflame2*(Fv22-InvFv22) + factorWithLogDetFv*InvFv22;
                    this->locRes(q)(1,2) = coefflame2*(Fv23-InvFv32) + factorWithLogDetFv*InvFv32;
                    this->locRes(q)(2,0) = coefflame2*(Fv31-InvFv13) + factorWithLogDetFv*InvFv13;
                    this->locRes(q)(2,1) = coefflame2*(Fv32-InvFv23) + factorWithLogDetFv*InvFv23;
                    this->locRes(q)(2,2) = coefflame2*(Fv33-InvFv33) + factorWithLogDetFv*InvFv33;
                }
                else
                {
                    M_locEvalPrecomputeLogDetF[q] = bulkModulus*math::log(detFv) - coefflame2;
                    M_locEvalPrecomputePowDetF[q] = /*coefflame1/*/math::pow(detFv,2);
                    M_locEvalPrecomputeDetF[q] = detFv;
                }

            }
        }
        // using super_type::evalijq; // fix clang warning

        ret_type
        evalijq( uint16_type i, uint16_type j, uint16_type q ) const override
        {
            return evalijq( i,j,q, mpl::int_<expr_type::specific_expr_type::value>() );
        }

    private :
        ret_type
        evalijq( uint16_type i, uint16_type j, uint16_type q, mpl::int_<ExprApplySolidMecFirstPiolaKirchhoff::EVAL> /**/ ) const
        {
            CHECK( false ) << "not allow";
            return ret_type(this->locMatrixShape().data());
        }

        ret_type
        evalijq( uint16_type i, uint16_type j, uint16_type q, mpl::int_<ExprApplySolidMecFirstPiolaKirchhoff::JACOBIAN_TRIAL_DISP> /**/ ) const
        {
            return evalijq( i,j,q, mpl::int_<ExprApplySolidMecFirstPiolaKirchhoff::JACOBIAN_TRIAL_DISP>(),mpl::int_<expr_type::nRealDim>() );
        }
        ret_type
        evalijq( uint16_type i, uint16_type j, uint16_type q, mpl::int_<ExprApplySolidMecFirstPiolaKirchhoff::JACOBIAN_TRIAL_DISP> /**/, mpl::int_<2> /**/ ) const
        {
            const value_type bulkModulus = this->M_localAssemblyBulkModulus[q];
            const value_type coefflame2 = this->M_localAssemblyLameSecond[q];

            auto const& gradDisplacementEval = this->locGradDisplacement(q);
            const value_type Fv11 = 1.+gradDisplacementEval(0,0), Fv12 = gradDisplacementEval(0,1);
            const value_type Fv21 = gradDisplacementEval(1,0), Fv22 = 1.+gradDisplacementEval(1,1);

            auto const& gradTrial = this->fecTrial()->grad( j, q );
            const value_type dF11/*du1tdx*/ = gradTrial(0,0,0), dF12/*du1tdy*/ = gradTrial(0,1,0);
            const value_type dF21/*du2tdx*/ = gradTrial(1,0,0), dF22/*du2tdy*/ = gradTrial(1,1,0);

            const value_type ddetF = dF11*Fv22 + Fv11*dF22 - Fv21*dF12 - dF21*Fv12;
            //const value_type ddetF = dF22+dF11+dF11*Fv22+Fv11*dF22 -Fv21*dF12 - dF21*Fv12;
            //const value_type dtraceC = 2*Fv11*dF11 + 2*Fv21*dF21 + 2*Fv12*dF12 + 2*Fv22*dF22;

            //auto InvFtMat = mat<2,2>( dF22,-dF21,-dF12,dF11);//already transpose
            //auto InvFvMat = mat<2,2>( Fv22,-Fv12,-Fv21,Fv11);
            //auto dFmt = -(ddetF/pow(detFv,2))*trans(InvFvMat)+(cst(1.)/detFv)*/*trans*/(InvFtMat);
            const value_type detFv = M_locEvalPrecomputeDetF[q];
            const value_type scaleUsedWithInvFv = 1./detFv;
            const value_type precomputePowDetF = M_locEvalPrecomputePowDetF[q];//pow(detFv,2)  //idv(CoeffLame1)/pow(detFv,2)

            const value_type factorOther = ddetF/precomputePowDetF;//math::pow(detFv,2);
            const value_type dFmt11 = -factorOther*Fv22 + scaleUsedWithInvFv*dF22;
            const value_type dFmt12 =  factorOther*Fv21 - scaleUsedWithInvFv*dF21;
            const value_type dFmt21 =  factorOther*Fv12 - scaleUsedWithInvFv*dF12;
            const value_type dFmt22 = -factorOther*Fv11 + scaleUsedWithInvFv*dF11;


            //idv(CoeffLame2)*dF
            const value_type dFS_neohookean_a11 = coefflame2*dF11;
            const value_type dFS_neohookean_a12 = coefflame2*dF12;
            const value_type dFS_neohookean_a21 = coefflame2*dF21;
            const value_type dFS_neohookean_a22 = coefflame2*dF22;

            // _expr= val( idv(CoeffLame1)*vf::log(/*vf::abs*/(detFv)) -idv(CoeffLame2) )*trace(dFmt*trans(grad(v)) ),
            const value_type precomputeLog = M_locEvalPrecomputeLogDetF[q]; // idv(CoeffLame1)*vf::log(/*vf::abs*/(detFv)) -idv(CoeffLame2)
            const value_type dFS_neohookean_b11 = precomputeLog*dFmt11;
            const value_type dFS_neohookean_b12 = precomputeLog*dFmt12;
            const value_type dFS_neohookean_b21 = precomputeLog*dFmt21;
            const value_type dFS_neohookean_b22 = precomputeLog*dFmt22;

            //_expr= ddetF*trace( val(idv(CoeffLame1)*trans(InvFvMat)/pow(detFv,2) )*trans(grad(v)) ),
            const value_type factorOther2 = ddetF*bulkModulus/precomputePowDetF;//        ddetF/precomputePowDetF;//math::pow(detFv,2);
            const value_type dFS_neohookean_c11 =  factorOther2*Fv22;
            const value_type dFS_neohookean_c12 = -factorOther2*Fv21;
            const value_type dFS_neohookean_c21 = -factorOther2*Fv12;
            const value_type dFS_neohookean_c22 =  factorOther2*Fv11;


            /*const value_type traceCv = M_locEvalPrecomputeTraceC[q](0,0);
            const value_type precomputeInvDetF = M_locEvalPrecomputeInvDetF[q](0,0);
            //const value_type precomputeInvDetF = 1./detFv;
            const value_type precomputeDetF1 = M_locEvalPrecomputeDetF1[q](0,0);*/

            this->locMatrixShape()(0,0) = dFS_neohookean_a11+dFS_neohookean_b11+dFS_neohookean_c11;
            this->locMatrixShape()(1,0) = dFS_neohookean_a21+dFS_neohookean_b21+dFS_neohookean_c21;
            this->locMatrixShape()(0,1) = dFS_neohookean_a12+dFS_neohookean_b12+dFS_neohookean_c12;
            this->locMatrixShape()(1,1) = dFS_neohookean_a22+dFS_neohookean_b22+dFS_neohookean_c22;

            return ret_type(this->locMatrixShape().data());
        }
        ret_type
        evalijq( uint16_type i, uint16_type j, uint16_type q, mpl::int_<ExprApplySolidMecFirstPiolaKirchhoff::JACOBIAN_TRIAL_DISP> /**/, mpl::int_<3> /**/ ) const
        {
            const value_type bulkModulus = this->M_localAssemblyBulkModulus[q];
            const value_type coefflame2 = this->M_localAssemblyLameSecond[q];

            auto const& gradDisplacementEval = this->locGradDisplacement(q);
            const value_type Fv11 = 1.+gradDisplacementEval(0,0), Fv12 =    gradDisplacementEval(0,1), Fv13 =    gradDisplacementEval(0,2);
            const value_type Fv21 =    gradDisplacementEval(1,0), Fv22 = 1.+gradDisplacementEval(1,1), Fv23 =    gradDisplacementEval(1,2);
            const value_type Fv31 =    gradDisplacementEval(2,0), Fv32 =    gradDisplacementEval(2,1), Fv33 = 1.+gradDisplacementEval(2,2);

            const value_type Fav11 = gradDisplacementEval(0,0), Fav12 = Fv12, Fav13 = Fv13;
            const value_type Fav21 = Fv21, Fav22 = gradDisplacementEval(1,1), Fav23 = Fv23;
            const value_type Fav31 = Fv31, Fav32 = Fv32, Fav33 = gradDisplacementEval(2,2);

            auto const& gradTrial = this->fecTrial()->grad( j, q );
            const value_type dF11/*du1tdx*/ = gradTrial(0,0,0), dF12/*du1tdy*/ = gradTrial(0,1,0), dF13/*du1tdz*/ = gradTrial(0,2,0);
            const value_type dF21/*du2tdx*/ = gradTrial(1,0,0), dF22/*du2tdy*/ = gradTrial(1,1,0), dF23/*du2tdz*/ = gradTrial(1,2,0);
            const value_type dF31/*du3tdx*/ = gradTrial(2,0,0), dF32/*du3tdy*/ = gradTrial(2,1,0), dF33/*du3tdz*/ = gradTrial(2,2,0);

            const value_type ddetF = dF11 + dF22 + dF33 + (dF22*Fav33 + Fav22*dF33) + (dF11*Fav22 + Fav11*dF22) + (dF11*Fav33 + Fav11*dF33)
            + dF11*Fav22*Fav33 + Fav11*dF22*Fav33 + Fav11*Fav22*dF33
            + dF12*Fav23*Fav31 + Fav12*dF23*Fav31 + Fav12*Fav23*dF31
            + dF13*Fav21*Fav32 + Fav13*dF21*Fav32 + Fav13*Fav21*dF32
            - (dF23*Fav32 + Fav23*dF32) - (dF12*Fav21 + Fav12*dF21) - (dF13*Fav31 + Fav13*dF31)
            - (dF11*Fav23*Fav32 + Fav11*dF23*Fav32 + Fav11*Fav23*dF32)
            - (dF12*Fav21*Fav33 + Fav12*dF21*Fav33 + Fav12*Fav21*dF33)
            - (dF13*Fav22*Fav31 + Fav13*dF22*Fav31 + Fav13*Fav22*dF31) ;


            /*auto InvFvMat = mat<3,3>( Fv22*Fv33-Fv23*Fv32 , Fv13*Fv32-Fv12*Fv33 , Fv12*Fv23-Fv13*Fv22,
              Fv23*Fv31-Fv21*Fv33 , Fv11*Fv33-Fv13*Fv31 , Fv13*Fv21-Fv11*Fv23,
              Fv21*Fv32-Fv22*Fv31 , Fv12*Fv31-Fv11*Fv32 , Fv11*Fv22-Fv12*Fv21 );*/
            // InvFv ( without 1/detF )
            const value_type InvFv11 = Fv22*Fv33-Fv23*Fv32;
            const value_type InvFv12 = Fv13*Fv32-Fv12*Fv33;
            const value_type InvFv13 = Fv12*Fv23-Fv13*Fv22;
            const value_type InvFv21 = Fv23*Fv31-Fv21*Fv33;
            const value_type InvFv22 = Fv11*Fv33-Fv13*Fv31;
            const value_type InvFv23 = Fv13*Fv21-Fv11*Fv23;
            const value_type InvFv31 = Fv21*Fv32-Fv22*Fv31;
            const value_type InvFv32 = Fv12*Fv31-Fv11*Fv32;
            const value_type InvFv33 = Fv11*Fv22-Fv12*Fv21;

            const value_type FmtNL11 = dF22+dF33+dF22*Fav33+Fav22*dF33-dF23*Fav32-Fav23*dF32;
            const value_type FmtNL12 = dF23*Fav31+Fav23*dF31-dF21-dF21*Fav33-Fav21*dF33;
            const value_type FmtNL13 = dF21*Fav32+Fav21*dF32-dF31-dF31*Fav22-Fav31*dF22;
            const value_type FmtNL21 = dF13*Fav32+Fav13*dF32-dF12-dF12*Fav33-Fav12*dF33;
            const value_type FmtNL22 = dF11+dF33+dF11*Fav33+Fav11*dF33-dF13*Fav31-Fav13*dF31;
            const value_type FmtNL23 = dF12*Fav31+Fav12*dF31-dF32-dF32*Fav11-Fav32*dF11;
            const value_type FmtNL31 = dF12*Fav23+Fav12*dF23-dF13-dF13*Fav22-Fav13*dF22;
            const value_type FmtNL32 = dF13*Fav21+Fav13*dF21-dF23-dF23*Fav11-Fav23*dF11;
            const value_type FmtNL33 = dF11+dF22+dF11*Fav22+Fav11*dF22-dF12*Fav21-Fav12*dF21;

            /*auto InvFtMat =  mat<3,3>(FmtNL11,FmtNL12,FmtNL13,
                                  FmtNL21,FmtNL22,FmtNL23,
                                  FmtNL31,FmtNL32,FmtNL33 );*/
            //auto dFmt = -(ddetF/pow(detFv,2))*trans(InvFvMat)+(cst(1.)/detFv)*/*trans*/(InvFtMat) ;

            const value_type detFv = M_locEvalPrecomputeDetF[q];
            const value_type scaleUsedWithInvFv = 1./detFv;
            const value_type precomputePowDetF = M_locEvalPrecomputePowDetF[q];//pow(detFv,2)  //idv(CoeffLame1)/pow(detFv,2)

            const value_type factorOther = ddetF/precomputePowDetF;//math::pow(detFv,2);
            const value_type dFmt11 = -factorOther*InvFv11 + scaleUsedWithInvFv*FmtNL11;
            const value_type dFmt12 = -factorOther*InvFv21 + scaleUsedWithInvFv*FmtNL12;
            const value_type dFmt13 = -factorOther*InvFv31 + scaleUsedWithInvFv*FmtNL13;
            const value_type dFmt21 = -factorOther*InvFv12 + scaleUsedWithInvFv*FmtNL21;
            const value_type dFmt22 = -factorOther*InvFv22 + scaleUsedWithInvFv*FmtNL22;
            const value_type dFmt23 = -factorOther*InvFv32 + scaleUsedWithInvFv*FmtNL23;
            const value_type dFmt31 = -factorOther*InvFv13 + scaleUsedWithInvFv*FmtNL31;
            const value_type dFmt32 = -factorOther*InvFv23 + scaleUsedWithInvFv*FmtNL32;
            const value_type dFmt33 = -factorOther*InvFv33 + scaleUsedWithInvFv*FmtNL33;

            //--------------------------------//
            //idv(CoeffLame2)*dF
            const value_type dFS_neohookean_a11 = coefflame2*dF11;
            const value_type dFS_neohookean_a12 = coefflame2*dF12;
            const value_type dFS_neohookean_a13 = coefflame2*dF13;
            const value_type dFS_neohookean_a21 = coefflame2*dF21;
            const value_type dFS_neohookean_a22 = coefflame2*dF22;
            const value_type dFS_neohookean_a23 = coefflame2*dF23;
            const value_type dFS_neohookean_a31 = coefflame2*dF31;
            const value_type dFS_neohookean_a32 = coefflame2*dF32;
            const value_type dFS_neohookean_a33 = coefflame2*dF33;

            // _expr= val( idv(CoeffLame1)*vf::log(/*vf::abs*/(detFv)) -idv(CoeffLame2) )*trace(dFmt*trans(grad(v)) ),
            const value_type precomputeLog = M_locEvalPrecomputeLogDetF[q]; // idv(CoeffLame1)*vf::log(/*vf::abs*/(detFv)) -idv(CoeffLame2)
            const value_type dFS_neohookean_b11 = precomputeLog*dFmt11;
            const value_type dFS_neohookean_b12 = precomputeLog*dFmt12;
            const value_type dFS_neohookean_b13 = precomputeLog*dFmt13;
            const value_type dFS_neohookean_b21 = precomputeLog*dFmt21;
            const value_type dFS_neohookean_b22 = precomputeLog*dFmt22;
            const value_type dFS_neohookean_b23 = precomputeLog*dFmt23;
            const value_type dFS_neohookean_b31 = precomputeLog*dFmt31;
            const value_type dFS_neohookean_b32 = precomputeLog*dFmt32;
            const value_type dFS_neohookean_b33 = precomputeLog*dFmt33;

            //_expr= ddetF*trace( val(idv(CoeffLame1)*trans(InvFvMat)/pow(detFv,2) )*trans(grad(v)) ),
            const value_type factorOther2 = ddetF*bulkModulus/precomputePowDetF;//        ddetF/precomputePowDetF;//math::pow(detFv,2);
            const value_type dFS_neohookean_c11 = factorOther2*InvFv11;
            const value_type dFS_neohookean_c12 = factorOther2*InvFv21;
            const value_type dFS_neohookean_c13 = factorOther2*InvFv31;
            const value_type dFS_neohookean_c21 = factorOther2*InvFv12;
            const value_type dFS_neohookean_c22 = factorOther2*InvFv22;
            const value_type dFS_neohookean_c23 = factorOther2*InvFv32;
            const value_type dFS_neohookean_c31 = factorOther2*InvFv13;
            const value_type dFS_neohookean_c32 = factorOther2*InvFv23;
            const value_type dFS_neohookean_c33 = factorOther2*InvFv33;


            this->locMatrixShape()(0,0) = dFS_neohookean_a11+dFS_neohookean_b11+dFS_neohookean_c11;
            this->locMatrixShape()(1,0) = dFS_neohookean_a21+dFS_neohookean_b21+dFS_neohookean_c21;
            this->locMatrixShape()(2,0) = dFS_neohookean_a31+dFS_neohookean_b31+dFS_neohookean_c31;
            this->locMatrixShape()(0,1) = dFS_neohookean_a12+dFS_neohookean_b12+dFS_neohookean_c12;
            this->locMatrixShape()(1,1) = dFS_neohookean_a22+dFS_neohookean_b22+dFS_neohookean_c22;
            this->locMatrixShape()(2,1) = dFS_neohookean_a32+dFS_neohookean_b32+dFS_neohookean_c32;
            this->locMatrixShape()(0,2) = dFS_neohookean_a13+dFS_neohookean_b13+dFS_neohookean_c13;
            this->locMatrixShape()(1,2) = dFS_neohookean_a23+dFS_neohookean_b23+dFS_neohookean_c23;
            this->locMatrixShape()(2,2) = dFS_neohookean_a33+dFS_neohookean_b33+dFS_neohookean_c33;

            return ret_type(this->locMatrixShape().data());
        }

        ret_type
        evalijq( uint16_type i, uint16_type j, uint16_type q, mpl::int_<ExprApplySolidMecFirstPiolaKirchhoff::JACOBIAN_TRIAL_PRES> /**/ ) const
            {
                CHECK( false ) << "TODO";
                return ret_type(this->locMatrixShape().data());
            }

        template<typename... TheArgsType>
        void updateForUse( const TheArgsType&... theInitArgs )
            {
                std::set<std::string> propUsed = { "displacement", "Lame-second-parameter", "bulk-modulus" };
                this->initSubTensor(propUsed,theInitArgs...);
            }
    private :
        typename super_type::array_value_type M_locEvalPrecomputeLogDetF,M_locEvalPrecomputePowDetF,M_locEvalPrecomputeDetF;
    };




               


           
             
               
             
           


               



    /**
     * -------------------------------------------------------------------------------------------------  *
     * -------------------------------------------------------------------------------------------------  *
     * MooneyRivlinCompressible :
     *   S = TODO
     * -------------------------------------------------------------------------------------------------  *
     * -------------------------------------------------------------------------------------------------  *
     */

#endif
               


           
             
               
             
           


               


/**
 * \class SolidMecStressTensorImpl
 * \brief first piola Kirchhoff expression
 *
 * @author Vincent Chabannes
 * @see
 */
template<typename ExprType>
class SolidMecStressTensorImpl
{
    using expr_type = ExprType;
public:

    using this_type = SolidMecStressTensorImpl<ExprType>;

    static constexpr size_type context = ExprType::context;

    using value_type = typename ExprType::value_type;

    using expr_fpk_base_type = ExprType;

    static constexpr bool is_terminal = ExprType::is_terminal;



    template<typename Func>
    struct HasTestFunction
    {
        static const bool result = ExprType::template HasTestFunction<Func>;
    };

    template<typename Func>
    struct HasTrialFunction
    {
        static const bool result = ExprType::template HasTrialFunction<Func>;
    };
    using test_basis = typename ExprType::test_basis;
    using trial_basis = typename ExprType::trial_basis;

    SolidMecStressTensorImpl( std::shared_ptr<expr_type> expr )
        :
        M_exprFirstPiolaKirchhoffBase( expr )
        {}
    SolidMecStressTensorImpl( SolidMecStressTensorImpl const& ) = default;
    SolidMecStressTensorImpl( SolidMecStressTensorImpl && ) = default;

    size_type dynamicContext() const
        {
            return M_exprFirstPiolaKirchhoffBase->dynamicContext();
        }

    //! polynomial order
    uint16_type polynomialOrder() const
        {
#if 0
            if ( expr_type::QuadOrder>=0 )
                return QuadOrder;
#endif
            return M_exprFirstPiolaKirchhoffBase->polynomialOrder();
        }

    //! expression is polynomial?
    bool isPolynomial() const { return true; }

    template <typename TheSymbolExprType>
    bool hasSymbolDependency( std::string const& symb, TheSymbolExprType const& se ) const
        {
            return M_exprFirstPiolaKirchhoffBase->hasSymbolDependency( symb,se );
        }

    template <typename TheSymbolExprType>
    void dependentSymbols( std::string const& symb, std::map<std::string,std::set<std::string>> & res, TheSymbolExprType const& se ) const
        {
            CHECK( false )  << "TODO";
        }

    template <typename OtherSymbolsExprType>
    auto applySymbolsExpr( OtherSymbolsExprType const& se ) const
        {
            using new_expr_type = std::decay_t<decltype( unwrap_ptr(M_exprFirstPiolaKirchhoffBase->applySymbolsExpr( se ) ) )>;
            return SolidMecStressTensorImpl<new_expr_type>( M_exprFirstPiolaKirchhoffBase->applySymbolsExpr( se ) );
        }

    template <int diffOrder, typename TheSymbolExprType>
    auto diff( std::string const& diffVariable, WorldComm const& world, std::string const& dirLibExpr,
               TheSymbolExprType const& se ) const
        {
            CHECK( false ) << "not implemented";
            return *this;
        }



    std::shared_ptr<expr_fpk_base_type> exprFirstPiolaKirchhoffBase() const { return M_exprFirstPiolaKirchhoffBase; }
    expr_fpk_base_type const& expr() const { return *M_exprFirstPiolaKirchhoffBase; }

    template<typename Geo_t, typename Basis_i_t, typename Basis_j_t>
    struct tensor
    {
        using tensor_expr_type = typename expr_type::template tensor<Geo_t,Basis_i_t,Basis_j_t>;
        using value_type = typename tensor_expr_type::value_type;
        using shape = typename tensor_expr_type::shape;

        struct is_zero { static const bool value = false; };

        using ret_type = typename tensor_expr_type::ret_type;

#if 0
        tensor( this_type const& expr,
                Geo_t const& geom, Basis_i_t const& fev, Basis_j_t const& feu )
            :
            M_expr( expr )
            {
                this->initTensor( expr,geom,fev,feu );
            }

        tensor( this_type const& expr,
                Geo_t const& geom, Basis_i_t const& fev )
            :
            M_expr( expr )
            {
                this->initTensor( expr,geom,fev );
            }
        tensor( this_type const& expr, Geo_t const& geom )
            :
            M_expr( expr )
            {
                this->initTensor( expr,geom );
            }
#endif
        template<typename ... TheArgsType>
        tensor( this_type const& expr, Geo_t const& geom, const TheArgsType&... theInitArgs )
            :
            M_expr( expr )
            {
                this->initTensor( expr,geom, theInitArgs... );
            }

        template<typename TheExprExpandedType,typename TupleTensorSymbolsExprType, typename... TheArgsType>
        tensor( std::true_type /**/, TheExprExpandedType const& exprExpanded, TupleTensorSymbolsExprType & ttse,
                this_type const& expr, Geo_t const& geom, const TheArgsType&... theInitArgs )
            :
            M_expr( expr )
            {
                this->initTensor( std::true_type{}, exprExpanded, ttse, expr, geom, theInitArgs... );
            }

        tensor( tensor const& t ) = default;

        void update( Geo_t const& geom, Basis_i_t const& /*fev*/, Basis_j_t const& /*feu*/ )
        {
            update(geom);
        }
        void update( Geo_t const& geom, Basis_i_t const& /*fev*/ )
        {
            update(geom);
        }
        void update( Geo_t const& geom )
        {
            M_tensorbase->update( geom );
        }

        template<typename TheExprExpandedType,typename TupleTensorSymbolsExprType, typename... TheArgsType>
        void update( std::true_type /**/, TheExprExpandedType const& exprExpanded, TupleTensorSymbolsExprType & ttse,
                     Geo_t const& geom, const TheArgsType&... theUpdateArgs )
            {
                M_tensorbase->update(  std::true_type{}, *(exprExpanded.exprFirstPiolaKirchhoffBase()), ttse, geom, theUpdateArgs... );
            }

        ret_type
        evalijq( uint16_type i, uint16_type j, uint16_type q ) const
        {
            return M_tensorbase->evalijq( i,j,q );
        }
        value_type
        evalijq( uint16_type i, uint16_type j, uint16_type c1, uint16_type c2, uint16_type q ) const
        {
            return M_tensorbase->evalijq( i,j,c1,c2,q );
        }

        value_type
        evaliq( uint16_type i, uint16_type c1, uint16_type c2, uint16_type q ) const
        {
            return M_tensorbase->evaliq( i,c1,c2,q );
        }
        ret_type
        evaliq( uint16_type i, uint16_type q ) const
        {
            return M_tensorbase->evaliq( i, q );
        }

        value_type
        evalq( uint16_type c1, uint16_type c2, uint16_type q ) const
        {
            return M_tensorbase->evalq( c1,c2,q );
        }
        ret_type
        evalq( uint16_type q ) const
        {
            return M_tensorbase->evalq( q );
        }
    private :
        template<typename... TheArgsType>
        void initTensor( this_type const& expr, const TheArgsType&... theInitArgs )
            {
                M_tensorbase = expr.expr().template evaluator<Geo_t, Basis_i_t, Basis_j_t>( theInitArgs... );
            }
        template<typename TheExprExpandedType,typename TupleTensorSymbolsExprType, typename... TheArgsType>
        void initTensor( std::true_type /**/,TheExprExpandedType const& exprExpanded, TupleTensorSymbolsExprType & ttse,
                         this_type const& expr, Geo_t const& geom, const TheArgsType&... theInitArgs )
            {
                M_tensorbase = expr.exprFirstPiolaKirchhoffBase()->template evaluator<Geo_t, Basis_i_t, Basis_j_t>( std::true_type{}, *(exprExpanded.exprFirstPiolaKirchhoffBase()), ttse, geom, theInitArgs... );
            }

#if 0
        template<typename... TheArgsType>
        void initTensor( this_type const& expr, const TheArgsType&... theInitArgs )
            {
                if ( expr.physicSolidData().equation() == "Elasticity" )
                {
                    if ( expr.physicSolidData().useDisplacementPressureFormulation() )
                        M_tensorbase.reset( new tensorSolidMecFirstPiolaKirchhoffLinearElasticity<Geo_t, Basis_i_t, Basis_j_t,this_type,true>(expr,theInitArgs...) );
                    else
                        M_tensorbase.reset( new tensorSolidMecFirstPiolaKirchhoffLinearElasticity<Geo_t, Basis_i_t, Basis_j_t,this_type,false>(expr,theInitArgs...) );
                }
                else if ( expr.physicSolidData().equation() == "Hyper-Elasticity" )
                {
                    if ( expr.physicSolidData().materialModel() == "StVenantKirchhoff" )
                    {
                        if ( expr.physicSolidData().useDisplacementPressureFormulation() )
                            M_tensorbase.reset( new tensorSolidMecFirstPiolaKirchhoffStVenantKirchhoff<Geo_t, Basis_i_t, Basis_j_t,this_type,true>(expr,theInitArgs...) );
                        else
                            M_tensorbase.reset( new tensorSolidMecFirstPiolaKirchhoffStVenantKirchhoff<Geo_t, Basis_i_t, Basis_j_t,this_type,false>(expr,theInitArgs...) );
                    }
                    else if ( expr.physicSolidData().materialModel() == "NeoHookean" )
                    {
                        if ( expr.physicSolidData().useDisplacementPressureFormulation() )
                        {
                            if ( expr.physicSolidData().compressibleNeoHookeanVariantName() == "default" )
                                M_tensorbase.reset( new tensorSolidMecFirstPiolaKirchhoffNeoHookeanCompressible<Geo_t, Basis_i_t, Basis_j_t,this_type,true>(expr,theInitArgs...) );
                            else if ( expr.physicSolidData().compressibleNeoHookeanVariantName() == "molecular-theory" )
                                M_tensorbase.reset( new tensorSolidMecFirstPiolaKirchhoffNeoHookeanCompressibleMolecularTheory<Geo_t, Basis_i_t, Basis_j_t,this_type,true>(expr,theInitArgs...) );
                            else
                                CHECK( false ) << "invalid";
                        }
                        else
                        {
                            if ( expr.physicSolidData().compressibleNeoHookeanVariantName() == "default" )
                                M_tensorbase.reset( new tensorSolidMecFirstPiolaKirchhoffNeoHookeanCompressible<Geo_t, Basis_i_t, Basis_j_t,this_type,false>(expr,theInitArgs...) );
                            else if ( expr.physicSolidData().compressibleNeoHookeanVariantName() == "molecular-theory" )
                                M_tensorbase.reset( new tensorSolidMecFirstPiolaKirchhoffNeoHookeanCompressibleMolecularTheory<Geo_t, Basis_i_t, Basis_j_t,this_type,false>(expr,theInitArgs...) );
                            else if ( expr.physicSolidData().compressibleNeoHookeanVariantName() == "molecular-theory-simo1985" )
                                M_tensorbase.reset( new tensorSolidMecFirstPiolaKirchhoffNeoHookeanCompressibleMolecularTheoryAndSimo1985<Geo_t, Basis_i_t, Basis_j_t,this_type>(expr,theInitArgs...) );
                            else
                                CHECK( false ) << "invalid";
                        }
                    }
                    else
                        CHECK ( false ) << "invalid materialLaw : "<< expr.physicSolidData().materialModel();
                }

            }
        template<typename TheExprExpandedType,typename TupleTensorSymbolsExprType, typename... TheArgsType>
        void initTensor( std::true_type /**/, TheExprExpandedType const& exprExpanded, TupleTensorSymbolsExprType & ttse,
                         this_type const& expr, Geo_t const& geom, const TheArgsType&... theInitArgs )
            {
                CHECK( false ) << "TODO VINCENT";
            }
#endif

    private:
        this_type const& M_expr;
        std::shared_ptr<tensor_expr_type> M_tensorbase;
    };

private :
    std::shared_ptr<expr_fpk_base_type> M_exprFirstPiolaKirchhoffBase;
};

/**
 * \brief FirstPiolaKirchhoffTensor
 */
template<int QuadOrder=-1,typename ElementDisplacementType, typename ElementPressureType, typename ModelPhysicSolidType, typename MaterialPropertiesType,typename SymbolsExprType>
inline
auto
solidMecFirstPiolaKirchhoffTensor( ElementDisplacementType const& u, std::shared_ptr<ElementPressureType> const& p,
                                   std::shared_ptr<ModelPhysicSolidType> const& physicSolidData, MaterialPropertiesType const& matProperties,
                                   SymbolsExprType const& se, double timeSteppingScaling = 1.0 )
{
    using expr_evaluate_displacement_opertors_type = ExprEvaluateFieldOperators<ElementDisplacementType>;
    auto exprEvaluateDispOperators = std::make_shared<expr_evaluate_displacement_opertors_type>( u );

    using expr_evaluate_pressure_opertors_type = ExprEvaluateFieldOperators<std::shared_ptr<ElementPressureType>>;
    std::shared_ptr<expr_evaluate_pressure_opertors_type> exprEvaluatePressureOperators;
    if ( p && physicSolidData->useDisplacementPressureFormulation() )
        exprEvaluatePressureOperators = std::make_shared<expr_evaluate_pressure_opertors_type>( p );

    using fe_disp_type = std::nullptr_t;//typename unwrap_ptr_t<ElementDisplacementType>::functionspace_type::reference_element_type;
    using fe_pressure_type = std::nullptr_t;//typename unwrap_ptr_t<ElementPressureType>::functionspace_type::reference_element_type;
    typedef SolidMecFirstPiolaKirchhoffBase<expr_evaluate_displacement_opertors_type, fe_disp_type,
                                            expr_evaluate_pressure_opertors_type, fe_pressure_type,
                                            ModelPhysicSolidType,MaterialPropertiesType,SymbolsExprType,ExprApplyTypeSolidMecFirstPiolaKirchhoff::EVAL, QuadOrder > smstresstensor_base_t;

    using smstresstensor_t = SolidMecStressTensorImpl<smstresstensor_base_t>;
    return Expr< smstresstensor_t >( smstresstensor_t( smstresstensor_base_t::New( exprEvaluateDispOperators,exprEvaluatePressureOperators,physicSolidData,matProperties,se,timeSteppingScaling ) ) );
}

template<int QuadOrder=-1,typename ElementDisplacementType, typename ElementPressureType, typename ModelPhysicSolidType, typename MaterialPropertiesType, typename SymbolsExprType >
inline
auto
solidMecFirstPiolaKirchhoffTensorJacobianTrialDisplacement( ElementDisplacementType const& u, std::shared_ptr<ElementPressureType> const& p,
                                                            std::shared_ptr<ModelPhysicSolidType> const& physicSolidData, MaterialPropertiesType const& matProperties,
                                                            SymbolsExprType const& se  )
{
    using expr_evaluate_displacement_opertors_type = ExprEvaluateFieldOperators<ElementDisplacementType>;
    auto exprEvaluateDispOperators = std::make_shared<expr_evaluate_displacement_opertors_type>( u );
    using expr_evaluate_pressure_opertors_type = ExprEvaluateFieldOperators<std::shared_ptr<ElementPressureType>>;
    std::shared_ptr<expr_evaluate_pressure_opertors_type> exprEvaluatePressureOperators;
    if ( p && physicSolidData->useDisplacementPressureFormulation() )
        exprEvaluatePressureOperators = std::make_shared<expr_evaluate_pressure_opertors_type>( p );

    using fe_disp_type = typename unwrap_ptr_t<ElementDisplacementType>::functionspace_type::reference_element_type;
    using fe_pressure_type = std::nullptr_t;//typename unwrap_ptr_t<ElementPressureType>::functionspace_type::reference_element_type;

    typedef SolidMecFirstPiolaKirchhoffBase<expr_evaluate_displacement_opertors_type, fe_disp_type,
                                            expr_evaluate_pressure_opertors_type, fe_pressure_type,
                                            ModelPhysicSolidType,MaterialPropertiesType,SymbolsExprType,ExprApplyTypeSolidMecFirstPiolaKirchhoff::JACOBIAN_TRIAL_DISP, QuadOrder > smstresstensor_base_t;

    using smstresstensor_t = SolidMecStressTensorImpl<smstresstensor_base_t>;
    return Expr< smstresstensor_t >( smstresstensor_t( smstresstensor_base_t::New(exprEvaluateDispOperators,exprEvaluatePressureOperators,physicSolidData,matProperties,se) ) );
}

template<int QuadOrder=-1,typename ElementDisplacementType, typename ElementPressureType, typename ModelPhysicSolidType, typename MaterialPropertiesType, typename SymbolsExprType >
inline
auto
solidMecFirstPiolaKirchhoffTensorJacobianTrialPressure( ElementDisplacementType const& u, std::shared_ptr<ElementPressureType> const& p, std::shared_ptr<ModelPhysicSolidType> const& physicSolidData, MaterialPropertiesType const& matProperties, SymbolsExprType const& se )
{
    using expr_evaluate_displacement_opertors_type = ExprEvaluateFieldOperators<ElementDisplacementType>;
    auto exprEvaluateDispOperators = std::make_shared<expr_evaluate_displacement_opertors_type>( u );
    using expr_evaluate_pressure_opertors_type = ExprEvaluateFieldOperators<std::shared_ptr<ElementPressureType>>;
    std::shared_ptr<expr_evaluate_pressure_opertors_type> exprEvaluatePressureOperators;
    if ( p && physicSolidData->useDisplacementPressureFormulation() )
        exprEvaluatePressureOperators = std::make_shared<expr_evaluate_pressure_opertors_type>( p );

    using fe_disp_type = std::nullptr_t;
    using fe_pressure_type = typename unwrap_ptr_t<ElementPressureType>::functionspace_type::reference_element_type;

    typedef SolidMecFirstPiolaKirchhoffBase<expr_evaluate_displacement_opertors_type, fe_disp_type,
                                            expr_evaluate_pressure_opertors_type, fe_pressure_type,
                                            ModelPhysicSolidType,MaterialPropertiesType,SymbolsExprType,ExprApplyTypeSolidMecFirstPiolaKirchhoff::JACOBIAN_TRIAL_PRES, QuadOrder > smstresstensor_base_t;

    using smstresstensor_t = SolidMecStressTensorImpl<smstresstensor_base_t>;
    return Expr< smstresstensor_t >( smstresstensor_t( smstresstensor_base_t::New(exprEvaluateDispOperators,exprEvaluatePressureOperators,physicSolidData,matProperties,se) ) );

}


} // namespace FeelModels
} // namespace Feel
#endif /* __SOLIDMEC_FIRSTPIOLAKIRCHHOFF_H */
