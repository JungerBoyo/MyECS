add_subdirectory(ECS)

add_executable(${PROJECT_NAME} main.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC ECS)
target_link_directories(${PROJECT_NAME} PUBLIC ECS)

target_link_libraries(${PROJECT_NAME} EntityManager
                                      ComponentManager
                                      TypeIdGenerator
                                      Bits
                                      System)