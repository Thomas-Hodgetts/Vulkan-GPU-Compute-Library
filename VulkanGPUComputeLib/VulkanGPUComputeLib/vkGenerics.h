#pragma once

#include <vulkan/vulkan.hpp>
#include <memory>
#include <string>
#include <optional>
#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>
#include <array>
#include <set>
#include <fstream>
#include  <type_traits>

namespace vkDebug
{
	//		throw std::runtime_error("");
	inline static VkDebugReportCallbackEXT CreateDebugCallback(const VkInstance* instance);
	inline void ThrowError(std::wstring functionName, std::wstring actualError, std::wstring possibleRemidies) { std::wcout << L"ERROR: Function " + functionName + L" has thrown an error because " + actualError + L". Possible fix if filled in: " + possibleRemidies << std::endl; };

	struct MemoryErrorDetails
	{
		std::wstring bufferName = L"";
		void* bufferLocation = nullptr;
		std::size_t bufferSize = 0;
		std::size_t newDataSize = 0;
	};

	inline void ThrowMemoryError(std::wstring operationType, MemoryErrorDetails details)
	{
		std::wcout << L"MEMORY ERROR: " + operationType + " operation. " L" Buffer " + details.bufferName + L". The buffers size is  " + std::to_wstring(details.bufferSize); std::wcout << L" but the new datas size is ." + std::to_wstring(details.newDataSize) << std::endl;
	};

	inline void ThrowWarning(std::wstring functionName, std::wstring actualWarning) { std::wcout << L"Warning: Function " + functionName + L" has thrown an warning because " + actualWarning << std::endl; };

	inline VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)
	{
		std::cout << pMessage << std::endl;
		std::cout << "" << std::endl;
		return VK_FALSE;
	}


	//inline VKAPI_ATTR void VKAPI_CALL DestroyDebugReportCallback(VkInstance instance, VkDebugUtilsMessengerEXT messenger, VkAllocationCallbacks const* pAllocator)
	//{
	//	return pfnVkDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
	//}

	/// <summary>
	/// Destorys a Vulkan Object
	/// </summary>
	inline VKAPI_ATTR void VKAPI_CALL Destroy(VkInstance& instance, VkDebugReportCallbackEXT callback)
	{
		PFN_vkDestroyDebugReportCallbackEXT destroyFunc = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"));
		if (!destroyFunc)
		{
			ThrowError(L"VkDebug::Destroy()", L"Unable to locate extension", L"Check error message");
		}
		destroyFunc(instance, callback, nullptr);
	};
}

namespace vkGenerics
{

	template<typename DST, typename SRC>
	DST UnsignedRecast(SRC value)
	{
		static_assert(std::is_unsigned_v<SRC> && std::is_unsigned_v<DST>, "Only unsigned types can be cast using this function");
		DST result = static_cast<DST>(value);
		assert(result == value);
		return result;
	}


	enum class vkQueueFamilyBit
	{
		BASE = 1,
		GRAPHICS = 2,
		SWAPCHAIN = 4,
		COMPUTE = 8,
		TRANSFER = 16
	};

	inline constexpr vkQueueFamilyBit operator~ (vkQueueFamilyBit a) { return (vkQueueFamilyBit)~(int)a; }
	inline constexpr vkQueueFamilyBit operator| (vkQueueFamilyBit a, vkQueueFamilyBit b) { return (vkQueueFamilyBit)((int)a | (int)b); }
	inline constexpr vkQueueFamilyBit operator& (vkQueueFamilyBit a, vkQueueFamilyBit b) { return (vkQueueFamilyBit)((int)a & (int)b); }
	inline constexpr vkQueueFamilyBit operator^ (vkQueueFamilyBit a, vkQueueFamilyBit b) { return (vkQueueFamilyBit)((int)a ^ (int)b); }
	inline constexpr vkQueueFamilyBit& operator|= (vkQueueFamilyBit& a, vkQueueFamilyBit b) { return (vkQueueFamilyBit&)((int&)a |= (int)b); }
	inline constexpr vkQueueFamilyBit& operator&= (vkQueueFamilyBit& a, vkQueueFamilyBit b) { return (vkQueueFamilyBit&)((int&)a &= (int)b); }
	inline constexpr vkQueueFamilyBit& operator^= (vkQueueFamilyBit& a, vkQueueFamilyBit b) {
		return (vkQueueFamilyBit&)((int&)a ^= (int)b);
	}
	inline constexpr vkQueueFamilyBit& operator< (vkQueueFamilyBit& a, int b) { return (vkQueueFamilyBit&)((int&)a = (int)b); }

	enum class vkQueueFamilyBitAccumulated
	{
		BASE = 1,
		GRAPHICS = (int)(vkQueueFamilyBit::GRAPHICS | vkQueueFamilyBit::BASE),
		SWAPCHAIN = (int)(vkQueueFamilyBit::SWAPCHAIN | vkQueueFamilyBit::BASE),
		COMPUTE = (int)(vkQueueFamilyBit::COMPUTE | vkQueueFamilyBit::BASE),
		SWAPCHAIN_GRAPHICS = (int)(vkQueueFamilyBit::SWAPCHAIN | vkQueueFamilyBit::GRAPHICS | vkQueueFamilyBit::BASE),
		GRAPHICS_COMPUTE = (int)(vkQueueFamilyBit::GRAPHICS | vkQueueFamilyBit::COMPUTE | vkQueueFamilyBit::BASE),
		SWAPCHAIN_GRAPHICS_COMPUTE = (int)(vkQueueFamilyBit::SWAPCHAIN | vkQueueFamilyBit::GRAPHICS | vkQueueFamilyBit::COMPUTE  | vkQueueFamilyBit::BASE)
	};
	inline constexpr vkQueueFamilyBitAccumulated& operator< (vkQueueFamilyBitAccumulated& a, vkQueueFamilyBit b) { return (vkQueueFamilyBitAccumulated&)((int&)a = (int)b); }

	enum class vkVsync
	{
		NO_VSYNC = VK_PRESENT_MODE_IMMEDIATE_KHR,
		REFRESH_RATE_CAP = VK_PRESENT_MODE_MAILBOX_KHR,
		VSYNC_UNLIMITED = VK_PRESENT_MODE_MAILBOX_KHR,
		VSYNC_DYNAMIC = VK_PRESENT_MODE_FIFO_RELAXED_KHR,
	};

	enum class vkFrameBufferMode
	{
		NO_BUFFER = 1,
		DOUBLE_BUFFER,
		TRIPLE_BUFFER
	};

	struct vkSwaphcainCapabilities
	{
		VkSurfaceCapabilitiesKHR m_SurfaceCapabilities;
		std::vector<VkSurfaceFormatKHR> m_Formats;
		std::vector<VkPresentModeKHR> m_PresentationModes;
	};

	/// Instance Functions

	VkResult CreateInstance(VkInstanceCreateInfo createInfo, VkInstance* instance);

	VkResult CreateDebugInstance(VkInstanceCreateInfo createInfo, VkInstance* instance, VkDebugReportCallbackEXT* callback);

	///Execute Functions

	void DispatchComputeCommands(VkCommandBuffer& buffer, uint32_t size_x, uint32_t size_y, uint32_t size_z);

	void TestDispatchCompute(VkCommandBuffer& buffer);

	VkResult ExecuteQueue(const VkQueue& queue, const VkCommandBuffer* cmdBuffer, uint32_t cmdBufferCount, VkSemaphore* sigSemaphores, uint32_t sigSemaphoresCount, VkSemaphore* waitSemaphores, uint32_t waitSemaphoresCount, VkPipelineStageFlags* flags, VkFence& fence);
	
	VkResult ExecuteQueue(const VkQueue& queue, std::vector<VkCommandBuffer>& cmdBuffers, std::vector<VkSemaphore>& sigSemaphores, std::vector<VkSemaphore>& waitSemaphores, VkPipelineStageFlags* flags);

	/// Device Functions

	VkResult CreatePhysicalDevice(VkPhysicalDevice& m_PhysicalDev, VkInstance* instance, std::vector<const char*> requestedExtensions);

	VkResult CreateLogicalDevice(VkDevice& device, VkPhysicalDevice& physicalDev, std::vector<const char*> requestedExtensions, std::unordered_map<vkGenerics::vkQueueFamilyBit, int>& familes);

	VkResult CreateLogicalDevice(VkDevice& device, VkPhysicalDevice& physicalDev, std::vector<const char*> requestedExtensions, std::vector<int> queueIndicies);

	VkResult CreateLogicalDevice(VkDevice& device, VkPhysicalDevice& physicalDev, std::vector<const char*> requestedExtensions, int queueIndex);

	bool SuitableDevice(const VkPhysicalDevice& device, std::vector<const char*> requestedExtensions);

	bool CheckDeviceExtensionSupport(const VkPhysicalDevice& device, std::vector<const char*> requestedExtensions);

	VkPhysicalDeviceProperties GetPhysicalDeviceProperties(const VkPhysicalDevice& device);

	///Shaders

	VkResult CreateShaderModule(const std::wstring fileLocation, const VkDevice& device, VkShaderModule& module);

	///Frame buffer

	VkResult CreateFramebuffer(const VkDevice& device, VkFramebuffer& newBuffer, VkRenderPass& renderPass, VkImageView& imageView, VkExtent2D& extent);

	///Swapchain

	struct vkSwapchainImages
	{
		VkImage m_Image;
		VkImageView m_ImageView;
	};

	VkExtent2D SelectIdealImageResolution(const VkSurfaceCapabilitiesKHR& surfaceCapabilities, int width, int height);

	VkResult GetSwapchainCapabilities(const VkPhysicalDevice& device, const VkSurfaceKHR& surface, vkSwaphcainCapabilities& swapchainDetails);

	VkResult CreateSwapchain(const VkDevice& device, const VkPhysicalDevice& physicalDevice, VkSwapchainKHR& swapchain, vkVsync vsyncMode, VkSurfaceFormatKHR selectedFormat, VkExtent2D extent, VkSurfaceKHR& surface, int graphicsFam, int swapchainFam, vkFrameBufferMode mode);

	VkResult GetVkImages(const VkDevice& device, VkSwapchainKHR& swapchain, VkFormat& format, std::vector<vkSwapchainImages>& images);

	VkResult CreateImageView(const VkDevice& device, VkImage& image, VkImageView& view, VkFormat format, VkImageAspectFlags aspectFlags);

	///Queues

	void CreateQueue(VkDevice device, VkQueue& queue, int queueIndex);

	void CreateQueues(VkDevice device, std::unordered_map<int, VkQueue>& Queues);

	VkResult CreateQueues(VkDevice device, std::wstring deviceNames, std::unordered_map<std::wstring, VkQueue>& queues, std::unordered_map<vkQueueFamilyBit, int>& familes, vkQueueFamilyBitAccumulated bits);

	VkResult AquireFamilyIndices(const vkQueueFamilyBitAccumulated& bits, const VkPhysicalDevice& device, std::unordered_map<vkQueueFamilyBit, int>& familyIndicies );

	VkResult AquireFamilyIndices(const vkQueueFamilyBitAccumulated& bits, const VkPhysicalDevice& device, std::unordered_map<vkQueueFamilyBit, int>& outMap);

	VkResult AquireQueueIndex(const VkPhysicalDevice& device, const vkQueueFamilyBit& bit, int& indexOut);

	VkResult AquireQueueIndex(const VkPhysicalDevice& device, const VkQueueFlagBits& bit, int& indexOut);

	///Command Pool & Buffer

	VkResult CreateCommandPool(VkDevice& device, VkCommandPool& cmdPool, vkQueueFamilyBit bit, std::map<vkGenerics::vkQueueFamilyBit, int>& familes);

	VkResult CreateCommandPool(VkDevice& device, VkCommandPool& cmdPool, int& queueIndex);

	VkResult CreateCommandPool(VkDevice& device, VkCommandPool& cmdPool, int queueFamilyIndex, VkCommandPoolCreateFlags flags);

	VkResult ResetCommandPool(const VkDevice& device, const VkCommandPool& cmdPool);

	VkResult ResetCommandPool(const VkDevice& device, const VkCommandPool& cmdPool, VkCommandPoolResetFlagBits flag);

	VkResult CreateCommandBuffer(const VkDevice& device, VkCommandBuffer* buffers, VkCommandBufferAllocateInfo commandBufferInfo);

	VkResult CreateCommandBuffer(const VkDevice& device, VkCommandBuffer& buffer, VkCommandPool& commandPool, VkCommandBufferLevel level);

	VkResult CreateCommandBuffer(const VkDevice& device, std::vector<VkCommandBuffer>& buffers, VkCommandPool& commandPool, VkCommandBufferLevel level);

	template<size_t N>
	VkResult CreateCommandBuffer(const VkDevice& device, std::array<VkCommandBuffer, N> buffers, VkCommandBufferAllocateInfo commandBufferInfo,  VkCommandPool& commandPool, VkCommandBufferLevel level);

	VkResult BeginCommandBuffer(VkCommandBuffer& commandBuff);

	VkResult BeginCommandBuffer(VkCommandBuffer& commandBuff, VkCommandBufferUsageFlagBits bits);

	VkResult BeginCommandBuffer(VkCommandBuffer& commandBuff, VkCommandBufferBeginInfo* infoType);

	VkResult EndCommandBuffer(VkCommandBuffer& commandBuff);


	///Descriptors

	VkDescriptorSetLayoutCreateInfo PopulateVkDescriptorSetLayoutCreateInfo(uint32_t size, int descriptorCount, VkDescriptorType type, VkShaderStageFlags flags, VkSampler* sampler);

	VkResult CreateDescriptorSetLayout(const VkDevice& device, VkDescriptorSetLayout& setLayout, VkDescriptorSetLayoutCreateInfo setLayoutInfo);

	VkResult CreateDescriptorSet(const VkDevice& device, VkDescriptorSet& setLayout, VkDescriptorSetAllocateInfo setLayoutInfo);

	VkResult CreateDescriptorSet(const VkDevice& device, VkDescriptorSet& setLayout, VkDescriptorPool& pool, int descLayoutCount, VkDescriptorSetLayout* layouts);

	VkResult CreateDescriptorPool(const VkDevice& device, VkDescriptorPool& descriptorPool, VkDescriptorPoolCreateInfo descPoolCreateInfo);

	VkResult CreateDescriptorPool(const VkDevice& device, VkDescriptorPool& descriptorPool, int poolSize, std::vector<int> descriptorCounts, VkDescriptorType type);

	void BindDescriptorSetsToCompute(const VkCommandBuffer& commandBuff, const VkPipelineLayout& pipelineLayout, const VkDescriptorSet& descSet);

	void BindDescriptorSetsToGraphics(const VkCommandBuffer& commandBuff, const VkPipelineLayout& pipelineLayout, const VkDescriptorSet& descSet);

	void BindDescriptorSetsToCompute(const VkCommandBuffer& commandBuff, const VkPipelineLayout& pipelineLayout, const VkDescriptorSet* descSet, uint32_t setCount, uint32_t firstSet);
																																																																			  
	void BindDescriptorSetsToGraphics(const VkCommandBuffer& commandBuff, const VkPipelineLayout& pipelineLayout, const VkDescriptorSet* descSet, uint32_t setCount, uint32_t firstSet);

	struct UpdateDescData
	{
		VkBuffer* m_Buffer;
		uint32_t m_Offset;
		VkDescriptorType m_Type;
		VkDescriptorSet* m_DstSet;
		uint32_t m_DstCount;
		uint32_t m_Binding;
		VkBufferView* m_TextelBuff = nullptr;
		VkDescriptorImageInfo* m_ImageInfo = nullptr;
		void* m_pNext = VK_NULL_HANDLE;
	};

	void UpdateDescriptorBufferInfo(const UpdateDescData& updateData, VkDevice& device);

	///Pipelines

	VkResult CreateComputePipeline(const VkDevice& device, VkPipeline& pipeline, VkComputePipelineCreateInfo pipelineInfo);

	void BindComputePipeline(const VkCommandBuffer& commandBuff, const VkPipeline& pipeline);

	VkResult CreateGraphicsPipeline(const VkDevice& device, VkPipeline& pipeline, VkGraphicsPipelineCreateInfo pipelineInfo);

	void BindGraphicsPipeline(const VkCommandBuffer& commandBuff, const VkPipeline& pipeline);

	VkResult CreatePipelineLayout(const VkDevice& device, VkPipelineLayout& layout,VkPipelineLayoutCreateInfo pipelineLayoutInfo);

	VkResult CreatePipelineLayout(const VkDevice& device, VkPipelineLayout& layout, std::vector<VkDescriptorSetLayout> descSets);

	///Synchronisation

	VkResult CreateSemaphore(const VkDevice& device, VkSemaphore& semaphore,VkSemaphoreCreateFlags flags);

	VkResult CreateFence(const VkDevice& device, VkFence& fence, VkFenceCreateFlags flags);

	VkResult CreateVkEvent(const VkDevice& device, VkEvent& vkEvent);

	//Replace
	void SetComputeEvent(VkCommandBuffer& buffer, VkEvent& vkEvent);
	//Replace
	void WaitOnEvent(VkCommandBuffer& buffer, VkEvent& vkEvent);

	///Memory Functions

	uint32_t FindMemoryIndex(uint32_t allowedTypes, VkMemoryPropertyFlags propertyFlags, const VkPhysicalDevice& device);

	void GetBufferMemoryRequirements(const VkDevice& device, VkBuffer& buffer, VkMemoryRequirements& memRequirments);

	VkResult CreateMemoryBuffer(const VkDevice& device, VkBuffer& buffer, VkBufferCreateInfo& bufferInfo);

	VkResult CreateMemoryBuffer(const VkDevice& device, VkBuffer& buffer, uint32_t size, VkBufferUsageFlags usageMode, VkSharingMode  sharingMode, uint32_t queueFamilyIndexCount, uint32_t* queueFamilyIndices);

	VkResult AllocateMemory(const VkDevice& device, VkDeviceMemory& buffer, VkMemoryAllocateInfo allocateInfo);

	VkResult AllocateMemory(const VkDevice& device, VkDeviceMemory& deviceMemory, VkBuffer& buffer, uint32_t memoryIndex);

	VkResult BindBufferToMemory(const VkDevice& device, VkDeviceMemory& deviceMemory, VkBuffer& buffer);

	VkResult BindBufferToMemory(const VkDevice& device, VkDeviceMemory& deviceMemory, VkBuffer& buffer, uint32_t offset);

	VkResult MapMemoryToPointer(const VkDevice& device, VkDeviceMemory& deviceMemory, void* address, uint32_t sizeOfBuffer);

	VkResult MapMemoryToPointer(const VkDevice& device, VkDeviceMemory& deviceMemory, VkDeviceSize offset, void* address, uint32_t sizeOfBuffer);

	VkResult MapMemoryToPointer(const VkDevice& device, VkDeviceMemory& deviceMemory, void** address, uint32_t sizeOfBuffer);

	VkResult MapMemoryToPointer(const VkDevice& device, VkDeviceMemory& deviceMemory, VkDeviceSize offset, void** address, uint32_t sizeOfBuffer);

	void UnmapMemory(const VkDevice& device, VkDeviceMemory& deviceMemory);

	void BindComputeMemory(const VkCommandBuffer& commandBuff, const VkPipelineLayout& pipeLayout, const VkDescriptorSet* descSets);

	///Graphics

	VkResult CreateRenderpass(VkDevice& device, VkRenderPass& renderpass, VkFormat& format);

	VkResult RecordGraphicsCommands(VkPipeline& pipeline, VkCommandBuffer& buffer, VkRenderPass& renderPass, VkFramebuffer& frameBuffer, VkExtent2D& extent, VkBuffer& VertexBuffer, uint32_t vertexCount);

	VkResult PresentImage(VkQueue& queue, VkSwapchainKHR& swapchain, uint32_t currentFrame, std::vector<VkSemaphore>& semaphore);

	///Swapchain

	VkResult AquireNextFrame(const VkDevice& device, VkSwapchainKHR& swapchain, VkSemaphore& semaphore, uint32_t newFrameIndex);

	///Sync

	VkResult WaitOnFence(const VkDevice& device, VkFence& fence);

	VkResult WaitOnFence(const VkDevice& device, VkFence& fence, VkBool32& waitAll, uint64_t waitTime);

	VkResult ResetFence(const VkDevice& device, VkFence& fence);

	///Destory Functions

	/// <summary>   
	/// Destorys a Vulkan Object
	/// </summary>
	static void Destroy(VkInstance instance) { vkDestroyInstance(instance, nullptr); };

	/// <summary>
	/// Destorys a Vulkan Object
	/// </summary>
	static void Destroy(VkDevice device) { vkDestroyDevice(device, nullptr); };

	/// <summary>
	/// Destorys a VkSurfaceKHR
	/// </summary>
	static void Destroy(VkInstance instance, VkSurfaceKHR surface) { vkDestroySurfaceKHR(instance, surface, nullptr); };

	/// <summary>
	/// Destorys a Vulkan Object
	/// </summary>
	static void Destroy(VkDevice device, VkSwapchainKHR swapchain) { vkDestroySwapchainKHR(device, swapchain, nullptr); };

	/// <summary>
	/// Destorys a Vulkan Object
	/// </summary>
	static void Destroy(VkDevice device, std::vector<VkImageView> swapchainImages)
	{
		for (VkImageView i : swapchainImages)
		{
			vkDestroyImageView(device, i, nullptr);
		}
	};

	/// <summary>
	/// Destorys a Vulkan Object
	/// </summary>
	static void Destroy(VkDevice device, VkImageView swapchainImages) { vkDestroyImageView(device, swapchainImages, nullptr); };

	/// <summary>
	/// Destorys a Vulkan Object
	/// </summary>
	static void Destroy(VkDevice device, VkImage image) { vkDestroyImage(device, image, nullptr); };

	/// <summary>
	/// Destorys a Vulkan Object
	/// </summary>
	static void Destroy(VkDevice device, VkRenderPass pass) { vkDestroyRenderPass(device, pass, nullptr); };

	/// <summary>
	/// Destorys a Vulkan Object
	/// </summary>
	static void Destroy(const VkDevice& device, VkShaderModule shaderModule) { vkDestroyShaderModule(device, shaderModule, nullptr); };

	/// <summary>
	/// Destorys a Vulkan Object
	/// </summary>
	static void Destroy(const VkDevice& device, VkCommandPool& comPool) { vkDestroyCommandPool(device, comPool, nullptr); };

	/// <summary>
	/// Destorys a Vulkan Object
	/// </summary>
	static void Destroy(const VkDevice& device, VkCommandBuffer& comBuff) { };

	/// <summary>
	/// Destorys a Vulkan Descriptor Pool
	/// </summary>
	static void Destroy(const VkDevice& device, VkDescriptorPool& descPool) { vkDestroyDescriptorPool(device, descPool, nullptr); };

	/// <summary>
	/// Destorys a Vulkan Descriptor Pool
	/// </summary>
	static void Destroy(const VkDevice& device, VkDescriptorSetLayout& descLayout) { vkDestroyDescriptorSetLayout(device, descLayout, nullptr); };

	/// <summary>
	/// Destorys a Vulkan	Semaphore
	/// </summary>
	static void Destroy(const VkDevice& device, VkSemaphore& semaphore) { vkDestroySemaphore(device, semaphore, nullptr); };

	/// <summary>
	/// Destorys a Vulkan Fence
	/// </summary>
	static void Destroy(const VkDevice& device, VkFence& fence) { vkDestroyFence(device, fence, nullptr); };

	/// <summary>
/// Destorys a Vulkan Event
/// </summary>
	static void Destroy(const VkDevice& device, VkEvent& event) { vkDestroyEvent(device, event, nullptr); };

	/// <summary>
	/// Destorys a Vulkan Pipeline Layout
	/// </summary>
	static void Destroy(const VkDevice& device, VkPipelineLayout& pipelineLayout) { vkDestroyPipelineLayout(device, pipelineLayout, nullptr); };

	/// <summary>
	/// Destorys a Vulkan Fence
	/// </summary>
	static void Destroy(const VkDevice& device, VkPipeline& pipeline) { vkDestroyPipeline(device, pipeline, nullptr); };

	/// <summary>
	/// Destorys a VkBuffer
	/// </summary>
	static void Destroy(const VkDevice& device, VkBuffer& buffer) { vkDestroyBuffer(device, buffer, nullptr); };

	/// Destorys a VkDeviceMemory
	/// </summary>
	static void Destroy(const VkDevice& device, VkDeviceMemory& buffer) { vkFreeMemory(device, buffer, nullptr); };

	/// Destorys a Framebuffer
	/// </summary>
	static void Destroy(const VkDevice& device, VkFramebuffer& buffer) { vkDestroyFramebuffer(device, buffer, nullptr); };




}




