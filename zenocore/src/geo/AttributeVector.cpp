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

    ZENO_API AttributeVector::AttributeVector(const Any& val_or_vec, size_t size) : m_size(size){
        set(val_or_vec);
    }

    ZENO_API size_t AttributeVector::size() const {
        return m_size;
    }

    ZENO_API Any AttributeVector::get() {
        //vec?
        return self->value();
    }

    ZENO_API void AttributeVector::set(const Any& val_or_vec) {
        auto& valType = val_or_vec.type();
        if (valType == type_info<vec2i>()) {
            auto& _vec = any_cast<vec2i&>(val_or_vec);
            x_comp = std::make_shared<AttrColumn>(_vec[0], m_size);
            y_comp = std::make_shared<AttrColumn>(_vec[1], m_size);
        }
        else if (valType == type_info<vec2f>()) {
            auto& _vec = any_cast<vec2f&>(val_or_vec);
            x_comp = std::make_shared<AttrColumn>(_vec[0], m_size);
            y_comp = std::make_shared<AttrColumn>(_vec[1], m_size);
        }
        else if (valType == type_info<vec3i>()) {
            auto& _vec = any_cast<vec3i&>(val_or_vec);
            x_comp = std::make_shared<AttrColumn>(_vec[0], m_size);
            y_comp = std::make_shared<AttrColumn>(_vec[1], m_size);
            z_comp = std::make_shared<AttrColumn>(_vec[2], m_size);
        }
        else if (valType == type_info<vec3f>()) {
            auto& _vec = any_cast<vec3f&>(val_or_vec);
            x_comp = std::make_shared<AttrColumn>(_vec[0], m_size);
            y_comp = std::make_shared<AttrColumn>(_vec[1], m_size);
            z_comp = std::make_shared<AttrColumn>(_vec[2], m_size);
        }
        else if (valType == type_info<vec4i>()) {
            auto& _vec = any_cast<vec4i&>(val_or_vec);
            x_comp = std::make_shared<AttrColumn>(_vec[0], m_size);
            y_comp = std::make_shared<AttrColumn>(_vec[1], m_size);
            z_comp = std::make_shared<AttrColumn>(_vec[2], m_size);
            w_comp = std::make_shared<AttrColumn>(_vec[3], m_size);
        }
        else if (valType == type_info<vec4f>()) {
            auto& _vec = any_cast<vec4f&>(val_or_vec);
            x_comp = std::make_shared<AttrColumn>(_vec[0], m_size);
            y_comp = std::make_shared<AttrColumn>(_vec[1], m_size);
            z_comp = std::make_shared<AttrColumn>(_vec[2], m_size);
            w_comp = std::make_shared<AttrColumn>(_vec[3], m_size);
        }
        else if (valType == type_info<std::vector<vec2i>>()) {
            auto& _vec = any_cast<std::vector<vec2i>&>(val_or_vec);
            std::vector<int> xvec(m_size), yvec(m_size);
            for (int i = 0; i < m_size; i++) {
                xvec[i] = _vec[i][0];
                yvec[i] = _vec[i][1];
            }
            x_comp = std::make_shared<AttrColumn>(xvec, m_size);
            y_comp = std::make_shared<AttrColumn>(yvec, m_size);
        }
        else if (valType == type_info<std::vector<vec2f>>()) {
            auto& _vec = any_cast<std::vector<vec2f>&>(val_or_vec);
            std::vector<float> xvec(m_size), yvec(m_size);
            for (float i = 0; i < m_size; i++) {
                xvec[i] = _vec[i][0];
                yvec[i] = _vec[i][1];
            }
            x_comp = std::make_shared<AttrColumn>(xvec, m_size);
            y_comp = std::make_shared<AttrColumn>(yvec, m_size);
        }
        else if (valType == type_info<std::vector<vec3i>>()) {
            auto& _vec = any_cast<std::vector<vec3i>&>(val_or_vec);
            std::vector<int> xvec(m_size), yvec(m_size), zvec(m_size);
            for (int i = 0; i < m_size; i++) {
                xvec[i] = _vec[i][0];
                yvec[i] = _vec[i][1];
                zvec[i] = _vec[i][2];
            }
            x_comp = std::make_shared<AttrColumn>(xvec, m_size);
            y_comp = std::make_shared<AttrColumn>(yvec, m_size);
            z_comp = std::make_shared<AttrColumn>(zvec, m_size);
        }
        else if (valType == type_info<std::vector<vec3f>>()) {
            auto& _vec = any_cast<std::vector<vec3f>&>(val_or_vec);
            std::vector<float> xvec(m_size), yvec(m_size), zvec(m_size);
            for (int i = 0; i < m_size; i++) {
                xvec[i] = _vec[i][0];
                yvec[i] = _vec[i][1];
                zvec[i] = _vec[i][2];
            }
            x_comp = std::make_shared<AttrColumn>(xvec, m_size);
            y_comp = std::make_shared<AttrColumn>(yvec, m_size);
            z_comp = std::make_shared<AttrColumn>(zvec, m_size);
        }
        else if (valType == type_info<std::vector<vec4i>>()) {
            auto& _vec = any_cast<std::vector<vec4i>&>(val_or_vec);
            std::vector<int> xvec(m_size), yvec(m_size), zvec(m_size), wvec(m_size);
            for (int i = 0; i < m_size; i++) {
                xvec[i] = _vec[i][0];
                yvec[i] = _vec[i][1];
                zvec[i] = _vec[i][2];
                wvec[i] = _vec[i][3];
            }
            x_comp = std::make_shared<AttrColumn>(xvec, m_size);
            y_comp = std::make_shared<AttrColumn>(yvec, m_size);
            z_comp = std::make_shared<AttrColumn>(zvec, m_size);
            w_comp = std::make_shared<AttrColumn>(wvec, m_size);
        }
        else if (valType == type_info<std::vector<vec4f>>()) {
            auto& _vec = any_cast<std::vector<vec4f>&>(val_or_vec);
            std::vector<float> xvec(m_size), yvec(m_size), zvec(m_size), wvec(m_size);
            for (int i = 0; i < m_size; i++) {
                xvec[i] = _vec[i][0];
                yvec[i] = _vec[i][1];
                zvec[i] = _vec[i][2];
                wvec[i] = _vec[i][3];
            }
            x_comp = std::make_shared<AttrColumn>(xvec, m_size);
            y_comp = std::make_shared<AttrColumn>(yvec, m_size);
            z_comp = std::make_shared<AttrColumn>(zvec, m_size);
            w_comp = std::make_shared<AttrColumn>(wvec, m_size);
        }
        else {
            self = std::make_shared<AttrColumn>(val_or_vec, m_size);
        }
    }

}