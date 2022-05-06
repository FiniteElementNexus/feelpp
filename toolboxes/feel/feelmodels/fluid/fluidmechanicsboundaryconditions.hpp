/* -*- mode: c++; coding: utf-8; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; show-trailing-whitespace: t -*- vim:fenc=utf-8:ft=cpp:et:sw=4:ts=4:sts=4
 */

#ifndef FEELPP_TOOLBOXES_FLUID_FLUIDMECHANICSBOUNDARYCONDITIONS_HPP
#define FEELPP_TOOLBOXES_FLUID_FLUIDMECHANICSBOUNDARYCONDITIONS_HPP

#include <feel/feelmodels/modelcore/genericboundaryconditions.hpp>

namespace Feel
{
namespace FeelModels
{

/**
 * @brief Boundary Conditions in fluid mechanics
 *
 * @tparam Dim real dimension
 */
template <uint16_type Dim>
class FluidMechanicsBoundaryConditions : public BoundaryConditionsBase
{
    using super_type = BoundaryConditionsBase;
public:
    using self_type = FluidMechanicsBoundaryConditions<Dim>;
    enum class Type { VelocityImposed=0, MeshVelocityImposed };

    class VelocityImposed : public GenericDirichletBoundaryCondition<Dim,1>
    {
        using super_type = GenericDirichletBoundaryCondition<Dim,1>;
    public:
        VelocityImposed( std::string const& name, std::shared_ptr<ModelBase> const& tbParent ) : super_type( name, tbParent ) {}
        VelocityImposed( VelocityImposed const& ) = default;
        VelocityImposed( VelocityImposed && ) = default;

        virtual self_type::Type type() const { return self_type::Type::VelocityImposed; }
    };

    class MeshVelocityImposed : public VelocityImposed
    {
        using super_type = VelocityImposed;
    public:
        MeshVelocityImposed( std::string const& name, std::shared_ptr<ModelBase> const& tbParent ) : super_type( name,tbParent ) {}
        MeshVelocityImposed( MeshVelocityImposed const& ) = default;
        MeshVelocityImposed( MeshVelocityImposed && ) = default;
        void setup( nl::json const& jarg, ModelIndexes const& indexes ) override;

        self_type::Type type() const override { return self_type::Type::MeshVelocityImposed; }

        //! update informations
        void updateInformationObject( nl::json & p ) const override;
        //! return tabulate information from json info
        static tabulate_informations_ptr_t tabulateInformations( nl::json const& jsonInfo, TabulateInformationProperties const& tabInfoProp );
    };


    class Inlet
    {
    public:
        enum class Shape { constant=0,parabolic };
        enum class Constraint { velocity_max=0,flow_rate };

        Inlet( std::string const& name ) : M_name( name ), M_shape( Shape::parabolic ), M_constraint( Constraint::velocity_max ) {}
        Inlet( Inlet const& ) = default;
        Inlet( Inlet && ) = default;

        //! setup bc from json
        void setup( ModelBase const& mparent, nl::json const& jarg, ModelIndexes const& indexes );

        Shape shape() const { return M_shape; }
        Constraint constraint() const { return M_constraint; }

        //! return expression
        template <typename SymbolsExprType = symbols_expression_empty_t>
        auto expr( SymbolsExprType const& se = symbols_expression_empty_t{} ) const
        {
            return Feel::vf::expr( M_mexpr.template expr<1,1>(), se );
        }
        //! return markers
        std::set<std::string> const& markers() const { return M_markers; }

        void setParameterValues( std::map<std::string,double> const& paramValues ) { M_mexpr.setParameterValues( paramValues ); }

        //! update informations
        void updateInformationObject( nl::json & p ) const;
        //! return tabulate information from json info
        static tabulate_informations_ptr_t tabulateInformations( nl::json const& jsonInfo, TabulateInformationProperties const& tabInfoProp );

    private:
        std::string M_name;
        Shape M_shape;
        Constraint M_constraint;
        ModelExpression M_mexpr;
        std::set<std::string> M_markers;
    };

    class NormalStress
    {
    public:

        NormalStress( std::string const& name ) : M_name( name ) {}
        NormalStress( NormalStress const& ) = default;
        NormalStress( NormalStress && ) = default;

        //! setup bc from json
        void setup( ModelBase const& mparent, nl::json const& jarg, ModelIndexes const& indexes );

        //! return true if expr is scalar
        bool isScalarExpr() const { return M_mexpr.template hasExpr<1,1>(); }

        //! return true if expr is vectorial
        bool isVectorialExpr() const { return M_mexpr.template hasExpr<Dim,1>(); }

        //! return true if expr is matrix
        bool isMatrixExpr() const { return M_mexpr.template hasExpr<Dim,Dim>(); }

        //! return scalar expression
        template <typename SymbolsExprType = symbols_expression_empty_t>
        auto exprScalar( SymbolsExprType const& se = symbols_expression_empty_t{} ) const
        {
            return Feel::vf::expr( M_mexpr.template expr<1,1>(), se );
        }

        //! return vectorial expression
        template <typename SymbolsExprType = symbols_expression_empty_t>
        auto exprVectorial( SymbolsExprType const& se = symbols_expression_empty_t{} ) const
        {
            return Feel::vf::expr( M_mexpr.template expr<Dim,1>(), se );
        }

        //! return matrix expression
        template <typename SymbolsExprType = symbols_expression_empty_t>
        auto exprMatrix( SymbolsExprType const& se = symbols_expression_empty_t{} ) const
        {
            return Feel::vf::expr( M_mexpr.template expr<Dim,Dim>(), se );
        }

        //! return markers
        std::set<std::string> const& markers() const { return M_markers; }

        void setParameterValues( std::map<std::string,double> const& paramValues ) { M_mexpr.setParameterValues( paramValues ); }

        //! update informations
        void updateInformationObject( nl::json & p ) const;
        //! return tabulate information from json info
        static tabulate_informations_ptr_t tabulateInformations( nl::json const& jsonInfo, TabulateInformationProperties const& tabInfoProp );

    private:
        std::string M_name;
        ModelExpression M_mexpr;
        std::set<std::string> M_markers;
    };

    class PressureImposed
    {
    public:

        PressureImposed( std::string const& name ) : M_name( name ) {}
        PressureImposed( PressureImposed const& ) = default;
        PressureImposed( PressureImposed && ) = default;

        //! setup bc from json
        void setup( ModelBase const& mparent, nl::json const& jarg, ModelIndexes const& indexes );

        //! return expression
        template <typename SymbolsExprType = symbols_expression_empty_t>
        auto expr( SymbolsExprType const& se = symbols_expression_empty_t{} ) const
        {
            return Feel::vf::expr( M_mexpr.template expr<1,1>(), se );
        }
        //! return markers
        std::set<std::string> const& markers() const { return M_markers; }

        void setParameterValues( std::map<std::string,double> const& paramValues ) { M_mexpr.setParameterValues( paramValues ); }

        //! update informations
        void updateInformationObject( nl::json & p ) const;
        //! return tabulate information from json info
        static tabulate_informations_ptr_t tabulateInformations( nl::json const& jsonInfo, TabulateInformationProperties const& tabInfoProp );

    private:
        std::string M_name;
        ModelExpression M_mexpr;
        std::set<std::string> M_markers;
    };

    class OutletFree
    {
    public:

        OutletFree( std::string const& name ) : M_name( name ) {}
        OutletFree( OutletFree const& ) = default;
        OutletFree( OutletFree && ) = default;

        //! setup bc from json
        void setup( ModelBase const& mparent, nl::json const& jarg, ModelIndexes const& indexes );

        //! return markers
        std::set<std::string> const& markers() const { return M_markers; }

        //! update informations
        void updateInformationObject( nl::json & p ) const;
        //! return tabulate information from json info
        static tabulate_informations_ptr_t tabulateInformations( nl::json const& jsonInfo, TabulateInformationProperties const& tabInfoProp );

    private:
        std::string M_name;
        std::set<std::string> M_markers;
    };

    class OutletWindkessel
    {
    public:
        enum class CouplingType { Implicit=0, Explicit };

        OutletWindkessel( std::string const& name ) : M_name( name ), M_couplingType( CouplingType::Implicit ) {}
        OutletWindkessel( OutletWindkessel const& ) = default;
        OutletWindkessel( OutletWindkessel && ) = default;

        //! setup bc from json
        void setup( ModelBase const& mparent, nl::json const& jarg, ModelIndexes const& indexes );

        //! return markers
        std::set<std::string> const& markers() const { return M_markers; }

        //! return expression of distal resistance
        template <typename SymbolsExprType = symbols_expression_empty_t>
        auto expr_Rd( SymbolsExprType const& se = symbols_expression_empty_t{} ) const
        {
            return Feel::vf::expr( M_mexpr_Rd.template expr<1,1>(), se );
        }
        //! return expression of proximal resistance
        template <typename SymbolsExprType = symbols_expression_empty_t>
        auto expr_Rp( SymbolsExprType const& se = symbols_expression_empty_t{} ) const
        {
            return Feel::vf::expr( M_mexpr_Rp.template expr<1,1>(), se );
        }
        //! return expression of capacitance
        template <typename SymbolsExprType = symbols_expression_empty_t>
        auto expr_Cd( SymbolsExprType const& se = symbols_expression_empty_t{} ) const
        {
            return Feel::vf::expr( M_mexpr_Cd.template expr<1,1>(), se );
        }

        //! return coupling type of this bc (Implicit or Explicit)
        CouplingType couplingType() const { return M_couplingType; }

        bool useImplicitCoupling() const { return M_couplingType == CouplingType::Implicit; }

        void setParameterValues( std::map<std::string,double> const& paramValues )
            {
                M_mexpr_Rd.setParameterValues( paramValues );
                M_mexpr_Rp.setParameterValues( paramValues );
                M_mexpr_Cd.setParameterValues( paramValues );
            }

        //! update informations
        void updateInformationObject( nl::json & p ) const;
        //! return tabulate information from json info
        static tabulate_informations_ptr_t tabulateInformations( nl::json const& jsonInfo, TabulateInformationProperties const& tabInfoProp );

    private:
        std::string M_name;
        std::set<std::string> M_markers;
        ModelExpression M_mexpr_Rd, M_mexpr_Rp, M_mexpr_Cd;
        CouplingType M_couplingType;
    };

    class BodyInterface
    {
    public :
        BodyInterface( std::string const& name ) : M_name( name ) {}
        BodyInterface( BodyInterface const& ) = default;
        BodyInterface( BodyInterface && ) = default;

        //! setup bc from json
        void setup( ModelBase const& mparent, nl::json const& jarg, ModelIndexes const& indexes );

        //! return markers
        std::set<std::string> const& markers() const { return M_markers; }

        ModelExpression const& mexprTranslationalVelocity() const { return M_mexprTranslationalVelocity; }
        ModelExpression const& mexprAngularVelocity() const { return M_mexprAngularVelocity; }
        nl::json const& jsonMaterials() const { return M_jsonMaterials; }
        std::map<std::string, std::tuple< ModelExpression, std::set<std::string>>> const& elasticVelocityExprBC() const { return M_elasticVelocityExprBC; }
        std::map<std::string, std::tuple< ModelExpression, std::set<std::string>>> const& elasticDisplacementExprBC() const { return M_elasticDisplacementExprBC; }
        std::map<std::string,ModelExpression> const& articulationTranslationalVelocityExpr() const { return M_articulationTranslationalVelocityExpr; }

        void setParameterValues( std::map<std::string,double> const& paramValues ) { /*M_mexpr.setParameterValues( paramValues );*/ }

        //! update informations
        void updateInformationObject( nl::json & p ) const;
        //! return tabulate information from json info
        static tabulate_informations_ptr_t tabulateInformations( nl::json const& jsonInfo, TabulateInformationProperties const& tabInfoProp );

    private:
        std::string M_name;
        std::set<std::string> M_markers;
        ModelExpression M_mexprTranslationalVelocity, M_mexprAngularVelocity;
        nl::json M_jsonMaterials;
        std::map<std::string, std::tuple< ModelExpression, std::set<std::string>>> M_elasticVelocityExprBC;
        std::map<std::string, std::tuple< ModelExpression, std::set<std::string>>> M_elasticDisplacementExprBC;
        std::map<std::string,ModelExpression> M_articulationTranslationalVelocityExpr;

    };


    FluidMechanicsBoundaryConditions( std::shared_ptr<ModelBase> const& tbParent ) : super_type( tbParent ) {}
    FluidMechanicsBoundaryConditions( FluidMechanicsBoundaryConditions const& ) = default;
    FluidMechanicsBoundaryConditions( FluidMechanicsBoundaryConditions && ) = default;

    //! return velocity imposed
    std::map<std::pair<Type,std::string>,std::shared_ptr<VelocityImposed>> const& velocityImposed() const { return M_velocityImposed; }
    //! return normal stress
    std::map<std::string,std::shared_ptr<NormalStress>> const& normalStress() const { return M_normalStress; }
    //! return inlet
    std::map<std::string,std::shared_ptr<Inlet>> const& inlet() const { return M_inlet; }
    //! return outlet free (no normal stress)
    std::map<std::string,std::shared_ptr<OutletFree>> const& outletFree() const { return M_outletFree; }
    //! return outlet windkessel
    std::map<std::string,std::shared_ptr<OutletWindkessel>> const& outletWindkessel() const { return M_outletWindkessel; }
    //! return pressure imposed
    std::map<std::string,std::shared_ptr<PressureImposed>> const& pressureImposed() const { return M_pressureImposed; }
    //! return body interface
    std::map<std::string,std::shared_ptr<BodyInterface>> const& bodyInterface() const { return M_bodyInterface; }

    //! return true if a bc is type of dof eliminitation
    bool hasTypeDofElimination() const
        {
            for ( auto const& [bcId,bcData] : M_velocityImposed )
                if ( bcData->isMethodElimination() )
                    return true;
            if ( !M_bodyInterface.empty() )
                return true;
            return !M_inlet.empty() || !M_pressureImposed.empty();
        }

    //! apply dof elimination in linear context
    template <typename BfType, typename RhsType,typename MeshType, typename EltType, typename SymbolsExprType>
    void
    applyDofEliminationLinear( BfType& bilinearForm, RhsType& F, MeshType const& mesh, EltType const& u, SymbolsExprType const& se ) const
        {
            Feel::FeelModels::detail::applyDofEliminationLinearOnBoundaryConditions( M_velocityImposed, bilinearForm, F, mesh, u, se );
        }

    //! apply Newton initial guess (on dof elimination context)
    template <typename MeshType, typename EltType, typename SymbolsExprType>
    void
    applyNewtonInitialGuess( MeshType const& mesh, EltType & u, SymbolsExprType const& se ) const
        {
            Feel::FeelModels::detail::applyNewtonInitialGuessOnBoundaryConditions( M_velocityImposed, mesh, u, se );
        }

    bool hasVelocityImposedLagrangeMultiplier() const { return hasVelocityImposed( VelocityImposed::Method::lagrange_multiplier ); }

    std::map<std::pair<Type,std::string>,std::shared_ptr<VelocityImposed>> velocityImposedLagrangeMultiplier() const
        {
            return this->velocityImposed( VelocityImposed::Method::lagrange_multiplier );
        }
    bool hasVelocityImposedNitsche() const { return hasVelocityImposed( VelocityImposed::Method::nitsche ); }

    std::map<std::pair<Type,std::string>,std::shared_ptr<VelocityImposed>> velocityImposedNitsche() const
        {
            return this->velocityImposed( VelocityImposed::Method::nitsche );
        }

    void setParameterValues( std::map<std::string,double> const& paramValues );

    //! setup bc from json
    void setup( nl::json const& jarg );

    //! update informations
    void updateInformationObject( nl::json & p ) const;
    //! return tabulate information from json info
    static tabulate_informations_ptr_t tabulateInformations( nl::json const& jsonInfo, TabulateInformationProperties const& tabInfoProp );

private:
    bool hasVelocityImposed( typename VelocityImposed::Method method ) const
        {
            for ( auto const& [bcId,bcData] : M_velocityImposed )
                if ( bcData->isMethod( method ) )
                    return true;
            return false;
        }

    std::map<std::pair<Type,std::string>,std::shared_ptr<VelocityImposed>> velocityImposed( typename VelocityImposed::Method method ) const
        {
            std::map<std::pair<Type,std::string>,std::shared_ptr<VelocityImposed>> ret;
            for ( auto const& [bcId,bcData] : M_velocityImposed )
                if ( bcData->isMethod( method ) )
                    ret.emplace( bcId, bcData );
            return ret;
        }

private:
    std::map<std::pair<Type,std::string>,std::shared_ptr<VelocityImposed>> M_velocityImposed;
    std::map<std::string,std::shared_ptr<NormalStress>> M_normalStress;
    std::map<std::string,std::shared_ptr<Inlet>> M_inlet;
    std::map<std::string,std::shared_ptr<OutletFree>> M_outletFree;
    std::map<std::string,std::shared_ptr<OutletWindkessel>> M_outletWindkessel;
    std::map<std::string,std::shared_ptr<PressureImposed>> M_pressureImposed;
    std::map<std::string,std::shared_ptr<BodyInterface>> M_bodyInterface;

}; // FluidMechanicsBoundaryConditions

} // namespace FeelModels
} // namespace Feel

#endif
