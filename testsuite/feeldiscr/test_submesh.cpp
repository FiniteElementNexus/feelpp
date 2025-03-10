/* -*- mode: c++; coding: utf-8; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; show-trailing-whitespace: t  -*-

  This file is part of the Feel library

  Author(s): Christophe Prud'homme <christophe.prudhomme@feelpp.org>
       Date: 2010-10-21

  Copyright (C) 2010 Université Joseph Fourier (Grenoble I)

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
/**
   \file test_submesh.cpp
   \author Christophe Prud'homme <christophe.prudhomme@feelpp.org>
   \date 2010-10-21
 */

// give a name to the testsuite
#define BOOST_TEST_MODULE submesh testsuite
#include <feel/feelcore/testsuite.hpp>

#include <feel/feeldiscr/pch.hpp>
#include <feel/feeldiscr/pchv.hpp>
#include <feel/feelfilters/loadmesh.hpp>
#include <feel/feelfilters/geotool.hpp>
#include <feel/feelfilters/unithypercube.hpp>
#include <feel/feelfilters/exporter.hpp>
#include <feel/feelmesh/meshmover.hpp>
#include <feel/feelvf/vf.hpp>
#include <feel/feeldiscr/operatorinterpolation.hpp>

const double DEFAULT_MESH_SIZE=0.1;

namespace Feel
{
template<typename T, int Dim, int Order = 1>
struct imesh
{
    typedef Simplex<Dim, Order> convex_type;
    typedef Mesh<convex_type, T > type;
    typedef std::shared_ptr<type> ptrtype;
};

template<typename value_type = double, int Dim=2>
struct test_submesh: public Application
{
    typedef typename imesh<value_type,Dim>::convex_type convex_type;
    typedef typename imesh<value_type,Dim>::type mesh_type;
    typedef typename imesh<value_type,Dim>::ptrtype mesh_ptrtype;
    typedef FunctionSpace<mesh_type, bases<Lagrange<3, Scalar> >, double> space_type;
    typedef std::shared_ptr<space_type> space_ptrtype;
    typedef typename space_type::element_type element_type;
    //typedef typename mesh_type::location_element_const_iterator location_element_const_iterator;
    typedef Backend<value_type> backend_type;

    test_submesh()
        :
        Application(),
        backend( Backend<double>::build(  soption( _name="backend" ) ) ),
        meshSize( this->vm()["hsize"].template as<double>() ),
        shape( this->vm()["shape"].template as<std::string>() ),
        mesh()
    {
        mesh = createGMSHMesh( _mesh=new mesh_type,
                               _desc=domain( _name=( boost::format( "%1%-%2%" ) % shape % Dim ).str() ,
                                             _usenames=true,
                                             _convex=( convex_type::is_hypercube )?"Hypercube":"Simplex",
                                             _shape=shape,
                                             _dim=Dim,
                                             _h=meshSize ),
                               _update=MESH_CHECK|MESH_UPDATE_EDGES|MESH_UPDATE_FACES );

    }
    void operator()()
    {
        //mesh_ptrtype meshbdy( new mesh_type );
        auto meshbdy = createSubmesh( _mesh=mesh, _range=boundaryelements( mesh ) );
        BOOST_CHECK_EQUAL( nelements(elements(meshbdy),false), nelements( boundaryelements( mesh ) ) );
        //saveGMSHMesh( _mesh=meshbdy, _filename=shape+"_sub.msh" );
        using namespace Feel::vf;
        double intm1 = integrate( _range=elements( meshbdy ), _expr=cst( 1. ) ).evaluate()( 0,0 );
        double intm2 = integrate( _range=boundaryelements( mesh ), _expr=cst( 1. ) ).evaluate()( 0,0 );
        BOOST_CHECK_CLOSE( intm1, intm2, 1e-12 );
        //double intm11 = integrate( boundaryfaces(meshbdy), cst(1.) ).evaluate()(0,0);
        //double intm21 = integrate( boundaryfaces(mesh), cst(1.) ).evaluate()(0,0);
        //BOOST_CHECK_CLOSE( intm11, intm21, 1e-12 );

        double intm12 = integrate( _range=markedfaces( meshbdy,"Dirichlet" ), _expr=cst( 1. ) ).evaluate()( 0,0 );
        double intm22 = integrate( _range=markedfaces( mesh,"Dirichlet" ), _expr=cst( 1. ) ).evaluate()( 0,0 );
        BOOST_CHECK_CLOSE( intm12, intm22, 1e-12 );
        double intm13 = integrate( _range=markedfaces( meshbdy,"Neumann" ), _expr=cst( 1. ) ).evaluate()( 0,0 );
        double intm23 = integrate( _range=markedfaces( mesh,"Neumann" ), _expr=cst( 1. ) ).evaluate()( 0,0 );
        BOOST_CHECK_CLOSE( intm13, intm23, 1e-12 );


        //mesh_ptrtype meshint( new mesh_type );
        auto meshint = createSubmesh( _mesh=mesh, _range=internalelements(mesh) );
        BOOST_CHECK_EQUAL( nelements(elements(meshint),false), nelements(internalelements(mesh)) );
        //saveGMSHMesh( _mesh=meshbdy, _filename="meshbdy" );

        using namespace Feel::vf;
        double intm3 = integrate( _range=elements( meshint ), _expr=cst( 1. ) ).evaluate()( 0,0 );
        double intm4 = integrate( _range=internalelements( mesh ), _expr=cst( 1. ) ).evaluate()( 0,0 );
        BOOST_CHECK_CLOSE( intm3, intm4, 1e-12 );
        //saveGMSHMesh( _mesh=meshint, _filename="meshint" );
#if 0
        auto Xh = space_type::New( mesh );
        auto u = Xh->element();
        auto v = Xh->element();
        auto D = backend->newMatrix( _test=Xh, _trial=Xh );
        form2( Xh, Xh, D, _init=true ) = integrate( _range=internalelements( mesh ), _expr=idt( u )*id( v ) );
        D->close();

        auto Yh = space_type::New( meshint );
        auto ui = Yh->element();
        auto vi = Yh->element();
        auto Fi = backend->newVector( Yh );
        form1( _test=Yh, _vector=Fi, _init=true ) = integrate( _range=elements( meshint ), _expr=id( vi ) );
        Fi->close();

        auto Di = backend->newMatrix( _test=Yh, _trial=Yh );
        form2( _test=Yh, _trial=Yh, _matrix=Di, _init=true ) = integrate( _range=elements( meshint ), _expr=gradt( ui )*trans( grad( vi ) ) );
        Di->close();
        form2( _test=Yh, _trial=Yh, _matrix=Di ) += on( _range=boundaryfaces( meshint ), _element=ui, _rhs=Fi, _expr=cst( 0. ) );

        backend->solve( _matrix=Di, _rhs=Fi, _solution=ui );

        std::shared_ptr<Exporter<mesh_type> > exporter( Exporter<mesh_type>::New( "gmsh", std::string( "submesh" )
                + "_"
                + mesh_type::shape_type::name() ) );

        exporter->step( 0 )->setMesh( meshint );
        exporter->step( 0 )->add( "u", ui );
        exporter->save();

        u = vf::project( Xh, elements( mesh ), cst( 1. ) );
        ui = vf::project( Yh, elements( meshint ), cst( 1. ) );

        BOOST_CHECK_CLOSE( Di->energy( ui, ui ), intm3, 5e-12 );
        BOOST_CHECK_CLOSE( D->energy( u, u ), Di->energy( ui, ui ), 5e-12 );
#endif
    }
    std::shared_ptr<backend_type> backend;
    double meshSize;
    std::string shape;
    mesh_ptrtype mesh;

};

} // Feel
inline
Feel::po::options_description
makeOptions()
{
    Feel::po::options_description integrationoptions( "Test Submesh options" );
    integrationoptions.add_options()
    ( "hsize", Feel::po::value<double>()->default_value( 0.1 ), "h value" )
    ( "shape", Feel::po::value<std::string>()->default_value( "hypercube" ), "shape of the domain (hypercube, simplex, ellipsoid)" )
    ;
    return integrationoptions.add( Feel::feel_options() );
}

inline
Feel::AboutData
makeAbout()
{
    Feel::AboutData about( "test_submesh" ,
                           "test_submesh" ,
                           "0.2",
                           "1D/2D/3D submesh",
                           Feel::AboutData::License_GPL,
                           "Copyright (C) 2010 Université Joseph Fourier (Grenoble I)" );

    about.addAuthor( "Christophe Prud'homme", "developer", "christophe.prudhomme@feelpp.org", "" );
    return about;

}

FEELPP_ENVIRONMENT_WITH_OPTIONS( makeAbout(), makeOptions() )

BOOST_AUTO_TEST_SUITE( submesh )

typedef boost::mpl::list<boost::mpl::int_<1>,boost::mpl::int_<2>,boost::mpl::int_<3> > dim_types;
//typedef boost::mpl::list<boost::mpl::int_<2>,boost::mpl::int_<3> > dim_types;
//typedef boost::mpl::list<boost::mpl::int_<1>,boost::mpl::int_<2> > dim_types;
//typedef boost::mpl::list<boost::mpl::int_<1> > dim_types;
//typedef boost::mpl::list<boost::mpl::int_<2> > dim_types;
//typedef boost::mpl::list<boost::mpl::int_<3> > dim_types;
//typedef boost::mpl::list<boost::mpl::int_<2>,boost::mpl::int_<3>,boost::mpl::int_<1> > dim_types;

#if 1
BOOST_AUTO_TEST_CASE_TEMPLATE( test_submesh, T, dim_types )
{
    if ( ( T::value == 1 ) &&
         Feel::Environment::numberOfProcessors() > 1 )
        return;

    BOOST_TEST_MESSAGE( "Test submesh (" << T::value << "D)" );
    Feel::test_submesh<double,T::value> t;
    t();
    BOOST_TEST_MESSAGE( "Test submesh (" << T::value << "D) done." );
}

typedef boost::mpl::list<boost::mpl::int_<2>,boost::mpl::int_<3> > dim2_types;
BOOST_AUTO_TEST_CASE_TEMPLATE( test_submesh2, T, dim2_types )
{
    using namespace Feel;
    BOOST_TEST_MESSAGE( "Test submesh2 " << T::value << "D" );
    auto mesh = unitHypercube<T::value>();
    auto Xh = Pch<1>( mesh );

    boost::mpi::timer t;
    // with optimization
    auto mesh2 = createSubmesh( _mesh=mesh, _range=boundaryelements( mesh ) );
    auto Yh = Pch<1>( mesh2 );

    auto opI=opInterpolation( _domainSpace=Xh,
                              _imageSpace=Yh,
                              _range=elements( mesh2 ) );
    auto u1 = project( _space=Xh, _range=elements(mesh), _expr=sin(pi*Px()*Py()) );
    auto u2 = Yh->element();
    opI->apply( u1, u2 );

    auto l2error = normL2( _range=boundaryelements(mesh), _expr=idv(u1)-idv(u2) );
    BOOST_CHECK_SMALL( l2error, 1e-14 );
    auto t1 = t.elapsed();t.restart();
    BOOST_TEST_MESSAGE( "Test submesh : elapsed time for optimized version : " << t1 << "s\n" );

    // without optimization
    t.restart();
    auto mesh3 = createSubmesh( _mesh=mesh, _range=boundaryelements( mesh ), _context=0 );
    auto Zh = Pch<1>( mesh3 );

    auto opI2=opInterpolation( _domainSpace=Xh,
                               _imageSpace=Zh,
                               _range=elements( mesh3 ) );
    auto u3 = Yh->element();
    opI2->apply( u1, u3 );

    auto l2error2 = normL2( _range=boundaryelements(mesh), _expr=idv(u1)-idv(u3) );
    BOOST_CHECK_SMALL( l2error2, 1e-14 );

    auto t2 = t.elapsed();t.restart();
    BOOST_TEST_MESSAGE( "Test submesh : elapsed time for non-optimized version : " << t2 << "s\n" );

    //BOOST_CHECK_GT( t2, t1 );

    // with optimization
    auto opI3=opInterpolation( _domainSpace=Yh,
                               _imageSpace=Xh,
                              _range=elements( mesh ) );
    auto u4 = Xh->element();
    opI3->apply( u2, u4 );

    t1 = t.elapsed();t.restart();
    l2error = normL2( _range=boundaryelements(mesh), _expr=idv(u4)-idv(u2) );
    BOOST_CHECK_SMALL( l2error, 1e-14 );
    BOOST_TEST_MESSAGE( "Test submesh : elapsed time for optimized version (transpose) : " << t1 << "s\n" );

    // without optimization
    t.restart();
    auto opI4=opInterpolation( _domainSpace=Zh,
                               _imageSpace=Xh,
                               _range=boundaryelements( mesh ) );
    auto u5 = Xh->element();
    opI4->apply( u3, u5 );

    t2 = t.elapsed();t.restart();
    l2error2 = normL2( _range=boundaryelements(mesh), _expr=idv(u5)-idv(u3) );
    BOOST_CHECK_SMALL( l2error2, 1e-14 );
    BOOST_TEST_MESSAGE( "Test submesh : elapsed time for non-optimized version (transpose) : " << t2 << "s\n" );

    //BOOST_CHECK_GT( t2, t1 );

    // exporter
    auto e1 = exporter( _mesh=mesh, _name=(boost::format("mesh1-%1%d")%T::value).str() );
    e1->add( "u1", u1 );
    e1->add( "u4", u4 );
    e1->add( "u5", u5 );
    e1->save();
    auto e2 = exporter( _mesh=mesh2, _name=(boost::format("mesh2-%1%d")%T::value).str() );
    e2->add( "u2", u2 );
    e2->add( "u3", u3 );
    e2->save();


    BOOST_TEST_MESSAGE( "Test submesh2 "  << T::value << "D done" );
}
#endif
//typedef boost::mpl::list<boost::mpl::int_<2>,boost::mpl::int_<3> > dim2_types;
//typedef boost::mpl::list<boost::mpl::int_<2> > dim2_types;
BOOST_AUTO_TEST_CASE_TEMPLATE( test_submesh3, T, dim2_types )
{
    using namespace Feel;
    BOOST_TEST_MESSAGE( "Test submesh3 " << T::value << "D" );
    double meshSize = ( T::value==3 )? 0.5 : 0.1;
    auto mesh = unitHypercube<T::value>(meshSize);
    auto Xh = Pch<1>( mesh );
    auto v = Xh->element();
    v = project( _space=Xh, _range=elements(mesh), _expr=cst(1.) );

    BOOST_TEST_MESSAGE("optimized version");
    // with optimization
    auto mesh2 = createSubmesh( _mesh=mesh, _range=elements( mesh ) );
    auto Yh = Pch<2>( mesh2 );
    auto u = Yh->element();

    boost::mpi::timer t;

    auto a = form2( _test=Xh, _trial=Yh );
    a = integrate( _range=elements(mesh2), _expr=idt(u)*id(v) );
    u = project( _space=Yh, _range=elements(mesh2), _expr=Px()*Py() );
    double mass1 = a( v, u );
    BOOST_CHECK_CLOSE( mass1, .25, 4e-13 );
    BOOST_TEST_MESSAGE( "time mass matrix : " << t.elapsed() << "s\n" );

    t.restart();
    auto c = form1( _test=Xh );
    c = integrate( _range=elements(mesh), _expr=idv(u)*id(v) );
    double mass3 = c( v );
    BOOST_CHECK_CLOSE( mass3, .25, 1e-12 );
    BOOST_TEST_MESSAGE( "time linear form (opt) : " << t.elapsed() << "s\n" );

    if (Environment::worldComm().size() == 1)
    {
    BOOST_TEST_MESSAGE("non optimized version");
    // without optimization
    auto mesh3 = createSubmesh( _mesh=mesh, _range=allelements( mesh ), _context=0 );
    BOOST_TEST_MESSAGE( "mesh generated" );
    //LOG(INFO) << "mesh generated\n";     google::FlushLogFiles(google::GLOG_INFO);
    BOOST_CHECK_EQUAL( mesh->numElements(), mesh3->numElements() );
    auto Zh = Pch<2>( mesh3 );
    LOG(INFO) << "space generated\n";     google::FlushLogFiles(google::GLOG_INFO);
    auto w = Zh->element();
    LOG(INFO) << "element generated\n";     google::FlushLogFiles(google::GLOG_INFO);

    t.restart();
    auto b = form2( _test=Xh, _trial=Zh );
    LOG(INFO) << "form generated\n";     google::FlushLogFiles(google::GLOG_INFO);
    b = integrate( _range=elements(mesh3), _expr=idt(w)*id(v) );
    LOG(INFO) << "b computed\n";     google::FlushLogFiles(google::GLOG_INFO);
    w = project( _space=Zh, _range=elements(mesh3), _expr=Px()*Py() );
    LOG(INFO) << "w computed\n";     google::FlushLogFiles(google::GLOG_INFO);
    double mass2 = b( v, w );
    LOG(INFO) << "energy computed\n";     google::FlushLogFiles(google::GLOG_INFO);
    BOOST_CHECK_CLOSE( mass2, .25, 1e-12 );
    BOOST_TEST_MESSAGE( "time mass matrix : " << t.elapsed() << "s\n" );
    //BOOST_CHECK_CLOSE( mass1, mass2, 1e-14 );

    t.restart();
    auto d = form1( _test=Xh );
    d = integrate( _range=elements(mesh), _expr=idv(w)*id(v) );
    double mass4 = d( v );
    BOOST_CHECK_CLOSE( mass4, .25, 1e-12 );
    BOOST_TEST_MESSAGE( "time linear form (non opt) : " << t.elapsed() << "s\n" );
    BOOST_TEST_MESSAGE( "Test submesh3 "  << T::value << "D done" );
    } // if (Environment::worldComm().size() == 1)

}

BOOST_AUTO_TEST_CASE_TEMPLATE( test_submesh_view, T, dim2_types )
    //BOOST_AUTO_TEST_CASE( test_submesh_view, T, dim2_types )
{
    using namespace Feel;
    static const int nDim = T::value;
    typedef Mesh<Simplex<nDim>> mesh_type;

    std::shared_ptr<mesh_type> mesh;
    int nBoundaryMarker = 4;
    std::string exprDispStr = "{0,0.1*sin(3*pi*x)}:x";
    double meshSize = doption(_name="gmsh.hsize");
    if constexpr ( nDim == 2 )
    {
        GeoTool::Node x1( 0,0 );
        GeoTool::Node x2( 1,1 );
        GeoTool::Rectangle R( meshSize,"OMEGA",x1,x2 );
        R.setMarker(_type="line",_name="Boundary0",_marker1=true);
        R.setMarker(_type="line",_name="Boundary1",_marker2=true);
        R.setMarker(_type="line",_name="Boundary2",_marker3=true);
        R.setMarker(_type="line",_name="Boundary3",_marker4=true);
        R.setMarker(_type="surface",_name="Omega",_markerAll=true);
        mesh = R.createMesh( _mesh=new mesh_type,_name="domainRectangle" );
    }
    else
    {
        GeoTool::Node x1(0,0,0);
        GeoTool::Node x2(1,1,1);
        GeoTool::Cube C( meshSize,"OMEGA",x1,x2);
        C.setMarker(_type="surface",_name="Boundary0",_marker1=true);
        C.setMarker(_type="surface",_name="Boundary1",_marker2=true);
        C.setMarker(_type="surface",_name="Boundary2",_marker3=true);
        C.setMarker(_type="surface",_name="Boundary3",_marker4=true);
        C.setMarker(_type="surface",_name="Boundary4",_marker5=true);
        C.setMarker(_type="surface",_name="Boundary5",_marker6=true);
        C.setMarker(_type="volume",_name="Omega",_markerAll=true);
        mesh = C.createMesh( _mesh=new mesh_type,_name="domainCube" );
        nBoundaryMarker=6;
        exprDispStr = "{0,0.1*sin(3*pi*x),0.1*sin(3*pi*y)}:x:y";
    }

    std::vector<Range<mesh_type,MESH_FACES>> rangeSubmesh;
    std::vector< typename mesh_type::template trace_mesh_ptrtype<>> submesh;
    std::vector<double> measureInitial;
    for (int k=0;k<nBoundaryMarker;++k)
    {
        rangeSubmesh.push_back( markedfaces(mesh,(boost::format("Boundary%1%")%k).str() ) );
        submesh.push_back( createSubmesh( _mesh=mesh, _range=rangeSubmesh[k],_view=true ) );

        double measureParent = measure(_range=rangeSubmesh[k] );
        double measureSubmesh = measure(_range=elements(submesh[k]));
        double measureSubmesh2 = submesh[k]->measure();
        BOOST_CHECK_CLOSE( measureParent, measureSubmesh, 1e-10 );
        if ( false ) // NOT WORK IN 3D, need to understant the bug with submesh[k]->measure();
            BOOST_CHECK_CLOSE( measureParent, measureSubmesh2, 1e-10  );
        measureInitial.push_back( measureParent );
    }

    auto Vh = Pchv<1>( mesh );
    auto uDisp = Vh->element();

    auto exprDisp = expr<mesh_type::nDim,1>( exprDispStr );
    uDisp.on(_range=elements(mesh),_expr=exprDisp );

    meshMove( mesh,uDisp );

    for (int k=0;k<nBoundaryMarker;++k)
    {
        double measureParent = measure(_range=rangeSubmesh[k] );
        double measureSubmesh = measure(_range=elements(submesh[k]));
        double measureSubmesh2 = submesh[k]->measure();
        BOOST_CHECK_CLOSE( measureParent, measureSubmesh, 1e-10  );
        if ( false ) // NOT WORK IN 3D, need to understant the bug with submesh[k]->measure();
            BOOST_CHECK_CLOSE( measureParent, measureSubmesh2, 1e-10  );
    }

    auto submeshFace = createSubmesh( _mesh=mesh,_range=faces(mesh),_view=true,_update=0 );
    auto VhF = Pchv<1>( submeshFace );
    auto uDispF = VhF->element();

    //uDispF.on(_range=elements(submeshFace),_expr= -idv(uDisp) ); // use localization, not mesh relation
    uDispF.on(_range=faces(mesh),_expr= -idv(uDisp) );

    meshMove( submeshFace,uDispF );

    for (int k=0;k<nBoundaryMarker;++k)
    {
        double measureParent = measure(_range=rangeSubmesh[k] );
        double measureSubmesh = measure(_range=elements(submesh[k]));
        double measureSubmesh2 = submesh[k]->measure();
        BOOST_CHECK_CLOSE( measureParent, measureSubmesh, 1e-10  );
        if ( false ) // NOT WORK IN 3D, need to understant the bug with submesh[k]->measure();
            BOOST_CHECK_CLOSE( measureParent, measureSubmesh2, 1e-10  );
        BOOST_CHECK_CLOSE( measureParent, measureInitial[k], 1e-10  );
    }

    // check meshesWithNodesShared data
    int nMeshWithNodeShared = 2 + submesh.size();
    BOOST_CHECK_EQUAL( mesh->meshesWithNodesShared().size(), nMeshWithNodeShared );
    for ( int k=0;k< submesh.size();++k )
        BOOST_CHECK_EQUAL( submesh[k]->meshesWithNodesShared().size(), nMeshWithNodeShared );
    BOOST_CHECK_EQUAL( submeshFace->meshesWithNodesShared().size(), nMeshWithNodeShared );
    submeshFace->clear();
    --nMeshWithNodeShared;
    BOOST_CHECK_EQUAL( mesh->meshesWithNodesShared().size(), nMeshWithNodeShared );
    for ( int k=0;k< submesh.size();++k )
        BOOST_CHECK_EQUAL( submesh[k]->meshesWithNodesShared().size(), nMeshWithNodeShared );
}

BOOST_AUTO_TEST_SUITE_END()
