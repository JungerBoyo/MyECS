#include <Inc/EntityManager.h>
#include <gtest/gtest.h>
#include <Inc/System.h>

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

struct EntityManagerTest : public testing::Test
{
    MyECS::EntityManager<ENTITY_COUNT, COMPONENTS_COUNT, BitsStorageType> man;
    std::vector<MyECS::Entity> entities;

    void SetUp() override
    {
        entities.reserve(ENTITY_COUNT);

        for(uint32_t i{0}; i<ENTITY_COUNT/4; ++i)
            entities.push_back(man.CreateEntity<CustomComponent2, CustomComponent3>({}, {}));

        for(uint32_t i{0}; i<ENTITY_COUNT/4; ++i)
            entities.push_back(man.CreateEntity<CustomComponent1, CustomComponent3>({}, {}));

        for(uint32_t i{0}; i<ENTITY_COUNT/4; ++i)
            entities.push_back(man.CreateEntity<std::string, float, int, double, CustomComponent3>("", 0.0f, 1, 0.0, {}));

        fmt::print("[Google Test] Test set up\n");
    }
    void TearDown() override { fmt::print("[Google Test] Test tore down\n"); }
};


TEST(EntityCreationTest, CreateEntitiesTest)
{
    MyECS::EntityManager<ENTITY_COUNT, COMPONENTS_COUNT, uint16_t> man;
    std::vector<MyECS::Entity> entities(ENTITY_COUNT);

    for(uint32_t i{0}; i<ENTITY_COUNT/4; ++i)
    {
        entities.push_back(man.CreateEntity<CustomComponent2, CustomComponent3>({}, {}));
        ASSERT_EQ((man.HasComponents<CustomComponent2, CustomComponent3>(entities.back())), true);
    }

    for(uint32_t i{0}; i<ENTITY_COUNT/4; ++i)
    {
        entities.push_back(man.CreateEntity<CustomComponent1, CustomComponent3>({}, {}));
        ASSERT_EQ((man.HasComponents<CustomComponent1, CustomComponent3>(entities.back())), true);
    }

    for(uint32_t i{0}; i<ENTITY_COUNT/4; ++i)
    {
        entities.push_back(man.CreateEntity<std::string, float, int, double, CustomComponent3>("", 0.0f, 1, 0.0, {}));
        ASSERT_EQ((man.HasComponents<std::string, float, int, double, CustomComponent3>(entities.back())), true);
    }

    for(uint32_t i{0}; i<ENTITY_COUNT/4; ++i)
    {
        entities.push_back(man.CreateEntity<CustomComponent3>({}));
        ASSERT_EQ((man.HasComponents<CustomComponent3>(entities.back())), true);
    }
}

TEST_F(EntityManagerTest, HasComponentTest)
{
    for(const auto entity : entities)
        ASSERT_EQ((man.HasComponent<CustomComponent3>(entity)), true);
}

TEST_F(EntityManagerTest, GetEntityComponentsTest)
{
    for(const auto entity : entities)
    {
        if(man.HasComponents<CustomComponent2, CustomComponent3>(entity))
        {
            auto[c2, c3] = man.GetEntityComponents<CustomComponent2, CustomComponent3>(entity);

            ASSERT_NE(c2, nullptr);
            ASSERT_NE(c3, nullptr);

            c2 = nullptr;
            c3 = nullptr;
        }

        if(man.HasComponents<CustomComponent1, CustomComponent3>(entity))
        {
            auto[c1, c3] = man.GetEntityComponents<CustomComponent1, CustomComponent3>(entity);

            ASSERT_NE(c1, nullptr);
            ASSERT_NE(c3, nullptr);

            c1 = nullptr;
            c3 = nullptr;
        }

        if(man.HasComponents<std::string, float, int, double, CustomComponent3>(entity))
        {
            const auto[str, f, i, d, c3] = man.GetEntityComponents<std::string, float, int, double, CustomComponent3>(entity);

            ASSERT_NE(str, nullptr);
            ASSERT_NE(f, nullptr);
            ASSERT_NE(i, nullptr);
            ASSERT_NE(d, nullptr);
            ASSERT_NE(c3, nullptr);
        }
    }
}

TEST_F(EntityManagerTest, DetachEntityComponentsTest)
{
    for(const auto entity : entities)
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
        man.AddComponents<CustomComponent4>(entity, {});
        ASSERT_EQ((man.HasComponent<CustomComponent4>(entity)), true);

        man.AddComponents<std::array<int,4>, std::array<float, 23>,
                         std::vector<std::string>>(entity, {0,0,0,0}, {}, std::vector<std::string>());

        ASSERT_EQ
        ((man.HasComponents< std::array<int,4>, std::array<float, 23>, std::vector<std::string> >(entity)), true);

        ASSERT_EQ((man.HasComponent<CustomComponent3>(entity)), true);
    }
}

TEST_F(EntityManagerTest, GetComponentsTest)
{
    const auto c1 = man.GetComponents<CustomComponent1>();
    const auto c2 = man.GetComponents<CustomComponent2>();
    const auto c3 = man.GetComponents<CustomComponent3>();
    const auto str = man.GetComponents<std::string>();
    const auto fl = man.GetComponents<float>();
    const auto i = man.GetComponents<int>();
    const auto db = man.GetComponents<double>();

    ASSERT_NE(c1, nullptr);
    ASSERT_NE(c2, nullptr);
    ASSERT_NE(c3, nullptr);
    ASSERT_NE(str, nullptr);
    ASSERT_NE(fl, nullptr);
    ASSERT_NE(i, nullptr);
    ASSERT_NE(db, nullptr);
}

int main()
{
   testing::InitGoogleTest();

   return RUN_ALL_TESTS();
}