#include "scene.h"
#include "entity.h"



Entity Scene::CreateEntity(const std::string& name) {
    Entity entity = Entity(registry.create(), &registry);
    entity.name = name;
    entity_list.push_back(entity);
    return entity;
}

Entity& Scene::GetEntity(const std::string &name) {
    Entity* e = nullptr;
    for(auto& entity: entity_list){
        if(entity.name == name) {
            e = &entity;
        }
    }
    return *e;
}