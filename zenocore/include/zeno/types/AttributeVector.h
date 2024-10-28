#pragma once

#include <zeno/core/common.h>
#include <zeno/core/IObject.h>
#include <zeno/core/data.h>
#include <zeno/types/AttrColumn.h>


namespace zeno {

    using namespace zeno::reflect;

    class AttributeVector
    {
    public:
        AttributeVector() = delete;
        ZENO_API AttributeVector(const AttrVar& val_or_vec, size_t size);
        ZENO_API AttributeVector(const AttributeVector& rhs);
        ZENO_API size_t size() const;
        ZENO_API void set(const AttrVar& val_or_vec);
        ZENO_API GeoAttrType type() const;
        ZENO_API AttrVarVec get();
        ZENO_API void to_prim_attr(std::shared_ptr<PrimitiveObject> spPrim, std::string const& name);

        template<class T, char CHANNEL = 0>
        std::vector<T> get_attrs() const {
            if constexpr (std::is_same_v<T, vec2f> || std::is_same_v<T, glm::vec2>) {
                std::vector<T> vec(m_size);
                for (int i = 0; i < m_size; i++) {
                    float xval = x_comp->get<float>(i);
                    float yval = y_comp->get<float>(i);
                    vec[i] = T(xval, yval);
                }
                return vec;
            }
            else if constexpr (std::is_same_v<T, vec3f> || std::is_same_v<T, glm::vec3>) {
                std::vector<T> vec(m_size);
                for (int i = 0; i < m_size; i++) {
                    float xval = x_comp->get<float>(i);
                    float yval = y_comp->get<float>(i);
                    float zval = z_comp->get<float>(i);
                    vec[i] = T(xval, yval, zval);
                }
                return vec;
            }
            else if constexpr (std::is_same_v<T, vec4f> || std::is_same_v<T, glm::vec4>) {
                std::vector<T> vec(m_size);
                for (int i = 0; i < m_size; i++) {
                    float xval = x_comp->get<float>(i);
                    float yval = y_comp->get<float>(i);
                    float zval = z_comp->get<float>(i);
                    float wval = w_comp->get<float>(i);
                    vec[i] = T(xval, yval, zval, wval);
                }
                return vec;
            }
            else {
                if constexpr (CHANNEL > 0) {
                    std::shared_ptr<AttrColumn> spComp;
                    if constexpr (CHANNEL == 'x') { spComp = x_comp; }
                    else if constexpr (CHANNEL == 'y') { spComp = y_comp; }
                    else if constexpr (CHANNEL == 'z') { spComp = z_comp; }
                    else if constexpr (CHANNEL == 'w') { spComp = w_comp; }
                    if (!spComp)
                        throw UnimplError("the variant doesn't contain component vector");

                    return std::visit([&](auto&& vec)->std::vector<T> {
                        using E = std::decay_t<decltype(vec)>;
                        if constexpr (std::is_same_v<E, std::vector<T>>) {
                            if (vec.size() == 1) {
                                return std::vector<T>(m_size, vec[0]);
                            }
                            else {
                                return vec;
                            }
                        }
                        else {
                            throw UnimplError("type dismatch");
                        }
                    }, spComp->value());
                }
                else {
                    //现在对于向量采用分列储存，因此实质上内部只有int float string三种类型
                    auto& varvec = self->value();
                    return std::visit([&](auto&& vec)->std::vector<T> {
                        using E = std::decay_t<decltype(vec)>;
                        if constexpr (
                            (std::is_same_v<E, std::vector<int>> && std::is_same_v<T, int>) ||
                            (std::is_same_v<E, std::vector<float>> && std::is_same_v<T, float>) ||
                            (std::is_same_v<E, std::vector<std::string>> && std::is_same_v<T, std::string>)) {
                            if (vec.size() == 1) {
                                return std::vector<T>(m_size, vec[0]);
                            }
                            return vec;
                        }
                        else {
                            throw UnimplError("internal type error of primitive type of attribute data");
                        }
                    }, varvec);
                }
            }
        }

        template<typename T, char CHANNEL = 0>
        T get_elem(size_t idx) const {
            if constexpr (std::is_same_v<T, vec2f> || std::is_same_v<T, glm::vec2>) {
                return T(x_comp->get<float>(idx), y_comp->get<float>(idx));
            }
            else if constexpr (std::is_same_v<T, vec3f> || std::is_same_v<T, glm::vec3>) {
                return T(x_comp->get<float>(idx), y_comp->get<float>(idx), z_comp->get<float>(idx));
            }
            else if constexpr (std::is_same_v<T, vec4f> || std::is_same_v<T, glm::vec4>) {
                return T(x_comp->get<float>(idx), y_comp->get<float>(idx), z_comp->get<float>(idx), w_comp->get<float>(idx));
            }
            else {
                if constexpr (CHANNEL > 0) {
                    std::shared_ptr<AttrColumn> spComp;
                    if constexpr (CHANNEL == 'x') { spComp = x_comp; }
                    else if constexpr (CHANNEL == 'y') { spComp = y_comp; }
                    else if constexpr (CHANNEL == 'z') { spComp = z_comp; }
                    else if constexpr (CHANNEL == 'w') { spComp = w_comp; }
                    if (!spComp)
                        throw UnimplError("the variant doesn't contain component vector");
                    return std::visit([&](auto&& vec)->T {
                        using E = std::decay_t<decltype(vec)>;
                        if constexpr (std::is_same_v<E, std::vector<T>>) {
                            if (vec.size() == 1) {
                                return vec[0];
                            }
                            else {
                                return vec[idx];
                            }
                        }
                        else {
                            throw;
                        }
                    }, spComp->value());
                }
                else {
                    AttrVarVec& attrval = self->value();
                    return self->get<T>(idx);
                }
            }
        }

        template<class T, char CHANNEL = 0>
        void set_elem(size_t idx, T val) {
            if constexpr (std::is_same_v<T, vec2f> || std::is_same_v<T, glm::vec2>) {
                if (x_comp.use_count() > 1)
                    x_comp = std::make_shared<AttrColumn>(*x_comp);
                if (y_comp.use_count() > 1)
                    y_comp = std::make_shared<AttrColumn>(*y_comp);
                x_comp->set(idx, val[0]);
                y_comp->set(idx, val[1]);
            }
            else if constexpr (std::is_same_v<T, vec3f> || std::is_same_v<T, glm::vec3>) {
                if (x_comp.use_count() > 1)
                    x_comp = std::make_shared<AttrColumn>(*x_comp);
                if (y_comp.use_count() > 1)
                    y_comp = std::make_shared<AttrColumn>(*y_comp);
                if (z_comp.use_count() > 1)
                    z_comp = std::make_shared<AttrColumn>(*z_comp);
                x_comp->set(idx, val[0]);
                y_comp->set(idx, val[1]);
                z_comp->set(idx, val[2]);
            }
            else if constexpr (std::is_same_v<T, vec4f> || std::is_same_v<T, glm::vec4>) {
                if (x_comp.use_count() > 1)
                    x_comp = std::make_shared<AttrColumn>(*x_comp);
                if (y_comp.use_count() > 1)
                    y_comp = std::make_shared<AttrColumn>(*y_comp);
                if (z_comp.use_count() > 1)
                    z_comp = std::make_shared<AttrColumn>(*z_comp);
                if (w_comp.use_count() > 1)
                    w_comp = std::make_shared<AttrColumn>(*z_comp);
                x_comp->set(idx, val[0]);
                y_comp->set(idx, val[1]);
                z_comp->set(idx, val[2]);
                w_comp->set(idx, val[3]);
            }
            else {
                if constexpr (CHANNEL > 0) {
                    std::shared_ptr<AttrColumn> spComp;
                    if constexpr (CHANNEL == 'x') {
                        if (x_comp.use_count() > 1) {
                            x_comp = std::make_shared<AttrColumn>(*x_comp);
                        }
                        x_comp->set(idx, val);
                    }
                    else if constexpr (CHANNEL == 'y') {
                        if (y_comp.use_count() > 1) {
                            y_comp = std::make_shared<AttrColumn>(*y_comp);
                        }
                        y_comp->set(idx, val);
                    }
                    else if constexpr (CHANNEL == 'z') {
                        if (z_comp.use_count() > 1) {
                            z_comp = std::make_shared<AttrColumn>(*z_comp);
                        }
                        z_comp->set(idx, val);
                    }
                    else if constexpr (CHANNEL == 'w') {
                        if (w_comp.use_count() > 1) {
                            w_comp = std::make_shared<AttrColumn>(*w_comp);
                        }
                        w_comp->set(idx, val);
                    }
                    else {
                        throw;
                    }
                }
                else {
                    if (self.use_count() > 1) {
                        self = std::make_shared<AttrColumn>(*self);
                    }
                    self->set(idx, val);
                }
            }
        }

        template<class T>
        void foreach_attr_update(std::function<T(T old_elem_value)>&& evalf) {
            if constexpr (std::is_same_v<T, vec2f>) {
                if (x_comp.use_count() > 1)
                    x_comp = std::make_shared<AttrColumn>(*x_comp);
                if (y_comp.use_count() > 1)
                    y_comp = std::make_shared<AttrColumn>(*y_comp);
                AttrVarVec& xvar = x_comp->value();
                AttrVarVec& yvar = y_comp->value();
                auto pXVec = std::get_if<std::vector<float>>(&xvar);
                auto pYVec = std::get_if<std::vector<float>>(&yvar);
                if (!pXVec || !pYVec)
                    throw UnimplError("type dismatch");
                int nx = pXVec->size(), ny = pYVec->size();
#pragma omp parallel for
                for (int i = 0; i < m_size; i++) {
                    int ix = std::min(i, nx-1), iy = std::min(i, ny-1);
                    T old_val(*pXVec[ix], (*pYVec)[iy]);
                    T new_val = evalf(old_val);
                    (*pXVec)[ix] = new_val[0];
                    (*pYVec)[iy] = new_val[1];
                }
            }
            else if constexpr (std::is_same_v<T, vec3f>) {
                if (x_comp.use_count() > 1)
                    x_comp = std::make_shared<AttrColumn>(*x_comp);
                if (y_comp.use_count() > 1)
                    y_comp = std::make_shared<AttrColumn>(*y_comp);
                if (z_comp.use_count() > 1)
                    z_comp = std::make_shared<AttrColumn>(*z_comp);

                AttrVarVec& xvar = x_comp->value();
                AttrVarVec& yvar = y_comp->value();
                AttrVarVec& zvar = z_comp->value();
                auto pXVec = std::get_if<std::vector<float>>(&xvar);
                auto pYVec = std::get_if<std::vector<float>>(&yvar);
                auto pZVec = std::get_if<std::vector<float>>(&zvar);
                if (!pXVec || !pYVec || !pZVec)
                    throw UnimplError("type dismatch");
                int nx = pXVec->size(), ny = pYVec->size(), nz = pZVec->size();
#pragma omp parallel for
                for (int i = 0; i < m_size; i++) {
                    int ix = std::min(i, nx-1), iy = std::min(i, ny-1), iz = std::min(i, nz-1);
                    T old_val((*pXVec)[ix], (*pYVec)[iy], (*pZVec)[iz]);
                    T new_val = evalf(old_val);
                    (*pXVec)[ix] = new_val[0];
                    (*pYVec)[iy] = new_val[1];
                    (*pZVec)[iz] = new_val[2];
                }
            }
            else if constexpr (std::is_same_v<T, vec4f>) {
                
            }
            else {
                AttrVarVec& attrval = self->value();
                
            }
        }

    private:
        //这里把vector的分量拆出来，仅仅考虑了内存储存优化的情况，如果后续基于共享内存，写法会改变。
        std::shared_ptr<AttrColumn> x_comp, y_comp, z_comp, w_comp, self;
        size_t m_size;
        GeoAttrType m_type;
    };
}