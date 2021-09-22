include_directories(Inc)
    add_library(EntityManager INTERFACE Inc/EntityManager.h Impl/EntityManager_impl.tpp)
    add_library(ComponentManager INTERFACE Inc/ComponentManager.h Impl/ComponentManager_impl.tpp)
    add_library(Bits INTERFACE Inc/Bits.h Impl/Bits_impl.tpp)
    add_library(TypeIdGenerator Inc/TypeIdGenerator.h Impl/TypeIdGenerator.cpp)
    add_library(System INTERFACE Inc/System.h Impl/System_impl.tpp)


