/* -*- mode: c++; coding: utf-8; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; show-trailing-whitespace: t -*- vim:fenc=utf-8:ft=cpp:et:sw=4:ts=4:sts=4

  This file is part of the Feel library

  Author(s): Romain Hild <romain.hild@cemosis.fr>
       Date: 2021-05-04

  Copyright (C) 2016 Feel++ Consortium

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
   \file mixedpoisson.hpp
   \author Romain Hild <romain.hild@cemosis.fr>
   \date 2021-05-04
 */

#ifndef _MIXEDPOISSON2_HPP
#define _MIXEDPOISSON2_HPP

#include <feel/feeldiscr/functionspace.hpp>
#include <feel/feelfilters/exporter.hpp>

#include <feel/feelmodels/modelcore/modelnumerical.hpp>
#include <feel/feelmodels/modelcore/modelphysics.hpp>
#include <feel/feelmodels/modelcore/markermanagement.hpp>
#include <feel/feelmodels/modelcore/options.hpp>
#include <feel/feelmodels/modelalg/modelalgebraicfactory.hpp>
#include <feel/feelmodels/modelmaterials/materialsproperties.hpp>
#include <feel/feeldiscr/pdh.hpp>
#include <feel/feeldiscr/pdhv.hpp>
#include <feel/feeldiscr/pch.hpp>
#include <feel/feeldiscr/pchv.hpp>
#include <feel/feeldiscr/product.hpp>
#include <feel/feelvf/blockforms.hpp>
#include <feel/feelts/bdf.hpp>

#include <feel/feelmodels/hdg/enums.hpp>

namespace Feel
{
namespace FeelModels
{

inline po::options_description
makeMixedPoissonOptions( std::string const&  _prefix = "", std::string const&  _toolbox_prefix = "hdg.poisson" )
{
    std::string prefix = _toolbox_prefix.empty()?"hdg.poisson":_toolbox_prefix;
    if ( !_prefix.empty() )
        prefix = prefixvm( prefix, _prefix );
    po::options_description mpOptions( "Mixed Poisson HDG options" );
    mpOptions.add_options()( "gmsh.submesh", po::value<std::string>()->default_value( "" ), "submesh extraction" )
        ( prefixvm( prefix, "tau_constant" ).c_str(), po::value<double>()->default_value( 1.0 ), "stabilization constant for hybrid methods" )
        ( prefixvm( prefix, "use-sc" ).c_str(), po::value<bool>()->default_value( true ), "use static condensation" )
        ( prefixvm( prefix, "time-stepping").c_str(), Feel::po::value< std::string >()->default_value("BDF"), "time integration schema : BDF, Theta")
        ( prefixvm( prefix, "time-stepping.theta.value").c_str(), Feel::po::value< double >()->default_value(0.5), " Theta value")
        ;
    mpOptions.add( modelnumerical_options( prefix ) );
    mpOptions.add( bdf_options( prefix ) );
    mpOptions.add( ts_options( prefix ) );
    mpOptions.add( backend_options( prefix + ".sc" ) );
    return mpOptions;
}

inline po::options_description
makeMixedPoissonLibOptions( std::string const&  prefix = "", std::string const&  _toolbox_prefix = "hdg.poisson" )
{
    po::options_description mpLibOptions( "Mixed Poisson HDG Lib options");
    // if ( !prefix.empty() )
    //     mpLibOptions.add( backend_options( prefix ) );
    return mpLibOptions;
}

/**
 * Toolbox MixedPoisson
 * @ingroup Toolboxes
 */
template<typename ConvexType, int Order, int E_Order = 4>
class MixedPoisson : public ModelNumerical,
                     public ModelPhysics<ConvexType::nDim>,
                     public std::enable_shared_from_this<MixedPoisson<ConvexType, Order, E_Order> >
{

public:
    using super_type = ModelNumerical;
    using self_type = MixedPoisson<ConvexType, Order, E_Order>;
    using self_ptrtype = std::shared_ptr<self_type>;

    // mesh
    using convex_type = ConvexType;
    static const uint16_type nDim = convex_type::nDim;
    static const uint16_type nOrderGeo = convex_type::nOrder;
    static const uint16_type nRealDim = convex_type::nRealDim;
    using mesh_type = Mesh<convex_type>;
    using mesh_ptrtype = std::shared_ptr<mesh_type>;

    // face mesh
    using face_convex_type = typename ReduceDim<convex_type>::type;
    using face_mesh_type = Mesh<face_convex_type>;
    using face_mesh_ptrtype = std::shared_ptr<face_mesh_type>;

    static const uint16_type expr_order = (Order+E_Order)*nOrderGeo;

    // using sparse_matrix_type = backend_type::sparse_matrix_type;
    // using sparse_matrix_ptrtype = backend_type::sparse_matrix_ptrtype;
    // using vector_type = backend_type::vector_type;
    // using vector_ptrtype = backend_type::vector_ptrtype;

    // Vh
    using space_flux_type = Pdhv_type<mesh_type,Order>;
    using space_flux_ptrtype = Pdhv_ptrtype<mesh_type,Order>;
    using element_flux_type = typename space_flux_type::element_type;
    using element_flux_ptrtype = typename space_flux_type::element_ptrtype;
    // Wh
    using space_potential_type = Pdh_type<mesh_type,Order>;
    using space_potential_ptrtype = Pdh_ptrtype<mesh_type,Order>;
    using element_potential_type = typename space_potential_type::element_type;
    using element_potential_ptrtype = typename space_potential_type::element_ptrtype;
    // Whp
    using space_postpotential_type = Pdh_type<mesh_type,Order+1>;
    using space_postpotential_ptrtype = Pdh_ptrtype<mesh_type,Order+1>;
    using element_postpotential_type = typename space_postpotential_type::element_type;
    using element_postpotential_ptrtype = typename space_postpotential_type::element_ptrtype;
    // Mh
    using space_trace_type = Pdh_type<face_mesh_type,Order>;
    using space_trace_ptrtype = Pdh_ptrtype<face_mesh_type,Order>;
    using element_trace_type = typename space_trace_type::element_type;
    using element_trace_ptrtype = typename space_trace_type::element_ptrtype;
    // Ch
    using space_traceibc_type = Pch_type<face_mesh_type,0>;
    using space_traceibc_ptrtype = Pch_ptrtype<face_mesh_type,0>;
    using element_traceibc_type = typename space_traceibc_type::element_type;
    using element_traceibc_ptrtype = typename space_traceibc_type::element_ptrtype;
    using element_traceibc_vector_type = std::vector<element_traceibc_ptrtype>;
    // P0dh
    using space_p0dh_type = Pdh_type<mesh_type,0>;
    using space_p0dh_ptrtype = Pdh_ptrtype<mesh_type,0>;

    using product2_space_type = ProductSpaces2<space_traceibc_ptrtype,
                                               space_flux_ptrtype,
                                               space_potential_ptrtype,
                                               space_trace_ptrtype>;
    using product2_space_ptrtype = std::shared_ptr<product2_space_type>;

    // materials properties
    typedef MaterialsProperties<nRealDim> materialsproperties_type;
    typedef std::shared_ptr<materialsproperties_type> materialsproperties_ptrtype;

    // exporter
    typedef Exporter<mesh_type,nOrderGeo> export_type;
    typedef std::shared_ptr<export_type> export_ptrtype;

    // measure tools for points evaluation
    typedef MeasurePointsEvaluation<space_potential_type, space_postpotential_type, space_flux_type> measure_points_evaluation_type;
    typedef std::shared_ptr<measure_points_evaluation_type> measure_points_evaluation_ptrtype;

    // time scheme
    using bdf_potential_type = Bdf<space_potential_type>;
    using bdf_potential_ptrtype = std::shared_ptr<bdf_potential_type>;

    // using integral_boundary_list_type = std::vector<ExpressionStringAtMarker>;

    struct FieldTag
    {
        static auto potential( self_type const* t ) { return ModelFieldTag<self_type,0>( t ); }
        static auto postpotential( self_type const* t ) { return ModelFieldTag<self_type,1>( t ); }
        static auto flux( self_type const* t ) { return ModelFieldTag<self_type,2>( t ); }
    };

protected:
    elements_reference_wrapper_t<mesh_type> M_rangeMeshElements;
    faces_reference_wrapper_t<mesh_type> M_gammaMinusIntegral;

    space_flux_ptrtype M_Vh; // flux
    space_potential_ptrtype M_Wh; // potential
    space_postpotential_ptrtype M_Whp; // postprocess potential
    space_trace_ptrtype M_Mh; // potential trace
    space_traceibc_ptrtype M_Ch; // Lagrange multiplier
    space_p0dh_ptrtype M_P0dh;
    product2_space_ptrtype M_ps;
    element_flux_ptrtype M_up; // flux solution
    element_potential_ptrtype M_pp; // potential solution
    element_postpotential_ptrtype M_ppp; // postprocess potential solution
    element_trace_ptrtype M_phat;
    element_traceibc_vector_type M_mup; // potential solution on the integral boundary conditions

    // physical parameter
    materialsproperties_ptrtype M_materialsProperties;
    MixedPoissonPhysics M_physic;
    std::map<std::string,std::string> M_physicMap;
    std::string M_potentialKey;
    std::string M_fluxKey;

    // boundary conditions
    MarkerManagementDirichletBC M_bcDirichletMarkerManagement;
    MarkerManagementNeumannBC M_bcNeumannMarkerManagement;
    MarkerManagementRobinBC M_bcRobinMarkerManagement;
    MarkerManagementIntegralBC M_bcIntegralMarkerManagement;

    backend_ptrtype M_backend;
    condensed_matrix_ptr_t<value_type> M_A;
    condensed_vector_ptr_t<value_type> M_F;
    condensed_matrix_ptr_t<value_type> M_App;
    condensed_vector_ptr_t<value_type> M_Fpp;
    vector_ptrtype M_U;

    // time discretisation
    std::string M_timeStepping;
    bdf_potential_ptrtype M_bdfPotential;
    double M_timeStepThetaValue;
    vector_ptrtype M_timeStepThetaSchemePreviousContrib;

    double M_tauCst;
    bool M_useSC;

    // bool M_isPicard;

    export_ptrtype M_exporter;
    measure_points_evaluation_ptrtype M_measurePointsEvaluation;

    mutable bool M_postMatrixInit;
public:

    BOOST_PARAMETER_MEMBER_FUNCTION(
        ( self_ptrtype ), static New, tag,
        ( required
          ( prefix,*( boost::is_convertible<mpl::_,std::string> ) )
          )
        ( optional
          ( physic, *, MixedPoissonPhysics::None )
          ( worldcomm, *, Environment::worldCommPtr() )
          ( repository, *, ModelBaseRepository() )
          ) )
        {
            return std::make_shared<self_type>( prefix, physic, worldcomm, "", repository );
        }

    // constructor
    MixedPoisson( std::string const& prefix = "hdg.poisson",
                  MixedPoissonPhysics const& physic = MixedPoissonPhysics::None,
                  worldcomm_ptr_t const& _worldComm = Environment::worldCommPtr(),
                  std::string const& subPrefix = "",
                  ModelBaseRepository const& modelRep = ModelBaseRepository() );

    std::shared_ptr<std::ostringstream> getInfo() const override;
    void updateInformationObject( nl::json & p ) const override;
    tabulate_informations_ptr_t tabulateInformations( nl::json const& jsonInfo, TabulateInformationProperties const& tabInfoProp ) const override;

    // Get Methods
    std::string potentialKey() const { return M_potentialKey; }
    std::string fluxKey() const { return M_fluxKey; }
    mesh_ptrtype mesh() const { return super_type::super_model_meshes_type::mesh<mesh_type>( this->keyword() ); }
    void setMesh( mesh_ptrtype const& mesh ) { super_type::super_model_meshes_type::setMesh( this->keyword(), mesh ); }
    elements_reference_wrapper_t<mesh_type> const& rangeMeshElements() const { return M_rangeMeshElements; }

    space_flux_ptrtype const& spaceFlux() const { return M_Vh; }
    element_flux_ptrtype const& fieldFluxPtr() const { return M_up; }
    element_flux_type const& fieldFlux() const { return *M_up; }
    space_potential_ptrtype const& spacePotential() const { return M_Wh; }
    element_potential_ptrtype const& fieldPotentialPtr() const { return M_pp; }
    element_potential_type const& fieldPotential() const { return *M_pp; }
    space_postpotential_ptrtype const& spacePostPotential() const { return M_Whp; }
    element_postpotential_ptrtype const& fieldPostPotentialPtr() const { return M_ppp; }
    element_postpotential_type const& fieldPostPotential() const { return *M_ppp; }
    space_trace_ptrtype const& spaceTrace() const { return M_Mh; }
    element_trace_ptrtype const& fieldTracePtr() const { return M_phat; }
    element_trace_type const& fieldTrace() const { return *M_phat; }
    space_traceibc_ptrtype const& spaceTraceIbc() const { return M_Ch; }
    product2_space_ptrtype const& spaceProductPtr() const { return M_ps; }
    product2_space_type const& spaceProduct() const { return *M_ps; }

    materialsproperties_ptrtype const& materialsProperties() const { return M_materialsProperties; }
    materialsproperties_ptrtype & materialsProperties() { return M_materialsProperties; }
    void setMaterialsProperties( materialsproperties_ptrtype mp ) { M_materialsProperties = mp; }

    backend_ptrtype const& backend() { return M_backend; }
    product2_space_ptrtype getPS() const { return M_ps; }
    condensed_vector_ptr_t<value_type> getF() { return M_F; }

    double tauCst() const { return M_tauCst; }
    void setTauCst(double cst) { M_tauCst = cst; }
    bool useSC() const { return M_useSC; }
    void setUseSC(bool sc) { M_useSC = sc; }

private :
    void loadParameterFromOptionsVm();
    void initMesh();
    void initBoundaryConditions();
    void initFunctionSpaces();
    void initTimeStep();
    void initPostProcess() override;

public:
    void init( bool buildModelAlgebraicFactory = true );
    // BlocksBaseGraphCSR buildBlockMatrixGraph() const override;
    // int nBlockMatrixGraph() const;
    // void initAlgebraicFactory();

    void updateParameterValues();
    void setParameterValues( std::map<std::string,double> const& paramValues );

    //___________________________________________________________________________________//
    // execute post-processing
    //___________________________________________________________________________________//

    void exportResults() { this->exportResults( this->currentTime() ); }
    void exportResults( double time );

    template <typename ModelFieldsType,typename SymbolsExpr,typename ExportsExprType>
    void exportResults( double time, ModelFieldsType const& mfields, SymbolsExpr const& symbolsExpr, ExportsExprType const& exportsExpr );

    template <typename SymbolsExpr>
    void exportResults( double time, SymbolsExpr const& symbolsExpr )
        {
            return this->exportResults( time, this->modelFields(), symbolsExpr, this->exprPostProcessExports( symbolsExpr ) );
        }

    template <typename ModelFieldsType, typename SymbolsExpr>
    void executePostProcessMeasures( double time, ModelFieldsType const& mfields, SymbolsExpr const& symbolsExpr );

    //___________________________________________________________________________________//
    // model context helper
    //___________________________________________________________________________________//

    template <typename ModelFieldsType>
    auto modelContext( ModelFieldsType const& mfields, std::string const& prefix = "" ) const
        {
            auto se = this->symbolsExpr( mfields ).template createTensorContext<mesh_type>();
            return Feel::FeelModels::modelContext( mfields, std::move( se ) );
        }
    auto modelContext( std::string const& prefix = "" ) const
        {
            auto mfields = this->modelFields( prefix );
            auto se = this->symbolsExpr( mfields ).template createTensorContext<mesh_type>();
            return Feel::FeelModels::modelContext( std::move( mfields ), std::move( se ) );
        }

    //___________________________________________________________________________________//
    // apply assembly and solver
    //___________________________________________________________________________________//

    void solve();
    void updateLinearPDE( DataUpdateHDG& data ) const;
    template <typename ModelContextType>
    void updateLinearPDE( DataUpdateHDG & data, ModelContextType const& mfields ) const;

    void solvePostProcess();
    void updatePostPDE( DataUpdateHDG& data ) const;
    template <typename ModelContextType>
    void updatePostPDE( DataUpdateHDG & data, ModelContextType const& mfields ) const;

    //___________________________________________________________________________________//
    // export expressions
    //___________________________________________________________________________________//

    template <typename SymbExprType>
    auto exprPostProcessExports( SymbExprType const& se, std::string const& prefix = "" ) const
        {
            return this->materialsProperties()->exprPostProcessExports( this->mesh(),this->physicsAvailable(),se );
        }

    //___________________________________________________________________________________//
    // toolbox fields
    //___________________________________________________________________________________//

    auto modelFields( std::string const& prefix = "" ) const
        {
            return Feel::FeelModels::modelFields(
                modelField<FieldCtx::ID|FieldCtx::GRAD|FieldCtx::GRAD_NORMAL>( FieldTag::potential(this), prefix, M_physicMap.at("potentialK"), this->fieldPotential(), M_physicMap.at("potentialSymbol"), this->keyword() ),
                modelField<FieldCtx::ID|FieldCtx::GRAD|FieldCtx::GRAD_NORMAL>( FieldTag::postpotential(this), prefix, "post"+M_physicMap.at("potentialK"), this->fieldPostPotential(), M_physicMap.at("potentialSymbol")+"pp", this->keyword() ),
                modelField<FieldCtx::ID>( FieldTag::flux(this), prefix, M_physicMap.at("fluxK"), this->fieldFlux(), M_physicMap.at("fluxSymbol"), this->keyword() )
                                                 );
        }

    //___________________________________________________________________________________//
    // symbols expressions
    //___________________________________________________________________________________//

    template <typename ModelFieldsType>
    auto symbolsExpr( ModelFieldsType const& mfields ) const
        {
            // auto seToolbox = this->symbolsExprToolbox( mfields );
            auto seParam = this->symbolsExprParameter();
            auto seMat = this->materialsProperties()->symbolsExpr();
            auto seFields = mfields.symbolsExpr();
            return Feel::vf::symbolsExpr( /*seToolbox,*/ seParam, seMat, seFields );
        }
    auto symbolsExpr( std::string const& prefix = "" ) const { return this->symbolsExpr( this->modelFields( prefix ) ); }
#if 0
    template <typename ModelFieldsType>
    auto symbolsExprToolbox( ModelFieldsType const& mfields ) const
        {
            auto const& v = mfields.field( FieldTag::potential(this), M_physicMap.at("potentialK") );

            // generate symbol electric_matName_current_density
            typedef decltype( this->currentDensityExpr(v,"") ) _expr_currentdensity_type;
            std::vector<std::tuple<std::string,_expr_currentdensity_type,SymbolExprComponentSuffix>> currentDensitySymbs;
            for ( std::string const& matName : this->materialsProperties()->physicToMaterials( this->physic() ) )
            {
                std::string symbolcurrentDensityStr = prefixvm( this->keyword(), (boost::format("%1%_current_density") %matName).str(), "_");
                auto _currentDensityExpr = this->currentDensityExpr( v, matName );
                currentDensitySymbs.push_back( std::make_tuple( symbolcurrentDensityStr, _currentDensityExpr, SymbolExprComponentSuffix( nDim,1 ) ) );
            }

            return Feel::vf::symbolsExpr( symbolExpr( currentDensitySymbs ) );
        }
#endif

    // time step scheme
    std::string const& timeStepping() const { return M_timeStepping; }
    bdf_potential_ptrtype const& timeStepBdfPotential() const { return M_bdfPotential; }
    std::shared_ptr<TSBase> timeStepBase() { return this->timeStepBdfPotential(); }
    std::shared_ptr<TSBase> timeStepBase() const { return this->timeStepBdfPotential(); }
    void startTimeStep();
    void updateTimeStep();

    template <typename SymbolsExprType>
    void updateInitialConditions( SymbolsExprType const& se );
    //___________________________________________________________________________________//
};

template< typename ConvexType, int Order, int E_Order>
template <typename ModelFieldsType, typename SymbolsExpr, typename ExportsExprType>
void
MixedPoisson<ConvexType,Order, E_Order>::exportResults( double time, ModelFieldsType const& mfields, SymbolsExpr const& symbolsExpr, ExportsExprType const& exportsExpr )
{
    this->log("MixedPoisson","exportResults", "start");
    this->timerTool("PostProcessing").start();

    this->executePostProcessExports( M_exporter, time, mfields, symbolsExpr, exportsExpr );
    this->executePostProcessMeasures( time, mfields, symbolsExpr );
    this->executePostProcessSave( invalid_uint32_type_value, mfields );

    this->timerTool("PostProcessing").stop("exportResults");
    if ( this->scalabilitySave() )
    {
        if ( !this->isStationary() )
            this->timerTool("PostProcessing").setAdditionalParameter("time",this->currentTime());
        this->timerTool("PostProcessing").save();
    }
    this->log("MixedPoisson","exportResults", "finish");
}

template< typename ConvexType, int Order, int E_Order>
template <typename ModelFieldsType, typename SymbolsExpr>
void
MixedPoisson<ConvexType,Order, E_Order>::executePostProcessMeasures( double time, ModelFieldsType const& mfields, SymbolsExpr const& symbolsExpr )
{
    bool hasMeasure = false;
    bool hasMeasureNorm = this->updatePostProcessMeasuresNorm( this->mesh(), M_rangeMeshElements, symbolsExpr, mfields );
    bool hasMeasureStatistics = this->updatePostProcessMeasuresStatistics( this->mesh(), M_rangeMeshElements, symbolsExpr, mfields );
    bool hasMeasurePoint = this->updatePostProcessMeasuresPoint( M_measurePointsEvaluation, mfields );
    if ( hasMeasureNorm || hasMeasureStatistics || hasMeasurePoint )
        hasMeasure = true;

    if ( hasMeasure )
    {
        if ( !this->isStationary() )
            this->postProcessMeasuresIO().setMeasure( "time", time );
        this->postProcessMeasuresIO().exportMeasures();
        this->upload( this->postProcessMeasuresIO().pathFile() );
    }
}

template< typename ConvexType, int Order, int E_Order>
template <typename SymbolsExprType>
void
MixedPoisson<ConvexType,Order, E_Order>::updateInitialConditions( SymbolsExprType const& se )
{
    if ( !this->doRestart() )
    {
        std::vector<element_potential_ptrtype> icPotentialFields;
        if ( this->isStationary() )
            icPotentialFields = { this->fieldPotentialPtr() };
        else
            icPotentialFields = M_bdfPotential->unknowns();

        super_type::updateInitialConditions( M_potentialKey, M_rangeMeshElements, se, icPotentialFields );

        if ( Environment::vm().count( prefixvm(this->prefix(),"initial-solution.potential").c_str() ) )
        {
            auto myexpr = expr( soption(_prefix=this->prefix(),_name="initial-solution.potential"),
                                "",this->worldComm(),this->repository().expr() );
            icPotentialFields[0]->on(_range=M_rangeMeshElements,_expr=myexpr);
            for ( int k=1;k<icPotentialFields.size();++k )
                *icPotentialFields[k] = *icPotentialFields[0];
        }

        if ( !this->isStationary() )
            *this->fieldPotentialPtr() = M_bdfPotential->unknown(0);
    }
}

} // Namespace FeelModels

} // Namespace Feel

#include <feel/feelmodels/hdg/mixedpoissonassembly.hpp>

#endif
