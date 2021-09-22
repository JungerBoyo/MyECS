#ifndef MYECS_ENTITYMANAGER_IMPL_TPP
#define MYECS_ENTITYMANAGER_IMPL_TPP


#include <typeinfo>
#include <fmt/core.h>
#include <experimental/source_location>
#include <Inc/EntityManager.h>

#include "Inc/TypeIdGenerator.h"


#define ENTITY_ERROR(entity)         constexpr auto loc = std::experimental::source_location::current(); \
                                     fmt::print("| [{}] {}:{}r,{}c | [ECS] [error] entity {} doesn't exist\n",\
                                                loc.file_name(), loc.function_name(), loc.line(), loc.column(), entity)

namespace MyECS
{
    template<size_t entities_capacity, size_t components_capacity, typename BitsStorageType>
    requires std::is_unsigned_v<BitsStorageType>
    template<typename... Args>
    Entity MyECS::EntityManager<entities_capacity, components_capacity, BitsStorageType>::CreateEntity(Args&&... components)
    {
        Entity entity;
        if(_freeEntities.empty())
        {
            entity = _entityCount;
            ++_entityCount;
        }
        else
        {
            entity = _freeEntities.back();
            _freeEntities.pop_back();
        }

        _entities[entity] = true;
        _entitiesComponentsSlots[entity] = _componentManager.AddComponents(entity, std::forward<Args>(components)...);

        return entity;
    }


    template<size_t entities_capacity, size_t components_capacity, typename BitsStorageType>
    requires std::is_unsigned_v<BitsStorageType>
    template<typename... Args>
    void EntityManager<entities_capacity, components_capacity, BitsStorageType>::AddComponents(Entity entity, Args &&... components)
    {
        if(_entities[entity])
            _entitiesComponentsSlots[entity] |= _componentManager.AddComponents(entity, std::forward<Args>(components)...);
        else
        {
            ENTITY_ERROR(entity);
        }
    }

    template<size_t entities_capacity, size_t components_capacity, typename BitsStorageType>
    requires std::is_unsigned_v<BitsStorageType>
    template<typename ...Args>
    void EntityManager<entities_capacity, components_capacity, BitsStorageType>::DetachComponents(Entity entity)
    {
        if(_entities[entity])
            (DetachComponent<Args>(entity), ...);
        else
        {
            ENTITY_ERROR(entity);
        }
    }

    template<size_t entities_capacity, size_t components_capacity, typename BitsStorageType>
    requires std::is_unsigned_v<BitsStorageType>
    template<typename T>
    void EntityManager<entities_capacity, components_capacity, BitsStorageType>::DetachComponent(Entity entity)
    {
        if(ID::ofType<T>() < components_capacity && _entitiesComponentsSlots[entity].GetBitState(ID::ofType<T>()))
        {
            _componentManager.template DeleteSpecificComponent<T>(entity);
            _entitiesComponentsSlots[entity].TryReset(ID::ofType<T>());
        }
        else
        {
            constexpr auto loc = std::experimental::source_location::current();
            fmt::print("| [{}] {}:{}r,{}c | [ECS] [error] entity {} doesn't have {} component\n",
                       loc.file_name(), loc.function_name(), loc.line(), loc.column(), entity, typeid(T).name());
        }
    }

    template<size_t entities_capacity, size_t components_capacity, typename BitsStorageType>
    requires std::is_unsigned_v<BitsStorageType>
    template<typename T>
    bool EntityManager<entities_capacity, components_capacity, BitsStorageType>::HasComponent(Entity entity) const
    {
        if(entity < entities_capacity && _entities[entity] && ID::ofType<T>() < components_capacity)
            return _entitiesComponentsSlots[entity].GetBitState(ID::ofType<T>());
        else
        {
            ENTITY_ERROR(entity);
            return false;
        }
    }

    template<size_t entities_capacity, size_t components_capacity, typename BitsStorageType>
    requires std::is_unsigned_v<BitsStorageType>
    template<typename... Args>
    bool EntityManager<entities_capacity, components_capacity, BitsStorageType>::HasComponents(Entity entity) const
    {
        if(entity < entities_capacity && _entities[entity])
            return (((ID::ofType<Args>() < components_capacity) &&
                     _entitiesComponentsSlots[entity].GetBitState(ID::ofType<Args>())) && ...);
        else
        {
            ENTITY_ERROR(entity);
            return false;
        }
    }

    template<size_t entities_capacity, size_t components_capacity, typename BitsStorageType>
    requires std::is_unsigned_v<BitsStorageType>
    template<typename ...Args>
    std::tuple<Args*...> EntityManager<entities_capacity, components_capacity, BitsStorageType>::GetEntityComponents(Entity entity)
    {
        if(entity < entities_capacity && _entities[entity])
            return _componentManager.template GetEntityComponents<Args...>(entity, _entitiesComponentsSlots[entity]);
        else
        {
            ENTITY_ERROR(entity);

            return {(Args*)(nullptr) ...};
        }
    }

    template<size_t entities_capacity, size_t components_capacity, typename BitsStorageType>
    requires std::is_unsigned_v<BitsStorageType>
    template<typename... Args>
    std::tuple<const Args *...>
    EntityManager<entities_capacity, components_capacity, BitsStorageType>::GetEntityComponents(Entity entity) const
    {
        if(entity < entities_capacity && _entities[entity])
            return _componentManager.template GetEntityComponents<Args...>(entity, _entitiesComponentsSlots[entity]);
        else
        {
            ENTITY_ERROR(entity);

            return {(Args*)(nullptr) ...};
        }
    }

    template<size_t entities_capacity, size_t components_capacity, typename BitsStorageType>
    requires std::is_unsigned_v<BitsStorageType>
    void EntityManager<entities_capacity, components_capacity, BitsStorageType>::RemoveEntity(Entity entity)
    {
        if(_entities[entity])
        {
            _entities[entity] = false;
            _freeEntities.push_back(entity);

            _componentManager.DeleteComponents(entity, std::move(_entitiesComponentsSlots[entity].GetOnes()));
        }
        else
        {
            ENTITY_ERROR(entity);
        }
    }

    template<size_t entities_capacity, size_t components_capacity, typename BitsStorageType>
    requires std::is_unsigned_v<BitsStorageType>
    template<typename T>
    const std::unordered_map<Entity, T> *
    EntityManager<entities_capacity, components_capacity, BitsStorageType>::GetComponents() const
    {
        return _componentManager.template GetComponents<T>();
    }

    template<size_t entities_capacity, size_t components_capacity, typename BitsStorageType>
    requires std::is_unsigned_v<BitsStorageType>
    template<typename T>
    std::unordered_map<Entity, T>* EntityManager<entities_capacity, components_capacity, BitsStorageType>::GetComponents()
    {
        return _componentManager.template GetComponents<T>();
    }
}

#endif