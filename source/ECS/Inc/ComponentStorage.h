#ifndef MYECS_COMPONENTMANAGER_H
#define MYECS_COMPONENTMANAGER_H

#include <unordered_map>
#include <vector>
#include <memory>
#include <Inc/Entity.h>
#include <Inc/Bits.h>
#include <Inc/TypeIdGenerator.h>
#include <mutex>

namespace MyECS
{
    #ifdef DEBUG_MyECS
        template<typename T>
        using ComponentReturnType = T*

        template<typename T>
        using ComponentReturnType_const = const T*;

    #else
        template<typename T>
        using ComponentReturnType = T&;

        template<typename T>
        using ComponentReturnType_const = const T&;
    #endif


    template<size_t components_capacity, typename BitsStorageType> requires std::is_unsigned_v<BitsStorageType>
    class BaseComponentsStorage
    {
        public:
            virtual void DeleteComponentInstance(Entity) = 0;
            virtual const Bits<BitsStorageType, components_capacity>& GetBits() const = 0;
    };

    template<size_t components_capacity, typename BitsStorageType, typename T, bool ThreadSafeStorage>
    requires std::is_unsigned_v<BitsStorageType>
    class ComponentsStorage : public BaseComponentsStorage<components_capacity, BitsStorageType>{};


    template<size_t components_capacity, typename BitsStorageType, typename T>
    class ComponentsStorage<components_capacity, BitsStorageType, T, true> : public BaseComponentsStorage<components_capacity, BitsStorageType>
    {
        template<size_t, size_t, typename BitsStorageType_> requires std::is_unsigned_v<BitsStorageType_>
        friend class EntityManager;

        public:
            ComponentsStorage()
            {
                _componentBits.Set(ID::get<T>());
            }

            void DeleteComponentInstance(Entity entity) override
            {
                std::lock_guard<std::mutex> lock{_mutex};

                _componentInstances[_entityToComponentIndex[entity]] = std::move(_componentInstances.back());

                _entityToComponentIndex[_componentIndexToEntity[_componentInstances.size() - 1]] = _entityToComponentIndex[entity];
                _componentIndexToEntity[_entityToComponentIndex[entity]] = _componentIndexToEntity[_componentInstances.size() - 1];

                _entityToComponentIndex.erase(entity);
                _componentIndexToEntity.erase(_componentInstances.size() - 1);

                _componentInstances.pop_back();
            }

            void AddComponentInstance(Entity entity, T&& instance)
            {
                std::lock_guard<std::mutex> lock{_mutex};
                _entityToComponentIndex[entity] = _componentInstances.size();
                _componentIndexToEntity[_componentInstances.size()] = entity;
                _componentInstances.emplace_back(instance);
            }

            const Bits<BitsStorageType, components_capacity>& GetBits() const override
            {
                return _componentBits;
            }

#ifdef DEBUG_MyECS
            const T* GetByEntity(Entity entity) const
            {
                std::lock_guard<std::mutex> lock{_mutex};
                return &_componentInstances[_entityToComponentIndex.at(entity)];
            }
#else
            const T& GetByEntity(Entity entity) const
            {
                std::lock_guard<std::mutex> lock{_mutex};
                return _componentInstances[_entityToComponentIndex.at(entity)];
            }
#endif

        private:
            mutable std::mutex _mutex;

            Bits<BitsStorageType, components_capacity> _componentBits;
            std::unordered_map<Entity, std::size_t> _entityToComponentIndex;
            std::unordered_map<std::size_t, Entity> _componentIndexToEntity;
            std::vector<T> _componentInstances;
    };

    template<size_t components_capacity, typename BitsStorageType, typename T>
    class ComponentsStorage<components_capacity, BitsStorageType, T, false> : public BaseComponentsStorage<components_capacity, BitsStorageType>
    {
        template<size_t, size_t, typename BitsStorageType_> requires std::is_unsigned_v<BitsStorageType_>
        friend class EntityManager;

        public:
            ComponentsStorage()
            {
                _componentBits.Set(ID::get<T>());
            }

            void DeleteComponentInstance(Entity entity) override
            {
                _componentInstances[_entityToComponentIndex[entity]] = std::move(_componentInstances.back());

                _entityToComponentIndex[_componentIndexToEntity[_componentInstances.size() - 1]] = _entityToComponentIndex[entity];
                _componentIndexToEntity[_entityToComponentIndex[entity]] = _componentIndexToEntity[_componentInstances.size() - 1];

                _entityToComponentIndex.erase(entity);
                _componentIndexToEntity.erase(_componentInstances.size() - 1);

                _componentInstances.pop_back();
            }

            void AddComponentInstance(Entity entity, T&& instance)
            {
                _entityToComponentIndex[entity] = _componentInstances.size();
                _componentIndexToEntity[_componentInstances.size()] = entity;
                _componentInstances.emplace_back(instance);
            }

            const Bits<BitsStorageType, components_capacity>& GetBits() const override
            {
                return _componentBits;
            }

#ifdef DEBUG_MyECS
            T* GetByEntity(Entity entity)
            {
                std::lock_guard<std::mutex> lock{_mutex};
                return &_componentInstances[_entityToComponentIndex.at(entity)];
            }

            const T* GetByEntity(Entity entity) const
            {
                std::lock_guard<std::mutex> lock{_mutex};
                return &_componentInstances[_entityToComponentIndex.at(entity)];
            }
#else
            T& GetByEntity(Entity entity)
            {
                return _componentInstances[_entityToComponentIndex.at(entity)];
            }

            const T& GetByEntity(Entity entity) const
            {
                return _componentInstances[_entityToComponentIndex.at(entity)];
            }
#endif

        private:
            Bits<BitsStorageType, components_capacity> _componentBits;
            std::unordered_map<Entity, std::size_t> _entityToComponentIndex;
            std::unordered_map<std::size_t, Entity> _componentIndexToEntity;
            std::vector<T> _componentInstances;
    };

}

#endif