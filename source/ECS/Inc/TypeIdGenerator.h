#ifndef MYECS_TYPEIDGENERATOR_H
#define MYECS_TYPEIDGENERATOR_H

#include <cinttypes>
#include <atomic>

namespace MyECS
{
        struct ID
        {
            template<typename T>
            static std::size_t get()
            {
                static std::size_t ID = counter++;
                return ID;
            }

            private:
                static std::atomic<std::size_t> counter;
        };
}

#endif