#pragma once
#include <random>

#include "vkGenerics.h"
#include "Journal.h"


typedef vkGenerics::vkQueueFamilyBitAccumulated GPUHandleMode;
typedef vkGenerics::vkQueueFamilyBit ModeIndex;

class vkInstanceContainer
{
public:
	vkInstanceContainer() { m_Instance = nullptr; };
	~vkInstanceContainer()
	{
		if (m_Instance == nullptr)
		{
			return;
		}

#ifdef _DEBUG
		vkDebug::Destroy(*m_Instance, m_DebugLayer);
		m_DebugLayer = VK_NULL_HANDLE;
#endif // _DEBUG

#ifdef _NDEBUG
		vkGenerics::Destroy(*m_Instance);
#endif // _NDEBUG

		delete m_Instance;
		m_Instance = nullptr;
	};

	void InitPointer() { m_Instance = new VkInstance(); };

	VkInstance* GetInstance() const { return m_Instance; };

	vkInstanceContainer(const vkInstanceContainer& obj) = delete;

#ifdef _DEBUG

	VkDebugReportCallbackEXT* ReturnDebugLayer() { return &m_DebugLayer; };
#endif // _DEBUG

protected:
	VkInstance* m_Instance = nullptr;

#ifdef _DEBUG
	VkDebugReportCallbackEXT m_DebugLayer = VK_NULL_HANDLE;
#endif // _DEBUG

};

struct VkExecuteHint
{
	VkExecuteHint()
	{
		m_Xcount = 1;
		m_Ycount = 1;
		m_Zcount = 1;
	}
	uint32_t m_Xcount;
	uint32_t m_Ycount;
	uint32_t m_Zcount;
};

class GPUHandle
{
public:
	GPUHandle(GPUHandleMode mode, std::vector<const char*> instanceExtensions, std::vector<const char*> deviceExtensions)
	{
		VkResult result;

		m_Instance = new vkInstanceContainer();
		m_Instance->InitPointer();

		VkApplicationInfo AppInfo = {};
		AppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		AppInfo.apiVersion = VK_API_VERSION_1_3;
		AppInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 1, 0);
		AppInfo.engineVersion = VK_MAKE_API_VERSION(0, 0, 1, 0);
		AppInfo.pEngineName = "Minerva";
		AppInfo.pApplicationName = "MinervaApplication";
		AppInfo.pNext = nullptr;
		//Creation information for a VkInstance
		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &AppInfo;
		createInfo.enabledExtensionCount = vkGenerics::UnsignedRecast<size_t, size_t>(instanceExtensions.size());
		createInfo.ppEnabledExtensionNames = instanceExtensions.data();
		createInfo.ppEnabledLayerNames = nullptr;
		createInfo.enabledLayerCount = 0;

#ifdef _DEBUG
		result = vkGenerics::CreateDebugInstance(createInfo, m_Instance->GetInstance(), m_Instance->ReturnDebugLayer());
#endif // _DEBUG

#ifdef NDEBUG
		result = vkGenerics::CreateInstance(createInfo, m_Instance->GetInstance());
#endif // NDEBUG

		m_PhysicalDevice = new VkPhysicalDevice();
		m_Device = new VkDevice();

		deviceExtensions.push_back("VK_KHR_external_memory");

		result = vkGenerics::CreatePhysicalDevice(*m_PhysicalDevice, m_Instance->GetInstance(), deviceExtensions);
		if (result != VK_SUCCESS)
		{
			Journal::AddErrorEntry(L"MinervaErrorLog", L"1", L"GPUHandle Unable to obtain a physical device.");
		}

		std::unordered_map<vkGenerics::vkQueueFamilyBit, int> labeledQueueIndices;

		switch (mode)
		{
		case vkGenerics::vkQueueFamilyBitAccumulated::GRAPHICS:
			vkGenerics::AquireQueueIndex(*m_PhysicalDevice, vkGenerics::vkQueueFamilyBit::GRAPHICS, labeledQueueIndices[vkGenerics::vkQueueFamilyBit::GRAPHICS]);
			break;
		case vkGenerics::vkQueueFamilyBitAccumulated::SWAPCHAIN:
			labeledQueueIndices[vkGenerics::vkQueueFamilyBit::SWAPCHAIN] = 2;
			break;
		case vkGenerics::vkQueueFamilyBitAccumulated::COMPUTE:
			vkGenerics::AquireQueueIndex(*m_PhysicalDevice, vkGenerics::vkQueueFamilyBit::COMPUTE, labeledQueueIndices[vkGenerics::vkQueueFamilyBit::COMPUTE]);
			break;
		case vkGenerics::vkQueueFamilyBitAccumulated::SWAPCHAIN_GRAPHICS:
			labeledQueueIndices[vkGenerics::vkQueueFamilyBit::SWAPCHAIN] = 2;
			vkGenerics::AquireQueueIndex(*m_PhysicalDevice, vkGenerics::vkQueueFamilyBit::GRAPHICS, labeledQueueIndices[vkGenerics::vkQueueFamilyBit::GRAPHICS]);
			break;
		case vkGenerics::vkQueueFamilyBitAccumulated::GRAPHICS_COMPUTE:
			vkGenerics::AquireQueueIndex(*m_PhysicalDevice, vkGenerics::vkQueueFamilyBit::GRAPHICS, labeledQueueIndices[vkGenerics::vkQueueFamilyBit::GRAPHICS]);
			vkGenerics::AquireQueueIndex(*m_PhysicalDevice, vkGenerics::vkQueueFamilyBit::COMPUTE, labeledQueueIndices[vkGenerics::vkQueueFamilyBit::COMPUTE]);
			break;
		case vkGenerics::vkQueueFamilyBitAccumulated::SWAPCHAIN_GRAPHICS_COMPUTE:
			vkGenerics::AquireQueueIndex(*m_PhysicalDevice, vkGenerics::vkQueueFamilyBit::GRAPHICS, labeledQueueIndices[vkGenerics::vkQueueFamilyBit::GRAPHICS]);
			vkGenerics::AquireQueueIndex(*m_PhysicalDevice, vkGenerics::vkQueueFamilyBit::COMPUTE, labeledQueueIndices[vkGenerics::vkQueueFamilyBit::COMPUTE]);
			labeledQueueIndices[vkGenerics::vkQueueFamilyBit::SWAPCHAIN] = 2;
			break;
		default:
			break;
		}

		result = vkGenerics::CreateLogicalDevice(*m_Device, *m_PhysicalDevice, deviceExtensions, labeledQueueIndices);
		if (result != VK_SUCCESS)
		{
			Journal::AddErrorEntry(L"MinervaErrorLog", L"2", L"GPUHandle Unable to create device.");
		}

		for (std::pair<vkGenerics::vkQueueFamilyBit, int> queueIndex : labeledQueueIndices)
		{
			vkGenerics::CreateQueue(*m_Device, m_Queues[queueIndex.first], queueIndex.second);
		}

		for (std::pair<vkGenerics::vkQueueFamilyBit, int> queueIndex : labeledQueueIndices)
		{
			vkGenerics::CreateQueue(*m_Device, m_Queues[queueIndex.first], queueIndex.second);
			m_Pools[queueIndex.first] = new VkCommandPool();
			result = vkGenerics::CreateCommandPool(*m_Device, *m_Pools[queueIndex.first], queueIndex.second, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
			if (result != VK_SUCCESS)
			{
				Journal::AddErrorEntry(L"MinervaErrorLog", L"4", L"GPUHandle Unable to create a command pool.");
			}
		}
	}

	GPUHandle(GPUHandleMode mode, VkInstance* instance, std::vector<const char*> extensions)
	{
		VkResult result;

		extensions.push_back("VK_KHR_external_memory");

		m_PhysicalDevice = new VkPhysicalDevice();
		m_Device = new VkDevice();

		result = vkGenerics::CreatePhysicalDevice(*m_PhysicalDevice, m_Instance->GetInstance(), extensions);
		if (result != VK_SUCCESS)
		{
			Journal::AddErrorEntry(L"MinervaErrorLog", L"1", L"GPUHandle Unable to obtain a physical device.");
		}

		std::unordered_map<vkGenerics::vkQueueFamilyBit, int> labeledQueueIndices;

		switch (mode)
		{
		case vkGenerics::vkQueueFamilyBitAccumulated::GRAPHICS:
			vkGenerics::AquireQueueIndex(*m_PhysicalDevice, vkGenerics::vkQueueFamilyBit::GRAPHICS, labeledQueueIndices[vkGenerics::vkQueueFamilyBit::GRAPHICS]);
			break;
		case vkGenerics::vkQueueFamilyBitAccumulated::SWAPCHAIN:
			labeledQueueIndices[vkGenerics::vkQueueFamilyBit::SWAPCHAIN] = 2;
			break;
		case vkGenerics::vkQueueFamilyBitAccumulated::COMPUTE:
			vkGenerics::AquireQueueIndex(*m_PhysicalDevice, vkGenerics::vkQueueFamilyBit::COMPUTE, labeledQueueIndices[vkGenerics::vkQueueFamilyBit::COMPUTE]);
			break;
		case vkGenerics::vkQueueFamilyBitAccumulated::SWAPCHAIN_GRAPHICS:
			labeledQueueIndices[vkGenerics::vkQueueFamilyBit::SWAPCHAIN] = 2;
			vkGenerics::AquireQueueIndex(*m_PhysicalDevice, vkGenerics::vkQueueFamilyBit::GRAPHICS, labeledQueueIndices[vkGenerics::vkQueueFamilyBit::GRAPHICS]);
			break;
		case vkGenerics::vkQueueFamilyBitAccumulated::GRAPHICS_COMPUTE:
			vkGenerics::AquireQueueIndex(*m_PhysicalDevice, vkGenerics::vkQueueFamilyBit::GRAPHICS, labeledQueueIndices[vkGenerics::vkQueueFamilyBit::GRAPHICS]);
			vkGenerics::AquireQueueIndex(*m_PhysicalDevice, vkGenerics::vkQueueFamilyBit::COMPUTE, labeledQueueIndices[vkGenerics::vkQueueFamilyBit::COMPUTE]);
			break;
		case vkGenerics::vkQueueFamilyBitAccumulated::SWAPCHAIN_GRAPHICS_COMPUTE:
			vkGenerics::AquireQueueIndex(*m_PhysicalDevice, vkGenerics::vkQueueFamilyBit::GRAPHICS, labeledQueueIndices[vkGenerics::vkQueueFamilyBit::GRAPHICS]);
			vkGenerics::AquireQueueIndex(*m_PhysicalDevice, vkGenerics::vkQueueFamilyBit::COMPUTE, labeledQueueIndices[vkGenerics::vkQueueFamilyBit::COMPUTE]);
			labeledQueueIndices[vkGenerics::vkQueueFamilyBit::SWAPCHAIN] = 2;
			break;
		default:
			break;
		}

		result = vkGenerics::AquireQueueIndex(*m_PhysicalDevice, VK_QUEUE_TRANSFER_BIT, labeledQueueIndices[vkGenerics::vkQueueFamilyBit::TRANSFER]);

		result = vkGenerics::CreateLogicalDevice(*m_Device, *m_PhysicalDevice, extensions, labeledQueueIndices);
		if (result != VK_SUCCESS)
		{
			Journal::AddErrorEntry(L"MinervaErrorLog", L"2", L"GPUHandle Unable to create device.");
		}

		for (std::pair<vkGenerics::vkQueueFamilyBit, int> queueIndex : labeledQueueIndices)
		{
			vkGenerics::CreateQueue(*m_Device, m_Queues[queueIndex.first], queueIndex.second);
		}

		for (std::pair<vkGenerics::vkQueueFamilyBit, int> queueIndex : labeledQueueIndices)
		{
			vkGenerics::CreateQueue(*m_Device, m_Queues[queueIndex.first], queueIndex.second);
			m_Pools[queueIndex.first] = new VkCommandPool();
			result = vkGenerics::CreateCommandPool(*m_Device, *m_Pools[queueIndex.first], queueIndex.second, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
			if (result != VK_SUCCESS)
			{
				Journal::AddErrorEntry(L"MinervaErrorLog", L"4", L"GPUHandle Unable to create a command pool.");
			}
		}
	}

	~GPUHandle()
	{
		for (std::pair<vkGenerics::vkQueueFamilyBit, VkCommandPool*> queues : m_Pools)
		{
			vkGenerics::Destroy(*m_Device, *queues.second);
		}

		vkGenerics::Destroy(*m_Device);

		if (m_Instance != nullptr)
		{
			delete m_Instance;
		}
	}

	VkInstance* GetInstance() { return m_Instance->GetInstance(); }

	VkPhysicalDevice* GetPhysicalDevice() { return m_PhysicalDevice; }

	VkDevice* GetDevice() { return m_Device; }

	VkQueue* GetQueue(ModeIndex modeIndex) { return &m_Queues[modeIndex]; }

	VkCommandPool* GetPool(ModeIndex modeIndex) { return m_Pools[modeIndex]; }

	int GetQueueIndex(ModeIndex modeIndex) { return m_QueueIndex[modeIndex]; }

	std::unordered_map<vkGenerics::vkQueueFamilyBit, int>& GetActiveQueueIndicies() { return m_QueueIndex; }

	std::vector<uint32_t> GetActiveQueueIndiciesVec()
	{ 
		std::vector<uint32_t> outputVec;
		outputVec.reserve(m_QueueIndex.size());

		for (std::pair<vkGenerics::vkQueueFamilyBit, int> queueIndex : m_QueueIndex)
		{
			outputVec.push_back(queueIndex.second);
		}
		return outputVec;
	}

	VkCommandBuffer* ExposeCommandBuffer(ModeIndex modeIndex)
	{
		VkResult result;

		m_CmdBuffers[modeIndex].push_back(VkCommandBuffer());

		result = vkGenerics::CreateCommandBuffer(*m_Device, m_CmdBuffers[modeIndex].back(), *m_Pools[modeIndex], VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY);

		return &m_CmdBuffers[modeIndex].back();
	}

	void AddCommandBuffer(ModeIndex modeIndex, VkCommandBuffer* cmdBuffer)
	{
		m_CmdBuffers[modeIndex].push_back(*cmdBuffer);
	}

	void Execute(GPUHandleMode executionSet, VkPipelineStageFlags executionFlag)
	{
		VkResult result;

		std::vector<VkSemaphore> buffers(0);

		switch (executionSet)
		{
		case GPUHandleMode::BASE:
			vkDebug::ThrowError(L"GPUHandle::Execute()", L"Invalid GPUHandleMode", L"N/A");
			break;
		case GPUHandleMode::GRAPHICS:

			result = vkGenerics::ExecuteQueue(m_Queues[ModeIndex::GRAPHICS], m_CmdBuffers[ModeIndex::GRAPHICS], buffers, buffers, &executionFlag);
			if (result != VK_SUCCESS)
			{
				vkDebug::ThrowError(L"GPUHandle::Execute()", L"Unable to execute queue", L"See outputed error");
			}
			vkQueueWaitIdle(m_Queues[ModeIndex::GRAPHICS]);
			for (VkCommandBuffer buff : m_CmdBuffers[ModeIndex::GRAPHICS])
			{
				vkGenerics::Destroy(*m_Device, buff);
			}
			m_CmdBuffers[ModeIndex::GRAPHICS].clear();
			vkGenerics::ResetCommandPool(*m_Device, *m_Pools[ModeIndex::GRAPHICS], VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);

			break;
		case GPUHandleMode::SWAPCHAIN:

			result = vkGenerics::ExecuteQueue(m_Queues[ModeIndex::SWAPCHAIN], m_CmdBuffers[ModeIndex::SWAPCHAIN], buffers, buffers, &executionFlag);
			if (result != VK_SUCCESS)
			{
				vkDebug::ThrowError(L"GPUHandle::Execute()", L"Unable to execute queue", L"See outputed error");
			}
			vkQueueWaitIdle(m_Queues[ModeIndex::SWAPCHAIN]);
			for (VkCommandBuffer buff : m_CmdBuffers[ModeIndex::SWAPCHAIN])
			{
				vkGenerics::Destroy(*m_Device, buff);
			}
			m_CmdBuffers[ModeIndex::SWAPCHAIN].clear();
			vkGenerics::ResetCommandPool(*m_Device, *m_Pools[ModeIndex::SWAPCHAIN], VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);

			break;
		case GPUHandleMode::COMPUTE:

			result = vkGenerics::ExecuteQueue(m_Queues[ModeIndex::COMPUTE], m_CmdBuffers[ModeIndex::COMPUTE], buffers, buffers, &executionFlag);
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

			break;
		case GPUHandleMode::SWAPCHAIN_GRAPHICS:

			result = vkGenerics::ExecuteQueue(m_Queues[ModeIndex::GRAPHICS], m_CmdBuffers[ModeIndex::GRAPHICS], buffers, buffers, &executionFlag);
			if (result != VK_SUCCESS)
			{
				vkDebug::ThrowError(L"GPUHandle::Execute()", L"Unable to execute queue", L"See outputed error");
			}
			vkQueueWaitIdle(m_Queues[ModeIndex::GRAPHICS]);
			for (VkCommandBuffer buff : m_CmdBuffers[ModeIndex::GRAPHICS])
			{
				vkGenerics::Destroy(*m_Device, buff);
			}
			m_CmdBuffers[ModeIndex::GRAPHICS].clear();
			vkGenerics::ResetCommandPool(*m_Device, *m_Pools[ModeIndex::GRAPHICS], VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);

			result = vkGenerics::ExecuteQueue(m_Queues[ModeIndex::SWAPCHAIN], m_CmdBuffers[ModeIndex::SWAPCHAIN], buffers, buffers, &executionFlag);
			if (result != VK_SUCCESS)
			{
				vkDebug::ThrowError(L"GPUHandle::Execute()", L"Unable to execute queue", L"See outputed error");
			}
			vkQueueWaitIdle(m_Queues[ModeIndex::SWAPCHAIN]);
			for (VkCommandBuffer buff : m_CmdBuffers[ModeIndex::SWAPCHAIN])
			{
				vkGenerics::Destroy(*m_Device, buff);
			}
			m_CmdBuffers[ModeIndex::SWAPCHAIN].clear();
			vkGenerics::ResetCommandPool(*m_Device, *m_Pools[ModeIndex::SWAPCHAIN], VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);

			break;
		case GPUHandleMode::GRAPHICS_COMPUTE:

			result = vkGenerics::ExecuteQueue(m_Queues[ModeIndex::COMPUTE], m_ComputeCmdBuffers, buffers, buffers, &executionFlag);
			if (result != VK_SUCCESS)
			{
				vkDebug::ThrowError(L"GPUHandle::Execute()", L"Unable to execute queue", L"See outputed error");
			}
			vkQueueWaitIdle(m_Queues[ModeIndex::COMPUTE]);
			for (VkCommandBuffer buff : m_ComputeCmdBuffers)
			{
				vkGenerics::Destroy(*m_Device, buff);
			}
			m_CmdBuffers[ModeIndex::COMPUTE].clear();
			vkGenerics::ResetCommandPool(*m_Device, *m_Pools[ModeIndex::COMPUTE], VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);

			result = vkGenerics::ExecuteQueue(m_Queues[ModeIndex::GRAPHICS], m_CmdBuffers[ModeIndex::GRAPHICS], buffers, buffers, &executionFlag);
			if (result != VK_SUCCESS)
			{
				vkDebug::ThrowError(L"GPUHandle::Execute()", L"Unable to execute queue", L"See outputed error");
			}
			vkQueueWaitIdle(m_Queues[ModeIndex::GRAPHICS]);
			for (VkCommandBuffer buff : m_CmdBuffers[ModeIndex::GRAPHICS])
			{
				vkGenerics::Destroy(*m_Device, buff);
			}
			m_CmdBuffers[ModeIndex::GRAPHICS].clear();
			vkGenerics::ResetCommandPool(*m_Device, *m_Pools[ModeIndex::GRAPHICS], VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);

			break;
		case GPUHandleMode::SWAPCHAIN_GRAPHICS_COMPUTE:

			result = vkGenerics::ExecuteQueue(m_Queues[ModeIndex::COMPUTE], m_ComputeCmdBuffers, buffers, buffers, &executionFlag);
			if (result != VK_SUCCESS)
			{
				vkDebug::ThrowError(L"GPUHandle::Execute()", L"Unable to execute queue", L"See outputed error");
			}
			vkQueueWaitIdle(m_Queues[ModeIndex::COMPUTE]);
			for (VkCommandBuffer buff : m_ComputeCmdBuffers)
			{
				vkGenerics::Destroy(*m_Device, buff);
			}
			m_CmdBuffers[ModeIndex::COMPUTE].clear();
			vkGenerics::ResetCommandPool(*m_Device, *m_Pools[ModeIndex::COMPUTE], VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);

			result = vkGenerics::ExecuteQueue(m_Queues[ModeIndex::GRAPHICS], m_CmdBuffers[ModeIndex::GRAPHICS], buffers, buffers, &executionFlag);
			if (result != VK_SUCCESS)
			{
				vkDebug::ThrowError(L"GPUHandle::Execute()", L"Unable to execute queue", L"See outputed error");
			}
			vkQueueWaitIdle(m_Queues[ModeIndex::GRAPHICS]);
			for (VkCommandBuffer buff : m_CmdBuffers[ModeIndex::GRAPHICS])
			{
				vkGenerics::Destroy(*m_Device, buff);
			}
			m_CmdBuffers[ModeIndex::GRAPHICS].clear();
			vkGenerics::ResetCommandPool(*m_Device, *m_Pools[ModeIndex::GRAPHICS], VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);

			result = vkGenerics::ExecuteQueue(m_Queues[ModeIndex::SWAPCHAIN], m_CmdBuffers[ModeIndex::SWAPCHAIN], buffers, buffers, &executionFlag);
			if (result != VK_SUCCESS)
			{
				vkDebug::ThrowError(L"GPUHandle::Execute()", L"Unable to execute queue", L"See outputed error");
			}
			vkQueueWaitIdle(m_Queues[ModeIndex::SWAPCHAIN]);
			for (VkCommandBuffer buff : m_CmdBuffers[ModeIndex::SWAPCHAIN])
			{
				vkGenerics::Destroy(*m_Device, buff);
			}
			m_CmdBuffers[ModeIndex::SWAPCHAIN].clear();
			vkGenerics::ResetCommandPool(*m_Device, *m_Pools[ModeIndex::SWAPCHAIN], VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
			break;
		default:
			vkDebug::ThrowError(L"GPUHandle::Execute()", L"Invalid GPUHandleMode", L"N/A");
			break;
		}
	}

	void ExecuteCompute();



private:

	vkInstanceContainer* m_Instance = nullptr;
	VkPhysicalDevice* m_PhysicalDevice = nullptr;
	VkDevice* m_Device = nullptr;
	std::unordered_map<vkGenerics::vkQueueFamilyBit, VkQueue> m_Queues;
	std::unordered_map<vkGenerics::vkQueueFamilyBit, VkCommandPool*> m_Pools;
	std::unordered_map <vkGenerics::vkQueueFamilyBit, std::vector<VkCommandBuffer>> m_CmdBuffers;
	std::vector<VkEvent> m_SystemEvents;

	std::vector<VkCommandBuffer> m_ComputeCmdBuffers;
	std::vector<VkCommandBuffer> m_ComputeGfxBuffers;
	std::vector<VkCommandBuffer> m_ComputeSwpChnBuffers;


	std::unordered_map<vkGenerics::vkQueueFamilyBit, int> m_QueueIndex;
};

struct MemoryLocation
{	
	uint32_t m_Size;
	uint32_t m_Offset;
};

//https://stackoverflow.com/questions/65524/generating-a-unique-id-in-c
union MemoryID
{
	MemoryLocation m_location;
	uint64_t m_ID;
};

class HostAccessablePtr
{
public:

	uint32_t GetOffset() { return memoryID.m_location.m_Offset; }

	uint32_t GetSize() { return memoryID.m_location.m_Size; }

	VkResult& GetState() { return m_State; };

	void* GetHome() { return m_Home; };

private: 

	friend class HostAccessableMemoryPool;

	void* m_Home;
	MemoryID memoryID;
	VkResult m_State;
};

class HostAccessableMemoryPool
{
public:
	HostAccessableMemoryPool(GPUHandle* handle, uint32_t size, VkBufferUsageFlags flags, std::vector<uint32_t>& indices);

	HostAccessableMemoryPool(VkPhysicalDevice* physDev, VkDevice* device, uint32_t size, VkBufferUsageFlags flags, std::vector<uint32_t>& indices);

	~HostAccessableMemoryPool()
	{
		vkGenerics::Destroy(*m_Device, *m_Buffer);
		vkGenerics::Destroy(*m_Device, *m_MemoryBuffer);

		m_Buffer = VK_NULL_HANDLE;
		m_MemoryBuffer = VK_NULL_HANDLE;
	};

	HostAccessablePtr MemAlloc(void* data, uint32_t size);

	void MemDelloc(HostAccessablePtr& location);

	template<typename T>
	T* ReadPtr(MemoryID& location)
	{
		if (location.m_location.m_Size != sizeof(T))
		{
			vkDebug::ThrowError(L"HostAccessableMemoryPool::ReadPtr()", L"Requested datat type size and ptr size do not match", L"Ensure that you have the correct data type");
			return nullptr;
		}

		if (find(location.m_ID) == -1)
		{
			vkDebug::ThrowError(L"HostAccessableMemoryPool::ReadPtr()", L"Requested datat does not exist on this pool", L"Ensure that you have the correct pool");
			return nullptr;
		}

		void* memoryLoaction = nullptr;
		T* data = nullptr;
		VkResult result = vkGenerics::MapMemoryToPointer(*m_Device, *m_MemoryBuffer, location.m_location.m_Offset, &memoryLoaction, location.m_location.m_Size);
		memcpy((void*)data, memoryLoaction, location.m_location.m_Size);
		vkGenerics::UnmapMemory(*m_Device, *m_MemoryBuffer);
		return data;
	}


	void WriteData(uint32_t offset, uint32_t size, void* dataIn)
	{
		void* memoryLoaction = nullptr;
		VkResult result = vkGenerics::MapMemoryToPointer(*m_Device, *m_MemoryBuffer, offset, &memoryLoaction, size);
		memcpy((void*)memoryLoaction, dataIn, size);
		vkGenerics::UnmapMemory(*m_Device, *m_MemoryBuffer);
	}

	void CopyData(uint32_t offset, uint32_t size, void* dataOut)
	{
		void* memoryLoaction = nullptr;
		VkResult result = vkGenerics::MapMemoryToPointer(*m_Device, *m_MemoryBuffer, offset, &memoryLoaction, size);
		memcpy((void*)dataOut, memoryLoaction, size);
		vkGenerics::UnmapMemory(*m_Device, *m_MemoryBuffer);
	}



	uint64_t find(HostAccessablePtr test)
	{
		for (uint64_t val : m_ActivePtr)
		{
			if (test.memoryID.m_ID == val)
			{
				return val;
			}
		}
		return -1;
	}

	uint64_t find(uint64_t test)
	{
		for (uint64_t val : m_ActivePtr)
		{
			if (test == val)
			{
				return val;
			}
		}
		return -1;
	}

	VkBuffer* ReturnVkBuffer() { return m_Buffer; };

	uint32_t GetMinimumGPUOffset() { return m_MinimumWriteSize; }

private:
	
	uint16_t m_Noise = 0;
	uint16_t m_Counter = 0;
	VkDevice* m_Device = nullptr;
	VkBuffer* m_Buffer = VK_NULL_HANDLE;
	VkDeviceMemory* m_MemoryBuffer = VK_NULL_HANDLE;
	uint32_t m_Size = 0;
	uint32_t m_MinimumWriteSize = 0;
	uint32_t m_LastOffset;
	uint32_t m_LastSize;

	std::vector<uint64_t> m_DeallocatedSpace;
	std::vector<uint64_t> m_ActivePtr;
};

class GPUMemoryHandle
{
public:

	//Creates a GPU memory handle based on an already existing device
	GPUMemoryHandle(GPUHandle* handle, uint32_t memoryPoolSize);

	//Creates a GPU memory handle using a new device with the vk external memory extension
	GPUMemoryHandle(GPUHandle* handle, uint32_t memoryPoolSize, std::vector<const char*> extensions);

	~GPUMemoryHandle();

	HostAccessablePtr WriteDataToCpuVisablePool(void* data, uint32_t size);

	void FreeHostAccessablePointer(HostAccessablePtr& ptr);

	void Declutter()
	{
	}

protected:
	VkDevice* m_Device = nullptr;
	VkPhysicalDevice* m_PhysicalDevice = nullptr;
	int m_QueueIndex;
	VkQueue* m_MemoryQueue = nullptr;
	VkCommandPool* m_MemoryCommandPool = nullptr;
	uint32_t m_PoolSize = 0;
	uint32_t m_MinimumGPUOffset = 0;
	bool m_Independent = false;

	std::vector<HostAccessableMemoryPool*> m_CPUAccessableMemoryPools;
	std::vector<HostAccessableMemoryPool*> m_CPUAccessableVertexMemoryPools;
	std::vector<HostAccessableMemoryPool*> m_CPUAccessableIndexMemoryPools;
	std::vector<uint32_t> m_DeviceQueueIndicies;

};

/// <summary>
/// The role of this class is to provide a descriptor set for a kernels. 
/// </summary>
class GPUMemoryAdapter
{
public:
	GPUMemoryAdapter(GPUHandle* handle, int addressableBuffers);
	~GPUMemoryAdapter();

	/// <summary>
	/// This updates the descriptor sets to point to a different place in the GPUs memory.
	/// </summary>
	/// <param name="newMemory"></param>
	void AttachMemory(HostAccessablePtr& newMemory);

	/// <summary>
	/// Returns the Descriptor Set Layout
	/// </summary>
	/// <returns></returns>
	VkDescriptorSetLayout* GetDescriptorSetLayout() { return &m_DescriptorSetLayout; }

	/// <summary>
	/// Returns the Descriptor Sets
	/// </summary>
	/// <returns></returns>
	VkDescriptorSet* GetDescriptorSet() { return &m_DescriptorSet; }

	/// <summary>
	/// Writes a passed pointer onto the memory pool
	/// </summary>
	/// <param name="data"></param>
	void WriteData(void* data);

	/// <summary>
	/// Writes a passed pointer onto the memory pool up to a size limit
	/// </summary>
	/// <param name="data"></param>
	void WriteData(void* data, uint32_t size);

	/// <summary>
	/// Writes a passed pointer onto the memory pool up to a size limit
	/// </summary>
	/// <param name="data"></param>
	void WriteData(void* data, uint32_t offset, uint32_t size);

	/// <summary>
	/// Reads data from the memory pool
	/// </summary>
	/// <param name="data"></param>
	void ReadData(void* data);

	/// <summary>
	/// Reads data from the memory pool
	/// </summary>
	/// <param name="data"></param>
	void ReadData(void* data, uint32_t size);

	/// <summary>
	/// Reads data from the memory pool
	/// </summary>
	/// <param name="data"></param>
	void ReadData(void* data, uint32_t offset, uint32_t size);

private:

	VkDevice* m_Device;
	VkDescriptorSetLayout m_DescriptorSetLayout;
	VkDescriptorSet m_DescriptorSet;
	VkDescriptorPool m_DescriptorPool;
	HostAccessablePtr m_Port;
	int m_AddressableBuffers = 0;
};


class GPUKernel
{
public:
	GPUKernel(GPUHandle* handle, std::wstring shaderFileLocation, std::string shaderEntryPoint);
	~GPUKernel();

	void LinkMemory(GPUMemoryAdapter* adapter, int PipelineFlags);

	void RequestExecution(GPUHandle* handle, VkExecuteHint executionHint);

	void RequestExecution2(GPUHandle* handle, VkExecuteHint executionHint);

	void WriteToMemory(void* data, uint32_t size);

	void WriteElementToMemory(void* data, uint32_t size, uint32_t offsetOfElement);

	void ReadFromMemory(void* data, uint32_t size);

	void ReadElementFromMemory(void* data, uint32_t size, uint32_t offsetOfElement);

	VkPipeline* GetPipeline() { return m_Pipeline; }

	VkPipelineLayout* GetPipelineLayout() { return m_PipelineLayout; }

	VkDescriptorSet* GetDescriptorSet() { return m_MemoryAdatper->GetDescriptorSet(); }

private:

	VkDevice* m_Device = nullptr;
	VkPipelineShaderStageCreateInfo* m_ShaderInfo = nullptr;
	GPUMemoryAdapter* m_MemoryAdatper = nullptr;
	VkPipeline* m_Pipeline = nullptr;
	VkPipelineLayout* m_PipelineLayout = nullptr;
	std::string m_EntryPointName;
};


class GPUExecutionSchedule
{
public:
	GPUExecutionSchedule(GPUHandle* handle, ModeIndex cmdBufferType)
	{
		VkResult result;
		m_Device = handle->GetDevice();
		m_CommandBuffer = new VkCommandBuffer();
		result = vkGenerics::CreateCommandBuffer(*m_Device, *m_CommandBuffer, *handle->GetPool(cmdBufferType), VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		result = vkGenerics::BeginCommandBuffer(*m_CommandBuffer, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
	}
	~GPUExecutionSchedule()
	{
		for (VkEvent& events : m_Events)
		{
			vkGenerics::Destroy(*m_Device, events);
		}
		vkGenerics::Destroy(*m_Device, *m_CommandBuffer);

		m_Device = nullptr;
		delete m_CommandBuffer;
	}

	void CreateNewGroup()
	{
		m_Events.push_back(VkEvent());
		vkGenerics::CreateVkEvent(*m_Device, m_Events.back());
		vkGenerics::SetComputeEvent(*m_CommandBuffer, m_Events.back());
	}

	void WaitOnGroup(int group)
	{
		vkGenerics::SetComputeEvent(*m_CommandBuffer, m_Events.back());
	}

	void RequestExecution(GPUKernel* kernel, VkExecuteHint executionHint)
	{
		vkGenerics::BindComputePipeline(*m_CommandBuffer, *kernel->GetPipeline());
		vkGenerics::BindDescriptorSetsToCompute(*m_CommandBuffer, *kernel->GetPipelineLayout(), *kernel->GetDescriptorSet());
		vkGenerics::DispatchComputeCommands(*m_CommandBuffer, executionHint.m_Xcount, executionHint.m_Ycount, executionHint.m_Zcount);
	}

	VkCommandBuffer* ReturnCommandBuffer()
	{
		VkResult result;
		result = vkGenerics::EndCommandBuffer(*m_CommandBuffer);
		return m_CommandBuffer;
	}

private:

	std::vector<VkEvent> m_Events;
	VkCommandBuffer* m_CommandBuffer;
	VkDevice* m_Device;
};







//class GPUComputeMemoryHandle : GPUMemoryHandle 
//{
//public:
//	GPUComputeMemoryHandle() {};
//
//	template<typename T>
//	GPUPointer WriteToCpuVisablePool();
//
//
//	GPUPointer WriteToCpuVisablePool(void* data, uint32_t size);
//
//	void CpuVisableFreePtr(GPUPointer& ptr);
//
//	void Declutter()
//	{
//
//	}
//
//
//	~GPUComputeMemoryHandle()
//	{
//		for (size_t i = 0; i < m_CPU_GPU_MemoryPool.size(); i++)
//		{
//			delete m_CPU_GPU_MemoryPool[i];
//		}
//
//	}
//
//private:
//
//	//Binary Search tree to store gaps in memory which can then be used to locate a suitable gap for best fit based to the size of the gap
//	std::vector<ManagedGPUMemoryPool*> m_CPU_GPU_MemoryPool;
//};