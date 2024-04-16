/* -*- mode: c++; coding: utf-8; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; show-trailing-whitespace: t -*-

   This file is part of the Feel library

   Author(s): JB WAHL <wahl.jb@gmail.com>
   Date: 2018-03-27

   Copyright (C) 2018 Feel++ Consortium

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
   \file deim.hpp
   \author JB Wahl
   \date 2018-03-27
 */
#ifndef FEELPP_CRB_DEIM_H
#define FEELPP_CRB_DEIM_H

#include <feel/feelmor/deimmodel.hpp>
#include <feel/feelmor/mdeim.hpp>


namespace Feel
{

template <typename ModelType>
class DEIM :
        public DEIMModel<ModelType,typename Backend<typename ModelType::value_type>::vector_type>
{
    typedef DEIMModel<ModelType,typename Backend<typename ModelType::value_type>::vector_type>  super_type;

public :
    typedef ModelType model_type;
    typedef std::shared_ptr<model_type> model_ptrtype;
    typedef typename super_type::parameter_type parameter_type;
    typedef typename super_type::parameterspace_ptrtype parameterspace_ptrtype;
    typedef typename super_type::sampling_ptrtype sampling_ptrtype;
    typedef typename super_type::tensor_ptrtype vector_ptrtype;
    typedef typename super_type::element_type element_type;
    typedef typename super_type::mesh_type mesh_type;
    typedef typename super_type::space_type space_type;

    DEIM() :
        super_type()
    {}

    DEIM( model_ptrtype model,
          sampling_ptrtype sampling, std::string prefix,
          std::string const& dbfilename, std::string const& dbdirectory, int tag, po::variables_map vm ) :
        super_type( model, sampling, prefix, dbfilename, dbdirectory, tag, vm )
    {
        this->M_store_tensors = boption( _name=prefixvm( this->M_prefix, "deim.store-vectors"), _vm=this->M_vm );
        this->init();
    }

    ~DEIM()
    {}

private :
    vector_ptrtype modelAssemble( parameter_type const& mu, bool online=false ) override
    {
        if ( online )
        {
            CHECK( this->M_online_model ) << "this->M_online_model not init : " << this->M_tag;
            return this->M_online_model->assembleForDEIM(mu,this->M_tag);
        }
        return this->M_model->assembleForDEIM(mu,this->M_tag);
    }

    vector_ptrtype modelAssemble( parameter_type const& mu, element_type const& u, bool online=false ) override
    {
        if ( online )
            return this->M_online_model->assembleForDEIMnl(mu,u,this->M_tag);
        return this->M_model->assembleForDEIMnl(mu,u,this->M_tag);
    }

    virtual void updateSubMesh() override
    {
        auto Xh = this->M_model->functionSpace();
        auto mesh = Xh->mesh();

        this->updateEltsId( this->M_index );

        auto submesh = createSubmesh( _mesh=mesh,
                                      _range=idelements(mesh, this->M_elts_ids.begin(), this->M_elts_ids.end()),
                                      _context=EXTRACTION_KEEP_MESH_RELATION);
        saveGMSHMesh( _mesh=submesh, _filename=this->name(true)+"-submesh.msh" );
        Environment::worldComm().barrier();

        auto seqmesh = loadMesh( _mesh=new mesh_type,
                                 _filename=this->name(true)+"-submesh.msh",
                                 _worldcomm= Environment::worldCommSeqPtr() );

        Rh = this->newInterpolationSpace(seqmesh);

        this->M_map->init( Rh, Xh );

        this->M_indexR.clear();
        for ( int i=0; i<this->M_index.size(); i++ )
        {
            auto index = this->M_index[i];
            int index_r = this->M_map->clusterToSequential( index );
            CHECK( index_r>=0 )<< "No matching reduced index in the map for Xh index "
                               << index <<std::endl;
            this->M_indexR.push_back( index_r );
        }

        this->M_online_model->setFunctionSpaces( Rh );
    }

private :
    using super_type::Rh;
};

template <typename ... Ts>
auto deim( Ts && ... v )
{
    auto args = NA::make_arguments( std::forward<Ts>(v)... );
    auto && model = args.get(_model);
    auto && sampling = args.get_else(_sampling, nullptr);
    std::string const& prefix = args.get_else(_prefix,"");
    std::string const& filename = args.get_else(_filename,"");
    std::string const& directory = args.get_else(_directory,"");
    int tag = args.get_else(_tag,0);
    using model_type = Feel::remove_shared_ptr_type<std::remove_pointer_t<std::decay_t<decltype(model)>>>;

    po::variables_map vm;
    auto _options = deimOptions(prefix);
    auto mycmdparser = Environment::commandLineParser();
    po::parsed_options parsed = mycmdparser.options( _options ).
        style(po::command_line_style::allow_long | po::command_line_style::long_allow_adjacent | po::command_line_style::long_allow_next).
        allow_unregistered().run();
    po::store(parsed,vm);
    for ( auto & configFile : Environment::configFiles() )
    {
        std::istringstream & iss = std::get<1>( configFile );
        po::store(po::parse_config_file(iss, _options,true), vm);
    }
    po::notify(vm);

    return std::make_shared<DEIM<model_type>>( model, sampling, prefix, filename, directory, tag, vm );
}



} //namespace Feel

#endif
