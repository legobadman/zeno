#pragma once

#include <zeno/core/common.h>
#include <zeno/core/IObject.h>
#include <zeno/core/data.h>
#include <zeno/types/AttrColumn.h>
#include "zeno_types/reflect/reflection.generated.hpp"


namespace zeno {

    using namespace zeno::reflect;

    template<class ElemType>
    std::vector<ElemType> getElemVec(const Any& anyv, int n)
    {
        if (anyv.type() == type_info<ElemType>()) {
            ElemType elemVal = any_cast<ElemType>(anyv);
            return std::vector<ElemType>(n, elemVal);
        }
        else if (anyv.type() == type_info<std::vector<ElemType>>()) {
            std::vector<ElemType> vec = any_cast<std::vector<ElemType>>(anyv);
            return vec;
        }
        else {
            throw;
        }
    }

    class AttributeVector
    {
        //以后可能会改用variant
        using AttrVectorVariant = std::variant
            < std::vector<vec3f>
            , std::vector<float>
            , std::vector<vec3i>
            , std::vector<int>
            , std::vector<vec2f>
            , std::vector<vec2i>
            , std::vector<vec4f>
            , std::vector<vec4i>
            >;

    public:
        AttributeVector() = delete;
        ZENO_API AttributeVector(const Any& val_or_vec, size_t size);
        ZENO_API AttributeVector(const AttributeVector& rhs);
        ZENO_API size_t size() const;
        ZENO_API void set(const Any& val_or_vec);
        ZENO_API Any get();

        template<class T>
        std::vector<T> get_attrs() const {
            if constexpr (std::is_same_v<T, vec2f> || std::is_same_v<T, vec2i>) {
                using ElemType = std::conditional<std::is_same_v<T, vec2f>, float, int>::type;
                std::vector<ElemType> xvals = getElemVec<ElemType>(x_comp->value(), m_size);
                std::vector<ElemType> yvals = getElemVec<ElemType>(y_comp->value(), m_size);
                std::vector<T> vec(m_size);
                for (int i = 0; i < m_size; i++) {
                    vec[i] = T(xvals[i], yvals[i]);
                }
                return vec;
            }
            else if constexpr (std::is_same_v<T, vec3f> || std::is_same_v<T, vec3i>) {
                using ElemType = std::conditional<std::is_same_v<T, vec3f>, float, int>::type;
                std::vector<ElemType> xvals = getElemVec<ElemType>(x_comp->value(), m_size);
                std::vector<ElemType> yvals = getElemVec<ElemType>(y_comp->value(), m_size);
                std::vector<ElemType> zvals = getElemVec<ElemType>(z_comp->value(), m_size);
                std::vector<T> vec(m_size);
                for (int i = 0; i < m_size; i++) {
                    vec[i] = T(xvals[i], yvals[i], zvals[i]);
                }
                return vec;
            }
            else if constexpr (std::is_same_v<T, vec4f> || std::is_same_v<T, vec4i>) {
                using ElemType = std::conditional<std::is_same_v<T, vec4f>, float, int>::type;
                std::vector<ElemType> xvals = getElemVec<ElemType>(x_comp->value(), m_size);
                std::vector<ElemType> yvals = getElemVec<ElemType>(y_comp->value(), m_size);
                std::vector<ElemType> zvals = getElemVec<ElemType>(z_comp->value(), m_size);
                std::vector<ElemType> wvals = getElemVec<ElemType>(w_comp->value(), m_size);
                std::vector<T> vec(m_size);
                for (int i = 0; i < m_size; i++) {
                    vec[i] = T(xvals[i], yvals[i], zvals[i], wvals[i]);
                }
                return vec;
            }
            else {
                return any_cast<std::vector<T>>(self->value());
            }
        }

        std::vector<float> get_attr_value_float() const {
            return any_cast<std::vector<float>>(self->value());
        }

        std::vector<int> get_attr_value_int() const {
            return any_cast<std::vector<int>>(self->value());
        }

        std::vector<zeno::vec3f> get_attr_value_vec3f() const {
            return any_cast<std::vector<zeno::vec3f>>(self->value());
        }

        template<class T>
        T get_elem(size_t idx) const {
            auto& attrval = self->value();
            const auto& valType = attrval.type();
            if constexpr (std::is_same_v<T, vec2f> || std::is_same_v<T, vec2i>) {
                using ElemType = std::conditional<std::is_same_v<T, vec2f>, float, int>::type;
                return T(any_cast<ElemType>(x_comp->get(idx)), any_cast<ElemType>(y_comp->get(idx)));
            }
            else if constexpr (std::is_same_v<T, vec3f> || std::is_same_v<T, vec3i>) {
                using ElemType = std::conditional<std::is_same_v<T, vec3f>, float, int>::type;
                return T(any_cast<ElemType>(x_comp->get(idx)), any_cast<ElemType>(y_comp->get(idx)), any_cast<ElemType>(z_comp->get(idx)));
            }
            else if constexpr (std::is_same_v<T, vec4f> || std::is_same_v<T, vec4i>) {
                using ElemType = std::conditional<std::is_same_v<T, vec4f>, float, int>::type;
                return T(any_cast<ElemType>(x_comp->get(idx)), any_cast<ElemType>(y_comp->get(idx)),
                        any_cast<ElemType>(z_comp->get(idx)), any_cast<ElemType>(w_comp->get(idx)));
            }
            else {
                if (valType == type_info<T>()) {
                    return any_cast<T>(attrval);
                }
                else if (valType == type_info<std::vector<T>>()) {
                    auto& xvec = any_cast<std::vector<T>&>(attrval);
                    return xvec[idx];
                }
                else {
                    throw;
                }
            }
        }

        template<class T>
        void set_elem(size_t idx, T val) {
            if constexpr (std::is_same_v<T, vec2f> || std::is_same_v<T, vec2i>) {
                if (x_comp.use_count() > 1)
                    x_comp = std::make_shared<AttrColumn>(*x_comp);
                if (y_comp.use_count() > 1)
                    y_comp = std::make_shared<AttrColumn>(*y_comp);
                x_comp->set(idx, val[0]);
                y_comp->set(idx, val[1]);
            }
            else if constexpr (std::is_same_v<T, vec3f> || std::is_same_v<T, vec3i>) {
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
            else if constexpr (std::is_same_v<T, vec4f> || std::is_same_v<T, vec4i>) {
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
                if (self.use_count() > 1) {
                    self = std::make_shared<AttrColumn>(*self);
                }
                self->set(idx, val);
            }
        }

        void set_comp(size_t idx, char channel, Any val) {
            if (channel == 'x') {
                if (x_comp.use_count() > 1)
                    x_comp = std::make_shared<AttrColumn>(*x_comp);
                x_comp->set(idx, val);
            }
            else if (channel == 'y') {
                if (y_comp.use_count() > 1)
                    y_comp = std::make_shared<AttrColumn>(*y_comp);
                y_comp->set(idx, val);
            }
            else if (channel == 'z') {
                if (z_comp.use_count() > 1)
                    z_comp = std::make_shared<AttrColumn>(*z_comp);
                z_comp->set(idx, val);
            }
            else if (channel == 'w') {
                if (w_comp.use_count() > 1)
                    w_comp = std::make_shared<AttrColumn>(*w_comp);
                w_comp->set(idx, val);
            }
            else {
                throw;
            }
        }

        template<class T>
        T get_comp(size_t idx, char channel) {
            if (channel == 'x') {
                return x_comp->get(idx);
            }
            else if (channel == 'y') {
                return y_comp->get(idx);
            }
            else if (channel == 'z') {
                return z_comp->get(idx);
            }
            else if (channel == 'w') {
                return w_comp->get(idx);
            }
            else {
                throw;
            }
        }

    private:
        //这里把vector的分量拆出来，仅仅考虑了内存储存优化的情况，如果后续基于共享内存，写法会改变。
        std::shared_ptr<AttrColumn> x_comp, y_comp, z_comp, w_comp, self;
        size_t m_size;
    };
}