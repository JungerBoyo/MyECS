#ifndef SYSTEM_IMPL_TPP
#define SYSTEM_IMPL_TPP

#include <Inc/System.h>
#include <Inc/EntityManager.h>

namespace MyECS
{
    template<size_t entities_capacity, size_t components_capacity, typename BitsStorageType, typename ...ManagedComponentsTypes>
    requires std::is_unsigned_v<BitsStorageType>
    System<entities_capacity, components_capacity, BitsStorageType, ManagedComponentsTypes...>::
    System(EntityManager<entities_capacity, components_capacity, BitsStorageType> &entityManagerContext)
            : _entityManagerContext(entityManagerContext)
    {

    }
}

#endif