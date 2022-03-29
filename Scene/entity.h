#pragma once 

#include <iostream>
#include <entt.hpp>
#include "logger.h"

class Entity {
public:
    Entity() = default;
    Entity(entt::entity handle, entt::registry* p_registry);
    Entity(const Entity& other) = default;

    template<typename T, typename... Args>
    T& AddComponent(Args&&... args) {
        if (HasComponent<T>()) {
            
        }

        T& component = p_registry->emplace<T>(entity_handle, std::forward<Args>(args)...);
        return component;

    }

    template<typename T>
    T& GetComponent()
    {
        if (!HasComponent<T>()) {
            ENGINE_CORE_WARN("Entity does not have component!");
        }
        else {
            return p_registry->get<T>(entity_handle);
        }
    }

    template<typename T>
    bool HasComponent()
    {
        return p_registry->all_of<T>(entity_handle);
    }

    template<typename T>
	void RemoveComponent() {
        if (!HasComponent<T>()) {
            ENGINE_CORE_WARN("Entity does not have component!");
        }
        else {
            p_registry->remove<T>(entity_handle);
        }
    }
    
    std::string name;
private:
    entt::entity entity_handle;
    entt::registry* p_registry;
};
