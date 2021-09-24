include_directories(Inc)
    add_library(EntityManager INTERFACE Inc/EntityManager.h Impl/EntityManager_impl.tpp)
    add_library(ComponentStorage INTERFACE Inc/ComponentStorage.h)
    add_library(System INTERFACE Inc/System.h Impl/System_impl.tpp)
    add_library(Bits INTERFACE Inc/Bits.h Impl/Bits_impl.tpp)
    add_library(TypeIdGenerator Inc/TypeIdGenerator.h Impl/TypeIdGenerator.cpp)
    add_library(ECS_errorlog INTERFACE Inc/ECS_errorlog.h)
    add_library(Entity INTERFACE Inc/Entity.h)


