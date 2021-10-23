#ifndef MYECS_ENTITYMANAGER_IMPL_TPP
#define MYECS_ENTITYMANAGER_IMPL_TPP

#include <Inc/TypeIdGenerator.h>
#include "Inc/EntityManager.h"

#ifdef DEBUG_MyECS
#include "Inc/ECS_errorlog.h"
using namespace MyECS::Debug;

#define ENTITY_CAPACITY_EXCEEDED_ERROR(C)             MyECS::Debug::ECS_errorlog<ConstStr<decltype("entity capacity {} exceeded\n"_cStr)>{}, std::size_t>(C)
#define ENTITY_ERROR(entity)                          MyECS::Debug::ECS_errorlog<ConstStr<decltype("entity {} doesn't exist\n"_cStr)>{}, uint32_t>(entity)
#define NON_EXISTENT_COMPONENT_ERROR(T)               MyECS::Debug::ECS_errorlog<ConstStr<decltype("component of type {} doesn't exist\n"_cStr)>{}, const char*>(typeid(T).name())
#define COMPONENT_COUNT_EXCEEDED_ERROR()              MyECS::Debug::ECS_errorlog<ConstStr<decltype("components count exceeded"_cStr)>{}>()
#define ENTITY_DOES_NOT_HAVE_COMPONENT_ERROR(e, T)    MyECS::Debug::ECS_errorlog<ConstStr<decltype("entity {} doesn't have {} component\n"_cStr)>{}, uint32_t, const char*>(e, typeid(T).name())
#define ENTITY_ALREADY_HAVE_COMP_ERROR(e, T)          MyECS::Debug::ECS_errorlog<ConstStr<decltype("entity {} already have component of type {}\n"_cStr)>{}, uint32_t, const char*>(e, typeid(T).name())

#endif


namespace MyECS
{
    template<size_t entities_capacity, size_t components_capacity, typename BitsStorageType>
    requires std::is_unsigned_v<BitsStorageType>
    template<bool ThreadSafeComponents, typename... Args>
    Entity EntityManager<entities_capacity, components_capacity, BitsStorageType>::CreateEntity(Args&&... components)
    {
        #ifdef DEBUG_MyECS
            if(_freeEntities.empty() && _activeEntities.size() >= entities_capacity)
            {
                ENTITY_CAPACITY_EXCEEDED_ERROR(entities_capacity);
                return 0;
            }
        #endif

        Entity entity;
        if(_freeEntities.empty())
        {
            entity = _activeEntities.size();
        }
        else
        {
            entity = _freeEntities.back();
            _freeEntities.pop_back();
        }


        _entitiesStates.Set(entity);
        _activeEntities[entity] = entity;
       (_entitiesComponentsSlots[entity].Set(AddComponent<ThreadSafeComponents>(entity, std::forward<Args>(components))), ...);

        for(auto& system : _systems)
            system->OnEntityAdd(entity, _entitiesComponentsSlots[entity]);

        return entity;
    }

    template<size_t entities_capacity, size_t components_capacity, typename BitsStorageType>
    requires std::is_unsigned_v<BitsStorageType>
    template<typename DerivedSystemType, typename ...ManagedTypes, template <typename...> class T, typename ...Args>
    requires std::is_base_of_v<System <components_capacity, BitsStorageType>, DerivedSystemType>
    DerivedSystemType* EntityManager<entities_capacity, components_capacity, BitsStorageType>::
    CreateSystem(T<ManagedTypes...>, Args&&... args)
    {
        auto system = new DerivedSystemType(std::forward<Args>(args)...);
        _systems.push_back(std::unique_ptr<System<components_capacity, BitsStorageType>>(system));

        auto managedEntities = GetEntitiesWithComponents<ManagedTypes...>();
        std::unordered_map<Entity, Entity> managedEntitiesMap;

        for(Entity i : managedEntities) managedEntitiesMap[i] = i;

        _systems.back()->_managedEntities = std::move(managedEntitiesMap);

        return system;
    }

    template<size_t entities_capacity, size_t components_capacity, typename BitsStorageType>
    requires std::is_unsigned_v<BitsStorageType>
    template<bool ThreadSafeComponents, typename... Args>
    void EntityManager<entities_capacity, components_capacity, BitsStorageType>::AddComponents(Entity entity, Args &&... components)
    {
        #ifdef DEBUG_MyECS
            if(entity < entities_capacity && _entitiesStates.GetBitState(entity))
            {
                (_entitiesComponentsSlots[entity].Set(AddComponent(entity, std::forward<Args>(components))), ...);

                for(auto& system : _systems)
                    system->OnEntityUpdate(entity, _entitiesComponentsSlots[entity]);
            }
            else { ENTITY_ERROR(entity); }
        #else
            (_entitiesComponentsSlots[entity].Set(AddComponent<ThreadSafeComponents>(entity, std::forward<Args>(components))), ...);

            if constexpr(!ThreadSafeComponents)
            {
                for(auto& system : _systems)
                    system->OnEntityUpdate(entity, _entitiesComponentsSlots[entity]);
            }
            else
            {
                for(auto& system : _systems)
                    _pendingUpdates.emplace_back(
                        [&system, entity, &slot = std::as_const(_entitiesComponentsSlots[entity])]{
                            system->OnEntityUpdate(entity, slot);
                        });
            }

        #endif
    }

    template<size_t entities_capacity, size_t components_capacity, typename BitsStorageType>
    requires std::is_unsigned_v<BitsStorageType>
    template<bool ThreadSafeComponent, typename T>
    std::size_t
    EntityManager<entities_capacity, components_capacity, BitsStorageType>::AddComponent(Entity entity, T&& component)
    {
        #ifdef DEBUG_MyECS
            if(ID::get<T>() < components_capacity)
            {
                if(!_activeComponentsMask.GetBitState(ID::get<T>()))
                {
                    _componentStorages[ID::get<T>()] = std::make_unique<ComponentsStorage<components_capacity, BitsStorageType, T>>();
                    ++_componentsCount;
                    _activeComponentsMask.Set(ID::get<T>());
                }

                if(!_entitiesComponentsSlots[entity].GetBitState(ID::get<T>()))
                {
                    StorageCaster<T>()->AddComponentInstance(entity, std::forward<T>(component));
                    return ID::get<T>();
                }
                else { ENTITY_ALREADY_HAVE_COMP_ERROR(entity, T); }
            }
            else { COMPONENT_COUNT_EXCEEDED_ERROR(); }

            return 0;
        #else
            if constexpr(!ThreadSafeComponent)
                if(!_activeComponentsMask.GetBitState(ID::get<T>()))
                {
                    _componentStorages[ID::get<T>()] = std::make_unique<ComponentsStorage<components_capacity, BitsStorageType, T, ThreadSafeComponent>>();
                    ++_componentsCount;
                    _activeComponentsMask.Set(ID::get<T>());
                }

            StorageCaster<T, ThreadSafeComponent>()->AddComponentInstance(entity, std::forward<T>(component));
            return ID::get<T>();
        #endif

    }

    template<size_t entities_capacity, size_t components_capacity, typename BitsStorageType>
    requires std::is_unsigned_v<BitsStorageType>
    template<typename ...Args>
    void EntityManager<entities_capacity, components_capacity, BitsStorageType>::DetachComponents(Entity entity)
    {
        #ifdef DEBUG_MyECS
            if(_entitiesStates.GetBitState(entity))
            {
                (DetachComponent<Args>(entity), ...);

                for(auto& system : _systems)
                    system->OnEntityUpdate(entity, _entitiesComponentsSlots[entity]);
            }
            else { ENTITY_ERROR(entity); }
        #else
            (DetachComponent<Args>(entity), ...);
            for(auto& system : _systems)
                system->OnEntityUpdate(entity, _entitiesComponentsSlots[entity]);
        #endif
    }

    template<size_t entities_capacity, size_t components_capacity, typename BitsStorageType>
    requires std::is_unsigned_v<BitsStorageType>
    template<typename T>
    void EntityManager<entities_capacity, components_capacity, BitsStorageType>::DetachComponent(Entity entity)
    {
        #ifdef DEBUG_MyECS
            if(ID::get<T>() < components_capacity)
            {
                if(_entitiesComponentsSlots[entity].GetBitState(ID::get<T>()))
                {
                    _componentStorages[ID::get<T>()]->DeleteComponentInstance(entity);
                    _entitiesComponentsSlots[entity].Reset(ID::get<T>());
                }
                else { ENTITY_DOES_NOT_HAVE_COMPONENT_ERROR(entity, T); }
            }
            else { COMPONENT_COUNT_EXCEEDED_ERROR(); }
        #else
            _componentStorages[ID::get<T>()]->DeleteComponentInstance(entity);
            _entitiesComponentsSlots[entity].Reset(ID::get<T>());
        #endif
    }

    template<size_t entities_capacity, size_t components_capacity, typename BitsStorageType>
    requires std::is_unsigned_v<BitsStorageType>
    template<typename T>
    bool EntityManager<entities_capacity, components_capacity, BitsStorageType>::HasComponent(Entity entity) const
    {
        #ifdef DEBUG_MyECS
            if(entity < entities_capacity && _entitiesStates.GetBitState(entity))
            {
                if(ID::get<T>() < components_capacity)
                    return _entitiesComponentsSlots[entity].GetBitState(ID::get<T>());
                else
                { COMPONENT_COUNT_EXCEEDED_ERROR(); }
            }
            else
            { ENTITY_ERROR(entity); }

            return false;
        #else
            return _entitiesComponentsSlots[entity].GetBitState(ID::get<T>());
        #endif
    }

    template<size_t entities_capacity, size_t components_capacity, typename BitsStorageType>
    requires std::is_unsigned_v<BitsStorageType>
    template<typename... Args>
    bool EntityManager<entities_capacity, components_capacity, BitsStorageType>::HasComponents(Entity entity) const
    {
        #ifdef DEBUG_MyECS
            if(entity < entities_capacity && _entitiesStates.GetBitState(entity))
            {
                if(((ID::get<Args>() < components_capacity) && ...))
                    return (_entitiesComponentsSlots[entity].GetBitState(ID::get<Args>()) && ...);
                else { COMPONENT_COUNT_EXCEEDED_ERROR(); }
            }
            else { ENTITY_ERROR(entity); }

            return false;
        #else
            return (_entitiesComponentsSlots[entity].GetBitState(ID::get<Args>()) && ...);
        #endif
    }

    template<size_t entities_capacity, size_t components_capacity, typename BitsStorageType>
    requires std::is_unsigned_v<BitsStorageType>
    template<typename ...Args>
    EntityComponentsReturnType<Args...>
    EntityManager<entities_capacity, components_capacity, BitsStorageType>::GetEntityComponents(Entity entity)
    {
        #ifdef DEBUG_MyECS
            if(entity < entities_capacity && _entitiesStates.GetBitState(entity))
            {
                if(((ID::get<Args>() < components_capacity) && ...))
                {
                    if ((_entitiesComponentsSlots[entity].GetBitState(ID::get<Args>()) && ...))
                        return std::tuple<Args*...>{StorageCaster<Args>()->GetByEntity(entity)...};
                    else
                        return {};
                }
                else { COMPONENT_COUNT_EXCEEDED_ERROR(); }
            }
            else { ENTITY_ERROR(entity); }

            return {};
        #else
            return {StorageCaster<Args, false>()->GetByEntity(entity)...};
        #endif
    }

    template<size_t entities_capacity, size_t components_capacity, typename BitsStorageType>
    requires std::is_unsigned_v<BitsStorageType>
    template<bool ThreadSafeComponents, typename... Args>
    EntityComponentsReturnType_const<Args...>
    EntityManager<entities_capacity, components_capacity, BitsStorageType>::GetEntityComponents(Entity entity) const
    {
        #ifdef DEBUG_MyECS
            if(entity < entities_capacity && _entitiesStates.GetBitState(entity))
            {
                if(((ID::get<Args>() < components_capacity) && ...))
                {
                    if ((_entitiesComponentsSlots[entity].GetBitState(ID::get<Args>()) && ...))
                        return std::tuple<const Args*...>{StorageCaster<Args>()->GetByEntity(entity)...};
                    else
                        return {};
                }
                else { COMPONENT_COUNT_EXCEEDED_ERROR(); }
            }
            else { ENTITY_ERROR(entity); }

            return {};
        #else
            return {StorageCaster<Args, ThreadSafeComponents>()->GetByEntity(entity)...};
        #endif
    }

    template<size_t entities_capacity, size_t components_capacity, typename BitsStorageType>
    requires std::is_unsigned_v<BitsStorageType>
    template<typename... Args>
    std::vector<Entity>
    EntityManager<entities_capacity, components_capacity, BitsStorageType>::GetEntitiesWithComponents()
    {
        std::vector<Entity> result(_activeEntities.size());

        for(const auto entity : _activeEntities)
            if(HasComponents<Args...>(entity.second))
                result.push_back(entity.second);

        return result;
    }


    template<size_t entities_capacity, size_t components_capacity, typename BitsStorageType>
    requires std::is_unsigned_v<BitsStorageType>
    void EntityManager<entities_capacity, components_capacity, BitsStorageType>::RemoveEntity(Entity entity)
    {
        #ifdef DEBUG_MyECS
            if(_entitiesStates.GetBitState(entity))
            {
                for(std::size_t i{0}; i<_componentsCount; ++i)
                    if(_componentStorages[i] && _componentStorages[i]->GetBits().IsAndNonZero(_entitiesComponentsSlots[entity]))
                        _componentStorages[i]->DeleteComponentInstance(entity);

                for(auto& system : _systems)
                    system->OnEntityRemove(entity);

                _entitiesStates.Reset(entity);
                _entitiesComponentsSlots[entity].ResetAll();
                _freeEntities.push_back(entity);
                _activeEntities.erase(entity);
            }
            else { ENTITY_ERROR(entity); }
        #else

            for(std::size_t i{0}; i<_componentsCount; ++i)
                if(_componentStorages[i]->GetBits().IsAndNonZero(_entitiesComponentsSlots[entity]))
                    _componentStorages[i]->DeleteComponentInstance(entity);

            for(auto& system : _systems)
                system->OnEntityRemove(entity);

            _entitiesStates.Reset(entity);
            _entitiesComponentsSlots[entity].ResetAll();
            _freeEntities.push_back(entity);
            _activeEntities.erase(entity);
        #endif
    }

    template<size_t entities_capacity, size_t components_capacity, typename BitsStorageType>
    requires std::is_unsigned_v<BitsStorageType>
    void EntityManager<entities_capacity, components_capacity, BitsStorageType>::ExecPendingUpdates()
    {
        while(!_pendingUpdates.empty())
        {
            _pendingUpdates.front()();
            _pendingUpdates.pop_front();
        }
    }

    template<size_t entities_capacity, size_t components_capacity, typename BitsStorageType>
    requires std::is_unsigned_v<BitsStorageType>
    template<typename T>
    void EntityManager<entities_capacity, components_capacity, BitsStorageType>::PreinitThreadSafeComponentStorage()
    {
        if(!_activeComponentsMask.GetBitState(ID::get<T>()))
        {
            _componentStorages[ID::get<T>()] = std::make_unique<ComponentsStorage<components_capacity, BitsStorageType, T, true>>();
            ++_componentsCount;
            _activeComponentsMask.Set(ID::get<T>());
        }
    }

    template<size_t entities_capacity, size_t components_capacity, typename BitsStorageType>
    requires std::is_unsigned_v<BitsStorageType>
    template<bool ThreadSafeComponents, typename T> ComponentsReturnType_const<T>
    EntityManager<entities_capacity, components_capacity, BitsStorageType>::GetComponents() const
    {
        #ifdef DEBUG_MyECS
            if(ID::get<T>() < components_capacity)
            {
                if(_componentStorages[ID::get<T>()])
                    return &StorageCaster<T>()->_componentInstances;
                else
                { NON_EXISTENT_COMPONENT_ERROR(T); }
            }
            else { COMPONENT_COUNT_EXCEEDED_ERROR(); }

            return {};
        #else
            return StorageCaster<T, ThreadSafeComponents>()->_componentInstances;
        #endif
    }

    template<size_t entities_capacity, size_t components_capacity, typename BitsStorageType>
    requires std::is_unsigned_v<BitsStorageType>
    template<typename T> ComponentsReturnType<T>
    EntityManager<entities_capacity, components_capacity, BitsStorageType>::GetComponents()
    {
        #ifdef DEBUG_MyECS
            if(ID::get<T>() < components_capacity)
            {
                if(_componentStorages[ID::get<T>()])
                    return &StorageCaster<T>()->_componentInstances;
                else
                { NON_EXISTENT_COMPONENT_ERROR(T); }
            }
            else
            { COMPONENT_COUNT_EXCEEDED_ERROR(); }

            return {};
        #else
            return StorageCaster<T, false>()->_componentInstances;
        #endif
    }
}

#endif