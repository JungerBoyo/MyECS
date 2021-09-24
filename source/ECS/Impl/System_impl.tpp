#ifndef SYSTEM_IMPL_TPP
#define SYSTEM_IMPL_TPP

#include <Inc/System.h>

namespace MyECS
{
    template<size_t components_capacity, typename BitsStorageType> requires std::is_unsigned_v<BitsStorageType>
    template<typename ...ManagedComponentsTypes>
    System<components_capacity, BitsStorageType>::
    System(SystemComponents<ManagedComponentsTypes...>&&)
    {
        (_managedComponentsBits.TrySet(ID::get<ManagedComponentsTypes>()), ...);
    }

    template<size_t components_capacity, typename BitsStorageType> requires std::is_unsigned_v<BitsStorageType>
    void System<components_capacity, BitsStorageType>::
    OnEntityUpdate(Entity entity, const Bits<BitsStorageType, components_capacity>& entityComponentsBits)
    {
        bool componentsMatch = (entityComponentsBits == _managedComponentsBits);
        bool entityExists = _managedEntities.contains(entity);

        if(!entityExists && componentsMatch)
        {
            _managedEntities.emplace(std::make_pair(entity, entity));
            OnEntityAdditionAction(entity);
        }
        else if(entityExists && !componentsMatch)
        {
            _managedEntities.erase(entity);
            OnEntityRemovalAction(entity);
        }
    }

    template<size_t components_capacity, typename BitsStorageType> requires std::is_unsigned_v<BitsStorageType>
    void System<components_capacity, BitsStorageType>::OnEntityRemove(Entity entity)
    {
        if(_managedEntities.contains(entity))
        {
            _managedEntities.erase(entity);
            OnEntityRemovalAction(entity);
        }
    }

    template<size_t components_capacity, typename BitsStorageType> requires std::is_unsigned_v<BitsStorageType>
    void System<components_capacity, BitsStorageType>::
    OnEntityAdd(Entity entity, const Bits <BitsStorageType, components_capacity> &entityComponentsBits)
    {
        if(entityComponentsBits == _managedComponentsBits)
        {
            _managedEntities.emplace(std::make_pair(entity, entity));
            OnEntityAdditionAction(entity);
        }
    }
}

#endif