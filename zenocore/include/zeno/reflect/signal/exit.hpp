#pragma once

#include <cstdint>
#include <zeno/reflect/macro.hpp>

namespace zeno
{
namespace reflect
{
    class IExitManager {
    public:
        static IExitManager& get();

        virtual ~IExitManager();
        virtual void graceful_exit(uint8_t exit_code) = 0;
    };
}
}
