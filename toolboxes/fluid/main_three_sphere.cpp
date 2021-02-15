/* -*- mode: c++; coding: utf-8; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; show-trailing-whitespace: t -*- vim:fenc=utf-8:ft=cpp:et:sw=4:ts=4:sts=4
 */

#include <feel/feelmodels/fluid/fluidmechanics.hpp>

namespace Feel
{

template <int nDim,uint16_type OrderVelocity,uint16_type OrderPressure, uint16_type OrderGeo = 1>
int
runApplicationFluid()
{
    using namespace Feel;
    tic();
    typedef FeelModels::FluidMechanics< Simplex<nDim,OrderGeo>,
                                        Lagrange<OrderVelocity, Vectorial,Continuous,PointSetFekete>,
                                        Lagrange<OrderPressure, Scalar,Continuous,PointSetFekete> > model_type;
    auto FM = model_type::New("fluid");

    FM->init();
    FM->printAndSaveInfo();
    toc("initialisation");
    if ( FM->isStationary() )
    {
        FM->solve();
        FM->exportResults();
    }
    else
    {
        if ( !FM->doRestart() )
            FM->exportResults(FM->timeInitial());
        tic();
        for ( FM->startTimeStep(); !FM->timeStepBase()->isFinished(); FM->updateTimeStep() )
        {
            if (FM->worldComm().isMasterRank())
            {
                std::cout << "============================================================\n";
                std::cout << "time simulation: " << FM->time() << "s \n";
                std::cout << "============================================================\n";
            }
            tic();
            FM->solve();
            toc("solve");
            tic();
            FM->exportResults();
            toc("export");
        }
        toc("end_of_simulation");
    }

    return !FM->checkResults();

}

} // namespace Feel

int
main( int argc, char** argv )
{
    using namespace Feel;
	po::options_description fluidmecoptions( "application fluid-mechanics options" );
    fluidmecoptions.add( toolboxes_options("fluid") );
    fluidmecoptions.add( bdf_options( "mybdf" )).add(ts_options("mybdf" ));
    fluidmecoptions.add(backend_options("Ip"));
    fluidmecoptions.add(backend_options("Iv"));
    fluidmecoptions.add(backend_options("Idisp"));
    fluidmecoptions.add_options()
        ("case.dimension", Feel::po::value<int>()->default_value( 3 ), "dimension")
        ("case.discretization", Feel::po::value<std::string>()->default_value( "P2P1G1" ), "discretization : P2P1G1,P2P1G2")
        ( "export.matlab", po::value<bool>()->default_value( true ), "export matrix and vector to matlab" )
        ("remesh.metric.layers",po::value<int>()->default_value( 2 ), "number of remeshing layers")
        ;

	Environment env( _argc=argc, _argv=argv,
                     _desc=fluidmecoptions,
                   _about=about(_name="application_fluid",
                                _author="Feel++ Consortium",
                                _email="feelpp-devel@feelpp.org"));

    int dimension = ioption(_name="case.dimension");
    std::string discretization = soption(_name="case.discretization");
    if ( discretization == "P2P1" )
        discretization = "P2P1G1";

    auto dimt = hana::make_tuple(hana::int_c<2>,hana::int_c<3>);

    auto discretizationt = hana::make_tuple( hana::make_tuple("P2P1G1", hana::make_tuple( hana::int_c<2>,hana::int_c<1>,hana::int_c<1>) ));

    int status = 0;
    hana::for_each( hana::cartesian_product(hana::make_tuple(dimt,discretizationt)), [&discretization,&dimension,&status]( auto const& d )
                    {
                        constexpr int _dim = std::decay_t<decltype(hana::at_c<0>(d))>::value;
                        std::string const& _discretization = hana::at_c<0>( hana::at_c<1>(d) );
                        constexpr int _uorder = std::decay_t<decltype(hana::at_c<0>(hana::at_c<1>( hana::at_c<1>(d)) ))>::value;
                        constexpr int _porder = std::decay_t<decltype(hana::at_c<1>(hana::at_c<1>( hana::at_c<1>(d)) ))>::value;
                        constexpr int _gorder = std::decay_t<decltype(hana::at_c<2>(hana::at_c<1>( hana::at_c<1>(d)) ))>::value;
                        if ( dimension == _dim && discretization == _discretization )
                            status = runApplicationFluid<_dim,_uorder,_porder,_gorder>();
                    } );
    return status;


}
