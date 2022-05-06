#ifndef FEELPP_TOOLBOXES_FLUIDMECHANICS_ASSEMBLY_JACOBIAN_HPP
#define FEELPP_TOOLBOXES_FLUIDMECHANICS_ASSEMBLY_JACOBIAN_HPP 1

namespace Feel
{
namespace FeelModels
{

template< typename ConvexType, typename BasisVelocityType, typename BasisPressureType>
template <typename ModelContextType>
void
FluidMechanics<ConvexType,BasisVelocityType,BasisPressureType>::updateNewtonInitialGuess( DataNewtonInitialGuess & data, ModelContextType const& mctx ) const
{
    if ( !M_boundaryConditions->hasTypeDofElimination() )
        return;

    this->log("FluidMechanics","updateNewtonInitialGuess","start");

    vector_ptrtype& U = data.initialGuess();
    size_type rowStartInVector = this->rowStartInVector();
    auto XhV = this->functionSpaceVelocity();
    auto u = XhV->element( U, rowStartInVector );
    auto mesh = this->mesh();
    auto const& se = mctx.symbolsExpr();

    M_boundaryConditions->applyNewtonInitialGuess( mesh, u, se );

    // inlet bc
    for ( auto const& [bcName,bcData] : M_boundaryConditions->inlet() )
    {
        auto itFindMark = M_fluidInletVelocityInterpolated.find(bcName);
        if ( itFindMark == M_fluidInletVelocityInterpolated.end() )
            continue;
        auto const& inletVel = std::get<0>( itFindMark->second );
        u.on(_range=markedfaces(mesh, bcData->markers()),
             _expr=-idv(inletVel)*N() );
    }

    // update info for synchronization
    this->updateDofEliminationIds( "velocity", data );

    if ( !M_boundaryConditions->pressureImposed().empty() )
    {
        auto rangePressureBC = boundaryfaces(M_meshLagrangeMultiplierPressureBC);
        size_type startBlockIndexPressureLM1 = this->startSubBlockSpaceIndex("pressurelm1");
        auto plm1 = M_spaceLagrangeMultiplierPressureBC->element( U, rowStartInVector+startBlockIndexPressureLM1 );
        plm1.on( _range=rangePressureBC,_expr=cst(0.) );
        this->updateDofEliminationIds( "pressurelm1", this->dofEliminationIds( "pressurebc-lm" ), data );
        if ( nDim == 3 )
        {
            size_type startBlockIndexPressureLM2 = this->startSubBlockSpaceIndex("pressurelm2");
            auto plm2 = M_spaceLagrangeMultiplierPressureBC->element( U, rowStartInVector+startBlockIndexPressureLM2 );
            plm2.on( _range=rangePressureBC,_expr=cst(0.) );
            this->updateDofEliminationIds( "pressurelm2", this->dofEliminationIds( "pressurebc-lm" ), data );
        }
    }

    // body bc
    bool hasDofEliminationIds_bodyBC_translationalVelocity = false, hasDofEliminationIds_bodyBC_angularVelocity = false;
    for ( auto const& [bpname,bpbc] : M_bodySetBC )
    {
        if ( bpbc.hasTranslationalVelocityExpr() )
        {
            std::string spaceName = "body-bc."+bpbc.name()+".translational-velocity";
            size_type startBlockIndexTranslationalVelocity = this->startSubBlockSpaceIndex( spaceName );
            auto uBodyTranslationalVelocity = bpbc.spaceTranslationalVelocity()->element( U, rowStartInVector+startBlockIndexTranslationalVelocity );
            uBodyTranslationalVelocity.on(_range=elements(bpbc.mesh()), _expr=bpbc.translationalVelocityExpr() );
            //this->updateDofEliminationIds( spaceName, this->dofEliminationIds( "body-bc.translational-velocity" ), data );
            this->updateDofEliminationIds( spaceName, data );
        }
        if ( bpbc.hasAngularVelocityExpr() )
        {
            std::string spaceName = "body-bc."+bpbc.name()+".angular-velocity";
            size_type startBlockIndexAngularVelocity = this->startSubBlockSpaceIndex( spaceName );
            auto uBodyAngularVelocity = bpbc.spaceAngularVelocity()->element( U, rowStartInVector+startBlockIndexAngularVelocity );
            uBodyAngularVelocity.on(_range=elements(bpbc.mesh()), _expr=bpbc.angularVelocityExpr() );
            //this->updateDofEliminationIds( spaceName, this->dofEliminationIds( "body-bc.angular-velocity" ), data );
            this->updateDofEliminationIds( spaceName, data );
        }

        if ( bpbc.hasElasticVelocity() && !M_bodySetBC.internal_elasticVelocity_is_v0() )
        {
            u.on( _range=bpbc.rangeMarkedFacesOnFluid(),
                  _expr=idv(bpbc.fieldElasticVelocityPtr()) );
        }
    }

    // imposed mean pressure (TODO use updateDofEliminationIds)
    if ( this->definePressureCst() && this->definePressureCstMethod() == "algebraic" )
    {
        auto pSol = this->functionSpacePressure()->element( U, this->rowStartInVector()+1 );
        CHECK( !M_definePressureCstAlgebraicOperatorMeanPressure.empty() ) << "mean pressure operator does not init";

        for ( int k=0;k<M_definePressureCstAlgebraicOperatorMeanPressure.size();++k )
        {
            double meanPressureImposed = 0;
            double meanPressureCurrent = inner_product( *M_definePressureCstAlgebraicOperatorMeanPressure[k].first, pSol );
            for ( size_type dofId : M_definePressureCstAlgebraicOperatorMeanPressure[k].second )
                pSol(dofId) += (meanPressureImposed - meanPressureCurrent);
        }
        sync( pSol, "=" );
    }

    this->log("FluidMechanics","updateNewtonInitialGuess","finish");

}

template< typename ConvexType, typename BasisVelocityType, typename BasisPressureType>
template <typename ModelContextType>
void
FluidMechanics<ConvexType,BasisVelocityType,BasisPressureType>::updateJacobian( DataUpdateJacobian & data, ModelContextType const& mctx ) const
{
    const vector_ptrtype& XVec = data.currentSolution();
    sparse_matrix_ptrtype& J = data.jacobian();
    bool _BuildCstPart = data.buildCstPart();
    bool usePicardLinearization = data.usePicardLinearization();
    std::string jacobianApprox = usePicardLinearization? "Picard-Linearization" : "Analytic";

    std::string sc=(_BuildCstPart)?" (build cst part)":" (build non cst part)";
    this->log("FluidMechanics","updateJacobian", fmt::format( "start {} : {}",sc,jacobianApprox));
    this->timerTool("Solve").start();

    boost::mpi::timer thetimer;

    bool BuildNonCstPart = !_BuildCstPart;
    bool BuildCstPart = _BuildCstPart;

    bool Build_TransientTerm = BuildNonCstPart;
    if ( !this->isStationaryModel() && this->timeStepBase()->strategy()==TS_STRATEGY_DT_CONSTANT )
        Build_TransientTerm=BuildCstPart;


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

    size_type startBlockIndexVelocity = this->startSubBlockSpaceIndex("velocity");
    size_type startBlockIndexPressure = this->startSubBlockSpaceIndex("pressure");
    size_type rowStartInMatrix = this->rowStartInMatrix();
    size_type colStartInMatrix = this->colStartInMatrix();
    size_type rowStartInVector = this->rowStartInVector();
    auto bilinearFormVV_PatternDefault = form2( _test=XhV,_trial=XhV,_matrix=J,
                                              _pattern=size_type(Pattern::DEFAULT),
                                              _rowstart=rowStartInMatrix,
                                              _colstart=colStartInMatrix );
    auto bilinearFormVV = form2( _test=XhV,_trial=XhV,_matrix=J,
                                              _pattern=size_type(Pattern::COUPLED),
                                              _rowstart=rowStartInMatrix,
                                              _colstart=colStartInMatrix );
    auto bilinearFormVP = form2( _test=XhV,_trial=XhP,_matrix=J,
                                 _pattern=size_type(Pattern::COUPLED),
                                 _rowstart=rowStartInMatrix+0,
                                 _colstart=colStartInMatrix+1 );
    auto bilinearFormPV = form2( _test=XhP,_trial=XhV,_matrix=J,
                                 _pattern=size_type(Pattern::COUPLED),
                                 _rowstart=rowStartInMatrix+1,
                                 _colstart=colStartInMatrix+0 );

    auto const& u = mctx.field( FieldTag::velocity(this), "velocity" );
    auto const& p = mctx.field( FieldTag::pressure(this), "pressure" );
    auto const& v = u;
    auto const& q = p;
    auto const& se = mctx.symbolsExpr();

    //--------------------------------------------------------------------------------------------------//

    // identity Matrix
    auto const Id = eye<nDim,nDim>();
    // strain tensor
    auto const deft = sym(gradt(u));
    //--------------------------------------------------------------------------------------------------//
    //--------------------------------------------------------------------------------------------------//
    //--------------------------------------------------------------------------------------------------//

    boost::mpi::timer timerAssemble;

    //--------------------------------------------------------------------------------------------------//
    for ( auto const& [physicName,physicData] : this->physicsFromCurrentType() )
    {
        auto physicFluidData = std::static_pointer_cast<ModelPhysicFluid<nDim>>(physicData);
        for ( std::string const& matName : this->materialsProperties()->physicToMaterials( physicName ) )
        {
            auto const& range = this->materialsProperties()->rangeMeshElementsByMaterial( this->mesh(),matName );
            auto const& matProps = this->materialsProperties()->materialProperties( matName );

            // stress tensor sigma : grad(v)
            if ( BuildCstPart )
                bilinearFormVP +=
                    integrate( _range=range,
                               _expr= -idt(p)*div(v),
                               _geomap=this->geomap() );

            bool doAssemblyStressTensor = ( physicFluidData->dynamicViscosity().isNewtonianLaw() && !physicFluidData->turbulence().isEnabled() )? BuildCstPart : BuildNonCstPart;
            if ( doAssemblyStressTensor )
            {
                auto StressTensorExprJac = Feel::FeelModels::fluidMecViscousStressTensorJacobian(/*gradv(u),*/u,*physicFluidData,matProps,se/*,usePicardLinearization*/);
                bilinearFormVV +=
                    integrate( _range=range,
                               _expr= timeSteppingScaling*inner( StressTensorExprJac,grad(v) ),
                               _geomap=this->geomap() );
            }
#if 0
            if ( physicFluidData->turbulence().isEnabled() && BuildNonCstPart )
            {
                auto lmix = min( 0.41*idv(M_fieldDist2Wall), cst(0.09)*cst(0.0635/2.) );
                auto mut = pow(lmix,2)*abs(gradv(u)(0,1));
                bilinearFormVV +=
                    integrate( _range=range,
                               _expr= timeSteppingScaling*inner( 2*mut*sym(gradt(u)),grad(v) ),
                               _geomap=this->geomap() );
                auto mutt = pow(lmix,2)*(gradv(u)(0,1)/(abs(gradv(u)(0,1))+1e-8*(abs(gradv(u)(0,1))<1e-8)  ))*gradt(u)(0,1);
                bilinearFormVV +=
                    integrate( _range=range,
                               _expr= timeSteppingScaling*inner( 2*mutt*sym(gradv(u)),grad(v) ),
                               _geomap=this->geomap() );
            }
#endif

            // convection terms
            if ( physicFluidData->equation() == "Navier-Stokes" )
            {
                auto densityExpr = expr( matProps.property("density").template expr<1,1>(), se );
                if ( BuildNonCstPart )
                {
                    auto const& beta_u = this->useSemiImplicitTimeScheme()? mctx.field( FieldTag::velocity_extrapolated(this), "velocity_extrapolated" ) : u;
                    auto convTerm = Feel::FeelModels::fluidMecConvectiveTermJacobian( u,physicFluidData,beta_u, this->useSemiImplicitTimeScheme() || usePicardLinearization );
                    bilinearFormVV +=
                        integrate ( _range=range,
                                    _expr=timeSteppingScaling*densityExpr*inner( convTerm, id(v) ),
                                    _geomap=this->geomap() );
#if 0
                    if ( this->useSemiImplicitTimeScheme() || usePicardLinearization )
                    {
                        bilinearFormVV +=
                            integrate ( _range=range,
                                        _expr= timeSteppingScaling*densityExpr*trans( gradt(u)*idv(beta_u) )*id(v),
                                        _geomap=this->geomap() );
                        if ( this->doStabConvectionEnergy() )
                            CHECK( false ) << "TODO";
                    }
                    else
                    {
                        if (this->doStabConvectionEnergy())
                        {
                            // convection term + stabilisation energy of convection with neumann bc (until outflow bc) ( see Nobile thesis)
                            // auto const convecTerm = (trans(val(gradv(u)*idv(*M_P0Rho))*idt(u)) + trans(gradt(u)*val(idv(u)*idv(*M_P0Rho)) ) )*id(v);
                            // stabTerm = trans(divt(u)*val(0.5*idv(*M_P0Rho)*idv(u))+val(0.5*idv(*M_P0Rho)*divv(u))*idt(u))*id(v)

                            auto const convecTerm = densityExpr*Feel::FeelModels::fluidMecConvectionJacobianWithEnergyStab(u);
                            bilinearFormVV +=
                                //bilinearForm_PatternDefault +=
                                integrate ( _range=range,
                                            _expr=timeSteppingScaling*convecTerm,
                                            _geomap=this->geomap() );
                        }
                        else
                        {
                            //auto const convecTerm = (trans(val(gradv(u)*idv(rho))*idt(u)) + trans(gradt(u)*val(idv(u)*idv(rho)) ) )*id(v);
                            auto const convecTerm = densityExpr*Feel::FeelModels::fluidMecConvectionJacobian(u);

                            bilinearFormVV +=
                                //bilinearForm_PatternDefault +=
                                integrate ( _range=range,
                                            _expr=timeSteppingScaling*convecTerm,
                                            _geomap=this->geomap() );
                        }
                    }
#endif
                }

                if ( data.hasVectorInfo( "explicit-part-of-solution" ) && BuildCstPart )
                {
                    auto uExplicitPartOfSolution = XhV->element( data.vectorInfo( "explicit-part-of-solution" ), rowStartInVector+startBlockIndexVelocity );
                    bilinearFormVV +=
                        integrate ( _range=range,
                                    _expr= timeSteppingScaling*densityExpr*trans( gradt(u)*idv(uExplicitPartOfSolution) + gradv(uExplicitPartOfSolution )*idt(u) )*id(v),
                                    _geomap=this->geomap() );
                }
            } // Navier-Stokes

#if defined( FEELPP_MODELS_HAS_MESHALE )
            if ( this->hasMeshMotion() && BuildCstPart && ( physicFluidData->equation() == "Navier-Stokes" ||  physicFluidData->equation() == "StokesTransient" ) )
            {
                auto densityExpr = expr( matProps.property("density").template expr<1,1>(), se );
                bilinearFormVV +=
                    //bilinearForm_PatternDefault +=
                    integrate (_range=range,
                               _expr= -timeSteppingScaling*trans(gradt(u)*densityExpr*idv( this->meshMotionTool()->velocity() ))*id(v),
                               _geomap=this->geomap() );
            }
#endif
            //transients terms
            if ( !this->isStationaryModel() && Build_TransientTerm )
            {
                auto densityExpr = expr( matProps.property("density").template expr<1,1>(), se );
                bilinearFormVV_PatternDefault +=
                    integrate( _range=range,
                               _expr= densityExpr*inner(idt(u),id(v))*M_bdfVelocity->polyDerivCoefficient(0),
                               _geomap=this->geomap() );
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

            // stabilization gls
            if ( M_stabilizationGLS && M_stabilizationGLSDoAssembly )
            {
                this->updateJacobianStabilizationGLS( data, mctx, *physicFluidData, matProps, range );
            }

        } // foreach mat
    } // foreach physic


    double timeElapsed = timerAssemble.elapsed();
    this->log("FluidMechanics","updateJacobian",(boost::format("assemble convection term in %1% s") %timeElapsed).str() );

    //--------------------------------------------------------------------------------------------------//
    // incompressibility term
    if (BuildCstPart)
    {
        bilinearFormPV +=
            integrate( _range=M_rangeMeshElements,
                       _expr= -divt(u)*id(q),
                       _geomap=this->geomap() );
    }

    //--------------------------------------------------------------------------------------------------//
    // peusdo transient continuation
    if ( BuildNonCstPart && data.hasInfo( "use-pseudo-transient-continuation" ) )
    {
#if 1
        double pseudoTimeStepDelta = data.doubleInfo("pseudo-transient-continuation.delta");
        CHECK( M_stabilizationGLSParameterConvectionDiffusion ) << "aie";
        auto XhP0d = M_stabilizationGLSParameterConvectionDiffusion->fieldTau().functionSpace();
        auto norm2_uu = XhP0d->element(); // TODO : improve this (maybe create an expression instead)
        //norm2_uu.on(_range=M_rangeMeshElements,_expr=norm2(idv(u))/h());
        auto fieldNormu = u->functionSpace()->compSpace()->element( norm2(idv(u)) );
        auto maxu = fieldNormu.max( XhP0d );
        //auto maxux = u[ComponentType::X].max( this->materialProperties()->fieldRho().functionSpace() );
        //auto maxuy = u[ComponentType::Y].max( this->materialProperties()->fieldRho().functionSpace() );
        //norm2_uu.on(_range=M_rangeMeshElements,_expr=norm2(vec(idv(maxux),idv(maxux)))/h());
        norm2_uu.on(_range=M_rangeMeshElements,_expr=idv(maxu)/h());

        bilinearFormVV_PatternDefault +=
            integrate(_range=M_rangeMeshElements,
                      _expr=(1./pseudoTimeStepDelta)*idv(norm2_uu)*inner(idt(u),id(u)),
                      //_expr=(1./pseudoTimeStepDelta)*(norm2(idv(u))/h())*inner(idt(u),id(u)),
                      //_expr=(1./pseudoTimeStepDelta)*inner(idt(u),id(u)),
                      _geomap=this->geomap() );
#else
        CHECK( false ) << "TODO VINCENT";
#endif
    }
    //--------------------------------------------------------------------------------------------------//
    // define pressure cst
    if ( this->definePressureCst() )
    {
        if ( this->definePressureCstMethod() == "penalisation" && BuildCstPart )
        {
            auto bilinearFormPP = form2( _test=XhP,_trial=XhP,_matrix=J,
                                         _pattern=size_type(Pattern::COUPLED),
                                         _rowstart=rowStartInMatrix+1,
                                         _colstart=colStartInMatrix+1 );
            double beta = this->definePressureCstPenalisationBeta();
            for ( auto const& rangeElt : M_definePressureCstMeshRanges )
                bilinearFormPP +=
                    integrate( _range=rangeElt,
                               _expr=beta*idt(p)*id(q),
                               _geomap=this->geomap() );
        }
        if ( this->definePressureCstMethod() == "lagrange-multiplier" && BuildCstPart )
        {
            CHECK( this->hasStartSubBlockSpaceIndex("define-pressure-cst-lm") ) << " start dof index for define-pressure-cst-lm is not present\n";
            size_type startBlockIndexDefinePressureCstLM = this->startSubBlockSpaceIndex("define-pressure-cst-lm");
            for ( int k=0;k<M_XhMeanPressureLM.size();++k )
            {
                auto lambda = M_XhMeanPressureLM[k]->element();
                form2( _test=XhP, _trial=M_XhMeanPressureLM[k], _matrix=J,
                       _rowstart=this->rowStartInMatrix()+1,
                       _colstart=this->colStartInMatrix()+startBlockIndexDefinePressureCstLM+k ) +=
                    integrate( _range=M_definePressureCstMeshRanges[k],
                               _expr= id(p)*idt(lambda) /*+ idt(p)*id(lambda)*/,
                               _geomap=this->geomap() );

                form2( _test=M_XhMeanPressureLM[k], _trial=XhP, _matrix=J,
                       _rowstart=this->rowStartInMatrix()+startBlockIndexDefinePressureCstLM+k,
                       _colstart=this->colStartInMatrix()+1 ) +=
                    integrate( _range=M_definePressureCstMeshRanges[k],
                               _expr= + idt(p)*id(lambda),
                               _geomap=this->geomap() );
            }
        }
    }
    //--------------------------------------------------------------------------------------------------//
    //--------------------------------------------------------------------------------------------------//
    //--------------------------------------------------------------------------------------------------//

    // Dirichlet bc by using Nitsche formulation
    if ( M_boundaryConditions->hasVelocityImposedNitsche() && BuildCstPart )
    {
        std::set<std::string> allmarkers;
        for ( auto const& [bcId,bcData] : M_boundaryConditions->velocityImposedNitsche() )
            allmarkers.insert( bcData->markers().begin(), bcData->markers().end() );

        //auto deft = sym(gradt(u));
        //auto viscousStressTensor = 2*idv(mu)*deft;
        auto viscousStressTensor = 2*this->dynamicViscosityExpr( se )*deft;
         bilinearFormVV +=
             integrate( _range=markedfaces(mesh,allmarkers),
                        _expr= -timeSteppingScaling*inner(viscousStressTensor*N(),id(v) )
                        /**/   + timeSteppingScaling*this->dirichletBCnitscheGamma()*inner(idt(u),id(v))/hFace(),
                        _geomap=this->geomap() );
        auto bilinearFormVP = form2( _test=XhV,_trial=XhP,_matrix=J,
                                     _pattern=size_type(Pattern::COUPLED),
                                     _rowstart=rowStartInMatrix,
                                     _colstart=colStartInMatrix+1 );
         bilinearFormVP +=
             integrate( _range=markedfaces(mesh,allmarkers),
                        _expr= timeSteppingScaling*inner( idt(p)*N(),id(v) ),
                        _geomap=this->geomap() );
    }

    //--------------------------------------------------------------------------------------------------//
    // Dirichlet bc by using Lagrange-multiplier
    if ( M_boundaryConditions->hasVelocityImposedLagrangeMultiplier() )
    {
        if ( BuildCstPart )
        {
            CHECK( this->hasStartSubBlockSpaceIndex("dirichletlm") ) << " start dof index for dirichletlm is not present\n";
            size_type startBlockIndexDirichletLM = this->startSubBlockSpaceIndex("dirichletlm");

            auto lambdaBC = this->XhDirichletLM()->element();
            form2( _test=XhV,_trial=this->XhDirichletLM(),_matrix=J,_pattern=size_type(Pattern::COUPLED),
                   _rowstart=rowStartInMatrix,
                   _colstart=colStartInMatrix+startBlockIndexDirichletLM )+=
                integrate( _range=elements(this->meshDirichletLM()),
                           _expr= inner( idt(lambdaBC),id(u) ) );

            form2( _test=this->XhDirichletLM(),_trial=XhV,_matrix=J,_pattern=size_type(Pattern::COUPLED),
                   _rowstart=rowStartInMatrix+startBlockIndexDirichletLM,
                   _colstart=colStartInMatrix ) +=
                integrate( _range=elements(this->meshDirichletLM()),
                           _expr= inner( idt(u),id(lambdaBC) ) );
        }
    }

    //--------------------------------------------------------------------------------------------------//
    // pressure bc
    if ( !M_boundaryConditions->pressureImposed().empty() )
    {
        CHECK( this->hasStartSubBlockSpaceIndex("pressurelm1") ) << " start dof index for pressurelm1 is not present\n";
        size_type startBlockIndexPressureLM1 = this->startSubBlockSpaceIndex("pressurelm1");
        if (BuildCstPart)
        {
            std::set<std::string> allmarkers;
            for ( auto const& [bcName,bcData] : M_boundaryConditions->pressureImposed() )
                allmarkers.insert( bcData->markers().begin(), bcData->markers().end() );
            auto rangeFacesPressureBC = markedfaces( this->mesh(),allmarkers );

            if ( nDim==2 )
            {
                form2( _test=XhV,_trial=M_spaceLagrangeMultiplierPressureBC,_matrix=J,_pattern=size_type(Pattern::COUPLED),
                       _rowstart=rowStartInMatrix,
                       _colstart=colStartInMatrix+startBlockIndexPressureLM1 ) +=
                    integrate( _range=rangeFacesPressureBC,
                               _expr=-timeSteppingScaling*trans(cross(id(u),N()))(0,0)*idt(M_fieldLagrangeMultiplierPressureBC1),
                               _geomap=this->geomap() );

                form2( _test=M_spaceLagrangeMultiplierPressureBC,_trial=XhV,_matrix=J,_pattern=size_type(Pattern::COUPLED),
                       _rowstart=rowStartInMatrix+startBlockIndexPressureLM1,
                       _colstart=colStartInMatrix ) +=
                    integrate( _range=rangeFacesPressureBC,
                               _expr=-trans(cross(idt(u),N()))(0,0)*id(M_fieldLagrangeMultiplierPressureBC1),
                               _geomap=this->geomap() );
            }
            else if ( nDim==3 )
            {
                auto alpha = 1./sqrt(1-Nz()*Nz());
                form2( _test=XhV,_trial=M_spaceLagrangeMultiplierPressureBC,_matrix=J,_pattern=size_type(Pattern::COUPLED),
                       _rowstart=rowStartInMatrix,
                       _colstart=colStartInMatrix+startBlockIndexPressureLM1 ) +=
                    integrate( _range=rangeFacesPressureBC,
                               _expr=-timeSteppingScaling*trans(cross(id(u),N()))(0,2)*idt(M_fieldLagrangeMultiplierPressureBC1)*alpha,
                               _geomap=this->geomap() );

                form2( _test=M_spaceLagrangeMultiplierPressureBC,_trial=XhV,_matrix=J,_pattern=size_type(Pattern::COUPLED),
                       _rowstart=rowStartInMatrix+startBlockIndexPressureLM1,
                       _colstart=colStartInMatrix ) +=
                    integrate( _range=rangeFacesPressureBC,
                               _expr=-trans(cross(idt(u),N()))(0,2)*id(M_fieldLagrangeMultiplierPressureBC1)*alpha,
                               _geomap=this->geomap() );

                CHECK( this->hasStartSubBlockSpaceIndex("pressurelm2") ) << " start dof index for pressurelm2 is not present\n";
                size_type startBlockIndexPressureLM2 = this->startSubBlockSpaceIndex("pressurelm2");

                form2( _test=XhV,_trial=M_spaceLagrangeMultiplierPressureBC,_matrix=J,_pattern=size_type(Pattern::COUPLED),
                       _rowstart=rowStartInMatrix,
                       _colstart=colStartInMatrix+startBlockIndexPressureLM2 ) +=
                    integrate( _range=rangeFacesPressureBC,
                               _expr= -timeSteppingScaling*trans(cross(id(u),N()))(0,0)*alpha*idt(M_fieldLagrangeMultiplierPressureBC2)*Ny()
                               +timeSteppingScaling*trans(cross(id(u),N()))(0,1)*alpha*idt(M_fieldLagrangeMultiplierPressureBC2)*Nx(),
                               _geomap=this->geomap() );

                form2( _test=M_spaceLagrangeMultiplierPressureBC,_trial=XhV,_matrix=J,_pattern=size_type(Pattern::COUPLED),
                       _rowstart=rowStartInMatrix+startBlockIndexPressureLM2,
                       _colstart=colStartInMatrix ) +=
                    integrate( _range=rangeFacesPressureBC,
                               _expr= -trans(cross(idt(u),N()))(0,0)*alpha*id(M_fieldLagrangeMultiplierPressureBC2)*Ny()
                               +trans(cross(idt(u),N()))(0,1)*alpha*id(M_fieldLagrangeMultiplierPressureBC2)*Nx(),
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
            if ( BuildCstPart )
            {
                size_type startBlockIndexWindkessel = this->startSubBlockSpaceIndex("windkessel");

                bool hasWindkesselActiveDof = M_fluidOutletWindkesselSpace->nLocalDofWithoutGhost() > 0;
                int blockStartWindkesselRow = rowStartInMatrix + startBlockIndexWindkessel + 2*indexOutletWindkessel;
                int blockStartWindkesselCol = colStartInMatrix + startBlockIndexWindkessel + 2*indexOutletWindkessel;
                auto const& basisToContainerGpPressureDistalRow = J->mapRow().dofIdToContainerId( blockStartWindkesselRow );
                auto const& basisToContainerGpPressureDistalCol = J->mapCol().dofIdToContainerId( blockStartWindkesselCol );
                auto const& basisToContainerGpPressureProximalRow = J->mapRow().dofIdToContainerId( blockStartWindkesselRow+1 );
                auto const& basisToContainerGpPressureProximalCol = J->mapCol().dofIdToContainerId( blockStartWindkesselCol+1 );
                if ( hasWindkesselActiveDof )
                    CHECK( !basisToContainerGpPressureDistalRow.empty() && !basisToContainerGpPressureDistalCol.empty() &&
                           !basisToContainerGpPressureProximalRow.empty() && !basisToContainerGpPressureProximalCol.empty() ) << "incomplete datamap info";
                const size_type gpPressureDistalRow = (hasWindkesselActiveDof)? basisToContainerGpPressureDistalRow[0] : 0;
                const size_type gpPressureDistalCol = (hasWindkesselActiveDof)? basisToContainerGpPressureDistalCol[0] : 0;
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

                // first equation
                if ( hasWindkesselActiveDof )
                {
                    J->add( gpPressureDistalRow, gpPressureDistalCol,
                            Cd*bdfDistal->polyDerivCoefficient(0)+1./Rd );
                }

                form2( _test=M_fluidOutletWindkesselSpace,_trial=XhV,_matrix=J,
                       _rowstart=blockStartWindkesselRow,
                       _colstart=colStartInMatrix ) +=
                    integrate( _range=rangeFaceFluidOutlet,
                               _expr=-inner(idt(u),N())*id(pDistal),
                               _geomap=this->geomap() );

                // second equation
                if ( hasWindkesselActiveDof )
                {
                    J->add( gpPressureProximalRow, gpPressureProximalCol,  1.);

                    J->add( gpPressureProximalRow, gpPressureDistalCol, -1.);
                }

                // really correct?
                form2( _test=M_fluidOutletWindkesselSpace,_trial=XhV,_matrix=J,
                       _rowstart=blockStartWindkesselRow+1,
                       _colstart=colStartInMatrix )+=
                    integrate( _range=rangeFaceFluidOutlet,
                               _expr=-Rp*inner(idt(u),N())*id(pProximal),
                               _geomap=this->geomap() );

                // coupling with fluid model
                form2( _test=XhV, _trial=M_fluidOutletWindkesselSpace, _matrix=J,
                       _rowstart=rowStartInMatrix,
                       _colstart=blockStartWindkesselCol+1 ) +=
                    integrate( _range=rangeFaceFluidOutlet,
                               _expr= timeSteppingScaling*idt(pProximal)*inner(N(),id(v)),
                               _geomap=this->geomap() );

            }
        }
        ++indexOutletWindkessel;
    }



#if 0 // VINCENT
    //--------------------------------------------------------------------------------------------------//

    // slip bc
    if (BuildCstPart && !this->markerSlipBC().empty() )
    {
        auto muExpr = this->dynamicViscosityExpr( u,se );
        auto densityExpr = this->materialsProperties()->template materialPropertyExpr<1,1>( "density", se );

        auto P = Id-N()*trans(N());
        double gammaN = doption(_name="bc-slip-gammaN",_prefix=this->prefix());
        double gammaTau = doption(_name="bc-slip-gammaTau",_prefix=this->prefix());
        auto betaExtrapolate = M_bdfVelocity->poly();
        //auto beta = Beta.element<0>();
        auto beta = vf::project( _space=XhV,
                                 _range=boundaryfaces(mesh),
                                 _expr=densityExpr*idv(betaExtrapolate) );
        auto Cn = val(gammaN*max(abs(trans(idv(beta))*N()),muExpr/vf::h()));
        auto Ctau = val(gammaTau*muExpr/vf::h() + max( -trans(idv(beta))*N(),cst(0.) ));

        bilinearFormVV +=
            integrate( _range= markedfaces(mesh,this->markerSlipBC()),
                       _expr= Cn*(trans(idt(u))*N())*(trans(id(v))*N())+
                       Ctau*trans(idt(u))*id(v),
                       //+ trans(idt(p)*Id*N())*id(v)
                       //- trans(id(v))*N()* trans(2*muExpr*deft*N())*N()
                       _geomap=this->geomap()
                       );
    }
#endif
    //--------------------------------------------------------------------------------------------------//
    if ( !M_bodySetBC.empty() )
    {
        this->log("FluidMechanics","updateJacobianWeakBC","assembly of body bc");

        for ( auto const& [bpname,bpbc] : M_bodySetBC )
        {
            J->setIsClosed( false );
            if ( BuildCstPart )
            {
                size_type startBlockIndexTranslationalVelocity = this->startSubBlockSpaceIndex("body-bc."+bpbc.name()+".translational-velocity");
                bool hasActiveDofTranslationalVelocity = bpbc.spaceTranslationalVelocity()->nLocalDofWithoutGhost() > 0;
                double massBody = bpbc.body().mass();
                if ( hasActiveDofTranslationalVelocity )
                {
                    auto const& basisToContainerGpTranslationalVelocityRow = J->mapRow().dofIdToContainerId( rowStartInMatrix+startBlockIndexTranslationalVelocity );
                    auto const& basisToContainerGpTranslationalVelocityCol = J->mapCol().dofIdToContainerId( colStartInMatrix+startBlockIndexTranslationalVelocity );
                    for (int d=0;d<nDim;++d)
                    {
                        J->add( basisToContainerGpTranslationalVelocityRow[d], basisToContainerGpTranslationalVelocityCol[d],
                                bpbc.bdfTranslationalVelocity()->polyDerivCoefficient(0)*massBody );
                    }
                }
            }

            if ( !bpbc.isInNBodyArticulated() || ( bpbc.getNBodyArticulated().masterBodyBC().name() == bpbc.name() ) )
            {
                size_type startBlockIndexAngularVelocity = this->startSubBlockSpaceIndex("body-bc."+bpbc.name()+".angular-velocity");
                int nLocalDofAngularVelocity = bpbc.spaceAngularVelocity()->nLocalDofWithoutGhost();
                bool hasActiveDofAngularVelocity = nLocalDofAngularVelocity > 0;
                auto const& momentOfInertia = bpbc.momentOfInertia_inertialFrame();
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
                    auto const& basisToContainerGpAngularVelocityRow = J->mapRow().dofIdToContainerId( rowStartInMatrix+startBlockIndexAngularVelocity );
                    auto const& basisToContainerGpAngularVelocityCol = J->mapCol().dofIdToContainerId( colStartInMatrix+startBlockIndexAngularVelocity );
                    if ( BuildCstPart )
                    {
                        for (int i=0;i<nLocalDofAngularVelocity;++i)
                        {
                            for (int j=0;j<nLocalDofAngularVelocity;++j)
                            {
                                double val = bpbc.bdfAngularVelocity()->polyDerivCoefficient(0)*momentOfInertia(i,j);
                                val += termWithTimeDerivativeOfMomentOfInertia(i,j);
                                J->add( basisToContainerGpAngularVelocityRow[i], basisToContainerGpAngularVelocityCol[j], val );
                            }
                        }
                    }

                    if ( BuildNonCstPart )
                    {
                        if constexpr ( nDim == 3 )
                        {
                            auto uAngularVelocity = bpbc.spaceAngularVelocity()->element( XVec, rowStartInVector+startBlockIndexAngularVelocity );
                            auto w_eval = idv(uAngularVelocity).evaluate(false);
                            auto Iw_eval = momentOfInertia*w_eval;

                            // jacobian of w x Iw
                            J->add( basisToContainerGpAngularVelocityRow[0], basisToContainerGpAngularVelocityCol[0],               w_eval(1)*momentOfInertia(2,0)-w_eval(2)*momentOfInertia(1,0) );
                            J->add( basisToContainerGpAngularVelocityRow[0], basisToContainerGpAngularVelocityCol[1], Iw_eval(2) +  w_eval(1)*momentOfInertia(2,1)-w_eval(2)*momentOfInertia(1,1) );
                            J->add( basisToContainerGpAngularVelocityRow[0], basisToContainerGpAngularVelocityCol[2], -Iw_eval(1) + w_eval(1)*momentOfInertia(2,2)-w_eval(2)*momentOfInertia(1,2) );
                            J->add( basisToContainerGpAngularVelocityRow[1], basisToContainerGpAngularVelocityCol[0], -Iw_eval(2) + w_eval(2)*momentOfInertia(0,0)-w_eval(0)*momentOfInertia(2,0) );
                            J->add( basisToContainerGpAngularVelocityRow[1], basisToContainerGpAngularVelocityCol[1],               w_eval(2)*momentOfInertia(0,1)-w_eval(0)*momentOfInertia(2,1) );
                            J->add( basisToContainerGpAngularVelocityRow[1], basisToContainerGpAngularVelocityCol[2], Iw_eval(0) +  w_eval(2)*momentOfInertia(0,2)-w_eval(0)*momentOfInertia(2,2) );
                            J->add( basisToContainerGpAngularVelocityRow[2], basisToContainerGpAngularVelocityCol[0], Iw_eval(1) +  w_eval(0)*momentOfInertia(1,0)-w_eval(1)*momentOfInertia(0,0) );
                            J->add( basisToContainerGpAngularVelocityRow[2], basisToContainerGpAngularVelocityCol[1], -Iw_eval(0) + w_eval(0)*momentOfInertia(1,1)-w_eval(1)*momentOfInertia(0,1) );
                            J->add( basisToContainerGpAngularVelocityRow[2], basisToContainerGpAngularVelocityCol[2],               w_eval(0)*momentOfInertia(1,2)-w_eval(1)*momentOfInertia(0,2) );
                        }
                    }
                }
            }
        } // for ( auto const& [bpname,bpbc] : M_bodySetBC )

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
                if ( BuildCstPart)
                {
                    J->setIsClosed( false );
                    if ( hasActiveDofTranslationalVelocityBody1 )
                    {
                        auto const& basisToContainerGpTranslationalVelocityBody1Row = J->mapRow().dofIdToContainerId( rowStartInMatrix+startBlockIndexTranslationalVelocityBody1 );
                        auto const& basisToContainerGpTranslationalVelocityBody1Col = J->mapCol().dofIdToContainerId( colStartInMatrix+startBlockIndexTranslationalVelocityBody1 );
                        auto const& basisToContainerGpArticulationLMTranslationalVelocityRow = J->mapRow().dofIdToContainerId( rowStartInMatrix+startBlockIndexArticulationLMTranslationalVelocity );
                        auto const& basisToContainerGpArticulationLMTranslationalVelocityCol = J->mapCol().dofIdToContainerId( colStartInMatrix+startBlockIndexArticulationLMTranslationalVelocity );
                        for (int d=0;d<nDim;++d)
                        {
                            J->add( basisToContainerGpTranslationalVelocityBody1Row[d],basisToContainerGpArticulationLMTranslationalVelocityCol[d],1.0 );
                            J->add( basisToContainerGpArticulationLMTranslationalVelocityRow[d],basisToContainerGpTranslationalVelocityBody1Col[d],1.0 );
                        }
                    }
                    if ( hasActiveDofTranslationalVelocityBody2 )
                    {
                        auto const& basisToContainerGpTranslationalVelocityBody2Row = J->mapRow().dofIdToContainerId( rowStartInMatrix+startBlockIndexTranslationalVelocityBody2 );
                        auto const& basisToContainerGpTranslationalVelocityBody2Col = J->mapCol().dofIdToContainerId( colStartInMatrix+startBlockIndexTranslationalVelocityBody2 );
                        auto const& basisToContainerGpArticulationLMTranslationalVelocityRow = J->mapRow().dofIdToContainerId( rowStartInMatrix+startBlockIndexArticulationLMTranslationalVelocity );
                        auto const& basisToContainerGpArticulationLMTranslationalVelocityCol = J->mapCol().dofIdToContainerId( colStartInMatrix+startBlockIndexArticulationLMTranslationalVelocity );
                        for (int d=0;d<nDim;++d)
                        {
                            J->add( basisToContainerGpArticulationLMTranslationalVelocityRow[d],basisToContainerGpTranslationalVelocityBody2Col[d],-1.0 );
                            J->add( basisToContainerGpTranslationalVelocityBody2Row[d],basisToContainerGpArticulationLMTranslationalVelocityCol[d],-1.0 );
                        }
                    }
                }
            } // ba
        } // nba

    }

    //--------------------------------------------------------------------------------------------------//

    this->updateJacobianStabilisation( data, unwrap_ptr(u),unwrap_ptr(p) );

    //--------------------------------------------------------------------------------------------------//

    timeElapsed = this->timerTool("Solve").stop();
    this->log("FluidMechanics","updateJacobian",(boost::format("finish %1% in %2% s") %sc %timeElapsed).str() );
}

} // namespace Feel
} // namespace FeelModels

#endif
