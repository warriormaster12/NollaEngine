#include "renderer.h"
#include "vk_context.h"


void Renderer::Init() {
    VkContext::InitContext();
}

void Renderer::CreateShaderProgram(std::vector<std::string> filepaths) {
    VkContext::CreatePipeline(filepaths);
}

void Renderer::InsertDrawCalls(std::function<void()>&& drawCalls) {
    VkContext::PrepareFrame();
    drawCalls();
    VkContext::SubmitFrame();
}


void Renderer::BeginNewRenderLayer(std::array<float, 4> color, float depth) {
    VkContext::BeginNewRenderLayer(color, depth);
}

void Renderer::BindShaderProgram() {

}

void Renderer::Draw() {

}

void Renderer::EndRenderLayer() {
    VkContext::EndRenderLayer();
}

void Renderer::Destroy(){
    VkContext::DestroyContext();
}