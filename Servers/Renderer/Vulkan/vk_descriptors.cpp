#include "vk_descriptors.h"
#include "vk_device.h"
#include "logger.h"
#include <memory>



void DescriptorAllocator::CleanUp() { 
    for (auto& pool : free_pools){
        vkDestroyDescriptorPool(DeviceManager::GetVkDevice().device, pool, nullptr);
    }
    for (auto& pool : used_pools){
        vkDestroyDescriptorPool(DeviceManager::GetVkDevice().device, pool, nullptr);
    }
}


VkDescriptorPool CreatePool(const DescriptorAllocator::PoolSizes& pool_sizes, int count, VkDescriptorPoolCreateFlags flags) {
    std::vector<VkDescriptorPoolSize> sizes;
    sizes.reserve(pool_sizes.sizes.size());

    for (auto& sz : pool_sizes.sizes){
        sizes.push_back({sz.first, uint32_t(sz.second * count)});
    }

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = flags;
    pool_info.maxSets = count;
    pool_info.poolSizeCount = (uint32_t)sizes.size();
    pool_info.pPoolSizes = sizes.data();

    VkDescriptorPool descriptorPool;
    vkCreateDescriptorPool(DeviceManager::GetVkDevice().device, &pool_info, nullptr, &descriptorPool);

    return descriptorPool;
}

VkDescriptorPool DescriptorAllocator::GrabPool() {

    //if there are free pools available
    if(free_pools.size() > 0) {
        //grab pool from the back of the vector and remove it from there.
		VkDescriptorPool pool = free_pools.back();
		free_pools.pop_back();
		return pool;
    }
    else
	{
		//no pools availible, so create a new one
		return CreatePool(descriptor_sizes, 1000, 0);
	}
}

bool DescriptorAllocator::Allocate(VkDescriptorSet& set, VkDescriptorSetLayout layout)
{
    //initialize the currentPool handle if it's null
    if (current_pool == VK_NULL_HANDLE){

        current_pool = GrabPool();
        used_pools.push_back(current_pool);
    }

    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;

    allocInfo.pSetLayouts = &layout;
    allocInfo.descriptorPool = current_pool;
    allocInfo.descriptorSetCount = 1;

    //try to allocate the descriptor set
    VkResult allocResult = vkAllocateDescriptorSets(DeviceManager::GetVkDevice().device, &allocInfo, &set);
    bool needReallocate = false;

    switch (allocResult) {
    case VK_SUCCESS:
        //all good, return
        return true;
    case VK_ERROR_FRAGMENTED_POOL:
    case VK_ERROR_OUT_OF_POOL_MEMORY:
        //reallocate pool
        needReallocate = true;
        break;
    default:
        //unrecoverable error
        return false;
    }

    if (needReallocate){
        //allocate a new pool and retry
        current_pool = GrabPool();
        used_pools.push_back(current_pool);

        allocResult = vkAllocateDescriptorSets(DeviceManager::GetVkDevice().device, &allocInfo, &set);

        //if it still fails then we have big issues
        if (allocResult == VK_SUCCESS){
            ENGINE_CORE_FATAL("failed to allocate descriptor sets");
            return true;
        }
    }

    return false;
}

void DescriptorAllocator::ResetPools(){
	//reset all used pools and add them to the free pools
	for (auto p : used_pools){
		vkResetDescriptorPool(DeviceManager::GetVkDevice().device, p, 0);
		free_pools.push_back(p);
	}

	//clear the used pools, since we've put them all in the free pools
	used_pools.clear();

	//reset the current pool handle back to null
	current_pool = VK_NULL_HANDLE;
}



VkDescriptorSetLayout DescriptorLayoutCache::CreateDescriptorLayout(VkDescriptorSetLayoutCreateInfo& info){
	DescriptorLayoutInfo layout_info;
	layout_info.bindings.reserve(info.bindingCount);
	bool isSorted = true;
	int lastBinding = -1;

	//copy from the direct info struct into our own one
	for (int i = 0; i < info.bindingCount; i++) {
		layout_info.bindings.push_back(info.pBindings[i]);

		//check that the bindings are in strict increasing order
		if (info.pBindings[i].binding > lastBinding){
			lastBinding = info.pBindings[i].binding;
		}
		else{
			isSorted = false;
		}
	}
	//sort the bindings if they aren't in order
	if (!isSorted){
		std::sort(layout_info.bindings.begin(), layout_info.bindings.end(), [](VkDescriptorSetLayoutBinding& a, VkDescriptorSetLayoutBinding& b ){
				return a.binding < b.binding;
		});
	}

	//try to grab from cache
	auto it = layout_cache.find(layout_info);
	if (it != layout_cache.end()){
		return (*it).second;
	}
	else {
        //create a new one (not found)
        VkDescriptorSetLayout layout;
        vkCreateDescriptorSetLayout(DeviceManager::GetVkDevice().device, &info, nullptr, &layout);

        //add to cache
        layout_cache[layout_info] = layout;
        return layout;
    }
}

bool DescriptorLayoutCache::DescriptorLayoutInfo::operator==(const DescriptorLayoutInfo& other) const{
	if (other.bindings.size() != bindings.size()){
		return false;
	}
	else {
		//compare each of the bindings is the same. Bindings are sorted so they will match
		for (int i = 0; i < bindings.size(); i++) {
			if (other.bindings[i].binding != bindings[i].binding){
				return false;
			}
			if (other.bindings[i].descriptorType != bindings[i].descriptorType){
				return false;
			}
			if (other.bindings[i].descriptorCount != bindings[i].descriptorCount){
				return false;
			}
			if (other.bindings[i].stageFlags != bindings[i].stageFlags){
				return false;
			}
		}
		return true;
	}
}

size_t DescriptorLayoutCache::DescriptorLayoutInfo::Hash() const{
    using std::size_t;
    using std::hash;

    size_t result = hash<size_t>()(bindings.size());

    for (const VkDescriptorSetLayoutBinding& b : bindings)
    {
        //pack the binding data into a single int64. Not fully correct but it's ok
        size_t binding_hash = b.binding | b.descriptorType << 8 | b.descriptorCount << 16 | b.stageFlags << 24;

        //shuffle the packed binding data and xor it with the main hash
        result ^= hash<size_t>()(binding_hash);
    }

    return result;
}

DescriptorBuilder DescriptorBuilder::Begin(DescriptorLayoutCache &layoutCache, DescriptorAllocator &allocator) {
    DescriptorBuilder builder;

    builder.cache = std::make_unique<DescriptorLayoutCache>(layoutCache);
    builder.alloc = std::make_unique<DescriptorAllocator>(allocator);
    return builder;
}


DescriptorBuilder& DescriptorBuilder::BindBuffer(uint32_t binding, VkDescriptorBufferInfo& buffer_info, VkDescriptorType type, VkShaderStageFlags stage_flags)
{
    VkDescriptorSetLayoutBinding newBinding{};

    newBinding.descriptorCount = 1;
    newBinding.descriptorType = type;
    newBinding.pImmutableSamplers = nullptr;
    newBinding.stageFlags = stage_flags;
    newBinding.binding = binding;

    bindings.push_back(newBinding);

    VkWriteDescriptorSet newWrite{};
    newWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    newWrite.pNext = nullptr;

    newWrite.descriptorCount = 1;
    newWrite.descriptorType = type;
    newWrite.pBufferInfo = &buffer_info;
    newWrite.dstBinding = binding;

    writes.push_back(newWrite);
    return *this;
}


DescriptorBuilder& DescriptorBuilder::BindImage(uint32_t binding, VkDescriptorImageInfo& image_info, VkDescriptorType type, VkShaderStageFlags stage_flags)
{
    VkDescriptorSetLayoutBinding newBinding{};

    newBinding.descriptorCount = 1;
    newBinding.descriptorType = type;
    newBinding.pImmutableSamplers = nullptr;
    newBinding.stageFlags = stage_flags;
    newBinding.binding = binding;

    bindings.push_back(newBinding);

    VkWriteDescriptorSet newWrite{};
    newWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    newWrite.pNext = nullptr;

    newWrite.descriptorCount = 1;
    newWrite.descriptorType = type;
    newWrite.pImageInfo = &image_info;
    newWrite.dstBinding = binding;

    writes.push_back(newWrite);
    return *this;
}




bool DescriptorBuilder::Build(VkDescriptorSet& set, VkDescriptorSetLayout& layout){
	//build layout first
	VkDescriptorSetLayoutCreateInfo layout_info{};
	layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layout_info.pNext = nullptr;

	layout_info.pBindings = bindings.data();
	layout_info.bindingCount = bindings.size();

	layout = cache->CreateDescriptorLayout(layout_info);

	//allocate descriptor
	bool success = alloc->Allocate(set, layout);
	if (!success) { return false; };

	//write descriptor
	for (VkWriteDescriptorSet& w : writes) {
		w.dstSet = set;
	}

	vkUpdateDescriptorSets(DeviceManager::GetVkDevice().device, writes.size(), writes.data(), 0, nullptr);

	return true;
}

bool DescriptorBuilder::Build(VkDescriptorSet& set){
	VkDescriptorSetLayout layout;
	return Build(set, layout);
}




