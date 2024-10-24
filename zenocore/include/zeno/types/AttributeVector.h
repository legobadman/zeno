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
        ZENO_API AttrVarVec get();
        ZENO_API void to_prim_attr(std::shared_ptr<PrimitiveObject> spPrim, std::string const& name);

        template<class T>
        std::vector<T> get_attrs() const {
            if constexpr (std::is_same_v<T, vec2f> || std::is_same_v<T, vec2i>) {
                using ElemType = std::conditional<std::is_same_v<T, vec2f>, float, int>::type;
                std::vector<T> vec(m_size);
                for (int i = 0; i < m_size; i++) {
                    ElemType xval = x_comp->get<ElemType>(i);
                    ElemType yval = y_comp->get<ElemType>(i);
                    vec[i] = T(xval, yval);
                }
                return vec;
            }
            else if constexpr (std::is_same_v<T, vec3f> || std::is_same_v<T, vec3i>) {
                using ElemType = std::conditional<std::is_same_v<T, vec3f>, float, int>::type;
                std::vector<T> vec(m_size);
                for (int i = 0; i < m_size; i++) {
                    ElemType xval = x_comp->get<ElemType>(i);
                    ElemType yval = y_comp->get<ElemType>(i);
                    ElemType zval = z_comp->get<ElemType>(i);
                    vec[i] = T(xval, yval, zval);
                }
                return vec;
            }
            else if constexpr (std::is_same_v<T, vec4f> || std::is_same_v<T, vec4i>) {
                using ElemType = std::conditional<std::is_same_v<T, vec4f>, float, int>::type;
                std::vector<T> vec(m_size);
                for (int i = 0; i < m_size; i++) {
                    ElemType xval = x_comp->get<ElemType>(i);
                    ElemType yval = y_comp->get<ElemType>(i);
                    ElemType zval = z_comp->get<ElemType>(i);
                    ElemType wval = w_comp->get<ElemType>(i);
                    vec[i] = T(xval, yval, zval, wval);
                }
                return vec;
            }
            else {
                auto& varvec = self->value();
                return std::visit([&](auto&& vec)->std::vector<T> {
                    using E = std::decay_t<decltype(vec)>;
                    if constexpr (std::is_same_v<E, std::vector<int>> ||
                        std::is_same_v < E, std::vector<float>> ||
                        std::is_same_v < E, std::vector<std::string>> ||
                        std::is_same_v < E, std::vector<vec2i>> ||
                        std::is_same_v < E, std::vector<vec2f>> ||
                        std::is_same_v < E, std::vector<vec3i>> ||
                        std::is_same_v < E, std::vector<vec3f>> ||
                        std::is_same_v < E, std::vector<vec4i>> ||
                        std::is_same_v < E, std::vector<vec4f>>) {
                        if (vec.size() == 1) {
                            return vector<T>(m_size, vec[0]);
                        }
                        return vec;
                    }
                    else {
                        throw;
                    }
                }, varvec);
            }
        }

        template<class T>
        T get_elem(size_t idx) const {
            if constexpr (std::is_same_v<T, vec2f> || std::is_same_v<T, vec2i>) {
                using ElemType = std::conditional<std::is_same_v<T, vec2f>, float, int>::type;
                return T(x_comp->get<ElemType>(idx), y_comp->get<ElemType>(idx));
            }
            else if constexpr (std::is_same_v<T, vec3f> || std::is_same_v<T, vec3i>) {
                using ElemType = std::conditional<std::is_same_v<T, vec3f>, float, int>::type;
                return T(x_comp->get<ElemType>(idx), y_comp->get<ElemType>(idx), z_comp->get<ElemType>(idx));
            }
            else if constexpr (std::is_same_v<T, vec4f> || std::is_same_v<T, vec4i>) {
                using ElemType = std::conditional<std::is_same_v<T, vec4f>, float, int>::type;
                return T(x_comp->get<ElemType>(idx), y_comp->get<ElemType>(idx), z_comp->get<ElemType>(idx), w_comp->get<ElemType>(idx));
            }
            else {
                AttrVarVec& attrval = self->value();
                return self->get<T>(idx);
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

        template<typename T>
        void set_comp(size_t idx, char channel, T val) {
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
                return x_comp->get<T>(idx);
            }
            else if (channel == 'y') {
                return y_comp->get<T>(idx);
            }
            else if (channel == 'z') {
                return z_comp->get<T>(idx);
            }
            else if (channel == 'w') {
                return w_comp->get<T>(idx);
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