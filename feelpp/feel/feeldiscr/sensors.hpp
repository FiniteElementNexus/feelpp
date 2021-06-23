//! -*- mode: c++; coding: utf-8; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; show-trailing-whitespace: t  -*- vim:fenc=utf-8:ft=cpp:et:sw=4:ts=4:sts=4
//!
//! This file is part of the Feel++ library
//!
//! This library is free software; you can redistribute it and/or
//! modify it under the terms of the GNU Lesser General Public
//! License as published by the Free Software Foundation; either
//! version 2.1 of the License, or (at your option) any later version.
//!
//! This library is distributed in the hope that it will be useful,
//! but WITHOUT ANY WARRANTY; without even the implied warranty of
//! MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//! Lesser General Public License for more details.
//!
//! You should have received a copy of the GNU Lesser General Public
//! License along with this library; if not, write to the Free Software
//! Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//!
//! @file
//! @author Christophe Prud'homme <christophe.prudhomme@feelpp.org>
//! @author Idrissa Niakh <>
//! @date 14 May 2019
//! @copyright 2019 Feel++ Consortium
//!
#ifndef FEELPP_CRB_SENSORS_HPP
#define FEELPP_CRB_SENSORS_HPP 1

#include <feel/feelvf/vf.hpp>
#include <feel/feeldiscr/fsfunctionallinear.hpp>
#include <feel/feelcore/json.hpp>

namespace Feel
{

//!
//! base class for sensors
//!
template<typename Space, typename ValueT = double>
class SensorBase: public FsFunctionalLinear<Space>
{
public:

    // -- TYPEDEFS --
    typedef SensorBase<Space> this_type;
    typedef FsFunctionalLinear<Space> super_type;

    using space_type = typename super_type::space_type;
    using space_ptrtype = typename super_type::space_ptrtype;
    using node_t = typename space_type::mesh_type::node_type;
    using mesh_type = typename space_type::mesh_type;


    SensorBase() = default;
    SensorBase( space_ptrtype const& space, node_t const& p, std::string const& name = "sensor" , std::string const& t = ""):
        super_type( space ),
        M_name( name ),
        M_position( p ),
        M_type( t )
    {}

    virtual ~SensorBase() = default;

    void setSpace( space_ptrtype const& space ) override
    {
        super_type::setSpace(space);
        this->init();
    }
    virtual void init() = 0;

    void setName( std::string const& n ) { M_name = n; }
    std::string const& name() const { return M_name; }

    void setPosition( node_t const& p ) { M_position = p; this->init(); }
    node_t const& position() const { return M_position; }

    std::string const& type() const { return M_type; }
    virtual json to_json() const
    {
        json j;
        j["type"] = M_type;
        j["coord"] = json::array();
        for(int i = 0; i < space_type::nDim; ++i)
            j["coord"].push_back(M_position(i));
        return j;
    }

    //!
    //! other interface may include:
    //!  - DB storage for past measurements
    //!  - DB storage for future measurements
    //!
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & M_name;
        ar & M_position;
        ar & M_type;
    }

protected:
    std::string M_name;
    node_t M_position;
    std::string M_type;
};

//!
//! pointwise type sensor
//!
template<typename Space>
class SensorPointwise: public SensorBase<Space>
{
public:

    // -- TYPEDEFS --
    typedef SensorPointwise<Space> this_type;
    typedef SensorBase<Space> super_type;

    using space_type = typename super_type::space_type;
    using space_ptrtype = typename super_type::space_ptrtype;
    using node_t = typename super_type::node_t;

    SensorPointwise() = default;
    SensorPointwise( space_ptrtype const& space, node_t const& p, std::string const& n = "pointwise"):
        super_type( space, p, n, "pointwise" )
    {
        this->init();
    }

    virtual ~SensorPointwise(){}

    void init() override
    {
        // auto v=this->space()->element();
        //auto expr=integrate(_range=elements(M_space->mesh()), _expr=id(v)*phi);
        //super_type::operator=( expr );
        // this->close();
    }

private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<super_type>(*this);
    }
};


//!
//! gaussian type sensor
//!
template<typename Space>
class SensorGaussian: public SensorBase<Space>
{
public:

    // -- TYPEDEFS --
    typedef SensorGaussian<Space> this_type;
    typedef SensorBase<Space> super_type;

    using space_type = typename super_type::space_type;
    using space_ptrtype = typename super_type::space_ptrtype;
    using mesh_type = typename super_type::mesh_type;
    using node_t = typename super_type::node_t;
    static const int nDim = space_type::mesh_type::nDim;

    SensorGaussian() = default;
    SensorGaussian( space_ptrtype const& space, node_t const& center,
                    double radius = 1., std::string const& n = "gaussian"):
        super_type( space, center, n, "gaussian" ),
        M_radius( radius )
    {
        this->init();
    }

    virtual ~SensorGaussian(){}

    void setRadius( double radius ) { M_radius = radius; this->init(); }
    double radius() { return M_radius; }

    void init() override
    {
        auto v = this->space()->element();
        auto phi = this->phiExpr( mpl::int_< space_type::nDim >() );
        auto n = integrate(_range=elements(this->space()->mesh()), _expr=phi).evaluate()(0,0);
        form1( _test=this->space(), _vector=this->containerPtr() ) =
            integrate(_range=elements(this->space()->mesh()), _expr=id(v)*phi/n );
        this->close();
    }

    json to_json() const override
    {
        json j = super_type::to_json();
        j["radius"] = M_radius;
        return j;
    }

private:
    auto phiExpr( mpl::int_<1> /**/ )
    {
        return exp( -inner(P()-vec(cst(this->M_position[0])))/(2*std::pow(M_radius,2)));
    }
    auto phiExpr( mpl::int_<2> /**/ )
    {
        return exp( -inner(P()-vec(cst(this->M_position[0]),cst(this->M_position[1])))/(2*std::pow(M_radius,2)));
    }
    auto phiExpr( mpl::int_<3> /**/ )
    {
        return exp( -inner(P()-vec(cst(this->M_position[0]),cst(this->M_position[1]),cst(this->M_position[2])))/(2*std::pow(M_radius,2)));
    }

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<super_type>(*this);
        ar & M_radius;
    }

    double M_radius;
};

template<typename Space>
class SensorMap : public std::map<std::string, std::shared_ptr<SensorBase<Space>>>
{
public:
    using this_type = SensorMap<Space>;
    using super_type = std::map<std::string, std::shared_ptr<SensorBase<Space>>>;
    using value_type = typename super_type::value_type;
    using element_type = SensorBase<Space>;

    using space_type = Space;
    using space_ptrtype = std::shared_ptr<space_type>;
    using fselement_type = typename space_type::element_type;

    using node_t = typename element_type::node_t;

    SensorMap() = default;
    explicit SensorMap( space_ptrtype const& Xh ): M_Xh(Xh) {}
    explicit SensorMap( space_ptrtype const& Xh, json j)
        : M_Xh(Xh),
          M_j(j)
    {
        for( auto const& /*it*/[name, sensor] : M_j.items() )
        {
            // auto name = it.first;
            // auto sensor = it.second;
            std::string type = sensor.value("type", "");
            node_t n(space_type::nDim);
            auto coords = sensor["coord"].template get<std::vector<double>>();
            for( int i = 0; i < space_type::nDim; ++i )
                n(i) = coords[i];

            if( type == "pointwise" )
            {
                auto s = std::make_shared<SensorPointwise<space_type>>(M_Xh, n, name);
                this->insert(value_type(name, s));
            }
            else if( type == "gaussian" )
            {
                double radius = sensor.value("radius", 0.1);
                auto s = std::make_shared<SensorGaussian<space_type>>(M_Xh, n, radius, name);
                this->insert(value_type(name, s));
            }
        }
    }

    Eigen::VectorXd apply(fselement_type const& u) const
    {
        Eigen::VectorXd v(this->size());
        int i = 0;
        for( auto const& it /*[name, sensor]*/ : *this )
            v(i++) = (*it.second)(u);
        return v;
    }

    json to_json()
    {
        if( M_j.empty() )
        {
            for( auto const& it /*[name, sensor]*/ : *this )
            {
                auto name = it.first;
                auto sensor = it.second;
                M_j[name] = sensor->to_json();
            }
        }
        return M_j;
    }

private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        // add any subtype of SensorBase here
        ar.template register_type<SensorPointwise<space_type>>();
        ar.template register_type<SensorGaussian<space_type>>();
        ar & boost::serialization::base_object<super_type>(*this);
        if( Archive::is_loading::value )
        {
            for( auto& it /*[name, sensor]*/ : *this)
                it.second->setSpace(M_Xh);
        }
    }

    space_ptrtype M_Xh;
    json M_j;
};

}
#endif /* _FEELPP_CRB_SENSORS_HPP */
