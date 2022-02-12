#pragma once 

#include "vk_tools.h"

#include <memory>
#include <vector>

class DescriptorAllocator {
public:
    struct PoolSizes {
        std::vector<std::pair<VkDescriptorType,float>> sizes =
        {
            { VK_DESCRIPTOR_TYPE_SAMPLER, 0.5f },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4.f },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 4.f },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1.f },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1.f },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1.f },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2.f },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2.f },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1.f },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1.f },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 0.5f }
        };
    };

    void ResetPools();
    bool Allocate(VkDescriptorSet& set, VkDescriptorSetLayout layout);

    void CleanUp();
private:
    VkDescriptorPool GrabPool();

    VkDescriptorPool current_pool{VK_NULL_HANDLE};
    PoolSizes descriptor_sizes;
    std::vector<VkDescriptorPool> used_pools;
    std::vector<VkDescriptorPool> free_pools;

};

class DescriptorLayoutCache {
public:
    void CleanUp();

    VkDescriptorSetLayout CreateDescriptorLayout(VkDescriptorSetLayoutCreateInfo& info);

    struct DescriptorLayoutInfo {
        //good idea to turn this into a inlined array
        std::vector<VkDescriptorSetLayoutBinding> bindings;

        bool operator==(const DescriptorLayoutInfo& other) const;

        size_t Hash() const;
    };



private:

    struct DescriptorLayoutHash		{

        std::size_t operator()(const DescriptorLayoutInfo& k) const{
            return k.Hash();
        }
    };

    std::unordered_map<DescriptorLayoutInfo, VkDescriptorSetLayout, DescriptorLayoutHash> layout_cache;
};


class DescriptorBuilder {
public:
	static DescriptorBuilder Begin(DescriptorLayoutCache& layoutCache, DescriptorAllocator& allocator);

	DescriptorBuilder& BindBuffer(uint32_t binding, VkDescriptorBufferInfo& buffer_info, VkDescriptorType type, VkShaderStageFlags stage_flags);
	DescriptorBuilder& BindImage(uint32_t binding, VkDescriptorImageInfo& image_info, VkDescriptorType type, VkShaderStageFlags stage_flags);

	bool Build(VkDescriptorSet& set, VkDescriptorSetLayout& layout);
	bool Build(VkDescriptorSet& set);

private:

	std::vector<VkWriteDescriptorSet> writes;
	std::vector<VkDescriptorSetLayoutBinding> bindings;

	std::unique_ptr<DescriptorLayoutCache> cache;
	std::unique_ptr<DescriptorAllocator> alloc;
};


