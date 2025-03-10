#pragma once

#include <zeno/core/common.h>
#include <zeno/core/IObject.h>
#include <zeno/core/data.h>


namespace zeno {

    using namespace zeno::reflect;

    class AttributeImpl {
    public:
        void set(AttrVarVec elemVal) {
            m_data = elemVal;
        }

        template<typename T>
        void set(size_t index, T elemVal) {
            if (index < 0 || index >= m_size) {
                throw makeError<UnimplError>("index exceed the range of array");
            }

            std::visit([&](auto&& vec) {
                using E = std::decay_t<decltype(vec)>;
                if constexpr (!std::is_same_v<E, std::vector<T>>) {
                    //TODO: 类型冲突时的处理
                    throw;
                }
                else if constexpr (std::is_same_v<E, std::vector<int>> ||
                        std::is_same_v<E, std::vector<float>> ||
                        std::is_same_v<E, std::vector<std::string>>) {
                    if (vec.size() == 1) {
                        vec.resize(m_size, vec[0]);
                    }
                    vec[index] = elemVal;
                }
            }, m_data);
        }

        template<typename T>
        T get(size_t index) const {
            return std::visit([&](auto&& vec)->T {
                using E = std::decay_t<decltype(vec)>;
                if constexpr (!std::is_same_v<E, std::vector<T>>) {
                    throw makeError<UnimplError>("the type T you want to get doesn't match with the type of variant");
                }
                else if constexpr (std::is_same_v<E, std::vector<int>>) {
                    if (vec.size() == 1) {
                        return vec[0];
                    }
                    return vec[index];
                }
                else if constexpr (std::is_same_v<E, std::vector<float>>) {
                    if (vec.size() == 1) {
                        return vec[0];
                    }
                    return vec[index];
                }
                else if constexpr (std::is_same_v<E, std::vector<std::string>>) {
                    if (vec.size() == 1) {
                        return vec[0];
                    }
                    return vec[index];
                }
                else {
                    throw;
                }
            }, m_data);
        }

        AttrVarVec get() const {
            return m_data;
        }

        template<typename T>
        void insert(size_t index, T elemVal) {
            if (index < 0 || index >= m_size) {
                throw makeError<UnimplError>("index exceed the range of array");
            }

            std::visit([&](auto&& vec) {
                using E = std::decay_t<decltype(vec)>;
                if constexpr (!std::is_same_v<E, std::vector<T>>) {
                    throw;
                }
                else if constexpr (std::is_same_v<E, std::vector<int>> ||
                    std::is_same_v < E, std::vector<float>> ||
                    std::is_same_v < E, std::vector<std::string>>) {
                    if (vec.size() == 1) {
                        vec.resize(m_size, vec[0]);
                    }
                    vec.insert(vec.begin() + index, elemVal);
                }
            }, m_data);
            m_size++;
        }

        template<typename T>
        void append(T elementVal) {
            std::visit([&](auto&& vec) {
                using E = std::decay_t<decltype(vec)>;
                if constexpr (!std::is_same_v<E, std::vector<T>>) {
                    throw;
                } else if constexpr (std::is_same_v<E, std::vector<int>> ||
                    std::is_same_v<E, std::vector<float>> ||
                    std::is_same_v<E, std::vector<std::string>>) {
                    if (vec.size() == 1) {
                        vec.resize(m_size, vec[0]);
                    }
                    vec.push_back(elementVal);
                }
            }, m_data);
            m_size++;
        }

        void remove(size_t index) {
            if (index < 0 || index >= m_size) {
                throw makeError<UnimplError>("index exceed the range of attribute data");
            }
            std::visit([&](auto&& vec) {
                using E = std::decay_t<decltype(vec)>;
                if constexpr (std::is_same_v<E, std::vector<int>> ||
                    std::is_same_v < E, std::vector<float>> ||
                    std::is_same_v < E, std::vector<std::string>>) {
                    if (vec.size() == 1) {
                        return;
                    }
                    vec.erase(vec.begin() + index);
                }
            }, m_data);
            m_size--;
        }

        void copy(const AttributeImpl& rattr, int fromIndex) {
            //将rattr整个vector的值，拷贝到m_data[fromIndex:]，特供给merge拷贝属性的时候用
            if (rattr.m_data.index() != m_data.index()) {
                throw makeError<UnimplError>("type dismatch when copy");
            }
            std::visit([&](auto&& vec) {
                using E = std::decay_t<decltype(vec)>;
                if constexpr (std::is_same_v<E, std::vector<int>>) {
                    auto& rvec = std::get<std::vector<int>>(rattr.m_data);
                    std::copy(rvec.begin(), rvec.end(), vec.begin() + fromIndex);
                }
                else if constexpr (std::is_same_v<E, std::vector<float>>) {
                    auto& rvec = std::get<std::vector<float>>(rattr.m_data);
                    std::copy(rvec.begin(), rvec.end(), vec.begin() + fromIndex);
                }
                else if constexpr (std::is_same_v<E, std::vector<std::string>>) {
                    auto& rvec = std::get<std::vector<std::string>>(rattr.m_data);
                    std::copy(rvec.begin(), rvec.end(), vec.begin() + fromIndex);
                }
                //目前向量是独立通道储存的，所以先不写vec类型
                else {
                    throw makeError<UnimplError>("unknown type in attrcolumn");
                }
            }, m_data);
        }

        size_t m_size;
        //暂时不储存核心类型，不过有可能出现类型错误赋值的情况
        AttrVarVec m_data;
    };

    class AttrColumn {
    public:
        ZENO_API AttrColumn() = delete;
        ZENO_API AttrColumn(const AttrColumn& rhs);
        ZENO_API AttrColumn(AttrVarVec value, size_t size);
        ZENO_API ~AttrColumn();
        ZENO_API AttrVarVec& value() const;

        template<typename T>
        T get(size_t index) const {
            return m_pImpl->get<T>(index);
        }

        AttrVarVec get() const;

        template<typename T>
        void set(size_t index, T val) {
            m_pImpl->set(index, val);
        }

        void set(const AttrVarVec& val);

        template<typename T>
        void insert(size_t index, T val) {
            m_pImpl->insert(index, val);
        }

        void copy(const AttrColumn& rCol, int fromIndex) {
            m_pImpl->copy(*rCol.m_pImpl, fromIndex);
        }

        template<typename T>
        void append(T val) {
            m_pImpl->append(val);
        }

        void remove(size_t index);
        size_t size() const;

    private:
        AttributeImpl* m_pImpl;
    };

}