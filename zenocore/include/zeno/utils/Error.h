#pragma once

#include <zeno/utils/api.h>
#include <string_view>
#include <typeinfo>
#include <string>
#include <memory>

namespace zeno {

struct Error {
    std::string message;

    ZENO_API explicit Error(std::string_view message) noexcept;
    ZENO_API virtual ~Error() noexcept;
    ZENO_API std::string const &what() const noexcept;

    Error(Error const &) = delete;
    Error &operator=(Error const &) = delete;
    Error(Error &&) = delete;
    Error &operator=(Error &&) = delete;
};

struct StdError : Error {
    std::exception_ptr eptr;

    ZENO_API explicit StdError(std::exception_ptr &&eptr) noexcept;
    ZENO_API ~StdError() noexcept override;
};

struct TypeError : Error {
    std::type_info const &expect;
    std::type_info const &got;
    std::string hint;

    ZENO_API explicit TypeError(std::type_info const &expect, std::type_info const &got, std::string_view hint) noexcept;
    ZENO_API ~TypeError() noexcept override;
};

struct KeyError : Error {
    std::string key;
    std::string hint;

    ZENO_API explicit KeyError(std::string_view key, std::string_view hint) noexcept;
    ZENO_API ~KeyError() noexcept override;
};

struct InterruputError : Error {
    std::string m_node;

    ZENO_API explicit InterruputError(std::string const &node) noexcept;
    ZENO_API ~InterruputError() noexcept override;
};

struct IndexError : Error {
    size_t index;
    size_t maxRange;
    std::string hint;

    ZENO_API explicit IndexError(size_t index, size_t maxRange, std::string_view hint) noexcept;
    ZENO_API ~IndexError() noexcept override;
};

struct UnimplError : Error {
    std::string hint;

    ZENO_API explicit UnimplError(std::string_view hint = {}) noexcept;
    ZENO_API ~UnimplError() noexcept override;
};

struct ZfxParseError : Error {
    ZENO_API explicit ZfxParseError() noexcept;
    ZENO_API ~ZfxParseError() noexcept override;
};

class ErrorException : public std::exception {
    std::shared_ptr<Error> const err;
    std::string node_path_info;

public:
    ZENO_API explicit ErrorException(std::shared_ptr<Error> &&err) noexcept;
    ZENO_API explicit ErrorException(const std::string& path, std::shared_ptr<Error>&& err) noexcept;
    ZENO_API ~ErrorException() noexcept override;
    ZENO_API char const *what() const noexcept override;
    ZENO_API std::shared_ptr<Error> getError() const noexcept;
    ZENO_API std::string get_node_info() const;

    ErrorException(ErrorException const &) = default;
    ErrorException &operator=(ErrorException const &) = delete;
    ErrorException(ErrorException &&) = default;
    ErrorException &operator=(ErrorException &&) = delete;
};

template <class T = Error, class ...Ts>
static ErrorException makeError(Ts &&...ts) {
    return ErrorException(std::make_shared<T>(std::forward<Ts>(ts)...));
}

template <class T = Error, class ...Ts>
static ErrorException makeNodeError(std::string const& node_path_info, Ts &&...ts) {
    return ErrorException(node_path_info, std::make_shared<T>(std::forward<Ts>(ts)...));
}

}
