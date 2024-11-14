#include <zeno/types/AttributeVector.h>


namespace zeno {

    ZENO_API AttributeVector::AttributeVector(const AttributeVector& rhs)
        : x_comp(rhs.x_comp)
        , y_comp(rhs.y_comp)
        , z_comp(rhs.z_comp)
        , w_comp(rhs.w_comp)
        , m_size(rhs.m_size)
        , m_type(rhs.m_type)
        , self(rhs.self)
    {
    }

    ZENO_API AttributeVector::AttributeVector(const AttrVar& val_or_vec, size_t size)
        : m_size(size)
        , m_type(ATTR_TYPE_UNKNOWN)
    {
        set(val_or_vec);
    }

    ZENO_API size_t AttributeVector::size() const {
        return m_size;
    }

    ZENO_API GeoAttrType AttributeVector::type() const {
        return m_type;
    }

    ZENO_API AttrVarVec AttributeVector::get() {
        //vec?
        return self->value();
    }

    ZENO_API void AttributeVector::set(const AttrVar& val_or_vec) {
        std::visit([&](auto&& val) {
            using E = std::decay_t<decltype(val)>;


            if constexpr (std::is_same_v<E, int>) {
                if (m_type != ATTR_TYPE_UNKNOWN && m_type != ATTR_INT && m_type != ATTR_FLOAT)
                    throw makeError<UnimplError>("type dismatch, cannot change type of attrvector");
                self = std::make_shared<AttrColumn>(std::vector<E>(1, val), m_size);
                m_type = ATTR_INT;
            }
            else if constexpr (std::is_same_v<E, float>) {
                if (m_type != ATTR_TYPE_UNKNOWN && m_type != ATTR_INT && m_type != ATTR_FLOAT)
                    throw makeError<UnimplError>("type dismatch, cannot change type of attrvector");
                self = std::make_shared<AttrColumn>(std::vector<E>(1, val), m_size);
                m_type = ATTR_FLOAT;
            }
            else if constexpr (std::is_same_v<E, std::string>) {
                if (m_type != ATTR_TYPE_UNKNOWN && m_type != ATTR_STRING)
                    throw makeError<UnimplError>("type dismatch, cannot change type of attrvector");
                self = std::make_shared<AttrColumn>(std::vector<E>(1, val), m_size);
                m_type = ATTR_STRING;
            }
            else if constexpr (std::is_same_v<E, vec2i> || std::is_same_v<E, vec2f> || std::is_same_v<E, glm::vec2>) {
                //只能以float储存
                if (m_type != ATTR_TYPE_UNKNOWN && m_type != ATTR_VEC2)
                    throw makeError<UnimplError>("type dismatch, cannot change type of attrvector");
                x_comp = std::make_shared<AttrColumn>(std::vector<float>(1, val[0]), m_size);
                y_comp = std::make_shared<AttrColumn>(std::vector<float>(1, val[0]), m_size);
                m_type = ATTR_VEC2;
            }
            else if constexpr (std::is_same_v<E, vec3i> || std::is_same_v<E, vec3f> || std::is_same_v<E, glm::vec3>) {
                //只能以float储存
                if (m_type != ATTR_TYPE_UNKNOWN && m_type != ATTR_VEC3)
                    throw makeError<UnimplError>("type dismatch, cannot change type of attrvector");
                x_comp = std::make_shared<AttrColumn>(std::vector<float>(1, val[0]), m_size);
                y_comp = std::make_shared<AttrColumn>(std::vector<float>(1, val[1]), m_size);
                z_comp = std::make_shared<AttrColumn>(std::vector<float>(1, val[2]), m_size);
                m_type = ATTR_VEC3;
            }
            else if constexpr (std::is_same_v<E, vec4i> || std::is_same_v<E, vec4f> || std::is_same_v<E, glm::vec4>) {
                //只能以float储存
                if (m_type != ATTR_TYPE_UNKNOWN && m_type != ATTR_VEC4)
                    throw makeError<UnimplError>("type dismatch, cannot change type of attrvector");
                x_comp = std::make_shared<AttrColumn>(std::vector<float>(1, val[0]), m_size);
                y_comp = std::make_shared<AttrColumn>(std::vector<float>(1, val[1]), m_size);
                z_comp = std::make_shared<AttrColumn>(std::vector<float>(1, val[2]), m_size);
                w_comp = std::make_shared<AttrColumn>(std::vector<float>(1, val[3]), m_size);
                m_type = ATTR_VEC4;
            }
            else if constexpr (std::is_same_v<E, std::vector<int>>) {
                if (m_type != ATTR_TYPE_UNKNOWN && m_type != ATTR_INT && m_type != ATTR_FLOAT)
                    throw makeError<UnimplError>("type dismatch, cannot change type of attrvector");
                self = std::make_shared<AttrColumn>(val, m_size);
                m_type = ATTR_INT;
            }
            else if constexpr (std::is_same_v<E, std::vector<float>>) {
                if (m_type != ATTR_TYPE_UNKNOWN && m_type != ATTR_INT && m_type != ATTR_FLOAT)
                    throw makeError<UnimplError>("type dismatch, cannot change type of attrvector");
                self = std::make_shared<AttrColumn>(val, m_size);
                m_type = ATTR_FLOAT;
            }
            else if constexpr (std::is_same_v<E, std::vector<std::string>>) {
                if (m_type != ATTR_TYPE_UNKNOWN && m_type != ATTR_STRING)
                    throw makeError<UnimplError>("type dismatch, cannot change type of attrvector");
                self = std::make_shared<AttrColumn>(val, m_size);
                m_type = ATTR_STRING;
            }
            else if constexpr (std::is_same_v<E, std::vector<vec2i>> || std::is_same_v<E, std::vector<vec2f>> || std::is_same_v<E, std::vector<glm::vec2>>) {
                if (m_type != ATTR_TYPE_UNKNOWN && m_type != ATTR_VEC2)
                    throw makeError<UnimplError>("type dismatch, cannot change type of attrvector");
                std::vector<float> xvec(m_size);
                std::vector<float> yvec(m_size);
                for (size_t i = 0; i < m_size; i++) {
                    xvec[i] = val[i][0];
                    yvec[i] = val[i][1];
                }
                x_comp = std::make_shared<AttrColumn>(xvec, m_size);
                y_comp = std::make_shared<AttrColumn>(yvec, m_size);
                m_type = ATTR_VEC2;
            }
            else if constexpr (std::is_same_v<E, std::vector<vec3i>> || std::is_same_v<E, std::vector<vec3f>> || std::is_same_v<E, std::vector<glm::vec3>>) {
                if (m_type != ATTR_TYPE_UNKNOWN && m_type != ATTR_VEC3)
                    throw makeError<UnimplError>("type dismatch, cannot change type of attrvector");
                std::vector<float> xvec(m_size);
                std::vector<float> yvec(m_size);
                std::vector<float> zvec(m_size);
                for (size_t i = 0; i < m_size; i++) {
                    xvec[i] = val[i][0];
                    yvec[i] = val[i][1];
                    zvec[i] = val[i][2];
                }
                x_comp = std::make_shared<AttrColumn>(xvec, m_size);
                y_comp = std::make_shared<AttrColumn>(yvec, m_size);
                z_comp = std::make_shared<AttrColumn>(zvec, m_size);
                m_type = ATTR_VEC3;
            }
            else if constexpr (std::is_same_v<E, std::vector<vec4i>> || std::is_same_v<E, std::vector<vec4f>> || std::is_same_v<E, std::vector<glm::vec4>>) {
                if (m_type != ATTR_TYPE_UNKNOWN && m_type != ATTR_VEC4)
                    throw makeError<UnimplError>("type dismatch, cannot change type of attrvector");
                std::vector<float> xvec(m_size);
                std::vector<float> yvec(m_size);
                std::vector<float> zvec(m_size);
                std::vector<float> wvec(m_size);
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
                m_type = ATTR_VEC4;
            }
        }, val_or_vec);
    }
}