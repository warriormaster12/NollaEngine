#pragma once 

#include <iostream>
#include <vector>
#include <string>

#include "entity.h"

class Scene {
public: 
    std::vector<Entity> entity_list;

    Entity CreateEntity(const std::string& name);
    Entity& GetEntity(const std::string& name);
private: 
    entt::registry registry;

};