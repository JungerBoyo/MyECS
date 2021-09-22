#ifndef MYECS_SYSTEM_H
#define MYECS_SYSTEM_H

#include <Inc/EntityManager.h>

namespace MyECS
{
    template<size_t entity_capacity, size_t component_capacity, typename BitsStorageType, typename ...ManagedComponentsTypes>
    requires std::is_unsigned_v<BitsStorageType>
    class System
    {
        public:
            explicit System(EntityManager<entity_capacity, component_capacity, BitsStorageType>& entityManagerContext);

        protected:


        protected:
            EntityManager<entity_capacity, component_capacity, BitsStorageType>& _entityManagerContext;
    };
}

#include "Impl/System_impl.tpp"

#endif
