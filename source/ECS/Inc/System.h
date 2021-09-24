#ifndef MYECS_SYSTEM_H
#define MYECS_SYSTEM_H

#include <Inc/Bits.h>
#include <Inc/Entity.h>
#include <unordered_map>

namespace MyECS
{
    template<typename ...Args>
    struct SystemComponents {};

    template<size_t components_capacity, typename BitsStorageType> requires std::is_unsigned_v<BitsStorageType>
    class System
    {
        template<size_t, size_t, typename BitsStorageType_> requires std::is_unsigned_v<BitsStorageType_>
        friend class EntityManager;

        protected:
            template<typename ...ManagedComponentsTypes>
            System(SystemComponents<ManagedComponentsTypes...>&&);

            virtual void OnEntityAdditionAction(Entity) {};
            virtual void OnEntityRemovalAction(Entity) {};

            const std::unordered_map<Entity, Entity>& GetSystemEntities() const { return _managedEntities; }

        private:
            void OnEntityUpdate(Entity, const Bits<BitsStorageType, components_capacity>& entityComponentsBits);
            void OnEntityAdd(Entity, const Bits<BitsStorageType, components_capacity>& entityComponentsBits);
            void OnEntityRemove(Entity);

        private:
            std::unordered_map<Entity, Entity> _managedEntities;
            Bits<BitsStorageType, components_capacity> _managedComponentsBits;
    };
}

#include "Impl/System_impl.tpp"

#endif
