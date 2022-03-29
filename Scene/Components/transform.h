#pragma once 

#include <glm/glm.hpp>
#include <iostream>

struct TransformComponent {
    glm::vec3 translation {};
    glm::vec3 rotation {};
    glm::vec3 scale {1.0f};

    // Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
    // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
    // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
    glm::mat4 get_transform();
};
