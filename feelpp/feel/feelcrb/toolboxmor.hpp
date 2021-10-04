#ifndef FEELPP_TOOLBOX_MOR_HPP
#define FEELPP_TOOLBOX_MOR_HPP

#include <feel/options.hpp>
#include <feel/feelalg/backend.hpp>
#include <feel/feelfilters/gmsh.hpp>
#include <feel/feelcrb/modelcrbbase.hpp>
#include <feel/feelmodels/modelproperties.hpp>

namespace Feel
{

namespace po = boost::program_options;

FEELPP_EXPORT po::options_description
makeToolboxMorOptions();
FEELPP_EXPORT AboutData
makeToolboxMorAbout( std::string const& str = "opusheat-tb" );

template<typename SpaceType, int Options = 0>
class FEELPP_EXPORT ToolboxMor : public ModelCrbBase< ParameterSpace<>, SpaceType, Options>
{
    using self_type = ToolboxMor<SpaceType, Options>;
    using super_type = ModelCrbBase< ParameterSpace<>, SpaceType, Options>;

  public:

    typedef typename super_type::beta_vector_light_type beta_vector_light_type;
    typedef typename super_type::affine_decomposition_light_type affine_decomposition_light_type;

    typedef typename super_type::element_type element_type;
    typedef typename super_type::element_ptrtype element_ptrtype;
    typedef typename super_type::space_type space_type;
    using mesh_ptrtype = typename super_type::mesh_ptrtype;

    using super_type::computeBetaQm;
    using parameter_type = typename super_type::parameter_type;
    using vectorN_type = typename super_type::vectorN_type;
    using beta_vector_type = typename super_type::beta_vector_type;
    using affine_decomposition_type = typename super_type::affine_decomposition_type;
    using sampling_type = typename ParameterSpace<>::sampling_type;
    using sampling_ptrtype = std::shared_ptr<sampling_type>;

    using vector_ptrtype = typename super_type::vector_ptrtype;
    using sparse_matrix_ptrtype = typename super_type::sparse_matrix_ptrtype;

    using deim_type = DEIM<self_type>;
    using deim_ptrtype = std::shared_ptr<deim_type>;
    using mdeim_type = MDEIM<self_type>;
    using mdeim_ptrtype = std::shared_ptr<mdeim_type>;
    using deim_function_type = std::function<vector_ptrtype(parameter_type const&)>;
    using mdeim_function_type = std::function<sparse_matrix_ptrtype(parameter_type const&)>;

    explicit ToolboxMor(std::string const& prefix = "");

    void setAssembleDEIM(deim_function_type const& fct ) { M_assembleForDEIM = fct; }
    void setAssembleMDEIM(mdeim_function_type const& fct ) { M_assembleForMDEIM = fct; }
    mesh_ptrtype getDEIMReducedMesh() { return M_deim->onlineModel()->functionSpace()->mesh(); }
    mesh_ptrtype getMDEIMReducedMesh() { return M_mdeim->onlineModel()->functionSpace()->mesh(); }
    void setOnlineAssembleDEIM(deim_function_type const& fct ) { M_deim->onlineModel()->setAssembleDEIM(fct); }
    void setOnlineAssembleMDEIM(mdeim_function_type const& fct ) { M_mdeim->onlineModel()->setAssembleMDEIM(fct); }

    void initModel() override;
    void postInitModel();

    sparse_matrix_ptrtype assembleForMDEIM( parameter_type const& mu, int const& tag ) override;
    vector_ptrtype assembleForDEIM( parameter_type const& mu, int const& tag ) override;

    typename super_type::betaqm_type
    computeBetaQm( parameter_type const& mu , double time , bool only_terms_time_dependent=false ) override;
    typename super_type::betaqm_type
    computeBetaQm( parameter_type const& mu ) override;

    double output( int output_index, parameter_type const& mu, element_type &T, bool need_to_solve=false ) override;

    // element_type solve(parameter_type const& mu) override;

    void setupSpecificityModel( boost::property_tree::ptree const& ptree, std::string const& dbDir ) override;
    void updateSpecificityModel( boost::property_tree::ptree & ptree ) const override;


  private :
    void assembleData();

    void updateBetaQ_impl( parameter_type const& mu , double time , bool only_terms_time_dependent );

    template<typename ModelType>
    typename super_type::betaqm_type
    computeBetaQ_impl( parameter_type const& mu , double time , bool only_terms_time_dependent=false,
                       typename std::enable_if< !ModelType::is_time_dependent >::type* = nullptr )
        {
            this->updateBetaQ_impl( mu,time,only_terms_time_dependent);
            return boost::make_tuple( this->M_betaAqm, this->M_betaFqm );
        }

    template<typename ModelType>
    typename super_type::betaqm_type
    computeBetaQ_impl( parameter_type const& mu , double time , bool only_terms_time_dependent=false,
                       typename std::enable_if< ModelType::is_time_dependent >::type* = nullptr )
        {
            this->updateBetaQ_impl( mu,time,only_terms_time_dependent);
            return boost::make_tuple( this->M_betaMqm, this->M_betaAqm, this->M_betaFqm );
        }


    template<typename ModelType>
    typename super_type::betaqm_type
    computeBetaQ_impl( parameter_type const& mu, typename std::enable_if< !ModelType::is_time_dependent >::type* = nullptr )
        {
            this->updateBetaQ_impl( mu,0,false);
            return boost::make_tuple( this->M_betaAqm, this->M_betaFqm );
        }

    template<typename ModelType>
    typename super_type::betaqm_type
    computeBetaQ_impl( parameter_type const& mu, typename std::enable_if< ModelType::is_time_dependent >::type* = nullptr )
        {
            this->updateBetaQ_impl( mu,0,false);
            return boost::make_tuple( this->M_betaMqm, this->M_betaAqm, this->M_betaFqm );
        }

    int Qa();
    int Nl();
    int Ql( int l );
    int mQA( int q );
    int mLQF( int l, int q );
    void resizeQm( bool resizeMat = true );

private:

    std::string M_propertyPath;
    std::shared_ptr<ModelProperties> M_modelProperties;

    int M_trainsetDeimSize;
    int M_trainsetMdeimSize;

    deim_ptrtype M_deim;
    mdeim_ptrtype M_mdeim;

    deim_function_type M_assembleForDEIM;
    mdeim_function_type M_assembleForMDEIM;

}; // class ToolboxMor

} // namespace Feel

#include "toolboxmor_impl.cpp"

#endif
