#ifndef MYECS_COMPONENTMANAGER_IMPL_TPP
#define MYECS_COMPONENTMANAGER_IMPL_TPP

#include <typeinfo>
#include <fmt/core.h>
#include <experimental/source_location>
#include <Inc/ComponentManager.h>

#include "Inc/TypeIdGenerator.h"

namespace MyECS
{
    template<size_t components_capacity, typename BitsStorageType>
    requires std::is_unsigned_v<BitsStorageType>
    template<typename ...Args>
    Bits<BitsStorageType, components_capacity>
    ComponentManager<components_capacity, BitsStorageType>::AddComponents(Entity entity, Args&&... components)
    {
        Bits<BitsStorageType, components_capacity> bitset;
        (bitset.TrySet(AddComponent(entity, std::forward<Args>(components))), ...);

        _activeComponentsMask |= bitset;

        return bitset;
    }

    template<size_t components_capacity, typename BitsStorageType>
    requires std::is_unsigned_v<BitsStorageType>
    template<typename T>
    std::size_t ComponentManager<components_capacity, BitsStorageType>::AddComponent(Entity entity, T&& component)
    {
        if(ID::ofType<T>() < components_capacity)
        {
            if(!_activeComponentsMask.GetBitState(ID::ofType<T>()))
                _components[ID::ofType<T>()] = new ComponentsStorage<T>();

            if(!((ComponentsStorage<T>*)(_components[ID::ofType<T>()]))->_componentInstances.contains(entity))
            {
                ((ComponentsStorage<T>*)(_components[ID::ofType<T>()]))->_componentInstances.emplace(std::make_pair(entity, component));
            }
            else
            {
                auto loc = std::experimental::source_location::current();
                fmt::print("| [{}] {}:{}r,{}c | [ECS] [warning] entity {} already have component of type {}\n",
                        loc.file_name(), loc.function_name(), loc.line(), loc.column(), entity, typeid(T).name());
            }
        }
        else
        {
            auto loc = std::experimental::source_location::current();
            fmt::print("| [{}] {}:{}r,{}c | [ECS] [warning] components count exceeded\n",
                    loc.file_name(), loc.function_name(), loc.line(), loc.column());
        }

        return ID::ofType<T>();
    }

    template<size_t components_capacity, typename BitsStorageType>
    requires std::is_unsigned_v<BitsStorageType>
    void
    ComponentManager<components_capacity, BitsStorageType>::DeleteComponents(Entity entity, std::vector<uint32_t>&& componentsIndices)
    {
        for(const auto index : componentsIndices)
            _components[index]->DeleteComponentInstance(entity);
    }

    template<size_t components_capacity, typename BitsStorageType>
    requires std::is_unsigned_v<BitsStorageType>
    ComponentManager<components_capacity, BitsStorageType>::~ComponentManager()
    {
        for(auto component : _components)
            if(component) delete component;
    }

    template<size_t components_capacity, typename BitsStorageType>
    requires std::is_unsigned_v<BitsStorageType>
    template<typename T>
    void ComponentManager<components_capacity, BitsStorageType>::DeleteSpecificComponent(Entity entity)
    {
        _components[ID::ofType<T>()]->DeleteComponentInstance(entity);
    }

    template<size_t components_capacity, typename BitsStorageType>
    requires std::is_unsigned_v<BitsStorageType>
    template<typename T>
    std::unordered_map<Entity, T>* ComponentManager<components_capacity, BitsStorageType>::GetComponents()
    {
        if(ID::ofType<T>() < components_capacity)
        {
            return &((ComponentsStorage<T>*)(_components[ID::ofType<T>()]))->_componentInstances;
        }
        else
        {
            auto loc = std::experimental::source_location::current();
            fmt::print("| [{}] {}:{}r,{}c | [ECS] [warning] component of type {} doesn't exist\n",
                    loc.file_name(), loc.function_name(), loc.line(), loc.column(), typeid(T).name());

            return nullptr;
        }
    }

    template<size_t components_capacity, typename BitsStorageType>
    requires std::is_unsigned_v<BitsStorageType>
    template<typename ...Args>
    std::tuple<Args*...>
    ComponentManager<components_capacity, BitsStorageType>::GetEntityComponents(Entity entity,
                                                            const Bits<BitsStorageType, components_capacity>& componentsSlots)
    {
        return {GetEntityComponent<Args>(entity, componentsSlots.TryGetBitState(ID::ofType<Args>())) ...};
    }

    template<size_t components_capacity, typename BitsStorageType>
    requires std::is_unsigned_v<BitsStorageType>
    template<typename T>
    T* ComponentManager<components_capacity, BitsStorageType>::GetEntityComponent(Entity entity, bool componentSlotState)
    {
        if(ID::ofType<T>() < components_capacity && componentSlotState)
        {
            return &((ComponentsStorage<T>*)(_components[ID::ofType<T>()]))->_componentInstances[entity];
        }
        else
        {
            auto loc = std::experimental::source_location::current();
            fmt::print("| [{}] {}:{}r,{}c | [ECS] [warning] entity {} doesn't have component of type {}\n",
                    loc.file_name(), loc.function_name(), loc.line(), loc.column(), entity, typeid(T).name());

            return nullptr;
        }
    }
}

#endif