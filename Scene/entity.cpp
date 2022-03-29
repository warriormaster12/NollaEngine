#include "entity.h"


Entity::Entity(entt::entity handle, entt::registry* p_registry)
    : entity_handle(handle), p_registry(p_registry) 
{
}