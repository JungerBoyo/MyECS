#ifndef MYECS_ENTITYMANAGER_H
#define MYECS_ENTITYMANAGER_H

#include <Inc/ComponentStorage.h>
#include <Inc/System.h>

namespace MyECS
{
    template<size_t entities_capacity, size_t components_capacity, typename BitsStorageType>
    requires std::is_unsigned_v<BitsStorageType>
    class EntityManager
    {
        template<typename T> auto
        StorageCaster()
        {
            return static_cast<ComponentsStorage<components_capacity, BitsStorageType, T>*>
            (_componentStorages[ID::get<T>()].get());
        }

        public:
            EntityManager()
            {
                _freeEntities.reserve(entities_capacity);
                _activeEntities.reserve(entities_capacity);
            }

            EntityManager(const EntityManager&) = delete;
            EntityManager& operator=(const EntityManager&) = delete;

            template<typename DerivedSystemType, typename ...ManagedTypes>
            requires std::is_base_of_v<System<components_capacity, BitsStorageType>, DerivedSystemType>
            DerivedSystemType* CreateSystem();

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
            std::optional<std::tuple<Args*...>> GetEntityComponents(Entity);

            template<typename ...Args>
            std::optional<std::tuple<const Args*...>> GetEntityComponents(Entity) const;

            template<typename T>
            std::optional<std::vector<T>*> GetComponents();

            template<typename T>
            std::optional<const std::vector<T>*> GetComponents() const;

            void RemoveEntity(Entity);

        private:
            template<typename T>
            std::size_t AddComponent(Entity, T&& component);

            template<typename T>
            void DetachComponent(Entity);

            template<typename ...Args>
            std::vector<Entity> GetEntitiesWithComponents();

        private:
            std::array<Bits<BitsStorageType, components_capacity>, entities_capacity> _entitiesComponentsSlots;
            Bits<BitsStorageType, entities_capacity> _entitiesStates;
            std::unordered_map<Entity, Entity> _activeEntities;
            std::vector<Entity> _freeEntities;

            std::array<std::unique_ptr<BaseComponentsStorage<components_capacity, BitsStorageType>>, components_capacity> _componentStorages;
            std::size_t _componentsCount{0};
            Bits<BitsStorageType, components_capacity> _activeComponentsMask;

            std::vector<std::unique_ptr<System<components_capacity, BitsStorageType>>> _systems;

    };
}

#include "Impl/EntityManager_impl.tpp"

#endif