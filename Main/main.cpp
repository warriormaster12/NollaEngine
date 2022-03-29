#include <iostream>
#include "glm/fwd.hpp"
#include "logger.h"
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <vector>

#include "scene.h"

#include "Components/transform.h"

#include "window.h"
#include "renderer.h"

struct TestBufferData {
    glm::vec4 color;
};

struct Camera {
    glm::vec4 data;
    glm::mat4 render_matrix;
};

struct Vertex {

	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 color;

};
int frame_number = 0;
Scene scene;
int main(int argc, char* argv[]) 
{
    Logger::Init();
    Window::CreateNewWindow();
    Renderer::Init();
    Renderer::CreateShaderProgram("triangle",{"Shaders/triangle_vert.spv", "Shaders/triangle_frag.spv"}, sizeof(Vertex), {offsetof(Vertex, position),offsetof(Vertex, normal), offsetof(Vertex, color)});
    Renderer::CreateDescriptorBuffer("triangle",0,sizeof(Camera), UNIFORM);
    Renderer::CreateDescriptorBuffer("triangle",1,sizeof(TestBufferData), UNIFORM);


    scene.CreateEntity("test");
    scene.GetEntity("test").AddComponent<TransformComponent>();
    

    for(auto& entity : scene.entity_list) {
        ENGINE_CORE_INFO(entity.name);
    }

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices = {
        0, 1, 2, 2, 3, 0
    };

    vertices.resize(4);

	//vertex positions
	vertices[0].position = { -1.0f, -1.0f, 0.0f };
	vertices[1].position = {1.0f, -1.0f, 0.0f };
	vertices[2].position = { 1.0f,1.0f, 0.0f };
    vertices[3].position = { -1.0f,1.0f, 0.0f };


	//vertex colors, rgb
	vertices[0].color = { 1.f, 0.f, 0.0f }; //pure red
	vertices[1].color = { 0.f, 1.f, 0.0f }; //pure green
	vertices[2].color = { 0.f, 0.f, 1.0f }; //pure blue
    vertices[3].color = { 1.f, 1.f, 1.0f }; //pure white

    Renderer::CreateBuffer("vertex", vertices.data(), sizeof(Vertex),vertices.size(), VERTEX);
    Renderer::CreateBuffer("index", indices.data(), sizeof(uint32_t),indices.size(), INDEX);
    Renderer::BuildDescriptors("triangle");
    while (Window::GetWindowStatus()) {
        Renderer::InsertDrawCalls([=](){
            Renderer::BeginNewRenderLayer({0.0f, 1.0f, 0.0f, 1.0f}, 1.0f);
            struct Pushdata
            {
                glm::vec4 color;
            }data;

            glm::vec3 camPos = { 0.f,0.f,-2.f };

            glm::mat4 view = glm::translate(glm::mat4(1.f), camPos);
            //camera projection
            glm::mat4 projection = glm::perspective(glm::radians(70.f), 1700.f / 900.f, 0.1f, 200.0f);
            projection[1][1] *= -1;

            scene.GetEntity("test").GetComponent<TransformComponent>().translation = glm::vec3(1.0f, 1.0f, 0.0f);
            scene.GetEntity("test").GetComponent<TransformComponent>().scale = glm::vec3 (1.0, 1.25f, 1.0f);
            glm::mat4 model = scene.GetEntity("test").GetComponent<TransformComponent>().get_transform();

            //calculate final mesh matrix
            glm::mat4 mesh_matrix = projection * view * model;

            Camera camera;
            camera.render_matrix = mesh_matrix;

            data.color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
            TestBufferData test_data;
            test_data.color = glm::vec4(1.0f, 0.5f ,0.25f,1.0f);

            Renderer::UpdateBuffer("triangle", 0,0, &camera, sizeof(Camera));
            Renderer::UpdateBuffer("triangle", 1,0, &test_data, sizeof(TestBufferData));
            Renderer::BindShaderProgram("triangle");
            Renderer::BindPushConstants(&data);
            Renderer::BindDescriptorSets();
            Renderer::BindVertexBuffers();
            Renderer::BindIndexBuffers();
            Renderer::DrawIndexed(indices.size(), 1);
            Renderer::EndRenderLayer();
            frame_number ++;
        });    
    }
    Renderer::DestroyDescriptorBuffer("triangle", 0,0);
    Renderer::DestroyDescriptorBuffer("triangle", 1,0);
    Renderer::DestroyBuffer("index");
    Renderer::DestroyBuffer("vertex");
    Renderer::DestroyShaderProgram("triangle");
    Renderer::Destroy();
    Window::DestroyWindow();
    return 0;
}