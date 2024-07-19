#ifndef FEELPP_TOOLBOXES_FLUIDMECHANICS_ASSEMBLY_LINEAR_HPP
#define FEELPP_TOOLBOXES_FLUIDMECHANICS_ASSEMBLY_LINEAR_HPP 1

namespace Feel
{
namespace FeelModels
{

template< typename ConvexType, typename BasisVelocityType, typename BasisPressureType>
template <typename ModelContextType>
void
FluidMechanics<ConvexType,BasisVelocityType,BasisPressureType>::updateLinearPDE( DataUpdateLinear & data, ModelContextType const& mctx ) const
{
    const vector_ptrtype& vecCurrentPicardSolution = data.currentSolution();
    sparse_matrix_ptrtype& A = data.matrix();
    vector_ptrtype& F = data.rhs();
    bool _BuildCstPart = data.buildCstPart();

    bool BuildNonCstPart = !_BuildCstPart;
    bool BuildCstPart = _BuildCstPart;

    bool build_ConvectiveTerm = BuildNonCstPart;
    bool build_Form2TransientTerm = BuildNonCstPart;
    bool build_Form1TransientTerm = BuildNonCstPart;
    bool build_SourceTerm = BuildNonCstPart;
    bool build_StressTensorNonNewtonian = BuildNonCstPart;
    if ( this->timeStepBase()->strategy()==TS_STRATEGY_DT_CONSTANT )
    {
        build_Form2TransientTerm=BuildCstPart;
    }
    if (this->useFSISemiImplicitScheme())
    {
        build_StressTensorNonNewtonian = BuildCstPart;
        if ( this->useSemiImplicitTimeScheme() )
            build_ConvectiveTerm=BuildCstPart;
        build_Form2TransientTerm=BuildCstPart;
        build_Form1TransientTerm=BuildCstPart;
        build_SourceTerm=BuildCstPart;
    }

    std::string sc=(_BuildCstPart)?" (build cst part)":" (build non cst part)";
    this->log("FluidMechanics","updateLinearPDE", "start"+sc );
    this->timerTool("Solve").start();

    double timeSteppingScaling = 1.;
    if ( !this->isStationaryModel() )
    {
        if ( M_timeStepping == "Theta" )
            timeSteppingScaling = M_timeStepThetaValue;
        data.addDoubleInfo( prefixvm(this->prefix(),"time-stepping.scaling"), timeSteppingScaling );
    }

    //--------------------------------------------------------------------------------------------------//

    auto mesh = this->mesh();
    auto XhV = this->functionSpaceVelocity();
    auto XhP = this->functionSpacePressure();

    auto rowStartInMatrix = this->rowStartInMatrix();
    auto colStartInMatrix = this->colStartInMatrix();
    auto rowStartInVector = this->rowStartInVector();
    auto bilinearFormVV_PatternDefault = form2( _test=XhV,_trial=XhV,_matrix=A,
                                                _pattern=size_type(Pattern::DEFAULT),
                                                _rowstart=rowStartInMatrix+0,
                                                _colstart=colStartInMatrix+0 );
    auto bilinearFormVV = form2( _test=XhV,_trial=XhV,_matrix=A,
                                 _pattern=size_type(Pattern::COUPLED),
                                 _rowstart=rowStartInMatrix+0,
                                 _colstart=colStartInMatrix+0 );
    auto bilinearFormVP = form2( _test=XhV,_trial=XhP,_matrix=A,
                                 _pattern=size_type(Pattern::COUPLED),
                                 _rowstart=rowStartInMatrix+0,
                                 _colstart=colStartInMatrix+1 );
    auto bilinearFormPV = form2( _test=XhP,_trial=XhV,_matrix=A,
                                 _pattern=size_type(Pattern::COUPLED),
                                 _rowstart=rowStartInMatrix+1,
                                 _colstart=colStartInMatrix+0 );

    auto myLinearFormV =form1( _test=XhV, _vector=F,
                               _rowstart=rowStartInVector+0 );

    auto const& u = mctx.field( FieldTag::velocity(this), "velocity" );
    auto const& v = this->fieldVelocity();
    auto const& p = mctx.field( FieldTag::pressure(this), "pressure" );
    auto const& q = this->fieldPressure();

    auto const& beta_u = this->useSemiImplicitTimeScheme()? mctx.field( FieldTag::velocity_extrapolated(this), "velocity_extrapolated" ) : u;

    auto const& se = mctx.symbolsExpr();


    // strain tensor (trial)
    auto deft = sym(gradt(u));
    //auto deft = 0.5*gradt(u);

    // identity matrix
    auto const Id = eye<nDim,nDim>();

    //--------------------------------------------------------------------------------------------------//

    for ( auto const& [physicName,physicData] : this->physicsFromCurrentType() )
    {
        auto physicFluidData = std::static_pointer_cast<ModelPhysicFluid<nDim>>(physicData);
        for ( std::string const& matName : this->materialsProperties()->physicToMaterials( physicName ) )
        {
            auto const& range = this->materialsProperties()->rangeMeshElementsByMaterial( this->mesh(),matName );
            auto const& matProps = this->materialsProperties()->materialProperties( matName );

            // stress tensor sigma : grad(v)
            this->timerTool("Solve").start();
            if ( ( physicFluidData->dynamicViscosity().isNewtonianLaw() && BuildCstPart ) ||
                 ( !physicFluidData->dynamicViscosity().isNewtonianLaw() && build_StressTensorNonNewtonian ) )
            {
                if ( physicFluidData->equation() == "Navier-Stokes" )
                {
                    // TODO : not good with turbulence!!
                    auto myViscosity = Feel::FeelModels::fluidMecViscosity(gradv(beta_u),*physicFluidData,matProps,se);
                    bilinearFormVV +=
                        integrate( _range=range,
                                   _expr= timeSteppingScaling*2*myViscosity*inner(deft,grad(v)),
                                   _geomap=this->geomap() );
                }
                else
                {
                    // case with steady Stokes
                    CHECK( physicFluidData->dynamicViscosity().isNewtonianLaw() ) << "not allow with non newtonian law";

                    auto myViscosity = Feel::FeelModels::fluidMecViscosity( vf::zero<nDim,nDim>(),*physicFluidData,matProps,se);
                    bilinearFormVV +=
                        integrate( _range=range,
                                   _expr= timeSteppingScaling*2*myViscosity*inner(deft,grad(v)),
                                   _geomap=this->geomap() );
                }
            }
            if ( BuildCstPart )
            {
                bilinearFormVP +=
                    integrate( _range=range,
                               _expr= -div(v)*idt(p),
                               _geomap=this->geomap() );
            }
            double timeElapsedStressTensor = this->timerTool("Solve").stop();
            this->log("FluidMechanics","updateLinearPDE","assembly stress tensor + incompres in "+(boost::format("%1% s") %timeElapsedStressTensor).str() );


            //--------------------------------------------------------------------------------------------------//
            // convection
            if ( physicFluidData->equation() == "Navier-Stokes" && build_ConvectiveTerm )
            {
                this->timerTool("Solve").start();
                auto densityExpr = expr( matProps.property("density").template expr<1,1>(), se );
#if 0
                //velocityExprFromFields
                double myvelX=0;
                for ( auto const& [bpname,bpbc] : M_bodySetBC )
                {
                    myvelX = bpbc.fieldTranslationalVelocityPtr()->operator()( 0 );
                    break;
                }
                auto myVelXEXPR = vec( cst(myvelX), cst(0.) );
#endif
                auto convTerm = Feel::FeelModels::fluidMecConvectiveTermJacobian( u,physicFluidData,beta_u, true );
                //bilinearFormVV_PatternDefault +=
                bilinearFormVV +=
                    integrate( _range=range,
                               _expr= timeSteppingScaling*densityExpr*inner( convTerm, id(v) ),
                               _geomap=this->geomap() );

                if ( this->hasMeshMotion() )
                {
#if defined( FEELPP_MODELS_HAS_MESHALE )
                    bilinearFormVV_PatternDefault +=
                        integrate( _range=range,
                                   _expr= -timeSteppingScaling*densityExpr*inner( gradt(u)*( idv( this->meshMotionTool()->velocity() )   /*+ myVelXEXPR*/ ), id(v) ),
                                   _geomap=this->geomap() );
#endif
                }

#if 0
                if ( this->hasMeshMotion() )
                {
#if defined( FEELPP_MODELS_HAS_MESHALE )
                    bilinearFormVV_PatternDefault +=
                        integrate( _range=range,
                                   _expr= timeSteppingScaling*densityExpr*trans( gradt(u)*( idv(beta_u) -idv( this->meshMotionTool()->velocity() )   /*-  myVelXEXPR*/  ))*id(v),
                                   _geomap=this->geomap() );
#endif
                }
                else
                {
                    bilinearFormVV_PatternDefault +=
                        integrate( _range=range,
                                   _expr= timeSteppingScaling*densityExpr*trans( gradt(u)*idv(beta_u) )*id(v),
                                   _geomap=this->geomap() );
                }

                if ( this->doStabConvectionEnergy() )
                {
                    bilinearFormVV +=
                        integrate( _range=range,
                                   _expr= timeSteppingScaling*0.5*densityExpr*divt(u)*trans(idv(beta_u))*id(v),
                                   _geomap=this->geomap() );
                }
#endif
                double timeElapsedConvection = this->timerTool("Solve").stop();
                this->log("FluidMechanics","updateLinearPDE","assembly convection in "+(boost::format("%1% s") %timeElapsedConvection).str() );
            }
            else if ( (  /*physicFluidData->equation() == "Stokes" ||*/  physicFluidData->equation() == "StokesTransient")
                      && build_ConvectiveTerm && this->hasMeshMotion() )
            {
                auto densityExpr = expr( matProps.property("density").template expr<1,1>(), se );
#if defined( FEELPP_MODELS_HAS_MESHALE )
                bilinearFormVV_PatternDefault +=
                    integrate( _range=range,
                               _expr= -timeSteppingScaling*densityExpr*trans( gradt(u)*(idv( this->meshMotionTool()->velocity() )))*id(v),
                               _geomap=this->geomap() );
#endif
            }

            //--------------------------------------------------------------------------------------------------//
            //transients terms
            if ( !this->isStationary() && physicFluidData->equation() != "Stokes" )  //!this->isStationaryModel())
            {
                auto densityExpr = expr( matProps.property("density").template expr<1,1>(), se );
                if (build_Form2TransientTerm)
                {
                    bilinearFormVV_PatternDefault +=
                        integrate( _range=range,
                                   _expr= densityExpr*inner(idt(u),id(v))*M_bdfVelocity->polyDerivCoefficient(0),
                                   _geomap=this->geomap() );
                }

                if (build_Form1TransientTerm)
                {
                    auto buzz = M_bdfVelocity->polyDeriv();
                    myLinearFormV +=
                        integrate( _range=range,
                                   _expr= densityExpr*inner(idv(buzz),id(v)),
                                   _geomap=this->geomap() );
                }
            }


            //! gravity force
            if ( physicFluidData->gravityForceEnabled() && false )
            {
                auto const& gravityForce = physicFluidData->gravityForceExpr();
                bool assembleGravityTerm = gravityForce.expression().isNumericExpression()? BuildCstPart : BuildNonCstPart;
                if ( assembleGravityTerm )
                {
                    auto const& gravityForceExpr = expr( gravityForce,se );
                    auto densityExpr = expr( matProps.property("density").template expr<1,1>(), se );
                    myLinearFormV +=
                        integrate( _range=range,
                                   _expr= timeSteppingScaling*densityExpr*inner(gravityForceExpr,id(v)),
                                   _geomap=this->geomap() );
                }
            }

            //--------------------------------------------------------------------------------------------------//
            // div u != 0
            if (!this->velocityDivIsEqualToZero() && BuildNonCstPart)
            {
                auto myLinearFormP =form1( _test=XhP, _vector=F,
                                           _rowstart=rowStartInVector+1 );
                myLinearFormP +=
                    integrate( _range=range,
                               _expr= -idv(this->velocityDiv())*id(q),
                               _geomap=this->geomap() );

                auto muExpr = expr( matProps.property("dynamic-viscosity").template expr<1,1>(), se );
                auto coeffDiv = (2./3.)*muExpr;
                myLinearFormV +=
                    integrate( _range=range,
                               _expr= val(timeSteppingScaling*coeffDiv*gradv(this->velocityDiv()))*id(v),
                               _geomap=this->geomap() );

                if ( this->doStabConvectionEnergy() )
                {
                    auto densityExpr = expr( matProps.property("density").template expr<1,1>(), se );
                    //auto const& betaU = *fieldVelocityPressureExtrapolated;
                    myLinearFormV +=
                        integrate( _range=range,
                                   _expr= timeSteppingScaling*0.5*densityExpr*idv(this->velocityDiv())*trans(idv(beta_u))*id(v),
                                   _geomap=this->geomap() );
                }
            }


            //--------------------------------------------------------------------------------------------------//
            // apply turbulent wall function : wall shear stress bc
            if ( physicFluidData->turbulence().isEnabled() && (physicFluidData->turbulence().model() == "k-epsilon" )  && BuildNonCstPart )
            {
                auto frictionVelocityExpr = physicFluidData->turbulence().frictionVelocityWallFunctionExpr( matName, se );
                auto u_plusExpr = cst(11.06); // TODO
                auto densityExpr = expr( matProps.property("density").template expr<1,1>(), se );
                //auto expr_turbulence_wst = expr( expr( "physics_fluid_fluid_fluid_Omega_turbulence_k_epsilon_u_tauBC_V2/11.06:physics_fluid_fluid_fluid_Omega_turbulence_k_epsilon_u_tauBC_V2" ) , se );
                for ( auto const& [bcName,bcWall] : M_turbulenceModelBoundaryConditions.wall() )
                {
                    bilinearFormVV_PatternDefault +=
                        integrate( _range=markedfaces(this->mesh(), bcWall.markers() /* {"Gamma1","Gamma3"}*/ ),
                                   //_expr= timeSteppingScaling*expr_turbulence_wst*inner( idt(u),id(v) ),
                                   _expr=timeSteppingScaling*densityExpr*(frictionVelocityExpr/u_plusExpr)*inner( idt(u),id(v) ),
                                   _geomap=this->geomap() );
                }
            }

            //--------------------------------------------------------------------------------------------------//
            if ( M_stabilizationGLS && M_stabilizationGLSDoAssembly )
            {
                this->updateLinearPDEStabilizationGLS( data, mctx, *physicFluidData, matProps, range );
            }


        } // foreach material

        // incompressibility term
        if ( BuildCstPart )
        {
            this->timerTool("Solve").start();
            bilinearFormPV +=
                integrate( _range=M_rangeMeshElements,
                           _expr= -divt(u)*id(q),
                           _geomap=this->geomap() );
            double timeElapsedIncomp = this->timerTool("Solve").stop();
            this->log("FluidMechanics","updateLinearPDE","assembly incompressibility term in "+(boost::format("%1% s") %timeElapsedIncomp).str() );
        }

    } //  for ( auto const& [physicName,physicData] : this->physicsFromCurrentType() )

    //--------------------------------------------------------------------------------------------------//
    // body forces
#if 0 // VINCENT
    if ( this->M_overwritemethod_updateSourceTermLinearPDE != NULL )
    {
        this->M_overwritemethod_updateSourceTermLinearPDE(F,BuildCstPart);
    }
    else
    {
        if ( build_SourceTerm )
        {
            for( auto const& d : this->M_volumicForcesProperties )
            {
                auto rangeBodyForceUsed = ( markers(d).empty() )? M_rangeMeshElements : markedelements(this->mesh(),markers(d));
                myLinearFormV +=
                    integrate( _range=rangeBodyForceUsed,
                               _expr= timeSteppingScaling*inner( expression(d,se),id(v) ),
                               _geomap=this->geomap() );
            }
        }
    }

    // source given by user
    if ( M_haveSourceAdded && BuildNonCstPart)
    {
        myLinearFormV +=
            integrate( _range=M_rangeMeshElements,
                       _expr= timeSteppingScaling*trans(idv(*M_SourceAdded))*id(v),
                       _geomap=this->geomap() );
    }
#endif

    //--------------------------------------------------------------------------------------------------//
    // define pressure cst
    if ( this->definePressureCst() )
    {
        if ( this->definePressureCstMethod() == "penalisation" && BuildCstPart  )
        {
            double beta = this->definePressureCstPenalisationBeta();
            auto bilinearFormPP = form2( _test=XhP,_trial=XhP,_matrix=A,
                                         _pattern=size_type(Pattern::COUPLED),
                                         _rowstart=rowStartInMatrix+1,
                                         _colstart=colStartInMatrix+1 );
            for ( auto const& rangeElt : M_definePressureCstMeshRanges )
                bilinearFormPP +=
                    integrate( _range=rangeElt,
                               _expr=beta*idt(p)*id(q),
                               _geomap=this->geomap() );
        }
        if ( this->definePressureCstMethod() == "lagrange-multiplier" )
        {
            CHECK( this->hasStartSubBlockSpaceIndex("define-pressure-cst-lm") ) << " start dof index for define-pressure-cst-lm is not present\n";
            size_type startBlockIndexDefinePressureCstLM = this->startSubBlockSpaceIndex("define-pressure-cst-lm");

            if (BuildCstPart)
            {
                for ( int k=0;k<M_XhMeanPressureLM.size();++k )
                {
                    auto lambda = M_XhMeanPressureLM[k]->element();
                    form2( _test=XhP, _trial=M_XhMeanPressureLM[k], _matrix=A,
                           _rowstart=this->rowStartInMatrix()+1,
                           _colstart=this->colStartInMatrix()+startBlockIndexDefinePressureCstLM+k ) +=
                        integrate( _range=M_definePressureCstMeshRanges[k],
                                   _expr= id(p)*idt(lambda) /*+ idt(p)*id(lambda)*/,
                                   _geomap=this->geomap() );

                    form2( _test=M_XhMeanPressureLM[k], _trial=XhP, _matrix=A,
                           _rowstart=this->rowStartInMatrix()+startBlockIndexDefinePressureCstLM+k,
                           _colstart=this->colStartInMatrix()+1 ) +=
                        integrate( _range=M_definePressureCstMeshRanges[k],
                                   _expr= + idt(p)*id(lambda),
                                   _geomap=this->geomap() );
                }
            }

#if defined(FLUIDMECHANICS_USE_LAGRANGEMULTIPLIER_MEANPRESSURE)
            if (BuildNonCstPart)
            {
                this->log("FluidMechanics","updateLinearPDE", "also add nonzero MEANPRESSURE" );
                for ( int k=0;k<M_XhMeanPressureLM.size();++k )
                {
                    auto lambda = M_XhMeanPressureLM[k]->element();
                    form1( _test=M_XhMeanPressureLM[k], _vector=F,
                           _rowstart=this->rowStartInMatrix()+startBlockIndexDefinePressureCstLM+k ) +=
                        integrate( _range=M_definePressureCstMeshRanges[k],
                                   _expr= FLUIDMECHANICS_USE_LAGRANGEMULTIPLIER_MEANPRESSURE(this->shared_from_this())*id(lambda),
                                   _geomap=this->geomap() );
                }
            }
#endif
        } // if ( this->definePressureCstMethod() == "lagrange-multiplier" )
    } // if ( this->definePressureCst() )


    //--------------------------------------------------------------------------------------------------//
    //--------------------------------------------------------------------------------------------------//
    //--------------------------------------------------------------------------------------------------//
#if 0 // VINCENT


    if (BuildNonCstPart && !this->markerSlipBC().empty() )
    {
        auto muExpr = this->dynamicViscosityExpr( se );
        auto densityExpr = this->materialsProperties()->template materialPropertyExpr<1,1>( "density", se );

        auto P = Id-N()*trans(N());
        double gammaN = doption(_name="bc-slip-gammaN",_prefix=this->prefix());
        double gammaTau = doption(_name="bc-slip-gammaTau",_prefix=this->prefix());
        auto const& betaExtrapolate = M_bdfVelocity->poly();
        auto beta = vf::project( _space=XhV,
                                 _range=boundaryfaces(mesh),
                                 _expr=densityExpr*idv(betaExtrapolate) );
        //auto beta = Beta.element<0>();
        auto Cn = gammaN*max(abs(trans(idv(beta))*N()),muExpr/vf::h());
        auto Ctau = gammaTau*muExpr/vf::h() + max( -trans(idv(beta))*N(),cst(0.) );

        bilinearFormVV +=
            integrate( _range= markedfaces(mesh,this->markerSlipBC()),
                       _expr= timeSteppingScaling*val(Cn)*(trans(idt(u))*N())*(trans(id(v))*N())+
                       timeSteppingScaling*val(Ctau)*trans(idt(u))*id(v),
                       //+ trans(idt(p)*Id*N())*id(v)
                       //- trans(id(v))*N()* trans(2*muExpr*deft*N())*N()
                       _geomap=this->geomap()
                       );
    }
#endif
    //--------------------------------------------------------------------------------------------------//

    if ( M_boundaryConditions->hasVelocityImposedLagrangeMultiplier() )
    {
        CHECK( this->hasStartSubBlockSpaceIndex("dirichletlm") ) << " start dof index for dirichletlm is not present\n";
        size_type startBlockIndexDirichletLM = this->startSubBlockSpaceIndex("dirichletlm");
        auto lambdaBC = this->XhDirichletLM()->element();
        if (BuildCstPart)
        {
            std::set<std::string> allmarkers;
            for ( auto const& [bcId,bcData] : M_boundaryConditions->velocityImposedLagrangeMultiplier() )
                allmarkers.insert( bcData->markers().begin(), bcData->markers().end() );

            form2( _test=XhV,_trial=this->XhDirichletLM(),_matrix=A,_pattern=size_type(Pattern::COUPLED),
                   _rowstart=rowStartInMatrix,
                   _colstart=colStartInMatrix+startBlockIndexDirichletLM ) +=
                integrate( _range=elements(this->meshDirichletLM()),
                           _expr= inner( idt(lambdaBC),id(u) ) );

            form2( _test=this->XhDirichletLM(),_trial=XhV,_matrix=A,_pattern=size_type(Pattern::COUPLED),
                   _rowstart=rowStartInMatrix+startBlockIndexDirichletLM,
                   _colstart=colStartInMatrix ) +=
                integrate( _range=elements(this->meshDirichletLM()),
                           _expr= inner( idt(u),id(lambdaBC) ) );
        }
        if ( BuildNonCstPart )
        {
            for ( auto const& [bcId,bcData] : M_boundaryConditions->velocityImposedLagrangeMultiplier() )
            {
                form1( _test=this->XhDirichletLM(),_vector=F,
                       _rowstart=this->rowStartInVector()+startBlockIndexDirichletLM ) +=
                    integrate( _range=markedfaces(this->mesh(),bcData->markers()),
                               _expr= inner( bcData->expr(se),id(lambdaBC) ),
                               _geomap=this->geomap() );
            }
        }
    }

    //--------------------------------------------------------------------------------------------------//
    // weak formulation of the boundaries conditions
    if ( M_boundaryConditions->hasVelocityImposedNitsche() )
    {
        if ( BuildCstPart)
        {
            std::set<std::string> allmarkers;
            for ( auto const& [bcId,bcData] : M_boundaryConditions->velocityImposedNitsche() )
                allmarkers.insert( bcData->markers().begin(), bcData->markers().end() );

            //auto viscousStressTensor = 2*idv(mu)*deft;
            auto viscousStressTensor = 2*this->dynamicViscosityExpr( se )*deft;
            bilinearFormVV +=
                integrate( _range=markedfaces(mesh,allmarkers),
                           _expr= -timeSteppingScaling*inner(viscousStressTensor*N(),id(v) )
                           /**/   + timeSteppingScaling*this->dirichletBCnitscheGamma()*inner(idt(u),id(v))/hFace(),
                           _geomap=this->geomap() );
            bilinearFormVP +=
                integrate( _range=markedfaces(mesh,allmarkers),
                           _expr= timeSteppingScaling*inner( idt(p)*N(), id(v) ),
                           _geomap=this->geomap() );
        }
        if ( BuildNonCstPart)
        {
            for ( auto const& [bcId,bcData] : M_boundaryConditions->velocityImposedNitsche() )
            {
                myLinearFormV +=
                    integrate( _range=markedfaces(this->mesh(),bcData->markers()),
                               _expr= timeSteppingScaling*this->dirichletBCnitscheGamma()*inner( bcData->expr(se),id(v) )/hFace(),
                               _geomap=this->geomap() );
            }
        }
    }

    //--------------------------------------------------------------------------------------------------//

    // normal stress bc
    bool build_BoundaryNeumannTerm = BuildNonCstPart;
    if ( this->useFSISemiImplicitScheme() )
    {
        build_BoundaryNeumannTerm = BuildCstPart;
    }
    if ( build_BoundaryNeumannTerm )
    {
        for ( auto const& [bcId,bcData] : M_boundaryConditions->normalStress() )
        {
            if ( bcData->isScalarExpr() )
            {
                auto normalStessExpr = bcData->exprScalar( se );
                myLinearFormV +=
                    integrate( _range=markedfaces(this->mesh(),bcData->markers()),
                               _expr= timeSteppingScaling*normalStessExpr*inner( N(),id(v) ),
                               _geomap=this->geomap() );
            }
            else if ( bcData->isVectorialExpr() )
            {
                auto normalStessExpr = bcData->exprVectorial( se );
                myLinearFormV +=
                    integrate( _range=markedfaces(this->mesh(),bcData->markers()),
                               _expr= timeSteppingScaling*inner( normalStessExpr,id(v) ),
                               _geomap=this->geomap() );
            }
            else if ( bcData->isMatrixExpr() )
            {
                auto normalStessExpr = bcData->exprMatrix( se );
                myLinearFormV +=
                    integrate( _range=markedfaces(this->mesh(),bcData->markers()),
                               _expr= timeSteppingScaling*inner( normalStessExpr*N(),id(v) ),
                               _geomap=this->geomap() );
            }
        }
    }
    //--------------------------------------------------------------------------------------------------//

    if ( !M_boundaryConditions->pressureImposed().empty() )
    {
        CHECK( this->hasStartSubBlockSpaceIndex("pressurelm1") ) << " start dof index for pressurelm1 is not present\n";
        size_type startBlockIndexPressureLM1 = this->startSubBlockSpaceIndex("pressurelm1");
        if ( BuildCstPart )
        {
            std::set<std::string> allmarkers;
            for ( auto const& [bcName,bcData] : M_boundaryConditions->pressureImposed() )
                allmarkers.insert( bcData->markers().begin(), bcData->markers().end() );
            auto rangeFacesPressureBC = markedfaces( this->mesh(),allmarkers );

            if ( nDim == 2 )
            {
                form2( _test=XhV,_trial=M_spaceLagrangeMultiplierPressureBC,_matrix=A,_pattern=size_type(Pattern::COUPLED),
                       _rowstart=rowStartInMatrix,
                       _colstart=colStartInMatrix+startBlockIndexPressureLM1 ) +=
                    integrate( _range=rangeFacesPressureBC,
                               _expr=-timeSteppingScaling*trans(cross(id(u),N()))(0,0)*idt(M_fieldLagrangeMultiplierPressureBC1),
                               _geomap=this->geomap() );

                form2( _test=M_spaceLagrangeMultiplierPressureBC,_trial=XhV,_matrix=A,_pattern=size_type(Pattern::COUPLED),
                       _rowstart=rowStartInMatrix+startBlockIndexPressureLM1,
                       _colstart=colStartInMatrix ) +=
                    integrate( _range=rangeFacesPressureBC,
                               _expr=-trans(cross(idt(u),N()))(0,0)*id(M_fieldLagrangeMultiplierPressureBC1),
                               _geomap=this->geomap() );
            }
            else if ( nDim == 3 )
            {
                auto alpha = 1./sqrt(1-Nz()*Nz());
                form2( _test=XhV,_trial=M_spaceLagrangeMultiplierPressureBC,_matrix=A,_pattern=size_type(Pattern::COUPLED),
                       _rowstart=rowStartInMatrix,
                       _colstart=colStartInMatrix+startBlockIndexPressureLM1 ) +=
                    integrate( _range=rangeFacesPressureBC,
                               _expr=-timeSteppingScaling*trans(cross(id(u),N()))(0,2)*idt(M_fieldLagrangeMultiplierPressureBC1)*alpha,
                               _geomap=this->geomap() );

                form2( _test=M_spaceLagrangeMultiplierPressureBC,_trial=XhV,_matrix=A,_pattern=size_type(Pattern::COUPLED),
                       _rowstart=rowStartInMatrix+startBlockIndexPressureLM1,
                       _colstart=colStartInMatrix ) +=
                    integrate( _range=rangeFacesPressureBC,
                               _expr=-trans(cross(idt(u),N()))(0,2)*id(M_fieldLagrangeMultiplierPressureBC1)*alpha,
                               _geomap=this->geomap() );

                CHECK( this->hasStartSubBlockSpaceIndex("pressurelm2") ) << " start dof index for pressurelm2 is not present\n";
                size_type startBlockIndexPressureLM2 = this->startSubBlockSpaceIndex("pressurelm2");

                form2( _test=XhV,_trial=M_spaceLagrangeMultiplierPressureBC,_matrix=A,_pattern=size_type(Pattern::COUPLED),
                       _rowstart=rowStartInMatrix,
                       _colstart=colStartInMatrix+startBlockIndexPressureLM2 ) +=
                    integrate( _range=rangeFacesPressureBC,
                               _expr= -timeSteppingScaling*trans(cross(id(u),N()))(0,0)*alpha*idt(M_fieldLagrangeMultiplierPressureBC2)*Ny()
                               +timeSteppingScaling*trans(cross(id(u),N()))(0,1)*alpha*idt(M_fieldLagrangeMultiplierPressureBC2)*Nx(),
                               _geomap=this->geomap() );

                form2( _test=M_spaceLagrangeMultiplierPressureBC,_trial=XhV,_matrix=A,_pattern=size_type(Pattern::COUPLED),
                       _rowstart=rowStartInMatrix+startBlockIndexPressureLM2,
                       _colstart=colStartInMatrix ) +=
                    integrate( _range=rangeFacesPressureBC,
                               _expr= -trans(cross(idt(u),N()))(0,0)*alpha*id(M_fieldLagrangeMultiplierPressureBC2)*Ny()
                               +trans(cross(idt(u),N()))(0,1)*alpha*id(M_fieldLagrangeMultiplierPressureBC2)*Nx(),
                               _geomap=this->geomap() );
            }
        }
        if ( BuildNonCstPart )
        {
            for ( auto const& [bcName,bcData] : M_boundaryConditions->pressureImposed() )
            {
                myLinearFormV +=
                    integrate( _range=markedfaces(this->mesh(),bcData->markers()),
                               _expr= -timeSteppingScaling*bcData->expr(se)*trans(N())*id(v),
                               _geomap=this->geomap() );
            }
        }

    }

    //--------------------------------------------------------------------------------------------------//


    int indexOutletWindkessel = 0;
    for ( auto const& [bcName,bcData] : M_boundaryConditions->outletWindkessel() )
    {
        if ( bcData->useImplicitCoupling() )
        {
            if ( BuildNonCstPart )
            {
                size_type startBlockIndexWindkessel = this->startSubBlockSpaceIndex("windkessel");

                bool hasWindkesselActiveDof = M_fluidOutletWindkesselSpace->nLocalDofWithoutGhost() > 0;
                int blockStartWindkesselRow = rowStartInMatrix + startBlockIndexWindkessel + 2*indexOutletWindkessel;
                int blockStartWindkesselCol = colStartInMatrix + startBlockIndexWindkessel + 2*indexOutletWindkessel;
                int blockStartWindkesselVec = rowStartInVector + startBlockIndexWindkessel + 2*indexOutletWindkessel;
                auto const& basisToContainerGpPressureDistalRow = A->mapRow().dofIdToContainerId( blockStartWindkesselRow );
                auto const& basisToContainerGpPressureDistalCol = A->mapCol().dofIdToContainerId( blockStartWindkesselCol );
                auto const& basisToContainerGpPressureDistalVec = F->map().dofIdToContainerId( blockStartWindkesselVec );
                auto const& basisToContainerGpPressureProximalRow = A->mapRow().dofIdToContainerId( blockStartWindkesselRow+1 );
                auto const& basisToContainerGpPressureProximalCol = A->mapCol().dofIdToContainerId( blockStartWindkesselCol+1 );
                auto const& basisToContainerGpPressureProximalVec = F->map().dofIdToContainerId( blockStartWindkesselVec+1 );
                if ( hasWindkesselActiveDof )
                    CHECK( !basisToContainerGpPressureDistalRow.empty() && !basisToContainerGpPressureDistalCol.empty() &&
                           !basisToContainerGpPressureProximalRow.empty() && !basisToContainerGpPressureProximalCol.empty() &&
                           !basisToContainerGpPressureDistalVec.empty() && !basisToContainerGpPressureProximalVec.empty() ) << "incomplete datamap info";
                const size_type gpPressureDistalRow = (hasWindkesselActiveDof)? basisToContainerGpPressureDistalRow[0] : 0;
                const size_type gpPressureDistalCol = (hasWindkesselActiveDof)? basisToContainerGpPressureDistalCol[0] : 0;
                const size_type gpPressureDistalVec = (hasWindkesselActiveDof)? basisToContainerGpPressureDistalVec[0] : 0;
                const size_type gpPressureProximalRow = (hasWindkesselActiveDof)? basisToContainerGpPressureProximalRow[0] : 0;
                const size_type gpPressureProximalCol = (hasWindkesselActiveDof)? basisToContainerGpPressureProximalCol[0] : 0;

                // windkessel parameters
                double Rd = bcData->expr_Rd( se ).evaluate()(0,0);
                double Rp = bcData->expr_Rp( se ).evaluate()(0,0);
                double Cd = bcData->expr_Cd( se ).evaluate()(0,0);

                auto const& windkesselData = M_fluidOutletWindkesselData.at(bcName);
                auto pDistal = std::get<0>( windkesselData ).at(0);
                auto pProximal = std::get<0>( windkesselData ).at(1);
                auto bdfDistal = std::get<1>( windkesselData ).at(0);

                auto rhsTimeDerivDistal = bdfDistal->polyDeriv();
                auto rangeFaceFluidOutlet = markedfaces(mesh,bcData->markers());

                F->setIsClosed( false );
                A->setIsClosed( false );

                // first equation
                if ( hasWindkesselActiveDof )
                {
                    A->add( gpPressureDistalRow,gpPressureDistalCol,
                            Cd*bdfDistal->polyDerivCoefficient(0)+1./Rd );
                }

                form2( _test=M_fluidOutletWindkesselSpace,_trial=XhV,_matrix=A,
                       _rowstart=blockStartWindkesselRow,
                       _colstart=colStartInMatrix ) +=
                    integrate( _range=rangeFaceFluidOutlet,
                               _expr=-inner(idt(u),N())*id(pDistal) );

                if ( hasWindkesselActiveDof )
                {
                    F->add( gpPressureDistalVec, Cd*rhsTimeDerivDistal(0) );
                }

                // second equation
                if ( hasWindkesselActiveDof )
                {
                    A->add( gpPressureProximalRow, gpPressureProximalCol,  1.);

                    A->add( gpPressureProximalRow, gpPressureDistalCol, -1.);
                }

                // really correct?
                form2( _test=M_fluidOutletWindkesselSpace,_trial=XhV,_matrix=A,
                       _rowstart=blockStartWindkesselRow+1,
                       _colstart=colStartInMatrix )+=
                    integrate( _range=rangeFaceFluidOutlet,
                               _expr=-Rp*inner(idt(u),N())*id(pProximal) );

                // coupling with fluid model
                form2( _test=XhV, _trial=M_fluidOutletWindkesselSpace, _matrix=A,
                       _rowstart=rowStartInMatrix,
                       _colstart=blockStartWindkesselCol+1 ) +=
                    integrate( _range=rangeFaceFluidOutlet,
                               _expr= timeSteppingScaling*idt(pProximal)*inner(N(),id(v)),
                               _geomap=this->geomap() );
            }
        }
        else
        {
            if ( BuildNonCstPart )
            {
                auto const& beta = M_bdfVelocity->poly();
                // windkessel parameters
                double Rd = bcData->expr_Rd( se ).evaluate()(0,0);
                double Rp = bcData->expr_Rp( se ).evaluate()(0,0);
                double Cd = bcData->expr_Cd( se ).evaluate()(0,0);

                auto const& windkesselData = M_fluidOutletWindkesselData.at(bcName);
                auto pDistal = std::get<0>( windkesselData ).at(0);
                auto pProximal = std::get<0>( windkesselData ).at(1);
                auto bdfDistal = std::get<1>( windkesselData ).at(0);

                auto rangeFaceFluidOutlet = markedfaces(mesh,bcData->markers());

                auto outletQ = integrate(_range=rangeFaceFluidOutlet,
                                         _expr=trans(idv(beta))*N() ).evaluate()(0,0);


                auto rhsTimeDerivDistal = bdfDistal->polyDeriv();

                double denum = Rd*Cd*bdfDistal->polyDerivCoefficient(0) + 1;
                pDistal->setConstant( Rd*outletQ/denum );
                pDistal->add( Rd*Cd/denum, rhsTimeDerivDistal );

                pProximal->setConstant( Rp*outletQ );
                pProximal->add( 1., *pDistal );

                myLinearFormV +=
                    integrate( _range=rangeFaceFluidOutlet,
                               //_expr= -timeSteppingScaling*M_fluidOutletWindkesselPressureProximal[k]*trans(N())*id(v),
                               _expr= -timeSteppingScaling*idv(pProximal)*inner(N(),id(v)),
                               _geomap=this->geomap() );
            }
        }
        ++indexOutletWindkessel;
    }


    //--------------------------------------------------------------------------------------------------//

    if ( !M_bodySetBC.empty() )
    {
        this->log("FluidMechanics","updateLinearPDE","assembly of body bc");

        for ( auto const& [bpname,bpbc] : M_bodySetBC )
        {
            //CHECK( this->hasStartSubBlockSpaceIndex("body-bc.translational-velocity") ) << " start dof index for body-bc.translational-velocity is not present\n";
            //CHECK( this->hasStartSubBlockSpaceIndex("body-bc.angular-velocity") ) << " start dof index for body-bc.angular-velocity is not present\n";
            size_type startBlockIndexTranslationalVelocity = this->startSubBlockSpaceIndex("body-bc."+bpbc.name()+".translational-velocity");
            bool hasActiveDofTranslationalVelocity = bpbc.spaceTranslationalVelocity()->nLocalDofWithoutGhost() > 0;
            double massBody = bpbc.body().mass();
            if ( BuildCstPart)
            {
                A->setIsClosed( false );
                if ( hasActiveDofTranslationalVelocity )
                {
                    auto const& basisToContainerGpTranslationalVelocityRow = A->mapRow().dofIdToContainerId( rowStartInMatrix+startBlockIndexTranslationalVelocity );
                    auto const& basisToContainerGpTranslationalVelocityCol = A->mapCol().dofIdToContainerId( colStartInMatrix+startBlockIndexTranslationalVelocity );
                    for (int d=0;d<nDim;++d)
                    {
                        A->add( basisToContainerGpTranslationalVelocityRow[d], basisToContainerGpTranslationalVelocityCol[d],
                                bpbc.bdfTranslationalVelocity()->polyDerivCoefficient(0)*massBody );
                    }
                }
            }

            if ( BuildNonCstPart )
            {
                F->setIsClosed( false );
                if ( hasActiveDofTranslationalVelocity )
                {
                    auto const& basisToContainerGpTranslationalVelocityVector = F->map().dofIdToContainerId( rowStartInVector+startBlockIndexTranslationalVelocity );
                    auto translationalVelocityPolyDeriv = bpbc.bdfTranslationalVelocity()->polyDeriv();
                    for (int d=0;d<nDim;++d)
                    {
                        F->add( basisToContainerGpTranslationalVelocityVector[d],
                                massBody*translationalVelocityPolyDeriv(d) );

                        if ( bpbc.gravityForceEnabled() )
                        {
                            F->add( basisToContainerGpTranslationalVelocityVector[d],
                                    bpbc.gravityForceWithMass()(d) );
                        }
                    }

                }
            }

            if ( !bpbc.isInNBodyArticulated() || ( bpbc.getNBodyArticulated().masterBodyBC().name() == bpbc.name() ) )
            {
                size_type startBlockIndexAngularVelocity = this->startSubBlockSpaceIndex("body-bc."+bpbc.name()+".angular-velocity");
                //auto momentOfInertiaExpr = bpbc.momentOfInertiaExpr();
                auto const& momentOfInertia = bpbc.momentOfInertia_inertialFrame();
                int nLocalDofAngularVelocity = bpbc.spaceAngularVelocity()->nLocalDofWithoutGhost();
                bool hasActiveDofAngularVelocity = nLocalDofAngularVelocity > 0;
                if ( BuildNonCstPart/*BuildCstPart*/ )
                {
                    A->setIsClosed( false );
                    if ( hasActiveDofAngularVelocity )
                    {
                        typename Body::moment_of_inertia_type termWithTimeDerivativeOfMomentOfInertia;
                        if constexpr ( nDim == 2 )
                            termWithTimeDerivativeOfMomentOfInertia = bpbc.timeDerivativeOfMomentOfInertia_bodyFrame(this->timeStep());
                        else
                        {
                            auto rotationMat = bpbc.rigidRotationMatrix();
                            termWithTimeDerivativeOfMomentOfInertia = rotationMat*bpbc.timeDerivativeOfMomentOfInertia_bodyFrame(this->timeStep())*(rotationMat.transpose());
                        }

                        auto const& basisToContainerGpAngularVelocityRow = A->mapRow().dofIdToContainerId( rowStartInMatrix+startBlockIndexAngularVelocity );
                        auto const& basisToContainerGpAngularVelocityCol = A->mapCol().dofIdToContainerId( colStartInMatrix+startBlockIndexAngularVelocity );
                        //auto timeDerivativeOfMomentOfInertia = bpbc.timeDerivativeOfMomentOfInertia(this->timeStep());
                        for (int i=0;i<nLocalDofAngularVelocity;++i)
                        {
                            for (int j=0;j<nLocalDofAngularVelocity;++j)
                            {
                                double val = bpbc.bdfAngularVelocity()->polyDerivCoefficient(0)*momentOfInertia(i,j);
                                val += termWithTimeDerivativeOfMomentOfInertia(i,j);
                                A->add( basisToContainerGpAngularVelocityRow[i], basisToContainerGpAngularVelocityCol[j], val );
                            }
                        }
                        if constexpr ( nDim == 3 )
                        {
                            auto const& uAngularVelocity = mctx.field( BodyBoundaryCondition::FieldTag::angular_velocity(&bpbc), "angular-velocity" );
                            auto w_eval = idv(uAngularVelocity).evaluate(false);
                            auto Iw_eval = momentOfInertia*w_eval;
                            //  w curl I w
                            A->add( basisToContainerGpAngularVelocityRow[0], basisToContainerGpAngularVelocityCol[1], Iw_eval(2) );
                            A->add( basisToContainerGpAngularVelocityRow[0], basisToContainerGpAngularVelocityCol[2], -Iw_eval(1) );
                            A->add( basisToContainerGpAngularVelocityRow[1], basisToContainerGpAngularVelocityCol[0], -Iw_eval(2) );
                            A->add( basisToContainerGpAngularVelocityRow[1], basisToContainerGpAngularVelocityCol[2], Iw_eval(0) );
                            A->add( basisToContainerGpAngularVelocityRow[2], basisToContainerGpAngularVelocityCol[0], Iw_eval(1) );
                            A->add( basisToContainerGpAngularVelocityRow[2], basisToContainerGpAngularVelocityCol[1], -Iw_eval(0) );
                        }
                    }
                }


                if ( BuildNonCstPart )
                {
                    F->setIsClosed( false );
                    if ( hasActiveDofAngularVelocity )
                    {
                        auto const& basisToContainerGpAngularVelocityVector = F->map().dofIdToContainerId( rowStartInVector+startBlockIndexAngularVelocity );
                        auto angularVelocityPolyDeriv = bpbc.bdfAngularVelocity()->polyDeriv();
                        //auto contribRhsAngularVelocity = (momentOfInertiaExpr*idv(angularVelocityPolyDeriv)).evaluate(false);
                        auto contribRhsAngularVelocity = momentOfInertia*(idv(angularVelocityPolyDeriv).evaluate(false));
                        for (int i=0;i<nLocalDofAngularVelocity;++i)
                        {
                            F->add( basisToContainerGpAngularVelocityVector[i],
                                    //momentOfInertia(0,0)*angularVelocityPolyDeriv(i)
                                    contribRhsAngularVelocity(i,0)
                                    );
                        }
                    }
                }
            }

        } //  for ( auto const& [bpname,bpbc] : M_bodySetBC )


        for ( auto const& nba : M_bodySetBC.nbodyArticulated() )
        {
            if ( nba.articulationMethod() != "lm" )
                continue;

            for ( auto const& ba : nba.articulations() )
            {
                auto const& bbc1 = ba.body1();
                auto const& bbc2 = ba.body2();
                size_type startBlockIndexTranslationalVelocityBody1 = this->startSubBlockSpaceIndex("body-bc."+bbc1.name()+".translational-velocity");
                size_type startBlockIndexTranslationalVelocityBody2 = this->startSubBlockSpaceIndex("body-bc."+bbc2.name()+".translational-velocity");
                bool hasActiveDofTranslationalVelocityBody1 = bbc1.spaceTranslationalVelocity()->nLocalDofWithoutGhost() > 0;
                bool hasActiveDofTranslationalVelocityBody2 = bbc2.spaceTranslationalVelocity()->nLocalDofWithoutGhost() > 0;
                size_type startBlockIndexArticulationLMTranslationalVelocity = this->startSubBlockSpaceIndex( "body-bc.articulation-lm."+ba.name()+".translational-velocity");
                if ( BuildCstPart )
                {
                    A->setIsClosed( false );
                    if ( hasActiveDofTranslationalVelocityBody1 )
                    {
                        auto const& basisToContainerGpTranslationalVelocityBody1Row = A->mapRow().dofIdToContainerId( rowStartInMatrix+startBlockIndexTranslationalVelocityBody1 );
                        auto const& basisToContainerGpTranslationalVelocityBody1Col = A->mapCol().dofIdToContainerId( colStartInMatrix+startBlockIndexTranslationalVelocityBody1 );
                        auto const& basisToContainerGpArticulationLMTranslationalVelocityRow = A->mapRow().dofIdToContainerId( rowStartInMatrix+startBlockIndexArticulationLMTranslationalVelocity );
                        auto const& basisToContainerGpArticulationLMTranslationalVelocityCol = A->mapCol().dofIdToContainerId( colStartInMatrix+startBlockIndexArticulationLMTranslationalVelocity );
                        for (int d=0;d<nDim;++d)
                        {
                            A->add( basisToContainerGpTranslationalVelocityBody1Row[d],basisToContainerGpArticulationLMTranslationalVelocityCol[d],1.0 );
                            A->add( basisToContainerGpArticulationLMTranslationalVelocityRow[d],basisToContainerGpTranslationalVelocityBody1Col[d],1.0 );
                        }
                    }
                    if ( hasActiveDofTranslationalVelocityBody2 )
                    {
                        auto const& basisToContainerGpTranslationalVelocityBody2Row = A->mapRow().dofIdToContainerId( rowStartInMatrix+startBlockIndexTranslationalVelocityBody2 );
                        auto const& basisToContainerGpTranslationalVelocityBody2Col = A->mapCol().dofIdToContainerId( colStartInMatrix+startBlockIndexTranslationalVelocityBody2 );
                        auto const& basisToContainerGpArticulationLMTranslationalVelocityRow = A->mapRow().dofIdToContainerId( rowStartInMatrix+startBlockIndexArticulationLMTranslationalVelocity );
                        auto const& basisToContainerGpArticulationLMTranslationalVelocityCol = A->mapCol().dofIdToContainerId( colStartInMatrix+startBlockIndexArticulationLMTranslationalVelocity );
                        for (int d=0;d<nDim;++d)
                        {
                            A->add( basisToContainerGpArticulationLMTranslationalVelocityRow[d],basisToContainerGpTranslationalVelocityBody2Col[d],-1.0 );
                            A->add( basisToContainerGpTranslationalVelocityBody2Row[d],basisToContainerGpArticulationLMTranslationalVelocityCol[d],-1.0 );
                        }
                    }
                }
                if ( BuildNonCstPart )
                {
                    F->setIsClosed( false );
                    if ( hasActiveDofTranslationalVelocityBody1 )
                    {
                        auto const& basisToContainerGpArticulationLMTranslationalVelocityVector = F->map().dofIdToContainerId( rowStartInVector+startBlockIndexArticulationLMTranslationalVelocity );
                        auto articulationTranslationalVelocityExpr = ba.translationalVelocityExpr( se ).evaluate(false);
                        for (int d=0;d<nDim;++d)
                        {
                            F->add( basisToContainerGpArticulationLMTranslationalVelocityVector[d],
                                    articulationTranslationalVelocityExpr(d) );
                        }
                    }
                }
            } // ba
        } // nba
    }
    //--------------------------------------------------------------------------------------------------//

#if 0
    if ( UsePeriodicity && BuildNonCstPart )
    {
        std::string marker1 = soption(_name="periodicity.marker1",_prefix=this->prefix());
        double pressureJump = doption(_name="periodicity.pressure-jump",_prefix=this->prefix());
        form1( _test=Xh, _vector=F,
               _rowstart=rowStartInVector ) +=
            integrate( _range=markedfaces( this->mesh(),this->mesh()->markerName(marker1) ),
                       _expr=inner(pressureJump*N(),id(v) ) );
    }
#endif

    //--------------------------------------------------------------------------------------------------//

    this->updateLinearPDEStabilisation( data );

    //--------------------------------------------------------------------------------------------------//

    double timeElapsed = this->timerTool("Solve").stop();
    this->log("FluidMechanics","updateLinearPDE","finish in "+(boost::format("%1% s") %timeElapsed).str() );
}

template< typename ConvexType, typename BasisVelocityType, typename BasisPressureType>
template <typename ModelContextType>
void
FluidMechanics<ConvexType,BasisVelocityType,BasisPressureType>::updateLinearPDEDofElimination( DataUpdateLinear & data, ModelContextType const& mctx ) const
{
    if ( !M_boundaryConditions->hasTypeDofElimination() )
        return;

    this->log("FluidMechanics","updateLinearPDEDofElimination","start" );
    this->timerTool("Solve").start();

    sparse_matrix_ptrtype& A = data.matrix();
    vector_ptrtype& F = data.rhs();
    auto XhV = this->functionSpaceVelocity();
    auto mesh = this->mesh();
    size_type startBlockIndexVelocity = this->startSubBlockSpaceIndex("velocity");
    auto bilinearFormVV = form2( _test=XhV,_trial=XhV,_matrix=A,
                                 _rowstart=this->rowStartInMatrix()+startBlockIndexVelocity,
                                 _colstart=this->colStartInMatrix()+startBlockIndexVelocity );
    auto const& u = this->fieldVelocity();
    auto const& se = mctx.symbolsExpr();

    M_boundaryConditions->applyDofEliminationLinear( bilinearFormVV, F, mesh, u, se );

    // inlet bc
    for ( auto const& [bcName,bcData] : M_boundaryConditions->inlet() )
    {
        auto const& inletVel = std::get<0>( M_fluidInletVelocityInterpolated.find(bcName)->second );
        bilinearFormVV +=
            on( _range=markedfaces(this->mesh(), bcData->markers()),
                _element=u, _rhs=F,
                _expr=-idv(inletVel)*N() );
    }

    if ( !M_boundaryConditions->pressureImposed().empty() )
    {
        auto rangePressureBC = boundaryfaces(M_meshLagrangeMultiplierPressureBC);
        size_type startBlockIndexPressureLM1 = this->startSubBlockSpaceIndex("pressurelm1");
        form2( _test=M_spaceLagrangeMultiplierPressureBC,_trial=M_spaceLagrangeMultiplierPressureBC,_matrix=A,
               _rowstart=this->rowStartInMatrix()+startBlockIndexPressureLM1,
               _colstart=this->colStartInMatrix()+startBlockIndexPressureLM1 ) +=
            on( _range=rangePressureBC, _rhs=F,
                _element=*M_fieldLagrangeMultiplierPressureBC1, _expr=cst(0.));
        if constexpr ( nDim == 3 )
        {
            size_type startBlockIndexPressureLM2 = this->startSubBlockSpaceIndex("pressurelm2");
            form2( _test=M_spaceLagrangeMultiplierPressureBC,_trial=M_spaceLagrangeMultiplierPressureBC,_matrix=A,
                   _rowstart=this->rowStartInMatrix()+startBlockIndexPressureLM2,
                   _colstart=this->colStartInMatrix()+startBlockIndexPressureLM2 ) +=
                on( _range=rangePressureBC, _rhs=F,
                    _element=*M_fieldLagrangeMultiplierPressureBC2, _expr=cst(0.));
        }
    }

    for ( auto const& [bpname,bpbc] : M_bodySetBC )
    {
        if ( bpbc.hasTranslationalVelocityExpr() )
        {
            size_type startBlockIndexTranslationalVelocity = this->startSubBlockSpaceIndex("body-bc."+bpbc.name()+".translational-velocity");
            form2( _test= bpbc.spaceTranslationalVelocity(),_trial=bpbc.spaceTranslationalVelocity(),_matrix=A,
                   _rowstart=this->rowStartInMatrix()+startBlockIndexTranslationalVelocity,
                   _colstart=this->colStartInMatrix()+startBlockIndexTranslationalVelocity ) +=
                on( _range=elements(bpbc.mesh()), _rhs=F,
                    _element=*bpbc.fieldTranslationalVelocityPtr(), _expr=bpbc.translationalVelocityExpr() );
        }
        if ( bpbc.hasAngularVelocityExpr() )
        {
            size_type startBlockIndexAngularVelocity = this->startSubBlockSpaceIndex("body-bc."+bpbc.name()+".angular-velocity");
            form2( _test=bpbc.spaceAngularVelocity(),_trial=bpbc.spaceAngularVelocity(),_matrix=A,
                   _rowstart=this->rowStartInMatrix()+startBlockIndexAngularVelocity,
                   _colstart=this->colStartInMatrix()+startBlockIndexAngularVelocity ) +=
                on( _range=elements(bpbc.mesh()), _rhs=F,
                    _element=*bpbc.fieldAngularVelocityPtr(), _expr=bpbc.angularVelocityExpr() );
        }

        if ( bpbc.hasElasticVelocity() && !M_bodySetBC.internal_elasticVelocity_is_v0() )
        {
            bilinearFormVV +=
                on( _range=bpbc.rangeMarkedFacesOnFluid(),
                    _element=u, _rhs=F,
                    _expr=idv(bpbc.fieldElasticVelocityPtr()) );
        }
        else
        {
             bilinearFormVV +=
                 on( _range=bpbc.rangeMarkedFacesOnFluid(),
                     _element=u, _rhs=F,
                     _expr=Feel::vf::zero<nDim,1>() );
        }
    }

    double timeElapsed = this->timerTool("Solve").stop();
    this->log("FluidMechanics","updateLinearPDEDofElimination","finish in "+(boost::format("%1% s") %timeElapsed).str() );

}

} // namespace Feel
} // namespace FeelModels

#endif

