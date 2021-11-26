#include <iostream>
#include "logger.h"
#include <entt.hpp>


struct position {
    float x;
    float y;
};

struct number {
    int num;
};

int main(int argc, char* argv[]) 
{
    Logger::Init();

    entt::registry registry;

    const auto entity = registry.create();
    const auto entity2 = registry.create();

    registry.emplace<position>(entity, 10.0f, 20.0f);
    registry.emplace<position>(entity2, 15.0f, 25.0f);
    registry.emplace<number>(entity2, 15);

    auto view = registry.view<const position, number>();
    ENGINE_TRACE(registry.get<number>(entity2).num);
    registry.get<number>(entity2).num = 16;
    ENGINE_TRACE(registry.get<number>(entity2).num);
    auto &pos = registry.get<position>(entity);
    ENGINE_TRACE(pos.x);

    ENGINE_CORE_INFO("Hello world!");
    return 0;
}