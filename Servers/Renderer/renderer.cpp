#include "renderer.h"
#include "vk_context.h"


void Renderer::Init() {
    VkContext::InitContext();
}

void Renderer::CreateShaderProgram(const std::string& shader_name,std::vector<std::string> filepaths) {
    VkContext::CreatePipeline(shader_name,filepaths);
}

void Renderer::CreateBuffer(const std::string& shader_name, size_t alloc_size, BufferTypes usage) {
    VkContext::CreateBuffer(shader_name,alloc_size, usage);
    
}
void Renderer::BuildDescriptors(const std::string& shader_name) {
    VkContext::BuildDescriptors(shader_name);
}

void Renderer::InsertDrawCalls(std::function<void()>&& drawCalls) {
    VkContext::PrepareFrame();
    drawCalls();
    VkContext::SubmitFrame();
}


void Renderer::BeginNewRenderLayer(std::array<float, 4> color, float depth) {
    VkContext::BeginNewRenderLayer(color, depth);
}


void Renderer::UpdateBuffer(const std::string& pipeline_name, int index, void* data, size_t data_size) {
    VkContext::UpdateBuffer(pipeline_name, index, data, data_size);
}

void Renderer::BindShaderProgram(const std::string& shader_name) {
    VkContext::BindPipeline(shader_name);
}

void Renderer::BindDescriptorSets() {
    VkContext::BindDescriptorSets();
}

void Renderer::BindPushConstants(const void* p_values) {
    VkContext::BindPushConstants(p_values);
}

void Renderer::Draw() {
    VkContext::Draw();
}

void Renderer::EndRenderLayer() {
    VkContext::EndRenderLayer();
}

void Renderer::DestroyBuffer(const std::string& pipeline_name, int index) {
    VkContext::DestroyBuffer(pipeline_name, index);
}

void Renderer::Destroy(){
    VkContext::DestroyContext();
}