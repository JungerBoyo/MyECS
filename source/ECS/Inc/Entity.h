#ifndef MYECSV_ENTITY_H
#define MYECSV_ENTITY_H

#include <cinttypes>

namespace MyECS
{
    using Entity = uint32_t;
    constexpr Entity InvalidEntity = UINT32_MAX;
}

#endif
