#ifndef MYECS_COMPONENTMANAGER_H
#define MYECS_COMPONENTMANAGER_H

#include <unordered_map>
#include <vector>
#include <memory>
#include "Bits.h"

namespace MyECS
{
    using Entity = uint32_t;


    class BaseComponentsStorage
    {
        protected:
            BaseComponentsStorage() = default;

        public:
            virtual void DeleteComponentInstance(Entity) = 0;
            virtual ~BaseComponentsStorage() = default;
    };

    template<typename T>
    class ComponentsStorage : BaseComponentsStorage
    {
        template<size_t, typename mt> requires std::is_unsigned_v<mt> friend class ComponentManager;

        public:
            ComponentsStorage() = default;
            void DeleteComponentInstance(Entity entity) override { _componentInstances.erase(entity); }

            ~ComponentsStorage() override = default;

        private:
            std::unordered_map<Entity, T> _componentInstances;
    };


    template<size_t components_capacity, typename BitsStorageType>
    requires std::is_unsigned_v<BitsStorageType>
    class ComponentManager
    {
        template<size_t, size_t, typename mt> requires std::is_unsigned_v<mt> friend class EntityManager;

        private:
            ComponentManager() { _components.fill(nullptr); }

            template<typename ...Args>
            Bits<BitsStorageType, components_capacity> AddComponents(Entity, Args&&... components);

            template<typename T>
            std::size_t AddComponent(Entity, T&& component);

            template<typename T>
            void DeleteSpecificComponent(Entity);

            void DeleteComponents(Entity, std::vector<uint32_t>&& componentsIndices);

            template<typename T>
            std::unordered_map<Entity, T>* GetComponents();

            template<typename ...Args>
            std::tuple<Args*...> GetEntityComponents(Entity entity, const Bits<BitsStorageType, components_capacity>& componentsSlots);

            template<typename T>
            T* GetEntityComponent(Entity entity, bool componentSlotState);

            ~ComponentManager();

        private:
            std::array<BaseComponentsStorage*, components_capacity> _components;
            Bits<BitsStorageType, components_capacity> _activeComponentsMask;
    };
}

#include "Impl/ComponentManager_impl.tpp"

#endif