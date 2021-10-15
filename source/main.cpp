//#define DEBUG_MyECS

#include <Inc/EntityManager.h>
#include <gtest/gtest.h>
#include <Inc/System.h>

#include <fmt/core.h>

#define ENTITY_COUNT 65536
#define COMPONENTS_COUNT 16
using BitsStorageType = uint8_t;

struct CustomComponent1
{
    CustomComponent1()
            : transposed(false)
    {
        mat4.fill(0.0f);
    }

    std::array<float, 16> mat4;
    bool transposed;
};

struct CustomComponent2
{
    CustomComponent2()
            : str("str")
    {
        for(const auto ch : str)
            map.insert(std::make_pair(ch, CustomComponent1{}));
    }

    std::string str;
    std::unordered_map<char, CustomComponent1> map;
};

struct CustomComponent3
{
    CustomComponent3() : str("str") { arr.fill(0); }

    std::string str;
    std::array<int32_t, COMPONENTS_COUNT> arr;
};

struct CustomComponent4
{
    CustomComponent4() : vec(20) {  }
    std::vector<int> vec;
};

struct CustomComponent5NotThreadSafe
{
    CustomComponent5NotThreadSafe() : vec(32) {  }
    std::vector<int> vec;
};

struct EntityManagerTest : public testing::Test
{
    MyECS::EntityManager<ENTITY_COUNT, COMPONENTS_COUNT, BitsStorageType> man;
    std::vector<MyECS::Entity> entities;

    void SetUp() override
    {
        entities.reserve(ENTITY_COUNT);

        for(uint32_t i{0}; i<ENTITY_COUNT/4; ++i)
            entities.push_back(man.CreateEntity<true, CustomComponent2, CustomComponent3>({}, {}));

        for(uint32_t i{0}; i<ENTITY_COUNT/4; ++i)
            entities.push_back(man.CreateEntity<true, CustomComponent1, CustomComponent3>({}, {}));

        for(uint32_t i{0}; i<ENTITY_COUNT/4; ++i)
            entities.push_back(man.CreateEntity<true, std::string, float, int, double, CustomComponent3>("", 0.0f, 1, 0.0, {}));

        fmt::print("[Google Test] Test set up\n");
    }
    void TearDown() override { fmt::print("[Google Test] Test tore down\n"); }
};


TEST(EntityCreationTest, CreateEntitiesTest)
{
    MyECS::EntityManager<ENTITY_COUNT, COMPONENTS_COUNT, BitsStorageType> man;
    std::vector<MyECS::Entity> entities; entities.reserve(ENTITY_COUNT);

    for(uint32_t i{0}; i<ENTITY_COUNT/4; ++i)
    {
        entities.push_back(man.CreateEntity<true, CustomComponent2, CustomComponent3>({}, {}));
        ASSERT_EQ((man.HasComponents<CustomComponent2, CustomComponent3>(entities.back())), true);
    }

    const auto& test = man.GetComponents<CustomComponent2>();

    for(uint32_t i{0}; i<ENTITY_COUNT/4; ++i)
    {
        entities.push_back(man.CreateEntity<true, CustomComponent1, CustomComponent3>({}, {}));
        ASSERT_EQ((man.HasComponents<CustomComponent1, CustomComponent3>(entities.back())), true);
    }

    for(uint32_t i{0}; i<ENTITY_COUNT/4; ++i)
    {
        entities.push_back(man.CreateEntity<true, std::string, float, int, double, CustomComponent3>("", 0.0f, 1, 0.0, {}));
        ASSERT_EQ((man.HasComponents<std::string, float, int, double, CustomComponent3>(entities.back())), true);
    }

    for(uint32_t i{0}; i<ENTITY_COUNT/4; ++i)
    {
        entities.push_back(man.CreateEntity<false, CustomComponent5NotThreadSafe>({}));
        ASSERT_EQ((man.HasComponents<CustomComponent5NotThreadSafe>(entities.back())), true);
    }
}

TEST_F(EntityManagerTest, HasComponentTest)
{
    for(const auto entity : entities)
        ASSERT_EQ((man.HasComponent<CustomComponent3>(entity)) || (man.HasComponent<CustomComponent5NotThreadSafe>(entity)), true);
}

TEST_F(EntityManagerTest, GetEntityComponentsTest)
{
    for(const auto entity : entities)
    {
        if(man.HasComponents<CustomComponent2, CustomComponent3>(entity))
        {
           // ASSERT_EQ((man.GetEntityComponents<CustomComponent2, CustomComponent3>(entity).has_value()), true);
        }

        if(man.HasComponents<CustomComponent1, CustomComponent3>(entity))
        {
          //  ASSERT_EQ((man.GetEntityComponents<CustomComponent1, CustomComponent3>(entity).has_value()), true);
        }

        if(man.HasComponents<std::string, float, int, double, CustomComponent3>(entity))
        {
         //   ASSERT_EQ((man.GetEntityComponents<std::string, float, int, double, CustomComponent3>(entity).has_value()), true);
        }
    }
}

TEST_F(EntityManagerTest, DetachEntityComponentsTest)
{
    for(MyECS::Entity entity{0}; entity<ENTITY_COUNT/4; ++entity)
    {
        if(man.HasComponents<CustomComponent2, CustomComponent3>(entity))
        {
            man.DetachComponents<CustomComponent2, CustomComponent3>(entity);
            ASSERT_EQ((man.HasComponents<CustomComponent2, CustomComponent3>(entity)), false);
        }


        if(man.HasComponents<CustomComponent1, CustomComponent3>(entity))
        {
            man.DetachComponents<CustomComponent1, CustomComponent3>(entity);
            ASSERT_EQ((man.HasComponents<CustomComponent1, CustomComponent3>(entity)), false);
        }

        if(man.HasComponents<std::string, float, int, double, CustomComponent3>(entity))
        {
            man.DetachComponents<std::string, float, int, double, CustomComponent3>(entity);
            ASSERT_EQ((man.HasComponents<std::string, float, int, double, CustomComponent3>(entity)), false);
        }
    }
}


TEST_F(EntityManagerTest, RemoveEntityTest)
{
    for(const auto entity : entities)
    {
        man.RemoveEntity(entity);
        ASSERT_EQ((man.HasComponent<CustomComponent3>(entity)), false);
    }
}


TEST_F(EntityManagerTest, AddEntityComponents)
{
    for(const auto entity : entities)
    {
        man.AddComponents<true, CustomComponent4>(entity, {});
        ASSERT_EQ((man.HasComponent<CustomComponent4>(entity)), true);

        man.AddComponents<true, std::array<int,4>, std::array<float, 23>,
                         std::vector<std::string>>(entity, {0,0,0,0}, {}, std::vector<std::string>());

        ASSERT_EQ
        ((man.HasComponents< std::array<int,4>, std::array<float, 23>, std::vector<std::string> >(entity)), true);

        ASSERT_EQ((man.HasComponent<CustomComponent3>(entity)) || (man.HasComponent<CustomComponent5NotThreadSafe>(entity)), true);
    }
}

TEST_F(EntityManagerTest, GetComponentsTest)
{
    /*
    ASSERT_EQ(man.GetComponents<CustomComponent1>().has_value(), true);
    ASSERT_EQ(man.GetComponents<CustomComponent2>().has_value(), true);
    ASSERT_EQ(man.GetComponents<CustomComponent3>().has_value(), true);
    ASSERT_EQ(man.GetComponents<std::string>().has_value(), true);
    ASSERT_EQ(man.GetComponents<float>().has_value(), true);
    ASSERT_EQ(man.GetComponents<int>().has_value(), true);
    ASSERT_EQ(man.GetComponents<double>().has_value(), true);*/
}


class derivedSystem : public MyECS::System<64, uint64_t>
{
public:
    derivedSystem()
        : MyECS::System<64, uint64_t>(MyECS::SystemComponents<std::string>{})
    {

    }
};

int main()
{
   testing::InitGoogleTest();



   return RUN_ALL_TESTS();
}