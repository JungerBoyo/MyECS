#ifndef MYECS_ENTITYMANAGER_H
#define MYECS_ENTITYMANAGER_H

#include <Inc/ComponentStorage.h>
#include <Inc/System.h>

namespace MyECS
{
#ifdef DEBUG_MyECS
    template<typename T>
    using ComponentsReturnType = std::optional<std::vector<T>*>;

    template<typename T>
    using ComponentsReturnType_const = std::optional<const std::vector<T>*>;

    template<typename ...Args>
    using EntityComponentsReturnType = std::optional<std::tuple<Args*...>>;

    template<typename ...Args>
    using EntityComponentsReturnType_const = std::optional<std::tuple<const Args*...>>;
#else
    template<typename T>
    using ComponentsReturnType = std::vector<T>&;

    template<typename T>
    using ComponentsReturnType_const = const std::vector<T>&;

    template<typename ...Args>
    using EntityComponentsReturnType = std::tuple<Args&...>;

    template<typename ...Args>
    using EntityComponentsReturnType_const = std::tuple<const Args&...>;
#endif

    template<size_t entities_capacity, size_t components_capacity, typename BitsStorageType>
    requires std::is_unsigned_v<BitsStorageType>
    class EntityManager
    {
        template<typename T, bool ThreadSafeStorage> auto
        StorageCaster()
        {
            return static_cast<ComponentsStorage<components_capacity, BitsStorageType, T, ThreadSafeStorage>*>
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

            template<typename DerivedSystemType, typename ...ManagedTypes, template <typename...> class T,
                    typename ...Args>
            requires std::is_base_of_v<System<components_capacity, BitsStorageType>, DerivedSystemType>
            DerivedSystemType* CreateSystem(T<ManagedTypes...>, Args&&...);

            template<bool ThreadSafeComponents, typename ...Args>
            Entity CreateEntity(Args&&... components);

            template<bool ThreadSafeComponents, typename ...Args>
            void AddComponents(Entity, Args&&... components);

            template<typename ...Args>
            void DetachComponents(Entity);

            template<typename T>
            bool HasComponent(Entity) const;

            template<typename ...Args>
            bool HasComponents(Entity) const;

            template<typename ...Args>
            EntityComponentsReturnType<Args...> GetEntityComponents(Entity);

            template<bool ThreadSafeComponents, typename ...Args>
            EntityComponentsReturnType_const<Args...> GetEntityComponents(Entity) const;

            template<typename T>
            ComponentsReturnType<T> GetComponents();

            template<bool ThreadSafeComponents, typename T>
            ComponentsReturnType_const<T> GetComponents() const;

            void RemoveEntity(Entity);

        private:
            template<bool ThreadSafeComponent, typename T>
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