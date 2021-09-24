#ifndef MYECS_COMPONENTMANAGER_H
#define MYECS_COMPONENTMANAGER_H

#include <unordered_map>
#include <vector>
#include <memory>
#include <Inc/Entity.h>
#include <Inc/Bits.h>
#include <Inc/TypeIdGenerator.h>

namespace MyECS
{
    template<size_t components_capacity, typename BitsStorageType> requires std::is_unsigned_v<BitsStorageType>
    class BaseComponentsStorage
    {
        protected:

        public:
            virtual void DeleteComponentInstance(Entity) = 0;
            virtual const Bits<BitsStorageType, components_capacity>& GetBits() const = 0;
    };

    template<size_t components_capacity, typename BitsStorageType, typename T> requires std::is_unsigned_v<BitsStorageType>
    class ComponentsStorage : public BaseComponentsStorage<components_capacity, BitsStorageType>
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
                _componentInstances.erase(_componentInstances.begin() + _entityToComponentInstances.at(entity));
                _entityToComponentInstances.erase(entity);
            }

            void AddComponentInstance(Entity entity, T&& instance)
            {
                _entityToComponentInstances[entity] = _componentInstances.size();
                _componentInstances.emplace_back(instance);
            }

            const Bits<BitsStorageType, components_capacity>& GetBits() const override
            {
                return _componentBits;
            }

            T* GetByEntity(Entity entity)
            {
                return &_componentInstances[_entityToComponentInstances.at(entity)];
            }

            const T* GetByEntity(Entity entity) const
            {
                return &_componentInstances[_entityToComponentInstances.at(entity)];
            }

        private:
            Bits<BitsStorageType, components_capacity> _componentBits;
            std::unordered_map<Entity, std::size_t> _entityToComponentInstances;
            std::vector<T> _componentInstances;
    };
}

#endif