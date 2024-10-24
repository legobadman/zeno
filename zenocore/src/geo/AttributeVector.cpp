#include <zeno/types/AttributeVector.h>


namespace zeno {

    ZENO_API AttributeVector::AttributeVector(const AttributeVector& rhs) {
        x_comp = rhs.x_comp;
        y_comp = rhs.y_comp;
        z_comp = rhs.z_comp;
        w_comp = rhs.w_comp;
        m_size = rhs.m_size;
        self = rhs.self;
    }

    ZENO_API AttributeVector::AttributeVector(const AttrVar& val_or_vec, size_t size) : m_size(size){
        set(val_or_vec);
    }

    ZENO_API size_t AttributeVector::size() const {
        return m_size;
    }

    ZENO_API AttrVarVec AttributeVector::get() {
        //vec?
        return self->value();
    }

    ZENO_API void AttributeVector::to_prim_attr(std::shared_ptr<PrimitiveObject> spPrim, std::string const& attr_name) {
        if (self) {
            auto& valvar = self->value();

            std::visit([&](auto&& vec) {
                using E = std::decay_t<decltype(vec)>;
                if constexpr (std::is_same_v<E, std::vector<int>>) {
                    auto& vec_attr = spPrim->verts.add_attr<int>(attr_name);
                    if (vec.size() == 1) {
                        vec_attr = std::vector<int>(m_size, vec[0]);
                    }
                    else {
                        vec_attr = vec;
                    }
                }
                else if constexpr (std::is_same_v < E, std::vector<float>>) {
                    auto& vec_attr = spPrim->verts.add_attr<float>(attr_name);
                    if (vec.size() == 1) {
                        vec_attr = std::vector<float>(m_size, vec[0]);
                    }
                    else {
                        vec_attr = vec;
                    }
                }
                else if constexpr (std::is_same_v < E, std::vector<std::string>>) {
                    // do not support string type on primitive object.
                    /*
                    auto& vec_attr = spPrim->verts.add_attr<std::string>(attr_name);
                    if (vec.size() == 1) {
                        vec_attr = std::vector<std::string>(m_size, vec[0]);
                    }
                    else {
                        vec_attr = vec;
                    }
                    */
                }
                else {
                    throw;
                }
            }, valvar);
        }
        else if (x_comp && y_comp) {

            auto& xvar = x_comp->value();
            auto& yvar = y_comp->value();
            assert(xvar.index() == yvar.index());

            std::visit([&](auto&& vec) {
                using E = std::decay_t<decltype(vec)>;
                if constexpr (std::is_same_v < E, std::vector<vec2i>>) {
                    auto& vec_attr = spPrim->verts.add_attr<zeno::vec2i>(attr_name);
                    for (int i = 0; i < m_size; i++) {
                        vec_attr[i] = zeno::vec2i(x_comp->get<int>(i), y_comp->get<int>(i));
                    }
                }
                else if constexpr (std::is_same_v < E, std::vector<vec2f>>) {
                    auto& vec_attr = spPrim->verts.add_attr<zeno::vec2f>(attr_name);
                    for (int i = 0; i < m_size; i++) {
                        vec_attr[i] = zeno::vec2f(x_comp->get<float>(i), y_comp->get<float>(i));
                    }
                }
                else if constexpr (std::is_same_v < E, std::vector<vec3i>>) {
                    assert(z_comp);
                    auto& vec_attr = spPrim->verts.add_attr<zeno::vec3i>(attr_name);
                    for (int i = 0; i < m_size; i++) {
                        vec_attr[i] = zeno::vec3i(x_comp->get<int>(i), y_comp->get<int>(i), z_comp->get<int>(i));
                    }
                }
                else if constexpr (std::is_same_v < E, std::vector<vec3f>>) {
                    assert(z_comp);
                    auto& vec_attr = spPrim->verts.add_attr<zeno::vec3f>(attr_name);
                    for (int i = 0; i < m_size; i++) {
                        vec_attr[i] = zeno::vec3f(x_comp->get<float>(i), y_comp->get<float>(i), z_comp->get<float>(i));
                    }
                }
                else if constexpr (std::is_same_v < E, std::vector<vec4i>>) {
                    assert(z_comp && w_comp);
                    auto& vec_attr = spPrim->verts.add_attr<zeno::vec4i>(attr_name);
                    for (int i = 0; i < m_size; i++) {
                        vec_attr[i] = zeno::vec4i(x_comp->get<int>(i), y_comp->get<int>(i), z_comp->get<int>(i), w_comp->get<int>(i));
                    }
                }
                else if constexpr (std::is_same_v < E, std::vector<vec4f>>) {
                    auto& vec_attr = spPrim->verts.add_attr<zeno::vec4f>(attr_name);
                    assert(z_comp && w_comp);
                    for (int i = 0; i < m_size; i++) {
                        vec_attr[i] = zeno::vec4f(x_comp->get<float>(i), y_comp->get<float>(i), z_comp->get<float>(i), w_comp->get<float>(i));
                    }
                }
                else {
                    throw;
                }
            }, x_comp->value());
        }
        else {
            throw;
        }
    }

    ZENO_API void AttributeVector::set(const AttrVar& val_or_vec) {

        std::visit([&](auto&& val) {
            using E = std::decay_t<decltype(val)>;
            if constexpr (std::is_same_v<E, int> || std::is_same_v<E, float> || std::is_same_v<E, std::string>) {
                self = std::make_shared<AttrColumn>(std::vector<E>(1, val), m_size);
            }
            else if constexpr (std::is_same_v<E, vec2i> || std::is_same_v<E, vec2f>) {
                using ElemType = std::conditional<std::is_same_v<E, vec2f>, float, int>::type;
                x_comp = std::make_shared<AttrColumn>(std::vector<ElemType>(1, val[0]), m_size);
                y_comp = std::make_shared<AttrColumn>(std::vector<ElemType>(1, val[0]), m_size);
            }
            else if constexpr (std::is_same_v<E, vec3i> || std::is_same_v<E, vec3f>) {
                using ElemType = std::conditional<std::is_same_v<E, vec3f>, float, int>::type;
                x_comp = std::make_shared<AttrColumn>(std::vector<ElemType>(1, val[0]), m_size);
                y_comp = std::make_shared<AttrColumn>(std::vector<ElemType>(1, val[1]), m_size);
                z_comp = std::make_shared<AttrColumn>(std::vector<ElemType>(1, val[2]), m_size);
            }
            else if constexpr (std::is_same_v<E, vec4i> || std::is_same_v<E, vec4f>) {
                using ElemType = std::conditional<std::is_same_v<E, vec4f>, float, int>::type;
                x_comp = std::make_shared<AttrColumn>(std::vector<ElemType>(1, val[0]), m_size);
                y_comp = std::make_shared<AttrColumn>(std::vector<ElemType>(1, val[1]), m_size);
                z_comp = std::make_shared<AttrColumn>(std::vector<ElemType>(1, val[2]), m_size);
            }
            else if constexpr (std::is_same_v<E, std::vector<int>> ||
                std::is_same_v<E, std::vector<float>> ||
                std::is_same_v<E, std::vector<std::string>>)
            {
                self = std::make_shared<AttrColumn>(val, m_size);
            }
            else if constexpr (std::is_same_v<E, std::vector<vec2i>> || std::is_same_v<E, std::vector<vec2f>>) {
                using ElemType = std::conditional<std::is_same_v<E, std::vector<vec2f>>, float, int>::type;
                std::vector<ElemType> xvec(m_size);
                std::vector<ElemType> yvec(m_size);
                for (size_t i = 0; i < m_size; i++) {
                    xvec[i] = val[i][0];
                    yvec[i] = val[i][1];
                }
                x_comp = std::make_shared<AttrColumn>(xvec, m_size);
                y_comp = std::make_shared<AttrColumn>(yvec, m_size);
            }
            else if constexpr (std::is_same_v<E, std::vector<vec3i>> || std::is_same_v<E, std::vector<vec3f>>) {
                using ElemType = std::conditional<std::is_same_v<E, std::vector<vec3f>>, float, int>::type;
                std::vector<ElemType> xvec(m_size);
                std::vector<ElemType> yvec(m_size);
                std::vector<ElemType> zvec(m_size);
                for (size_t i = 0; i < m_size; i++) {
                    xvec[i] = val[i][0];
                    yvec[i] = val[i][1];
                    zvec[i] = val[i][2];
                }
                x_comp = std::make_shared<AttrColumn>(xvec, m_size);
                y_comp = std::make_shared<AttrColumn>(yvec, m_size);
                z_comp = std::make_shared<AttrColumn>(zvec, m_size);
            }
            else if constexpr (std::is_same_v<E, std::vector<vec4i>> || std::is_same_v<E, std::vector<vec4f>>) {
                using ElemType = std::conditional<std::is_same_v<E, std::vector<vec4f>>, float, int>::type;
                std::vector<ElemType> xvec(m_size);
                std::vector<ElemType> yvec(m_size);
                std::vector<ElemType> zvec(m_size);
                std::vector<ElemType> wvec(m_size);
                for (size_t i = 0; i < m_size; i++) {
                    xvec[i] = val[i][0];
                    yvec[i] = val[i][1];
                    zvec[i] = val[i][2];
                    wvec[i] = val[i][3];
                }
                x_comp = std::make_shared<AttrColumn>(xvec, m_size);
                y_comp = std::make_shared<AttrColumn>(yvec, m_size);
                z_comp = std::make_shared<AttrColumn>(zvec, m_size);
                w_comp = std::make_shared<AttrColumn>(wvec, m_size);
            }
        }, val_or_vec);
    }

}