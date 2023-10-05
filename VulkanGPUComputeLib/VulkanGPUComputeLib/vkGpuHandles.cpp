#include "vkGpuHandles.h"

GPUMemoryHandle::GPUMemoryHandle(GPUHandle* handle, uint32_t memoryPoolSize)
{
	VkResult result;

	m_PhysicalDevice = handle->GetPhysicalDevice();

	result = vkGenerics::AquireQueueIndex(*m_PhysicalDevice, VK_QUEUE_TRANSFER_BIT, m_QueueIndex);

	m_Device = handle->GetDevice();

	m_DeviceQueueIndicies = handle->GetActiveQueueIndiciesVec();

	m_MemoryCommandPool = nullptr;

	m_PoolSize = memoryPoolSize;
}

GPUMemoryHandle::GPUMemoryHandle(GPUHandle* handle, uint32_t memoryPoolSize, std::vector<const char*> extensions)
{
	VkResult result;

	m_PhysicalDevice = handle->GetPhysicalDevice();

	result = vkGenerics::AquireQueueIndex(*m_PhysicalDevice, VK_QUEUE_TRANSFER_BIT, m_QueueIndex);

	extensions.push_back("VK_KHR_external_memory");

	m_Device = new VkDevice();

	result = vkGenerics::CreateLogicalDevice(*m_Device, *m_PhysicalDevice, extensions, m_QueueIndex);
	if (result != VK_SUCCESS)
	{
		Journal::AddErrorEntry(L"MinervaErrorLog", L"2", L"GPUMemoryHandle Unable to create device.");
	}

	m_DeviceQueueIndicies = handle->GetActiveQueueIndiciesVec();

	m_MemoryCommandPool = new VkCommandPool();

	result = vkGenerics::CreateCommandPool(*m_Device, *m_MemoryCommandPool, m_QueueIndex, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	if (result != VK_SUCCESS)
	{
		Journal::AddErrorEntry(L"MinervaErrorLog", L"4", L"GPUMemoryHandle unable to create a command pool.");
	}
	m_PoolSize = memoryPoolSize;
	m_Independent = true;
}

GPUMemoryHandle::~GPUMemoryHandle()
{
	for (HostAccessableMemoryPool* pool : m_CPUAccessableMemoryPools)
	{
		pool->~HostAccessableMemoryPool();
	}

	for (HostAccessableMemoryPool* pool : m_CPUAccessableVertexMemoryPools)
	{
		pool->~HostAccessableMemoryPool();
	}

	for (HostAccessableMemoryPool* pool : m_CPUAccessableIndexMemoryPools)
	{
		pool->~HostAccessableMemoryPool();
	}

	if (m_MemoryCommandPool != nullptr)
	{
		vkGenerics::Destroy(*m_Device, *m_MemoryCommandPool);
	}

	if (m_Independent)
	{
		vkGenerics::Destroy(*m_Device);
	}
}

HostAccessablePtr GPUMemoryHandle::WriteDataToCpuVisablePool(void* data, uint32_t size)
{
	
	if (m_CPUAccessableMemoryPools.size() == 0)
	{
		m_CPUAccessableMemoryPools.push_back(new HostAccessableMemoryPool(m_PhysicalDevice, m_Device, m_PoolSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, m_DeviceQueueIndicies));
	}

	HostAccessablePtr newPtr;

	for (HostAccessableMemoryPool* selectedPool : m_CPUAccessableMemoryPools)
	{
		newPtr = selectedPool->MemAlloc(data, size);
		if (newPtr.GetState() == VK_SUCCESS)
		{
			return newPtr;
		}
	}

	m_CPUAccessableMemoryPools.push_back(new HostAccessableMemoryPool(m_PhysicalDevice, m_Device, m_PoolSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, m_DeviceQueueIndicies));
	newPtr = m_CPUAccessableMemoryPools.back()->MemAlloc(data, size);

	if (newPtr.GetState() == VK_SUCCESS)
	{
		return newPtr;
	}
	else
	{
		HostAccessablePtr emptyPtr;
		emptyPtr.GetState() = VK_ERROR_UNKNOWN;
		return emptyPtr;
	}
}

void GPUMemoryHandle::FreeHostAccessablePointer(HostAccessablePtr& ptr)
{
	HostAccessableMemoryPool* home = (HostAccessableMemoryPool*)ptr.GetHome();
	home->MemDelloc(ptr);
}






//GPUPointer GPUComputeMemoryHandle::WriteToCpuVisablePool(void* data, uint32_t size)
//{
//	if (m_CPU_GPU_MemoryPool.size() == 0)
//	{
//		InitalizeMemoryStruct memStruct;
//		memStruct.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
//		memStruct.flags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
//		memStruct.physicalDev = *m_Handle->GetPhysicalDevice();
//		memStruct.queueFamilyIndicies = m_ActiveQueueIndices;
//		m_CPU_GPU_MemoryPool.push_back(new ManagedGPUMemoryPool(*m_Handle->GetDevice(), memStruct, m_PoolSize, m_MinimumGPUOffset));
//	}
//}

HostAccessableMemoryPool::HostAccessableMemoryPool(GPUHandle* handle, uint32_t size, VkBufferUsageFlags flags, std::vector<uint32_t>& indices)
{
	VkResult result;
	m_Device = handle->GetDevice();
	m_Buffer = new VkBuffer;
	m_MemoryBuffer = new VkDeviceMemory;
	m_Size = size;
	m_MinimumWriteSize = vkGenerics::GetPhysicalDeviceProperties(*handle->GetPhysicalDevice()).limits.minStorageBufferOffsetAlignment;

	result = vkGenerics::CreateMemoryBuffer(*m_Device, *m_Buffer, m_Size, flags, VK_SHARING_MODE_EXCLUSIVE, indices.size(), indices.data());
	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L"VulkanMemoryPool::VulkanMemoryPool()", L"The memory pool is unable to create a buffer.", L"See outputted error");
	}

	VkMemoryRequirements memRequirments;
	vkGenerics::GetBufferMemoryRequirements(*m_Device, *m_Buffer, memRequirments);
	VkMemoryAllocateInfo allocateInfo = {};
	allocateInfo.allocationSize = memRequirments.size;
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.memoryTypeIndex = vkGenerics::FindMemoryIndex(memRequirments.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, *handle->GetPhysicalDevice());
	allocateInfo.pNext = VK_NULL_HANDLE;
	result = vkGenerics::AllocateMemory(*m_Device, *m_MemoryBuffer, allocateInfo);
	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L"VulkanMemoryPool::VulkanMemoryPool()", L"Unable to allocate memory.", L"See outputted error");
	}

	result = vkGenerics::BindBufferToMemory(*m_Device, *m_MemoryBuffer, *m_Buffer);
	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L"VulkanMemoryPool::VulkanMemoryPool()", L"Unable to bind memory to VkBuffer.", L"See outputted error");
	}

	std::random_device dev;
	std::mt19937 rng(dev());
	std::uniform_int_distribution<std::mt19937::result_type> idDist(0, 2000);
	m_Noise = idDist(rng);
	m_LastOffset = 0;
}

HostAccessableMemoryPool::HostAccessableMemoryPool(VkPhysicalDevice* physDev, VkDevice* device, uint32_t size, VkBufferUsageFlags flags, std::vector<uint32_t>& indices)
{	
	VkResult result;
	m_Device = device;
	m_Buffer = new VkBuffer;
	m_MemoryBuffer = new VkDeviceMemory;
	m_Size = size;
	m_MinimumWriteSize = vkGenerics::GetPhysicalDeviceProperties(*physDev).limits.minStorageBufferOffsetAlignment;

	result = vkGenerics::CreateMemoryBuffer(*m_Device, *m_Buffer, m_Size, flags, VK_SHARING_MODE_EXCLUSIVE, indices.size(), indices.data());
	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L"VulkanMemoryPool::VulkanMemoryPool()", L"The memory pool is unable to create a buffer.", L"See outputted error");
	}

	VkMemoryRequirements memRequirments;
	vkGenerics::GetBufferMemoryRequirements(*m_Device, *m_Buffer, memRequirments);
	VkMemoryAllocateInfo allocateInfo = {};
	allocateInfo.allocationSize = memRequirments.size;
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.memoryTypeIndex = vkGenerics::FindMemoryIndex(memRequirments.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, *physDev);
	allocateInfo.pNext = VK_NULL_HANDLE;
	result = vkGenerics::AllocateMemory(*m_Device, *m_MemoryBuffer, allocateInfo);
	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L"VulkanMemoryPool::VulkanMemoryPool()", L"Unable to allocate memory.", L"See outputted error");
	}

	result = vkGenerics::BindBufferToMemory(*m_Device, *m_MemoryBuffer, *m_Buffer);
	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L"VulkanMemoryPool::VulkanMemoryPool()", L"Unable to bind memory to VkBuffer.", L"See outputted error");
	}

	std::random_device dev;
	std::mt19937 rng(dev());
	std::uniform_int_distribution<std::mt19937::result_type> idDist(0, 2000);
	m_Noise = idDist(rng);
	m_LastOffset = 0;
}

HostAccessablePtr HostAccessableMemoryPool::MemAlloc(void* data, uint32_t size)
{
	HostAccessablePtr newPtr;
	uint32_t memoryAllocationSize = size;
	if (memoryAllocationSize < m_MinimumWriteSize)
	{
		memoryAllocationSize = m_MinimumWriteSize;
	}
	if (m_LastOffset + memoryAllocationSize > m_Size)
	{
		newPtr.GetState() = VK_ERROR_OUT_OF_POOL_MEMORY;
		return newPtr;
	}

	auto position = m_DeallocatedSpace.begin();
	void* memoryLoaction = nullptr;
	int bestFitResults = -1;
	for (size_t i = 0; i < m_DeallocatedSpace.size(); i++)
	{
		MemoryID memID;
		position += i;
		memID.m_ID = m_DeallocatedSpace[i];
		if (memID.m_location.m_Size == size)
		{
			bestFitResults = memID.m_location.m_Offset;
		}
	}

	if (bestFitResults != -1)
	{
		VkResult result = vkGenerics::MapMemoryToPointer(*m_Device, *m_MemoryBuffer, bestFitResults, &memoryLoaction, memoryAllocationSize);
		if (result != VK_SUCCESS)
		{
			vkDebug::ThrowError(L"HostAccessableMemoryPool::MemAllocVec()", L"Unable to map GPU memory to pointer", L"See if the memory buffer is valid");
			return HostAccessablePtr();
		}
		memcpy(memoryLoaction, (void*)data, (size_t)memoryAllocationSize); //copy our vertex data
		vkGenerics::UnmapMemory(*m_Device, *m_MemoryBuffer);
		newPtr.memoryID.m_location.m_Offset = bestFitResults;
		newPtr.memoryID.m_location.m_Size = memoryAllocationSize;
		newPtr.m_Home = this;
		++m_Counter;
		++m_Noise;

		m_ActivePtr.push_back(newPtr.memoryID.m_ID);
		m_DeallocatedSpace.erase(position);
		return newPtr;
	}


	uint16_t newOffset = m_LastOffset + m_LastSize;
	VkResult result = vkGenerics::MapMemoryToPointer(*m_Device, *m_MemoryBuffer, newOffset, &memoryLoaction, memoryAllocationSize);
	memcpy(memoryLoaction, (void*)data, (size_t)memoryAllocationSize); //copy our vertex data
	vkGenerics::UnmapMemory(*m_Device, *m_MemoryBuffer);
	newPtr.memoryID.m_location.m_Offset = newOffset;
	newPtr.memoryID.m_location.m_Size = memoryAllocationSize;
	newPtr.m_Home = this;
	m_LastOffset = newOffset;
	m_LastSize = memoryAllocationSize;
	m_ActivePtr.push_back(newPtr.memoryID.m_ID);
	++m_Counter;
	++m_Noise;

	return newPtr;
}

void HostAccessableMemoryPool::MemDelloc(HostAccessablePtr& location)
{
	char* blankData = new char[location.memoryID.m_location.m_Size];
	void* memoryLoaction = nullptr;
	VkResult result = vkGenerics::MapMemoryToPointer(*m_Device, *m_MemoryBuffer, location.memoryID.m_location.m_Offset, &memoryLoaction, location.memoryID.m_location.m_Size);

	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L"HostAccessableMemoryPool::MemDelloc()", L"Unable to map GPU memory to pointer", L"Ensure that the memory is valid");
		return;
	}

	memcpy(memoryLoaction, (void*)blankData, (size_t)location.memoryID.m_location.m_Size); //copy our vertex data
	vkGenerics::UnmapMemory(*m_Device, *m_MemoryBuffer);
	m_DeallocatedSpace.push_back(location.memoryID.m_ID);
	
	for (size_t i = 0; i < m_ActivePtr.size(); i++)
	{
		if (m_ActivePtr[i] == location.memoryID.m_ID) 
		{
			m_ActivePtr.erase(m_ActivePtr.begin() + i);
		}
	}
}


GPUMemoryAdapter::GPUMemoryAdapter(GPUHandle* handle, int addressableBuffers)
{
	m_AddressableBuffers = addressableBuffers;
	m_Port = HostAccessablePtr();
	m_Device = handle->GetDevice();
	VkResult result;
	std::vector<VkDescriptorSetLayoutBinding> bindings(m_AddressableBuffers);

	int i = 0;
	for (VkDescriptorSetLayoutBinding& binding : bindings)
	{
		binding.binding = i;
		binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		binding.descriptorCount = 1;
		binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		++i;
	}

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
	descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutCreateInfo.flags = 0;
	descriptorSetLayoutCreateInfo.pNext = 0;
	descriptorSetLayoutCreateInfo.bindingCount = m_AddressableBuffers;
	descriptorSetLayoutCreateInfo.pBindings = bindings.data();
	result = vkGenerics::CreateDescriptorSetLayout(*m_Device, m_DescriptorSetLayout, descriptorSetLayoutCreateInfo);
	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L"VkKernalMemory::VkKernalMemory()", L"Failed to allocate a descriptor set layout", L"Check the creation of descriptor pools and set layout.");
	}


	std::vector<VkDescriptorPoolSize> poolSizes(m_AddressableBuffers);
	for (VkDescriptorPoolSize& poolSize : poolSizes)
	{
		poolSize.descriptorCount = 1;
		poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	}

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.pNext = VK_NULL_HANDLE;
	descriptorPoolCreateInfo.pPoolSizes = poolSizes.data();
	descriptorPoolCreateInfo.poolSizeCount = m_AddressableBuffers;
	descriptorPoolCreateInfo.flags = 0;
	descriptorPoolCreateInfo.maxSets = m_AddressableBuffers;
	result = vkGenerics::CreateDescriptorPool(*m_Device, m_DescriptorPool, descriptorPoolCreateInfo);
	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L"VkKernalMemory::VkKernalMemory()", L"Failed to allocate a descriptor set", L"Check the creation of descriptor pools and set layout.");
	}

	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.pNext = VK_NULL_HANDLE;
	descriptorSetAllocateInfo.descriptorPool = m_DescriptorPool;
	descriptorSetAllocateInfo.descriptorSetCount = 1;
	descriptorSetAllocateInfo.pSetLayouts = &m_DescriptorSetLayout;
	result = vkGenerics::CreateDescriptorSet(*m_Device, m_DescriptorSet, descriptorSetAllocateInfo);
	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L"vkGenerics::CreateDescriptorSet()", L"Failed to allocate a descriptor set", L"Check the creation of descriptor pools and set layout.");
	}
}

/*
	VkDevice* m_Device;
	VkDescriptorSetLayout m_DescriptorSetLayout;
	VkDescriptorSet m_DescriptorSet;
	VkDescriptorPool m_DescriptorPool;
	HostAccessablePtr m_Port;
	int m_AddressableBuffers = 0;
*/

GPUMemoryAdapter::~GPUMemoryAdapter()
{

	HostAccessableMemoryPool* pool = (HostAccessableMemoryPool*)m_Port.GetHome();

	pool->MemDelloc(m_Port);

	vkGenerics::Destroy(*m_Device, m_DescriptorSetLayout);
	//vkGenerics::Destroy(*m_Device, m_DescriptorSet);
	vkGenerics::Destroy(*m_Device, m_DescriptorPool);
}

void GPUMemoryAdapter::AttachMemory(HostAccessablePtr& newMemory)
{
	m_Port = newMemory;
	HostAccessableMemoryPool* pool = (HostAccessableMemoryPool*)m_Port.GetHome();

	uint8_t memoryOffset = m_Port.GetOffset();
	uint8_t minimumOffset = pool->GetMinimumGPUOffset();


	vkGenerics::UpdateDescData updateMem;



	updateMem.m_Buffer = pool->ReturnVkBuffer();
	updateMem.m_DstSet = &m_DescriptorSet;

	if (memoryOffset < minimumOffset && memoryOffset != 0)
	{
		updateMem.m_Offset = minimumOffset;
	}
	else
	{
		updateMem.m_Offset = memoryOffset;
	};

	updateMem.m_ImageInfo = VK_NULL_HANDLE;
	updateMem.m_pNext = VK_NULL_HANDLE;
	updateMem.m_TextelBuff = VK_NULL_HANDLE;
	updateMem.m_Type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	updateMem.m_DstCount = 1;

	for (size_t i = 0; i < m_AddressableBuffers; i++)
	{
		updateMem.m_Binding = i;
		vkGenerics::UpdateDescriptorBufferInfo(updateMem, *m_Device);
	}
}

void GPUMemoryAdapter::WriteData(void* data)
{
	HostAccessableMemoryPool* pool = (HostAccessableMemoryPool*)m_Port.GetHome();
	pool->WriteData(m_Port.GetOffset(), m_Port.GetSize(), data);
}

void GPUMemoryAdapter::WriteData(void* data, uint32_t size)
{
	HostAccessableMemoryPool* pool = (HostAccessableMemoryPool*)m_Port.GetHome();
	if (size > m_Port.GetSize())
	{
		vkDebug::ThrowError(L"GPUMemoryAdapter::WriteData()", L"Copying too much data", L"N/A");
		return;
	}
	pool->WriteData(m_Port.GetOffset(), size, data);
}

void GPUMemoryAdapter::WriteData(void* data, uint32_t offset, uint32_t size)
{
	HostAccessableMemoryPool* pool = (HostAccessableMemoryPool*)m_Port.GetHome();
	if (m_Port.GetOffset() + offset + size > m_Port.GetOffset() + m_Port.GetSize())
	{
		vkDebug::ThrowError(L"GPUMemoryAdapter::WriteData()", L"Copying too much data or the offset is too high", L"N/A");
		return;
	}
	pool->WriteData(m_Port.GetOffset() + offset, size, data);
}

void GPUMemoryAdapter::ReadData(void* data)
{
	HostAccessableMemoryPool* pool = (HostAccessableMemoryPool*)m_Port.GetHome();
	pool->CopyData(m_Port.GetOffset(), m_Port.GetSize(), data);
}

void GPUMemoryAdapter::ReadData(void* data, uint32_t size)
{
	HostAccessableMemoryPool* pool = (HostAccessableMemoryPool*)m_Port.GetHome();
	if (size > m_Port.GetSize())
	{
		vkDebug::ThrowError(L"GPUMemoryAdapter::ReadData()", L"Reading too much data", L"N/A");
		return;
	}
	pool->CopyData(m_Port.GetOffset(), size, data);
}

void GPUMemoryAdapter::ReadData(void* data, uint32_t offset, uint32_t size)
{
	HostAccessableMemoryPool* pool = (HostAccessableMemoryPool*)m_Port.GetHome();
	if (m_Port.GetOffset() + offset + size > m_Port.GetOffset() + m_Port.GetSize())
	{
		vkDebug::ThrowError(L"GPUMemoryAdapter::WriteData()", L"Copying too much data or the offset is too high", L"N/A");
		return;
	}
	pool->CopyData(m_Port.GetOffset() + offset, size, data);
}

GPUKernel::GPUKernel(GPUHandle* handle, std::wstring shaderFileLocation, std::string shaderEntryPoint)
{
	m_Device = handle->GetDevice();
	m_MemoryAdatper = nullptr;
	m_ShaderInfo = new VkPipelineShaderStageCreateInfo();

	m_EntryPointName = shaderEntryPoint;

	m_ShaderInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	m_ShaderInfo->stage = VK_SHADER_STAGE_COMPUTE_BIT;
	m_ShaderInfo->pName = shaderEntryPoint.c_str();
	m_ShaderInfo->pNext = VK_NULL_HANDLE;

	VkResult result = vkGenerics::CreateShaderModule(shaderFileLocation, *m_Device, m_ShaderInfo->module);
	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L"GPUKernel::GPUKernel()", L"Failed to construct. Unable to create shader", L"Check shader for bugs and its location");
	}

	m_PipelineLayout = new VkPipelineLayout();
	m_Pipeline = new VkPipeline();

}

GPUKernel::~GPUKernel()
{
	vkGenerics::Destroy(*m_Device, *m_PipelineLayout);
	vkGenerics::Destroy(*m_Device, *m_Pipeline);
	vkGenerics::Destroy(*m_Device, m_ShaderInfo->module);
	delete m_ShaderInfo;
	delete m_Pipeline;
	delete m_PipelineLayout;
}

void GPUKernel::LinkMemory(GPUMemoryAdapter* adapter, int PipelineFlags)
{
	m_MemoryAdatper = adapter;

	VkResult result;

	if (m_PipelineLayout != nullptr && m_Pipeline != nullptr)
	{
		vkGenerics::Destroy(*m_Device, *m_Pipeline);
		vkGenerics::Destroy(*m_Device, *m_PipelineLayout);
	}

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = m_MemoryAdatper->GetDescriptorSetLayout();
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;
	pipelineLayoutInfo.pNext = VK_NULL_HANDLE;
	result = vkCreatePipelineLayout(*m_Device, &pipelineLayoutInfo, nullptr, m_PipelineLayout);
	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L"GPUKernel::LinkMemory()", L"Failed to construct. Unable to create pipeline layout", L"Check the Descriptor Set layouts.");
	}

	m_ShaderInfo->pName = m_EntryPointName.c_str();

	VkComputePipelineCreateInfo ComputePipelineCreateInfo = {};
	ComputePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	ComputePipelineCreateInfo.stage = *m_ShaderInfo;
	ComputePipelineCreateInfo.layout = *m_PipelineLayout;
	ComputePipelineCreateInfo.flags = PipelineFlags;
	ComputePipelineCreateInfo.basePipelineIndex = -1;
	ComputePipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	ComputePipelineCreateInfo.pNext = VK_NULL_HANDLE;
	result = vkGenerics::CreateComputePipeline(*m_Device, *m_Pipeline, ComputePipelineCreateInfo);
	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L"GPUKernel::LinkMemory()", L"Failed to construct. Unable to create pipeline", L"Check the  included shaders and flags.");
	}
}

void GPUKernel::RequestExecution(GPUHandle* handle, VkExecuteHint executionHint)
{
	VkResult result;

	//VkCommandBuffer* buffer = handle->ExposeCommandBuffer(ModeIndex::COMPUTE);

	//result = vkGenerics::BeginCommandBuffer(*buffer, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
	//if (result != VK_SUCCESS)
	//{
	//	vkDebug::ThrowError(L"VkKernal::RequestExecution()", L"Failed to construct. Unable to create pipeline", L"Check the  included shaders and flags.");
	//}

	//vkGenerics::BindComputePipeline(*buffer, *m_Pipeline);
	//vkGenerics::BindDescriptorSetsToCompute(*buffer, *m_PipelineLayout, *m_MemoryAdatper->GetDescriptorSet());
	//vkGenerics::DispatchComputeCommands(*buffer, executionHint.m_Xcount, executionHint.m_Ycount, executionHint.m_Zcount);

	//vkGenerics::EndCommandBuffer(*buffer);
	//if (result != VK_SUCCESS)
	//{
	//	vkDebug::ThrowError(L"VkKernal::VkKernal()", L"Failed to construct. Unable to create pipeline", L"Check the  included shaders and flags.");
	//}

	VkCommandBuffer buffer = VkCommandBuffer();
	result = vkGenerics::CreateCommandBuffer(*m_Device, buffer, *handle->GetPool(ModeIndex::COMPUTE), VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	result = vkGenerics::BeginCommandBuffer(buffer, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
	vkGenerics::BindComputePipeline(buffer, *m_Pipeline);
	vkGenerics::BindDescriptorSetsToCompute(buffer, *m_PipelineLayout, *m_MemoryAdatper->GetDescriptorSet());
	vkGenerics::DispatchComputeCommands(buffer, executionHint.m_Xcount, executionHint.m_Ycount, executionHint.m_Zcount);
	vkGenerics::EndCommandBuffer(buffer);
	handle->AddCommandBuffer(ModeIndex::COMPUTE, &buffer);

}

void GPUKernel::RequestExecution2(GPUHandle* handle, VkExecuteHint executionHint)
{
	VkResult result;

	VkCommandBuffer* buffer = handle->ExposeCommandBuffer(ModeIndex::COMPUTE);

	result = vkGenerics::BeginCommandBuffer(*buffer, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L"VkKernal::RequestExecution()", L"Failed to construct. Unable to create pipeline", L"Check the  included shaders and flags.");
	}

	vkGenerics::BindComputePipeline(*buffer, *m_Pipeline);
	vkGenerics::BindDescriptorSetsToCompute(*buffer, *m_PipelineLayout, *m_MemoryAdatper->GetDescriptorSet());
	vkGenerics::DispatchComputeCommands(*buffer, executionHint.m_Xcount, executionHint.m_Ycount, executionHint.m_Zcount);

	vkGenerics::EndCommandBuffer(*buffer);
	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L"VkKernal::VkKernal()", L"Failed to construct. Unable to create pipeline", L"Check the  included shaders and flags.");
	}

}

void GPUKernel::WriteToMemory(void* data, uint32_t size)
{
	m_MemoryAdatper->WriteData(data, size);
}

void GPUKernel::WriteElementToMemory(void* data, uint32_t size, uint32_t offsetOfElement)
{
	m_MemoryAdatper->WriteData(data, offsetOfElement, size);
}

void GPUKernel::ReadFromMemory(void* data, uint32_t size)
{
	m_MemoryAdatper->ReadData(data, size);
}

void GPUKernel::ReadElementFromMemory(void* data, uint32_t size, uint32_t offsetOfElement)
{
	m_MemoryAdatper->ReadData(data, offsetOfElement, size);
}

void GPUHandle::ExecuteCompute()
{
	VkResult result;

	VkPipelineStageFlags flags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

	std::vector<VkSemaphore> buffers(0);

	result = vkGenerics::ExecuteQueue(m_Queues[ModeIndex::COMPUTE], m_CmdBuffers[ModeIndex::COMPUTE], buffers, buffers, &flags);
	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L"GPUHandle::Execute()", L"Unable to execute queue", L"See outputed error");
	}
	vkQueueWaitIdle(m_Queues[ModeIndex::COMPUTE]);
	for (VkCommandBuffer buff : m_CmdBuffers[ModeIndex::COMPUTE])
	{
		vkGenerics::Destroy(*m_Device, buff);
	}
	m_CmdBuffers[ModeIndex::COMPUTE].clear();
	vkGenerics::ResetCommandPool(*m_Device, *m_Pools[ModeIndex::COMPUTE], VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
}
