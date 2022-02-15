#include <iostream>
#include "logger.h"
#include <entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <vector>

#include "window.h"
#include "renderer.h"

struct TestBufferData {
    glm::vec4 color;
};

struct Camera {
    glm::mat4 render_matrix;
};

struct Vertex {

	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 color;

};

int main(int argc, char* argv[]) 
{
    Logger::Init();
    Window::CreateNewWindow();
    Renderer::Init();
    Renderer::CreateShaderProgram("triangle",{"Shaders/triangle_vert.spv", "Shaders/triangle_frag.spv"}, sizeof(Vertex), {offsetof(Vertex, position),offsetof(Vertex, normal), offsetof(Vertex, color)});
    Renderer::CreateDescriptorBuffer("triangle",0,sizeof(Camera), UNIFORM);
    Renderer::CreateDescriptorBuffer("triangle",1,sizeof(TestBufferData), UNIFORM);

    std::vector<Vertex> vertices;
    vertices.resize(3);

	//vertex positions
	vertices[0].position = { 1.f, 1.f, 0.0f };
	vertices[1].position = {-1.f, 1.f, 0.0f };
	vertices[2].position = { 0.f,-1.f, 0.0f };

	//vertex colors, rgb
	vertices[0].color = { 1.f, 0.f, 0.0f }; //pure red
	vertices[1].color = { 0.f, 1.f, 0.0f }; //pure green
	vertices[2].color = { 0.f, 0.f, 1.0f }; //pure blue

    Renderer::CreateBuffer("vertex", vertices.data(), sizeof(Vertex),vertices.size(), VERTEX);
    Renderer::BuildDescriptors("triangle");
    while (Window::GetWindowStatus()) {
        Renderer::InsertDrawCalls([](){
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
            //model rotation
            glm::mat4 model = glm::rotate(glm::mat4{ 1.0f }, glm::radians(0.0f), glm::vec3(0, 1, 0));

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
            Renderer::Draw();
            Renderer::EndRenderLayer();
        });    
    }
    Renderer::DestroyBuffer("triangle", 0,0);
    Renderer::DestroyBuffer("triangle", 1,0);
    Renderer::DestroyShaderProgram("triangle");
    Renderer::Destroy();
    Window::DestroyWindow();
    return 0;
}