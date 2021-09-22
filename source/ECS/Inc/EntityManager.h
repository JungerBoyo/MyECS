#ifndef MYECS_ENTITYMANAGER_H
#define MYECS_ENTITYMANAGER_H

#include "ComponentManager.h"

namespace MyECS
{
    template<size_t entities_capacity, size_t components_capacity, typename BitsStorageType>
    requires std::is_unsigned_v<BitsStorageType>
    class EntityManager
    {
        public: 
            EntityManager()
            {
                _freeEntities.reserve(entities_capacity);
            }

            EntityManager(const EntityManager&) = delete;
            EntityManager& operator=(const EntityManager&) = delete;

            template<typename ...Args>
            Entity CreateEntity(Args&&... components);

            template<typename ...Args>
            void AddComponents(Entity, Args&&... components);

            template<typename ...Args>
            void DetachComponents(Entity);

            template<typename T>
            bool HasComponent(Entity) const;

            template<typename ...Args>
            bool HasComponents(Entity) const;

            template<typename ...Args>
            std::tuple<Args*...> GetEntityComponents(Entity);

            template<typename ...Args>
            std::tuple<const Args*...> GetEntityComponents(Entity) const;

            template<typename T>
            std::unordered_map<Entity, T>* GetComponents();

            template<typename T>
            const std::unordered_map<Entity, T>* GetComponents() const;

            void RemoveEntity(Entity);

        private:
            template<typename T>
            void DetachComponent(Entity);

        private:
            std::array<bool, entities_capacity> _entities;
            uint32_t _entityCount{0};
            std::vector<Entity> _freeEntities;
            std::array<Bits<BitsStorageType, components_capacity>, entities_capacity> _entitiesComponentsSlots;

            ComponentManager<components_capacity, BitsStorageType> _componentManager;
    };
}

#include "Impl/EntityManager_impl.tpp"

#endif