#include "renderer.h"
#include "vk_context.h"


void Renderer::Init() {
    VkContext::InitContext();
}

void Renderer::CreateShaderProgram(const std::string& shader_name,std::vector<std::string> filepaths, uint32_t stride, std::vector<uint32_t> offsets) {
    VkContext::CreatePipeline(shader_name,filepaths, stride, offsets);
}

void Renderer::CreateDescriptorBuffer(const std::string& shader_name, int set_index,size_t alloc_size, BufferTypes usage) {
    VkContext::CreateDescriptorBuffer(shader_name,set_index,alloc_size, usage);
}

void Renderer::CreateBuffer(const std::string& buffer_name, void* data, uint32_t stride, size_t alloc_size, BufferTypes usage) {
    VkContext::CreateBuffer(buffer_name, data, stride,alloc_size, usage);   
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


void Renderer::UpdateBuffer(const std::string& pipeline_name,int set_index, int binding, void* data, size_t data_size) {
    VkContext::UpdateBuffer(pipeline_name, set_index,binding, data, data_size);
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

void Renderer::DestroyShaderProgram(const std::string& shader_name) {
    VkContext::DestroyPipeline(shader_name);
}

void Renderer::DestroyBuffer(const std::string& shader_name,int set_index, int index) {
    VkContext::DestroyBuffer(shader_name, set_index,index);
}

void Renderer::Destroy(){
    VkContext::DestroyContext();
}