#include "zensim/Logger.hpp"
#include "zensim/cuda/execution/ExecutionPolicy.cuh"
#include "zensim/omp/execution/ExecutionPolicy.hpp"
#include "zensim/geometry/PoissonDisk.hpp"
#include "zensim/geometry/VdbLevelSet.h"
#include "zensim/geometry/VdbSampler.h"
#include "zensim/io/MeshIO.hpp"
#include "zensim/math/bit/Bits.h"
#include "zensim/types/Property.h"
#include <atomic>
#include <zeno/VDBGrid.h>
#include <zeno/types/ListObject.h>
#include <zeno/types/NumericObject.h>
#include <zeno/types/PrimitiveObject.h>
#include <zeno/types/StringObject.h>

#include "kernel/gradient_field.hpp"
#include "kernel/tiled_vector_ops.hpp"
#include "zensim/container/Bvh.hpp"
#include "zensim/geometry/AnalyticLevelSet.h"
#include "zensim/math/MathUtils.h"


#include "Utils.hpp"
// #include "zensim/geometry/AnalyticLevelSet.h"


namespace zeno {

struct ZSEvalGradientField : zeno::INode {
    using T = float;
    using dtiles_t = zs::TileVector<T,32>;
    using tiles_t = typename ZenoParticles::particles_t;
    using vec3 = zs::vec<T,3>;
    using mat3 = zs::vec<T,3,3>;

    virtual void apply() override {
        using namespace zs;
        auto field = get_input<ZenoParticles>("field");
        auto& verts = field->getParticles();

        auto attr = get_param<std::string>("tag");
        auto attrg = get_param<std::string>("gtag");
        auto skip_boundary_gradient = get_param<int>("skip_boundary");
        auto btag = get_param<std::string>("btag");
        auto normalize = get_param<int>("normalize");

        if(!verts.hasProperty(attr)){
            fmt::print("the input field does not contain specified channel:{}\n",attr);
            throw std::runtime_error("the input field does not contain specified channel");
        }
        if(verts.getPropertySize(attr) != 1){
            fmt::print("only scaler field is currently supported\n");
            throw std::runtime_error("only scaler field is currently supported");
        }

        auto& eles = field->getQuadraturePoints();
        auto simplex_size = eles.getPropertySize("inds");
        if(simplex_size != 4 && simplex_size != 3)
            throw std::runtime_error("ZSEvalGradientField: invalid simplex size");

        static dtiles_t etemp(eles.get_allocator(),{{"g",3}},eles.size());
        static dtiles_t vtemp{verts.get_allocator(),{
            {"T",1},
        },verts.size()};

        etemp.resize(eles.size());
        vtemp.resize(verts.size());

        constexpr auto space = execspace_e::cuda;
        auto cudaPol = cuda_exec();

        eles.append_channels(cudaPol,{{attrg,3}});
        // copy the scaler field from verts to vtemp
        cudaPol(zs::range(verts.size()),
            [verts = proxy<space>({},verts),vtemp = proxy<space>({},vtemp),attr = zs::SmallString(attr),tag = zs::SmallString("T")]
                ZS_LAMBDA(int vi) mutable {
                    vtemp(tag,vi) = verts(attr,vi);
        });
        // compute_gradient(cudaPol,eles,verts,"x",vtemp,"T",etemp,"g",zs::wrapv(simplex_size));
        if(simplex_size == 4)
            compute_gradient<4>(cudaPol,eles,verts,"x",vtemp,"T",etemp,"g");
        if(simplex_size == 3)
            compute_gradient<3>(cudaPol,eles,verts,"x",vtemp,"T",etemp,"g");
        // copy the gradient field from etemp to eles
        cudaPol(zs::range(eles.size()),
            [verts = proxy<space>({},verts),eles = proxy<space>({},eles),etemp = proxy<space>({},etemp),gtag = zs::SmallString(attrg),normalize,skip_boundary_gradient,btag = zs::SmallString(btag),simplex_size]
                ZS_LAMBDA(int ei) mutable {
                    bool on_bou = false;
                    if(simplex_size == 3){
                        auto inds = eles.template pack<3>("inds",ei).reinterpret_bits<int>();
                        for(int i = 0;i < simplex_size;++i){
                            auto b = verts(btag,inds[i]);
                            if(b > 1e-6)
                                on_bou = true;
                        }
                    }
                    if(simplex_size == 4){
                        auto inds = eles.template pack<4>("inds",ei).reinterpret_bits<int>();
                        for(int i = 0;i < simplex_size;++i){
                            auto b = verts(btag,inds[i]);
                            if(b > 1e-6)
                                on_bou = true;
                        }
                    }

                    if(on_bou && skip_boundary_gradient){
                        eles.tuple<3>(gtag,ei) = vec3::zeros();
                    }else {
                        float alpha = 1;
                        if(normalize)
                            alpha = (etemp.template pack<3>("g",ei).norm());
                        eles.template tuple<3>(gtag,ei) = etemp.template pack<3>("g",ei)/alpha;
                        if(ei == 0)
                            printf("eles_grad : %f %f %f\n",(float)eles.template pack<3>(gtag,ei)[0],
                                        (float)eles.template pack<3>(gtag,ei)[1],
                                        (float)eles.template pack<3>(gtag,ei)[2]);

                    }
        });
        set_output("field",field);
    }
};

ZENDEFNODE(ZSEvalGradientField, {
                                    {"field"},
                                    {"field"},
                                    {
                                        {gParamType_String,"tag","T"},{gParamType_String,"gtag","gradT"},{gParamType_Int,"skip_boundary","0"},{gParamType_Int,"normalize","0"},{gParamType_String,"btag","btag"}
                                    },
                                    {"ZSGeometry"}
});


struct HeatmapObject : zeno::IObject {
    std::vector<zeno::vec3f> colors;

    zeno::vec3f interp(float x) const {
        x = zeno::clamp(x, 0, 1) * colors.size();
        int i = (int)zeno::floor(x);
        i = zeno::clamp(i, 0, colors.size() - 2);
        float f = x - i;
        return (1 - f) * colors.at(i) + f * colors.at(i + 1);
    }
};

struct ZSRetrieveVectorField : zeno::INode {
    using T = float;
    using dtiles_t = zs::TileVector<T,32>;
    using tiles_t = typename ZenoParticles::particles_t;
    using vec3 = zs::vec<T,3>;
    using mat3 = zs::vec<T,3,3>;


    virtual void apply() override {
        using namespace zs;
        auto field = get_input<ZenoParticles>("field");
        const auto& verts = field->getParticles();
        const auto& eles = field->getQuadraturePoints(); 

        auto type = get_param<std::string>("location");
        auto gtag = get_param<std::string>("gtag");
        auto xtag = get_param<std::string>("xtag");
        // auto normalize = get_param<int>("normalize");
        auto scale = (T)get_param<float>("scale");
        auto color_tag = get_param<std::string>("color_tag");

        bool on_elm = (type == "quad" || type == "tri");

        if((type == "quad" || type == "tri") && (!eles.hasProperty(gtag))){
            if(!eles.hasProperty(gtag))
                fmt::print("the elements does not contain element-wise gradient field : {}\n",gtag);
            // if(!eles.hasProperty(color_tag))
            //     fmt::print("the elements does not contain element-wise color_tag field : {}\n",color_tag);
            throw std::runtime_error("the volume does not contain element-wise gradient field");
        }
        if(type == "vert" && !verts.hasProperty(gtag)){
            fmt::print("the volume does not contain nodal-wize gradient field : {}\n",gtag);
            throw std::runtime_error("the volume does not contain nodal-wize gradient field");
        }
        if(!verts.hasProperty(xtag)){
            fmt::print("the volume does not contain specified position channel : {}\n",xtag);
            throw std::runtime_error("the volume does not contain specified position channel");
        }

        std::vector<zs::PropertyTag> tags{{"x",3},{"vec",3}};

        auto vec_buffer = typename ZenoParticles::particles_t(tags,on_elm ? eles.size() : verts.size(),zs::memsrc_e::device);
        auto zsvec_buffer = zs::Vector<float>(on_elm ? eles.size() : verts.size(),zs::memsrc_e::device);
        // transfer the data from gpu to cpu
        constexpr auto cuda_space = execspace_e::cuda;
        auto cudaPol = cuda_exec();

        int elm_dim = eles.getPropertySize("inds");

        cudaPol(zs::range(vec_buffer.size()),
            [vec_buffer = proxy<cuda_space>({},vec_buffer),verts = proxy<cuda_space>({},verts),eles = proxy<cuda_space>({},eles),
                gtag = zs::SmallString(gtag),xtag = zs::SmallString(xtag),color_tag = zs::SmallString(color_tag),on_elm,scale,elm_dim,
                zsvec_buffer = proxy<cuda_space>(zsvec_buffer)] ZS_LAMBDA(int i) mutable {
                    if(on_elm){
                        auto bx = vec3::zeros();
                        if(elm_dim == 4){
                            auto inds = eles.pack<4>("inds",i).reinterpret_bits<int>();
                            for(int j = 0;j != 4;++j)
                                bx += verts.pack<3>(xtag,inds[j]) / 4;
                        }else if(elm_dim == 3){
                            auto inds = eles.pack<3>("inds",i).reinterpret_bits<int>();
                            for(int j= 0;j != 3;++j)
                                bx += verts.pack<3>(xtag,inds[j]) / 3;
                        }
                        vec_buffer.tuple<3>("x",i) = bx;
                        vec_buffer.tuple<3>("vec",i) = scale * eles.pack<3>(gtag,i)/* / eles.pack<3>(gtag,i).norm()*/;
                        if(eles.hasProperty(color_tag))
                            zsvec_buffer[i] = eles(color_tag,i);
                        else
                            zsvec_buffer[i] = (T)1.0;
                    }else{
                        vec_buffer.tuple<3>("x",i) = verts.pack<3>(xtag,i);
                        vec_buffer.tuple<3>("vec",i) = scale * verts.pack<3>(gtag,i)/* / verts.pack<3>(gtag,i).norm()*/;
                        // vec_buffer(color_tag,i) = verts(color_tag,i);
                        if(verts.hasProperty(color_tag))
                            zsvec_buffer[i] = verts(color_tag,i);
                        else
                            zsvec_buffer[i] = (T)1.0;
                    }
        });

        vec_buffer = vec_buffer.clone({zs::memsrc_e::host});
        zsvec_buffer = zsvec_buffer.clone({zs::memsrc_e::host});
        int vec_size = vec_buffer.size();
        constexpr auto omp_space = execspace_e::openmp;
        auto ompPol = omp_exec();

        auto heatmap = get_input<HeatmapObject>("heatmap");

        auto vec_field = std::make_shared<zeno::PrimitiveObject>();
        vec_field->resize(vec_size * 2);
        auto& segs = vec_field->lines;
        segs.resize(vec_size);
        auto& sverts = vec_field->attr<zeno::vec3f>("pos");
        auto& scolors = vec_field->add_attr<zeno::vec3f>("clr");

        // detect the max and min of color_tag
        std::vector<float> max_res(1);
        zs::reduce(ompPol, std::begin(zsvec_buffer), std::end(zsvec_buffer), std::begin(max_res), zs::limits<float>::lowest(), zs::getmax<float>());
        std::vector<float> min_res(1);
        zs::reduce(ompPol, std::begin(zsvec_buffer), std::end(zsvec_buffer), std::begin(min_res), zs::limits<float>::max(), zs::getmin<float>());
        
        ompPol(zs::range(vec_buffer.size()),
            [vec_buffer = proxy<omp_space>({},vec_buffer),&segs,&sverts,vec_size,&heatmap,&min_res,&max_res,&scolors,
                    zsvec_buffer = proxy<omp_space>(zsvec_buffer)] (int i) mutable {
                segs[i] = zeno::vec2i(i * 2 + 0,i * 2 + 1);
                auto start = vec_buffer.pack<3>("x",i);
                auto end = start + vec_buffer.pack<3>("vec",i);
                sverts[i*2 + 0] = zeno::vec3f{start[0],start[1],start[2]};
                sverts[i*2 + 1] = zeno::vec3f{end[0],end[1],end[2]};

                auto x = (zsvec_buffer[i]-min_res[0])/(max_res[0]-min_res[0] + (T)1e-6);

                auto color = heatmap->interp(x);
                scolors[i*2 + 0] = color;     
                scolors[i*2 + 1] = color;
        });

        set_output("vec_field",std::move(vec_field));
    }    
};

ZENDEFNODE(ZSRetrieveVectorField, {
    {"field","heatmap"},
    {"vec_field"},
    {{"enum quad tri vert","location","quad"},{gParamType_String,"gtag","vec_field"},{gParamType_String,"xtag","x"},{gParamType_Float,"scale","1.0"},{gParamType_String,"color_tag","color_tag"}},
    {"ZSGeometry"},
});


struct ZSSampleQuadratureAttr2Vert : zeno::INode {
    using dtiles_t = zs::TileVector<float,32>;
    void apply() override {
        using namespace zs;
        auto field = get_input<ZenoParticles>("ZSParticles");
        auto& verts = field->getParticles();
        auto& quads = field->getQuadraturePoints();
    
        auto attr = get_param<std::string>("attr");
        auto weight = get_param<std::string>("wtag");

        auto skip_bou = get_param<int>("skip_bou");
        auto bou_tag = get_param<std::string>("bou_tag");

        // std::cout << "check here 0" << std::endl;

        if(!quads.hasProperty(attr)){
            fmt::print("the input quadrature does not have specified attribute : {}\n",attr);
            throw std::runtime_error("the input quadrature does not have specified attribute");
        }

        if(!quads.hasProperty(weight)){
            fmt::print("the input quadratures does not have specified weight attribute : {}\n",weight);
            throw std::runtime_error("the input quadratures does not have specified weight attribute");
        }

        if(skip_bou && !verts.hasProperty(bou_tag)) {
            fmt::print("the input vertices have no {} boudary tag when skip bou is on\n",bou_tag);
            throw std::runtime_error("the input vertices have no boudary tag when skip bou is on");
        }

        constexpr auto space = execspace_e::cuda;
        auto cudaPol = cuda_exec();

        int attr_dim = quads.getPropertySize(attr);
        int simplex_size = quads.getPropertySize("inds");

        // std::cout << "check here 1" << std::endl;

        if(!verts.hasProperty(attr)) {
            fmt::print("append new nodal attribute {}[{}]\n",attr,attr_dim);
            verts.append_channels(cudaPol,{{attr,attr_dim}});
        }else if(verts.getPropertySize(attr) != attr_dim){
            fmt::print("the verts' {} attr[{}] and quads' {} attr[{}] not matched\n",attr,verts.getPropertySize(attr),attr,attr_dim);
        }
        cudaPol(range(verts.size()),
            [skip_bou,bou_tag = zs::SmallString(bou_tag),verts = proxy<space>({},verts),attr_dim,attr = SmallString(attr)] 
                __device__(int vi) mutable {
                    if(skip_bou && verts(bou_tag,vi) > 1e-6)
                        return;
                    for(int i = 0;i != attr_dim;++i)
                        verts(attr,i,vi) = 0.;
        });

        // static dtiles_t vtemp(verts.get_allocator(),{{"wsum",1}},verts.size());
        // vtemp.resize(verts.size());
        // cudaPol(range(vtemp.size()),
        //     [vtemp = proxy<space>({},vtemp)] ZS_LAMBDA(int vi) mutable {
        //         vtemp("wsum",vi) = 0;
        // });    
        zs::Vector<float> wsum_vec{verts.get_allocator(),verts.size()};
        cudaPol(zs::range(wsum_vec),[] ZS_LAMBDA(auto& wsum){wsum = (float)0;});

        // std::cout << "check here 2" << std::endl;

        cudaPol(range(quads.size()),
            [verts = proxy<space>({},verts),quads = proxy<space>({},quads),attr_dim,attr = SmallString(attr),simplex_size,weight = SmallString(weight),
                execTag = wrapv<space>{},skip_bou = skip_bou,bou_tag = zs::SmallString(bou_tag),wsum_vec = proxy<space>(wsum_vec)]
                __device__(int ei) mutable {
                    auto w = quads(weight,ei);
                    // if(ei == 0)
                    //     printf("w : %f\n",(float)w);
                    // w = 1.0;// cancel out the specified weight info
                    // printf("quads[%s][%d] : %f\n",attr.asChars(),ei,(float)quads(attr,0,ei));
                    for(int i = 0;i != simplex_size;++i){
                        auto idx = reinterpret_bits<int>(quads("inds",i,ei));
                        if(skip_bou && verts(bou_tag,idx) > 1e-6)
                            continue;
                        auto alpha = w;
                        for(int j = 0;j != attr_dim;++j) {
                            // verts(attr,j,idx) += w * quads(attr,j,ei) / (float)simplex_size;
                            atomic_add(execTag,&verts(attr,j,idx),alpha * quads(attr,j,ei));
                        }
                        atomic_add(execTag,&wsum_vec[idx],alpha);
                    }   
        });

        // std::cout << "check here 3 aaaa" << std::endl;
        // std::cout << "attr_dim = " << attr_dim << std::endl;

        cudaPol(range(verts.size()),[skip_bou,bou_tag = zs::SmallString(bou_tag),simplex_size,
                verts = proxy<space>({},verts),attr = SmallString(attr),
                attr_dim,wsum_vec = proxy<space>(wsum_vec)] ZS_LAMBDA(int vi) mutable {
                // if(vi == 0)
                //     printf("wsum : %f\n",(float)vtemp("wsum",vi));
                if(skip_bou && verts(bou_tag,vi) > 1e-6)
                    return;
                for(int j = 0;j != attr_dim;++j) {
                    // verts(attr,j,idx) += w * quads(attr,j,ei) / (float)simplex_size;
                    verts(attr,j,vi) = verts(attr,j,vi) / (wsum_vec[vi] + (float)1e-6);
                    if(zs::isnan(verts(attr,j,vi) ))
                        printf("nan verts attr %s detected at %d %d\n",attr.asChars(),(int)j,(int)vi);
                }

                
        });

        // std::cout << "check here 4" << std::endl;

        set_output("ZSParticles",field);
    }
};

ZENDEFNODE(ZSSampleQuadratureAttr2Vert,{
    {"ZSParticles"},
    {"ZSParticles"},
    {
        {gParamType_String,"attr","attr"},{gParamType_String,"wtag","vol"},{gParamType_Int,"skip_bou","0"},{gParamType_String,"bou_tag","btag"}
    },
    {"ZSGeometry"}
});


struct ZSSampleVertAttr2Quadrature : zeno::INode {
    void apply() override {
        using namespace zs;
        constexpr auto space = execspace_e::cuda;
        auto cudaPol = cuda_exec();

        auto field = get_input<ZenoParticles>("field");
        auto& verts = field->getParticles();
        auto& quads = field->getQuadraturePoints();



        // auto skip_bou = get_param<int>("skip_bou");
        // auto bou_tag = get_param<std::string>("bou_tag");

        // if(skip_bou && !quads.hasProperty(bou_tag)) {
        //     fmt::print("the input vertices have no {} boudary tag when skip bou is on\n",bou_tag);
        //     throw std::runtime_error("the input vertices have no boudary tag when skip bou is on");
        // }


        auto attr = get_param<std::string>("attr");
        if(!verts.hasProperty(attr)){
            fmt::print("the input verts have no specified channel : {}\n",attr);
            throw std::runtime_error("the input verts have no specified channel");
        }

        // auto weight = get_param<std::string>("weight");
        // if(!verts.hasProperty(weight)){
        //     fmt::print("the input vertices have no specified weight channel : {}\n",weight);
        //     throw std::runtime_error("the input vertices have no specified weight channel");
        // }


        int simplex_size = quads.getPropertySize("inds");
        int attr_dim = verts.getPropertySize(attr);

        if(!quads.hasProperty(attr))
            quads.append_channels(cudaPol,{{attr,attr_dim}});
        else if(quads.getPropertySize(attr) != attr_dim) {
            fmt::print("the size of channel {} V[{}] and Q[{}] not match\n",attr,attr_dim,quads.getPropertySize(attr));
            throw std::runtime_error("the size of channel does not match");
        }

        cudaPol(range(quads.size()),
            [verts = proxy<space>({},verts),quads = proxy<space>({},quads),attr = SmallString(attr),simplex_size,attr_dim]
                __device__(int ei) mutable {
                    for(int i = 0;i != attr_dim;++i)
                        quads(attr,i,ei) = 0.0;

                    for(int i  = 0;i != simplex_size;++i){
                        auto idx = reinterpret_bits<int>(quads("inds",i,ei));
                        for(int j = 0;j != attr_dim;++j){
                            quads(attr,j,ei) += verts(attr,j,idx) / simplex_size;
                        }
                    }
        });

        set_output("field",field);
    }
};

ZENDEFNODE(ZSSampleVertAttr2Quadrature,{
    {"field"},
    {"field"},
    {
        {gParamType_String,"attr","attr"}
    },
    {"ZSGeometry"}
});


struct ZSNormalizeVectorField : zeno::INode {
    using tiles_t = typename ZenoParticles::particles_t;

    void apply() override {
        using namespace zs;
        auto field = get_input<ZenoParticles>("field");
        auto type = get_param<std::string>("type");
        auto attr = get_param<std::string>("attr");

        tiles_t& data = (type == "vertex") ? field->getParticles() : field->getQuadraturePoints();

        auto cudaPol = cuda_exec();
        constexpr auto space = execspace_e::cuda;

        int attr_dim = data.getPropertySize(attr);

        cudaPol(range(data.size()),
            [data = proxy<space>({},data),attr_dim,attr = SmallString(attr)] __device__(int di) mutable {
                float length = 0;
                for(int i = 0;i != attr_dim;++i){
                    length += data(attr,i,di) * data(attr,i,di);
                }
                length = zs::sqrt(length) + 1e-6;
                for(int i = 0;i != attr_dim;++i)
                    data(attr,i,di) /= length;
            });

        set_output("field",field);
    }
};

ZENDEFNODE(ZSNormalizeVectorField,{
    {"field"},
    {"field"},
    {
        {"enum vertex quad","type","vertex"},{gParamType_String,"attr","attr"}
    },
    {"ZSGeometry"}
});



// a temporary node for sampling a quadrature field to another quadrature using bvh
// which should  replaced by a more versatile neighbor-wrangle
struct ZSGaussianNeighborQuadatureSampler : zeno::INode {
    using vec3 = zs::vec<float,3>;
    using bv_t = zs::AABBBox<3, float>;

    constexpr float gauss_kernel(float dist,float sigma) {
        // using namespace zs;
        auto distds = dist/sigma;
        return 1/(sigma /** zs::sqrt(2*zs::g_pi)*/) * zs::exp(-0.5 * distds * distds);
    }

    template <typename TileVecT, int codim = 3>
    zs::Vector<zs::AABBBox<3, typename TileVecT::value_type>>
    retrieve_bounding_volumes(zs::CudaExecutionPolicy &pol, const TileVecT &vtemp,
                            const zs::SmallString &xTag,
                            const typename ZenoParticles::particles_t &eles,
                            zs::wrapv<codim>, int voffset) {
        using namespace zs;
        using T = typename TileVecT::value_type;
        using bv_t = AABBBox<3, T>;
        static_assert(codim >= 1 && codim <= 4, "invalid co-dimension!\n");
        constexpr auto space = execspace_e::cuda;
        zs::Vector<bv_t> ret{eles.get_allocator(), eles.size()};
        pol(range(eles.size()), [eles = proxy<space>({}, eles),
                                bvs = proxy<space>(ret),
                                vtemp = proxy<space>({}, vtemp),
                                codim_v = wrapv<codim>{}, xTag,
                                voffset] ZS_LAMBDA(int ei) mutable {
            constexpr int dim = RM_CVREF_T(codim_v)::value;
            auto inds =
                eles.template pack<dim>("inds", ei).template reinterpret_bits<int>() +
                voffset;
            auto x0 = vtemp.template pack<3>(xTag, inds[0]);
            bv_t bv{x0, x0};
            for (int d = 1; d != dim; ++d)
                merge(bv, vtemp.template pack<3>(xTag, inds[d]));
            bvs[ei] = bv;
        });
        return ret;
    }

    template<typename Pol,typename VertBuffer,typename QuadBuffer,typename CenterBuffer>
    void evaluate_quadrature_centers(Pol& pol,VertBuffer& verts,QuadBuffer& quads,CenterBuffer& centers,const std::string& xtag) {
        using namespace zs;
        constexpr auto space = execspace_e::cuda;   

        // fmt::print("begin evaluate centers\n");

        pol(range(centers.size()),
            [src_centers = proxy<space>(centers),
                    src_quads = proxy<space>({},quads),
                    src_verts = proxy<space>({},verts),
                    xtag = SmallString(xtag)] __device__(int ei) mutable {
                int simplex_size = src_quads.propertySize("inds");
                src_centers[ei] = vec3::zeros();
                // if(ei == 1)
                //     printf("simplex_size : %d\n",simplex_size);
                for(int i = 0;i != simplex_size;++i){
                    auto idx = reinterpret_bits<int>(src_quads("inds",i,ei));
                    // if(ei == 1)
                    //     printf("idx : %d\n",idx);
                    src_centers[ei] += src_verts.pack<3>(xtag,idx) / simplex_size;
                }
        });       
    }

    void apply() override {
        using namespace zs;
        constexpr auto space = execspace_e::cuda;
        auto cudaPol = cuda_exec();

        auto src_field = get_input<ZenoParticles>("source");
        auto dst_field = get_input<ZenoParticles>("dest");

        int use_append = get_input2<int>("use_append");

        auto& src_quads = src_field->getQuadraturePoints();
        auto& src_verts = src_field->getParticles();
        auto& dst_quads = dst_field->getQuadraturePoints();
        auto& dst_verts = dst_field->getParticles();

        auto radius_shrink = get_input2<float>("radius");
        auto mark = get_input2<float>("mark");
        auto mark_tag = get_param<std::string>("mark_tag");
        auto weight_tag = get_param<std::string>("weight_tag");

        // auto bvh_thickness = get_param<float>("bvh_thickness");

        int simplex_size = src_quads.getPropertySize("inds");
        if(simplex_size != 4){
            fmt::print("currently, only the tetrahedron is supported\n");
            throw std::runtime_error("currently, only the tetrahedron is supported");
        }

        auto xtag = get_param<std::string>("xtag");
        auto attr = get_param<std::string>("attr");
        auto sigma = get_param<float>("sigma");

        if(!src_quads.hasProperty(attr)){
            fmt::print("the input source quadrature does not have specified channel {}\n",attr);
            throw std::runtime_error("the input source quadrature does not have specified channel");
        }
        if(!dst_quads.hasProperty(attr)){
            fmt::print("the input dest quadrature does not have specified channel {}\n",attr);
            throw std::runtime_error("the input dest quadrature does not have specified channel");
        }
        if(!dst_quads.hasProperty(mark_tag)){
            fmt::print("the input dest quadrature does not have the specified mark tag {}\n",mark_tag);
            throw std::runtime_error("the input dest quadrature does not have the specified mark tag");
        }

        int attr_dim = src_quads.getPropertySize(attr);
        // dst_quads.append_channels(cudaPol,{{attr,attr_dim}});


        // initialize the buffer data to 0
        // cudaPol(range(dst_quads.size()),
        //     [dst_quads = proxy<space>({},dst_quads),attr = SmallString(attr),attr_dim] ZS_LAMBDA(int ei) mutable {
        //             for(int i = 0;i != attr_dim;++i)
        //                 dst_quads(attr,i,ei) = 0.0;
        // }); 

        // fmt::print("initial dst field\n");
        // build bvh for source
        auto bvs = retrieve_bounding_volumes(cudaPol,src_verts,xtag,src_quads,wrapv<4>{},0);
        auto quadsBvh = LBvh<3,int,float>{};
        quadsBvh.build(cudaPol,bvs);

        // fmt::print("finish setup bvh\n");
        // initial two buffers containing the quads' centers
        zs::Vector<vec3> src_centers(src_quads.get_allocator(),src_quads.size());
        zs::Vector<vec3> dst_centers(dst_quads.get_allocator(),dst_quads.size());

        // fmt::print("size check {} {} {}\n",src_quads.size(),src_verts.size(),src_centers.size());
        // fmt::print("size check {} {} {}\n",dst_quads.size(),dst_verts.size(),dst_centers.size());
        evaluate_quadrature_centers(cudaPol,src_verts,src_quads,src_centers,xtag);
        evaluate_quadrature_centers(cudaPol,dst_verts,dst_quads,dst_centers,xtag);

        // auto sigma2 = sigma*sigma;

        // fmt::print("finish evaluating the quadrature centers\n");

        cudaPol(range(dst_quads.size()),
            [ dst_quads = proxy<space>({},dst_quads),src_quads = proxy<space>({},src_quads),
                dst_verts = proxy<space>({},dst_verts),src_verts = proxy<space>({},src_verts),
                src_centers = proxy<space>(src_centers),dst_centers = proxy<space>(dst_centers),
                attr = SmallString(attr),xtag = SmallString(xtag),simplex_size,attr_dim,weight_tag = zs::SmallString(weight_tag),
                bvh = proxy<space>(quadsBvh),sigma,this,use_append,radius_shrink,mark_tag = SmallString(mark_tag),mark] __device__(int di) mutable {
                    // if(!use_append)
                    //     for(int i = 0;i != attr_dim;++i)
                    //         dst_quads(attr,i,di) = 0.0;
                    // else{
                    //     float field_norm = 0.f;
                    //     for(int i = 0;i != attr_dim;++i)
                    //         field_norm += dst_quads(attr,i,di) * dst_quads(attr,i,di);
                    //     field_norm = zs::sqrt(field_norm);
                    //     if(field_norm > 1e-6)
                    //         return;
                    // }
                    // compute the center of the src tet
                    auto dst_ct = dst_centers[di]; 
                    float radius = 0;

                    // float w_sum = 0;

                    // automatically detected the approapiate radius size
                    for(int i = 0; i != simplex_size;++i){
                        auto idx = reinterpret_bits<int>(dst_quads("inds",i,di));
                        auto dst_vert = dst_verts.pack<3>(xtag,idx);
                        auto dc_dist = (dst_vert - dst_ct).norm();
                        radius = radius < dc_dist ? dc_dist : radius;
                    }

                    radius *= radius_shrink;
                    // float alpha = zs::sqrt((float)zs::g_pi);
                    // if(di == 0){
                    //     printf("check %f %f\n",(float)alpha,(float)zs::g_pi);
                    // }

                    auto dst_bv = bv_t{get_bounding_box(dst_ct - radius, dst_ct + radius)};
                    bool first_iter = true;
                    bool has_been_sampled = false;
                    bvh.iter_neighbors(dst_bv,[&](int si){
                        auto src_ct = src_centers[si];
                        auto dist = (src_ct - dst_ct).norm();
                        if(dist > radius * 2)
                            return;

                        auto w = gauss_kernel(dist,sigma);
                        if(w < 1e-4)
                            return;

                        has_been_sampled = true;
                        if(first_iter && !use_append){
                            for(int i = 0;i != attr_dim;++i)
                                dst_quads(attr,i,di) = 0.0;
                            first_iter = false;
                        }
                
                        // float distds = dist/sigma;


                        // float beta = zs::exp(-0.5 * distds * distds);
                        // w = 1/(sigma /* zs::sqrt(2*zs::g_pi)*/) * zs::exp(-0.5 * distds * distds);

                        // w_sum += w;
                        dst_quads(weight_tag,di) += w;
                        // printf("sample neighbor : %d->%d %f %f %f\n",si,di,(float)w,(float)alpha,(float)zs::g_pi);
                        for(int i = 0;i != attr_dim;++i)
                            dst_quads(attr,i,di) += w * src_quads(attr,i,si);
                        // if(attr_dim == 1)
                        //     printf("dst_quads[%s][%d] sample src_quads[%s][%d] : %f\n",attr.asChars(),di,attr.asChars(),si,src_quads(attr,0,si));

                        dst_quads(mark_tag,di) = mark;
                    });

                    // if(w_sum < 1e-6){
                    //     printf("lost element %d\n",di);
                    // }
                    // if(has_been_sampled)
                    //     for(int i = 0;i != attr_dim;++i)
                    //         dst_quads(attr,i,di) /= (w_sum + 1e-6);
        });




        set_output("dest",dst_field);
    }
};

ZENDEFNODE(ZSGaussianNeighborQuadatureSampler,{
    {"source","dest",{gParamType_Int,"use_append","0"},{gParamType_Float,"radius","1"},{gParamType_Float,"mark","-1.0"}},
    {"dest"},
    {
        {gParamType_String,"weight_tag","weight_tag"},
        {gParamType_String,"mark_tag","mark_tag"},
        {gParamType_String,"attr","attr"},
        {gParamType_String,"xtag","x"},
        {gParamType_Float,"sigma","1"}
    },
    {"ZSGeometry"}
});


struct ZSGaussianSampler : zeno::INode {
    using T = float;
    using vec3 = zs::vec<T,3>;
    using bvh_t = zs::LBvh<3,int,T>;
    using bv_t = zs::AABBBox<3,T>;

    // constexpr T gauss_kernel(T dist,T sigma) {
    //     // using namespace zs;
    //     auto distds = dist/sigma;
    //     return 1/(sigma /** zs::sqrt(2*zs::g_pi)*/) * zs::exp(-0.5 * distds * distds);
    // }


    template<typename Pol,typename VertBuffer,typename QuadBuffer,typename CenterBuffer>
    void evaluate_quadrature_centers(Pol& pol,VertBuffer& verts,QuadBuffer& quads,CenterBuffer& centers,const std::string& xtag) {
        using namespace zs;
        constexpr auto space = execspace_e::cuda;   

        // fmt::print("begin evaluate centers\n");

        pol(range(centers.size()),
            [src_centers = proxy<space>({},centers),
                    src_quads = proxy<space>({},quads),
                    src_verts = proxy<space>({},verts),
                    xtag = SmallString(xtag)] __device__(int ei) mutable {
                int simplex_size = src_quads.propertySize("inds");
                src_centers.tuple(dim_c<3>,xtag,ei) = vec3::zeros();
                // if(ei == 1)
                //     printf("simplex_size : %d\n",simplex_size);
                for(int i = 0;i != simplex_size;++i){
                    auto idx = reinterpret_bits<int>(src_quads("inds",i,ei));
                    src_centers.tuple(dim_c<3>,xtag,ei) = src_centers.pack(dim_c<3>,xtag,ei) + src_verts.pack(dim_c<3>,xtag,idx) / (T)simplex_size;
                }
        });       
    }

    void apply() override {
        using namespace zs;
        constexpr auto space = execspace_e::cuda;
        auto cudaPol = cuda_exec();

        auto src_field = get_input<ZenoParticles>("source");
        auto dst_field = get_input<ZenoParticles>("dest");

        auto srcAttr = get_param<std::string>("srcAttr");
        auto dstAttr = get_param<std::string>("dstAttr");

        auto srcType = get_param<std::string>("srcType");
        auto dstType = get_param<std::string>("dstType");

        const auto& srcVerts = src_field->getParticles();
        const auto& srcQuads = src_field->getQuadraturePoints();

        auto& dstVerts = dst_field->getParticles();
        auto& dstQuads = dst_field->getQuadraturePoints();

        auto markTag = get_param<std::string>("markTag");
        auto markValue = get_param<float>("markValue");


        if(srcType == std::string("vert") && !srcVerts.hasProperty(srcAttr)){
            fmt::print("the input source verts have no specified channel {}\n",srcAttr);
            throw std::runtime_error("the input source verts have no specified channel");
        }
        if(srcType == std::string("quad") && !srcQuads.hasProperty(srcAttr)){
            fmt::print("the input source quads have no specified channel {}\n",srcAttr);
            throw std::runtime_error("the input source quads have no specified channel");
        }

        int source_dim = 0;
        int source_size = 0;

        if(srcType == std::string("vert"))
            source_dim = srcVerts.getPropertySize(srcAttr);
        else
            source_dim = srcQuads.getPropertySize(srcAttr);


        std::cout << "source_dim = " << source_dim << std::endl;

        if(dstType == std::string("vert") && !dstVerts.hasProperty(dstAttr)){
            dstVerts.append_channels(cudaPol,{{dstAttr,source_dim},{markTag,1}});
            // fmt::print("the input dst verts have no specified channel {}\n",dstAttr);
            // throw std::runtime_error("the input dst verts have no specified channel");            
        }
        if(dstType == std::string("quad") && !dstQuads.hasProperty(dstAttr)){
            dstQuads.append_channels(cudaPol,{{dstAttr,source_dim},{markTag,1}});
            // fmt::print("the input dst quads have no specified channel {}\n",dstAttr);
            // throw std::runtime_error("the input dst quads have no specified channel");            
        }


        int dst_dim = 0;
        int dst_size = 0;
        if(dstType == std::string("vert"))
            dst_dim = dstVerts.getPropertySize(srcAttr);
        else
            dst_dim = dstQuads.getPropertySize(dstAttr);
        if(source_dim != dst_dim){
            fmt::print("the dst's {} channel and src's {} channel do not match in size [{} {}]\n",
                dstAttr,srcAttr,source_dim,dst_dim);
            throw std::runtime_error("the dst's {} channel and src's {} channel do not match in size");
        }
        int dim = source_dim;

        auto src = typename ZenoParticles::particles_t({{"x",3},{"attr",dim},{"inds",1}},0,zs::memsrc_e::device);
        auto dst = typename ZenoParticles::particles_t({{"x",3},{"attr",dim},{"mark",1}},0,zs::memsrc_e::device);

        int source_simplex_size = srcQuads.getPropertySize("inds");
        if(srcType == "vert"){
            src.resize(srcVerts.size());
            TILEVEC_OPS::copy(cudaPol,srcVerts,"x",src,"x");
            TILEVEC_OPS::copy(cudaPol,srcVerts,srcAttr,src,"attr");
        }else{
            src.resize(srcQuads.size());
            evaluate_quadrature_centers(cudaPol,srcVerts,srcQuads,src,"x");
            auto src_verts_x_norm = TILEVEC_OPS::dot<3>(cudaPol,srcVerts,"x","x");
            auto src_x_norm = TILEVEC_OPS::dot<3>(cudaPol,src,"x","x");
            std::cout << "src_x_norm : " << src_x_norm << "   src_verts_x_norm : " << src_verts_x_norm << std::endl;
            TILEVEC_OPS::copy(cudaPol,srcQuads,srcAttr,src,"attr");
        }

        // int dst_simplex_size = dstQuads.getPropertySize("inds");
        if(dstType == "vert"){
            dst.resize(dstVerts.size());
            TILEVEC_OPS::copy(cudaPol,dstVerts,"x",dst,"x");
            TILEVEC_OPS::fill(cudaPol,dstVerts,dstAttr,(T)0.0);
        }else {
            dst.resize(dstQuads.size());
            evaluate_quadrature_centers(cudaPol,dstVerts,dstQuads,dst,"x");
            TILEVEC_OPS::fill(cudaPol,dst,dstAttr,(T)0.0);
        }
        cudaPol(zs::range(src.size()),
            [src = proxy<space>({},src)] ZS_LAMBDA(int si) mutable {
                src("inds",si) = reinterpret_bits<T>(si);
        });
        TILEVEC_OPS::fill(cudaPol,dst,"mark",(T)0.0);

        auto bvs = retrieve_bounding_volumes(cudaPol,src,src,wrapv<1>{},(T)0,"x");
        auto srcBvh = bvh_t{};
        srcBvh.build(cudaPol,bvs);

        auto thickness = get_input2<float>("thickness");
        auto sigma = get_input2<float>("sigma");


        cudaPol(zs::range(dst.size()),
            [dst = proxy<space>({},dst),thickness = thickness,sigma = sigma,
                src = proxy<space>({},src),dim = dim,
                srcAttr = zs::SmallString(srcAttr),
                dstAttr = zs::SmallString(dstAttr),
                srcBvh = proxy<space>(srcBvh),
                makrValue = markValue] ZS_LAMBDA(int di) mutable {
            auto dp = dst.pack(dim_c<3>,"x",di);
            auto dst_bv = bv_t{get_bounding_box(dp - thickness,dp + thickness)};
            T wsum = (T)0.0;
            // if(di == 0)
            //     printf("dp = %f %f %f\n",(float)dp[0],(float)dp[1],(float)dp[2]);
            srcBvh.iter_neighbors(dst_bv,[&](int si){
                auto sp = src.pack(dim_c<3>,"x",si);
                auto dist = (dp - sp).norm();
                // if(dist > thickness * 5)
                //     return;

                auto distds = dist/sigma;
                auto w = (T)1/(sigma /** zs::sqrt(2*zs::g_pi)*/) * zs::exp((T)-0.5 * distds * distds);
                if(w < 1e-4)
                    return;
                
                if(di == 0 && dim == 9)
                    printf("the current sample weight = [%f]\n",(float)w);

                wsum += w;
                for(int i = 0;i != dim;++i)
                    dst("attr",i,di) += w * src("attr",i,si);
            });
            if(wsum <= 1e-7)
                dst("mark",di) = (T)makrValue;
            for(int i = 0;i != dim;++i)
                dst("attr",i,di) /= (wsum + 1e-7);
            if(di == 0 && dim == 9){
                auto F = dst.pack(dim_c<3,3>,"attr",di);
                printf("dst.F[%f] : \n%f\t%f\t%f\n%f\t%f\t%f\n%f\t%f\t%f\n",(float)wsum,
                    (float)F(0,0),(float)F(0,1),(float)F(0,2),
                    (float)F(1,0),(float)F(1,1),(float)F(1,2),
                    (float)F(2,0),(float)F(2,1),(float)F(2,2));
            }
        });

        if(dstType == "vert") {
            TILEVEC_OPS::copy(cudaPol,dst,"attr",dstVerts,dstAttr);
            TILEVEC_OPS::copy(cudaPol,dst,"mark",dstVerts,markTag);
        }else {
            TILEVEC_OPS::copy(cudaPol,dst,"attr",dstQuads,dstAttr);
            TILEVEC_OPS::copy(cudaPol,dst,"mark",dstQuads,markTag);
        }

        set_output("dest",dst_field);
    }
};

ZENDEFNODE(ZSGaussianSampler,{
    {"source","dest",{gParamType_Float,"thickness","0.1"},{gParamType_Float,"sigma","1.0"}},
    {"dest"},
    {
        {gParamType_String,"srcAttr","srcAttr"},
        {gParamType_String,"dstAttr","dstAttr"},
        {gParamType_String,"markTag","bou_tag"},
        {gParamType_Float,"markValue","1.0"},
        {"enum vert quad","srcType","vert"},
        {"enum vert quad","dstType","vert"},

    },
    {"ZSGeometry"}
});


struct ZSAppendAttribute : zeno::INode {
    using tiles_t = typename ZenoParticles::particles_t;

    void apply() override {
        using namespace zs;
        constexpr auto space = execspace_e::cuda;
        auto cudaPol = cuda_exec();

        auto particles = get_input<ZenoParticles>("ZSParticles");
        auto attr = get_param<std::string>("attr");
        auto attr_dim = get_param<int>("attr_dim");

        auto type = get_param<std::string>("type");

        auto fill_val = get_input2<float>("fill");

        // tiles_t& data = (type == "particle") ? particles->getParticles() : particles->getQuadraturePoints();

        // auto& data = particles->getQuadraturePoints();

        if(type == "particle") {
            auto& data = particles->getParticles();
            data.append_channels(cudaPol,{{attr,attr_dim}});
            cudaPol(range(data.size()),
                [data = proxy<space>({},data),attr = SmallString(attr),attr_dim,fill_val] __device__(int vi) mutable {
                    for(int i = 0;i != attr_dim;++i)
                        data(attr,i,vi) = fill_val;
            });
        }else if (type == "quadature") {
            auto& data = particles->getQuadraturePoints();
            data.append_channels(cudaPol,{{attr,attr_dim}});
            cudaPol(range(data.size()),
                [data = proxy<space>({},data),attr = SmallString(attr),attr_dim,fill_val] __device__(int vi) mutable {
                    for(int i = 0;i != attr_dim;++i)
                        data(attr,i,vi) = fill_val;
            });            
        }


        
        // cudaPol(range(data.size()),
        //     [data = proxy<space>({},data),attr = SmallString(attr),attr_dim,fill_val] __device__(int vi) mutable {
        //         for(int i = 0;i != attr_dim;++i)
        //             data(attr,i,vi) = fill_val;
        // });

        set_output("ZSParticles",particles);
    }
};

ZENDEFNODE(ZSAppendAttribute,{
    {"ZSParticles",{gParamType_Float,"fill","0.0"}},
    {"ZSParticles"},
    {
        {gParamType_String,"attr","attr"},
        {gParamType_Int,"attr_dim","1"},
        {"enum particle quadature","type","particle"}
    },
    {"ZSGeometry"}
});


};