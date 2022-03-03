/* -*- mode: c++; coding: utf-8; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; show-trailing-whitespace: t -*- vim:fenc=utf-8:ft=cpp:et:sw=4:ts=4:sts=4
 */

#ifndef FEELPP_MODELS_VF_EVALONFENTITIES_H
#define FEELPP_MODELS_VF_EVALONFENTITIES_H

#include <feel/feelvf/expr.hpp>


namespace Feel
{
namespace vf
{

template<typename GmcT>
fusion::map<fusion::pair<vf::detail::gmc<0>, std::shared_ptr<GmcT>>>
mapgmcFix( std::shared_ptr<GmcT> const& ctx )
{
    return { fusion::make_pair<vf::detail::gmc<0> >( ctx ) };
}




template <typename ExprType>
class EvalOnFaces
{
public :
    using this_type = EvalOnFaces<ExprType>;
    using expression_type = ExprType;

    enum class InternalFacesEvalType { Average=0,Max,Min,Sum,One_Side };

    static const size_type context = expression_type::context;
    static const bool is_terminal = false;

    template<typename Func>
    struct HasTestFunction
    {
        static const bool result = expression_type::template HasTestFunction<Func>::result;
    };
    template<typename Func>
    struct HasTrialFunction
    {
        static const bool result = expression_type::template HasTrialFunction<Func>::result;
    };
    template<typename Funct>
    static const bool has_test_basis = expression_type::template has_test_basis<Funct>;
    template<typename Funct>
    static const bool has_trial_basis = expression_type::template has_trial_basis<Funct>;
    using test_basis = typename expression_type::test_basis;
    using trial_basis = typename expression_type::trial_basis;

    using value_type = typename expression_type::value_type;
    using evaluate_type = typename expression_type::evaluate_type;

#if 1
    template <typename TheExprType>
    EvalOnFaces( TheExprType && e, std::string const& internalFacesEvalutationType, std::set<std::string> const& requiresMarkersConnection )
        :
        M_expr( std::forward<TheExprType>(e) ),
        M_internalFacesEvalutationType( InternalFacesEvalType::One_Side ),
        M_requiresMarkersConnection( requiresMarkersConnection )
        {
            if ( internalFacesEvalutationType == "average" )
                M_internalFacesEvalutationType = InternalFacesEvalType::Average;
            else if ( internalFacesEvalutationType == "max" )
                M_internalFacesEvalutationType = InternalFacesEvalType::Max;
            else if ( internalFacesEvalutationType == "min" )
                M_internalFacesEvalutationType = InternalFacesEvalType::Min;
            else if ( internalFacesEvalutationType == "sum" )
                M_internalFacesEvalutationType = InternalFacesEvalType::Sum;
            else if ( internalFacesEvalutationType == "one_side" )
                M_internalFacesEvalutationType = InternalFacesEvalType::One_Side;
        }
#else
    EvalOnFaces( expression_type const& e, std::string const& internalFacesEvalutationType, std::set<std::string> const& requiresMarkersConnection )
        :
        M_expr( e ),
        M_internalFacesEvalutationType( InternalFacesEvalType::One_Side ),
        M_requiresMarkersConnection( requiresMarkersConnection )
        {
            if ( internalFacesEvalutationType == "average" )
                M_internalFacesEvalutationType = InternalFacesEvalType::Average;
            else if ( internalFacesEvalutationType == "max" )
                M_internalFacesEvalutationType = InternalFacesEvalType::Max;
            else if ( internalFacesEvalutationType == "min" )
                M_internalFacesEvalutationType = InternalFacesEvalType::Min;
            else if ( internalFacesEvalutationType == "sum" )
                M_internalFacesEvalutationType = InternalFacesEvalType::Sum;
            else if ( internalFacesEvalutationType == "one_side" )
                M_internalFacesEvalutationType = InternalFacesEvalType::One_Side;
        }
#endif
    EvalOnFaces( EvalOnFaces const& ) = default;
    EvalOnFaces( EvalOnFaces && ) = default;


    //! polynomial order
    uint16_type polynomialOrder() const { return M_expr.polynomialOrder(); }

    //! expression is polynomial?
    bool isPolynomial() const { return M_expr.isPolynomial(); }

    size_type dynamicContext() const { return M_expr.dynamicContext(); }

    template <typename SymbolsExprType>
    auto applySymbolsExpr( SymbolsExprType const& se ) const
        {
            auto newexpr = M_expr.applySymbolsExpr( se );
            using newexpr_type = std::decay_t<decltype(newexpr)>;
            return EvalOnFaces<newexpr_type>( newexpr );
        }

    template <typename TheSymbolExprType>
    bool hasSymbolDependency( std::string const& symb, TheSymbolExprType const& se ) const { return M_expr.hasSymbolDependency( symb,se ); }

    template <typename TheSymbolExprType>
    void dependentSymbols( std::string const& symb, std::map<std::string,std::set<std::string>> & res, TheSymbolExprType const& se ) const
        {
            M_expr.dependentSymbols( symb, res, se );
        }

    template <int diffOrder, typename TheSymbolExprType>
    auto diff( std::string const& diffVariable, WorldComm const& world, std::string const& dirLibExpr,
               TheSymbolExprType const& se ) const
        {
            CHECK( false ) << "TODO";
            return *this;
        }


    expression_type const& expr() const { return M_expr; }
    InternalFacesEvalType internalFacesEvalutationType() const { return M_internalFacesEvalutationType; }

    template<typename Geo_t, typename Basis_i_t, typename Basis_j_t>
    struct tensor
    {
        typedef mpl::int_<fusion::result_of::template size<Geo_t>::type::value> map_size;
        static constexpr bool has_two_side = map_size::value == 2;
        using gmcKey0 = vf::detail::gmc<0>;
        typedef typename mpl::if_<mpl::equal_to<map_size,mpl::int_<2> >,
               vf::detail::gmc<1>,
               vf::detail::gmc<0> >::type gmcKey1;
        typedef typename fusion::result_of::value_at_key<Geo_t,gmcKey0>::type left_gmc_ptrtype;
        typedef typename fusion::result_of::value_at_key<Geo_t,gmcKey0>::type::element_type left_gmc_type;
        typedef typename fusion::result_of::value_at_key<Geo_t,gmcKey1>::type right_gmc_ptrtype;
        typedef typename fusion::result_of::value_at_key<Geo_t,gmcKey1>::type::element_type right_gmc_type;
        typedef fusion::map<fusion::pair<vf::detail::gmc<0>, left_gmc_ptrtype> > map_left_gmc_type;
        typedef fusion::map<fusion::pair<vf::detail::gmc<0>, right_gmc_ptrtype> > map_right_gmc_type;
        typedef typename expression_type::template tensor<map_left_gmc_type, Basis_i_t, Basis_j_t> left_tensor_expr_type;
        typedef typename expression_type::template tensor<map_right_gmc_type, Basis_i_t, Basis_j_t> right_tensor_expr_type;

        using value_type = typename left_tensor_expr_type::value_type;
        using expr_shape = typename left_tensor_expr_type::shape;
        using shape = expr_shape;

        struct is_zero
        {
            static const bool value = false;
        };

        template <typename ... TheArgsType>
        tensor( this_type const& expr, Geo_t const& geom, const TheArgsType&... theInitArgs )
            :
            M_expr( expr ),
            M_useLeft( false ), M_useRight( false ),
            M_internalFacesEvalutationType( expr.internalFacesEvalutationType() ),
            M_hasRequiresMarkersConnection( false )
            {
                this->updateRequiresMarkerForUse( geom );
            }

        template<typename TheExprExpandedType,typename TupleTensorSymbolsExprType, typename... TheArgsType>
        tensor( std::true_type /**/, TheExprExpandedType const& exprExpanded, TupleTensorSymbolsExprType & ttse,
                this_type const& expr, Geo_t const& geom, const TheArgsType&... theInitArgs )
            :
            M_expr( expr ),
            M_useLeft( false ), M_useRight( false ),
            M_internalFacesEvalutationType( expr.internalFacesEvalutationType() ),
            M_hasRequiresMarkersConnection( false )
            {
                CHECK( false ) << "TODO";
            }

        // template <typename ... TheArgsType>
        // void update( Geo_t const& geom, const TheArgsType&... theUpdateArgs )
        // {
        // }
#if 1
        void update( Geo_t const& geom, Basis_i_t const& fev, Basis_j_t const& feu )
        {
            // NOTE :maybe handle trial/test expr?
            this->update( geom );
        }
        void update( Geo_t const& geom, Basis_i_t const& fev )
        {
            // NOTE : maybe handle test expr?
            this->update( geom );
        }
        void update( Geo_t const& geom )
        {
            M_useLeft = true;
            M_useRight = has_two_side;

            if ( M_hasRequiresMarkersConnection )
            {
                auto leftGmc = fusion::at_key<gmcKey0>( geom );
                auto const& leftElement = leftGmc->element();
                if ( M_requiresMarkersConnectionIds.find( leftElement.marker().value() ) == M_requiresMarkersConnectionIds.end() )
                    M_useLeft = false;

                 if constexpr( has_two_side )
                 {
                     auto rightGmc = fusion::at_key<gmcKey1>( geom );
                     auto const& rightElement = rightGmc->element();
                     if ( M_requiresMarkersConnectionIds.find( rightElement.marker().value() ) == M_requiresMarkersConnectionIds.end() )
                         M_useRight = false;
                 }

                 if ( !M_useLeft && !M_useRight )
                     return;
            }

            if constexpr( has_two_side )
            {
                if ( M_internalFacesEvalutationType == InternalFacesEvalType::One_Side )
                {
                    if ( M_useLeft )
                        M_useRight = false;
                }
            }

            if ( M_useLeft )
            {
                auto leftGeom = Feel::vf::mapgmcFix( fusion::at_key<gmcKey0>( geom ) );
                if ( !M_leftTensor )
                    M_leftTensor.emplace( M_expr.expr(), leftGeom );
                M_leftTensor->update( leftGeom );
            }

            if constexpr( has_two_side )
            {
                if ( M_useRight )
                {
                    auto rightGeom = Feel::vf::mapgmcFix( fusion::at_key<gmcKey1>( geom ) );
                    if ( !M_rightTensor )
                        M_rightTensor.emplace( M_expr.expr(), rightGeom );
                    M_rightTensor->update( rightGeom );
                }
            }
        }
#endif
        template<typename TheExprExpandedType,typename TupleTensorSymbolsExprType, typename... TheArgsType>
        void update( std::true_type /**/, TheExprExpandedType const& exprExpanded, TupleTensorSymbolsExprType & ttse,
                     Geo_t const& geom, const TheArgsType&... theUpdateArgs )
        {
            CHECK( false ) << "TODO";
        }

        value_type
        evalijq( uint16_type i, uint16_type j, uint16_type c1, uint16_type c2, uint16_type q ) const
        {
            value_type evalLeft = M_useLeft ? M_leftTensor->evalijq( i,j,c1,c2,q ) : 0;
            value_type evalRight = M_useRight ? M_rightTensor->evalijq( i,j,c1,c2,q ) : 0;
            return this->evalImpl( evalLeft,evalRight );
        }
        value_type
        evaliq( uint16_type i, uint16_type c1, uint16_type c2, uint16_type q ) const
        {
            value_type evalLeft = M_useLeft ? M_leftTensor->evaliq( i,c1,c2,q ) : 0;
            value_type evalRight = M_useRight ? M_rightTensor->evaliq( i,c1,c2,q ) : 0;
            return this->evalImpl( evalLeft,evalRight );
        }
        value_type
        evalq( uint16_type c1, uint16_type c2, uint16_type q ) const
        {
            value_type evalLeft = M_useLeft ? M_leftTensor->evalq( c1,c2,q ) : 0;
            value_type evalRight = M_useRight ? M_rightTensor->evalq( c1,c2,q ) : 0;
            return this->evalImpl( evalLeft,evalRight );
        }
    private:
        void updateRequiresMarkerForUse( Geo_t const& geom )
            {
                auto leftGmc = fusion::at_key<gmcKey0>( geom );
                auto const& leftElement = leftGmc->element();
                auto mesh = leftElement.mesh();
                if ( mesh )
                {
                    for ( std::string const& marker : M_expr.M_requiresMarkersConnection )
                    {
                        if ( !mesh->hasElementMarker( marker ) )
                            continue;
                        M_requiresMarkersConnectionIds.insert( mesh->markerId( marker ) );
                    }
                }
                if ( !M_requiresMarkersConnectionIds.empty() )
                    M_hasRequiresMarkersConnection = true;
            }

        value_type
        evalImpl( value_type evalLeft,value_type evalRight ) const
            {
                if ( !M_useLeft || !M_useRight )
                    return evalLeft+evalRight;

                value_type res(0);
                switch ( M_internalFacesEvalutationType )
                {
                case InternalFacesEvalType::Average:
                    res = 0.5*(evalLeft+evalRight);
                    break;
                case InternalFacesEvalType::Max:
                    res = std::max(evalLeft,evalRight);
                    break;
                case InternalFacesEvalType::Min:
                    res = std::min(evalLeft,evalRight);
                    break;
                default: // all others cases can be take into account here
                    res = evalLeft+evalRight;
                    break;
                }
                return res;
            }

    private:
        this_type const& M_expr;
        InternalFacesEvalType M_internalFacesEvalutationType;
        std::optional<left_tensor_expr_type> M_leftTensor;
        std::optional<right_tensor_expr_type> M_rightTensor;
        bool M_useLeft, M_useRight;
        bool M_hasRequiresMarkersConnection;
        std::set<flag_type> M_requiresMarkersConnectionIds;
    };

private :
    expression_type M_expr;
    InternalFacesEvalType M_internalFacesEvalutationType;
    std::set<std::string> M_requiresMarkersConnection;
};


template <typename ExprType>
auto evalOnFaces( ExprType && e, std::set<std::string> const& requiresMarkersConnection, std::string const& internalFacesEvalutationType = "" )
{
    return Feel::vf::expr( EvalOnFaces<std::decay_t<ExprType>>( std::forward<ExprType>(e), internalFacesEvalutationType, requiresMarkersConnection ) );
}


template <typename RangeType,typename ExprType>
decltype(auto) evalOnEntities( RangeType const& range, ExprType && e, std::set<std::string> const& requiresMarkersConnection, std::string const& internalFacesEvalutationType = "" )
{
    if constexpr( filter_enum_t<RangeType>::value == MESH_FACES )
        return evalOnFaces( std::forward<ExprType>( e ), requiresMarkersConnection, internalFacesEvalutationType );
    else
        return std::forward<ExprType>( e );
}

} // namespace vf
} // namespace Feel

#endif
