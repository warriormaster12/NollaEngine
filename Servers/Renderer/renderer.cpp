#include "renderer.h"
#include "vk_context.h"


void Renderer::Init() {
    VkContext::InitContext();
}

void Renderer::CreateShaderProgram(std::vector<std::string> filepaths) {
    VkContext::CreatePipeline(filepaths);
}

void Renderer::CreateBuffer(size_t alloc_size, BufferTypes usage) {
    VkContext::CreateBuffer(alloc_size, usage);
}

void Renderer::InsertDrawCalls(std::function<void()>&& drawCalls) {
    VkContext::PrepareFrame();
    drawCalls();
    VkContext::SubmitFrame();
}


void Renderer::BeginNewRenderLayer(std::array<float, 4> color, float depth) {
    VkContext::BeginNewRenderLayer(color, depth);
}


void Renderer::UpdateBuffer(void* data, size_t data_size) {
    VkContext::UpdateBuffer(data, data_size);
}

void Renderer::BindShaderProgram() {
    VkContext::BindPipeline();
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

void Renderer::DestroyBuffer() {
    VkContext::DestroyBuffer();
}

void Renderer::Destroy(){
    VkContext::DestroyContext();
}