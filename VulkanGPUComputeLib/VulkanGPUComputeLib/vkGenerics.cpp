#include "vkGenerics.h"



void vkGenerics::DispatchComputeCommands(VkCommandBuffer& buffer, uint32_t size_x, uint32_t size_y, uint32_t size_z)
{
	vkCmdDispatch(buffer, size_x, size_y, size_z);
}

void vkGenerics::TestDispatchCompute(VkCommandBuffer& buffer)
{
	vkCmdDispatch(buffer, 1, 1, 1);
}

VkResult vkGenerics::ExecuteQueue(const VkQueue& queue, const VkCommandBuffer* cmdBuffer, uint32_t cmdBufferCount, VkSemaphore* sigSemaphores, uint32_t sigSemaphoresCount, VkSemaphore* waitSemaphores, uint32_t waitSemaphoresCount, VkPipelineStageFlags* flags, VkFence& fence)
{
	VkResult result;

	VkSubmitInfo submitInfo = {};

	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = VK_NULL_HANDLE;
	submitInfo.pCommandBuffers = cmdBuffer;
	submitInfo.commandBufferCount = cmdBufferCount;
	submitInfo.pSignalSemaphores = sigSemaphores;
	submitInfo.signalSemaphoreCount = sigSemaphoresCount;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.waitSemaphoreCount = waitSemaphoresCount ;
	submitInfo.pWaitDstStageMask = flags;

	result = vkQueueSubmit(queue, 1, &submitInfo, fence);

	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L"vkGenerics::ExecuteQueue()", L"Unable to submit Queue for execution", L"Check inputted data and error message output");
	}

	return result;
}

VkResult vkGenerics::ExecuteQueue(const VkQueue& queue, std::vector<VkCommandBuffer>& cmdBuffers, std::vector<VkSemaphore>& sigSemaphores, std::vector<VkSemaphore>& waitSemaphores, VkPipelineStageFlags* flags)
{
	VkResult result;

	VkSubmitInfo submitInfo = {};

	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = VK_NULL_HANDLE;
	submitInfo.pCommandBuffers = cmdBuffers.data();
	submitInfo.commandBufferCount = UnsignedRecast<uint32_t, size_t>(cmdBuffers.size());
	submitInfo.pSignalSemaphores = sigSemaphores.data();
	submitInfo.signalSemaphoreCount = sigSemaphores.size();
	submitInfo.pWaitSemaphores = waitSemaphores.data();
	submitInfo.waitSemaphoreCount = waitSemaphores.size();
	submitInfo.pWaitDstStageMask = flags;

	result = vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);

	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L"vkGenerics::ExecuteQueue()", L"Unable to submit Queue for execution", L"Check inputted data and error message output");
	}
	return result;
}

VkResult vkGenerics::CreatePhysicalDevice(VkPhysicalDevice& m_PhysicalDev, VkInstance* instance, std::vector<const char*> requestedExtensions)
{
	//Enumerate Physcial devices
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(*instance, &deviceCount, nullptr);

	//If there are no devices
	if (deviceCount == 0)
	{
		vkDebug::ThrowError(L"vkDeviceList::CreatePhysicalDevice", L"Can't find GPUs that support Vulkan Instance", L"Check if this GPU supports Vulkan.");
		return VK_ERROR_UNKNOWN;
	}

	//Get Lists of devices
	std::vector<VkPhysicalDevice> deviceList(deviceCount);
	vkEnumeratePhysicalDevices(*instance, &deviceCount, deviceList.data());

	for (VkPhysicalDevice device : deviceList)
	{
		if (vkGenerics::SuitableDevice(device, requestedExtensions))
		{
			m_PhysicalDev = device;
			return VK_SUCCESS;
		}
	}
	vkDebug::ThrowWarning(L"vkGenerics::CreatePhysicalDevice", L"Function may have not found a sutable device, yet function completed");
	return VK_ERROR_UNKNOWN;
}

VkResult vkGenerics::CreateLogicalDevice(VkDevice& device, VkPhysicalDevice& physicalDev, std::vector<const char*> requestedExtensions, std::vector<int> queueIndicies )
{
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos(queueIndicies.size());

	VkDeviceQueueCreateInfo queueCreateInfo = {};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueCount = 1;
	float priority = 1.f;
	queueCreateInfo.pQueuePriorities = &priority;
	queueCreateInfo.pNext = nullptr;
	for (size_t i = 0; i < queueIndicies.size(); i++)
	{
		queueCreateInfos[i].queueFamilyIndex = queueIndicies[i];
	}

	// Information to create logical device (sometimes called "device")
	VkDeviceCreateInfo LogicDeviceInfo = {};
	LogicDeviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	LogicDeviceInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	LogicDeviceInfo.pQueueCreateInfos = queueCreateInfos.data();
	LogicDeviceInfo.enabledExtensionCount = static_cast<uint32_t>(requestedExtensions.size());
	LogicDeviceInfo.ppEnabledExtensionNames = requestedExtensions.data();

	VkPhysicalDeviceFeatures deviceFeatures = {};
	LogicDeviceInfo.pEnabledFeatures = &deviceFeatures;

	VkResult result = vkCreateDevice(physicalDev, &LogicDeviceInfo, nullptr, &device);
	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L"vkQueueFamilyIndices::CreateDevice", L"No Queue Family was detected, so the logical device wasn't created", L"N/A");
		return result;
	}
	return result;

}

VkResult vkGenerics::CreateLogicalDevice(VkDevice& device, VkPhysicalDevice& physicalDev, std::vector<const char*> requestedExtensions, int queueIndex)
{
	VkDeviceQueueCreateInfo  deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	deviceCreateInfo.queueFamilyIndex = queueIndex;
	deviceCreateInfo.queueCount = 1;
	float priority = 1;
	deviceCreateInfo.pQueuePriorities = &priority;
	deviceCreateInfo.pNext = nullptr;

	// Information to create logical device (sometimes called "device")
	VkDeviceCreateInfo LogicDeviceInfo = {};
	LogicDeviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	LogicDeviceInfo.queueCreateInfoCount = 1;
	LogicDeviceInfo.pQueueCreateInfos = &deviceCreateInfo;
	LogicDeviceInfo.enabledExtensionCount = static_cast<uint32_t>(requestedExtensions.size());
	LogicDeviceInfo.ppEnabledExtensionNames = requestedExtensions.data();

	VkPhysicalDeviceFeatures deviceFeatures = {};
	LogicDeviceInfo.pEnabledFeatures = &deviceFeatures;

	VkResult result = vkCreateDevice(physicalDev, &LogicDeviceInfo, nullptr, &device);
	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L"vkQueueFamilyIndices::CreateDevice", L"No Queue Family was detected, so the logical device wasn't created", L"N/A");
		return result;
	}
	return result;
}

VkResult vkGenerics::CreateLogicalDevice(VkDevice& device, VkPhysicalDevice& physicalDev, std::vector<const char*> requestedExtensions, std::unordered_map<vkGenerics::vkQueueFamilyBit, int>& familyIndicies)
{
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

	for (std::pair<vkGenerics::vkQueueFamilyBit, int>  queueFamilyIndex : familyIndicies)
	{
		VkDeviceQueueCreateInfo  VKDQC = {};
		VKDQC.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		VKDQC.queueFamilyIndex = queueFamilyIndex.second;
		VKDQC.queueCount = 1;
		float priority = 1;
		VKDQC.pQueuePriorities = &priority;
		VKDQC.pNext = nullptr;
		queueCreateInfos.push_back(VKDQC);
	}

	// Information to create logical device (sometimes called "device")
	VkDeviceCreateInfo LogicDeviceInfo = {};
	LogicDeviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	LogicDeviceInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	LogicDeviceInfo.pQueueCreateInfos = queueCreateInfos.data();
	LogicDeviceInfo.enabledExtensionCount = static_cast<uint32_t>(requestedExtensions.size());
	LogicDeviceInfo.ppEnabledExtensionNames = requestedExtensions.data();

	VkPhysicalDeviceFeatures deviceFeatures = {};
	LogicDeviceInfo.pEnabledFeatures = &deviceFeatures;

	VkResult result = vkCreateDevice(physicalDev, &LogicDeviceInfo, nullptr, &device);
	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L"vkQueueFamilyIndices::CreateDevice", L"No Queue Family was detected, so the logical device wasn't created", L"N/A");
		return result;
	}
	return result;
}

bool vkGenerics::SuitableDevice(const VkPhysicalDevice& device, std::vector<const char*> requestedExtensions)
{
	return CheckDeviceExtensionSupport(device, requestedExtensions);
}

bool vkGenerics::CheckDeviceExtensionSupport(const VkPhysicalDevice& device, std::vector<const char*> requestedExtensions)
{
	uint32_t ExtensionCount = 0;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &ExtensionCount, nullptr);

	if (ExtensionCount == 0)
	{
		return false;
	}

	std::vector<VkExtensionProperties> extensions(ExtensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &ExtensionCount, extensions.data());

	for (const auto& deviceExtension : requestedExtensions)
	{
		bool hasExtension = false;
		for (const auto& extension : extensions)
		{
			if (strcmp(deviceExtension, extension.extensionName) == 0)
			{
				hasExtension = true;
				break;
			}
		}
		if (!hasExtension)
		{
			vkDebug::ThrowError(L"vkGenerics::CheckDeviceExtensionSupport()", L" Physical Device Extension not found", L"Check to see if device extension is supported on your hardware");
			return false;
		}

	}

	return true;
}

VkPhysicalDeviceProperties vkGenerics::GetPhysicalDeviceProperties(const VkPhysicalDevice& device)
{
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	return deviceProperties;
}

VkResult vkGenerics::CreateShaderModule(const std::wstring fileLocation, const VkDevice& device, VkShaderModule& module)
{
	std::ifstream inFile(fileLocation, std::ios::binary | std::ios::ate);
	if (!inFile.is_open())
	{
		vkDebug::ThrowError(L"vkGenerics::CreateShaderModule", L"Unable to open file " + fileLocation, L"Check to see if the input file exists, is in the right location or is the right file/file type");
 		return VK_ERROR_UNKNOWN;
	}
	std::vector<char> shaderData((size_t)inFile.tellg());
	inFile.seekg(0);
	inFile.read(shaderData.data(), shaderData.size());
	inFile.close();

	VkShaderModuleCreateInfo SMCI = {};
	SMCI.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	SMCI.codeSize = shaderData.size();
	SMCI.pCode = reinterpret_cast<const uint32_t*>(shaderData.data());

	VkResult result = vkCreateShaderModule(device, &SMCI, nullptr, &module);

	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L"vkGenerics::CreateShaderModule()", L"Failed to create shader using  " + fileLocation, L"Check error code");
		return VK_ERROR_UNKNOWN;
	}
	return result;
}

VkResult vkGenerics::CreateFramebuffer(const VkDevice& device, VkFramebuffer& newBuffer , VkRenderPass& renderPass, VkImageView& imageView, VkExtent2D& extent)
{
	VkFramebufferCreateInfo frameBufferCreateInfo = {};
	frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	frameBufferCreateInfo.width = extent.width;
	frameBufferCreateInfo.height = extent.height;
	frameBufferCreateInfo.renderPass = renderPass; //Render Pass the framebuffer uses
	frameBufferCreateInfo.attachmentCount = 1; // list of attachments that should be 1 to one with renderpass
	frameBufferCreateInfo.pAttachments = &imageView;
	frameBufferCreateInfo.layers = 1; // framebuffer layers
	frameBufferCreateInfo.pNext = VK_NULL_HANDLE;
	frameBufferCreateInfo.flags = 0;

	VkResult result = vkCreateFramebuffer(device, &frameBufferCreateInfo, nullptr, &newBuffer);
	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L" vkGenerics::CreateFramebuffer()", L"Failed to create FrameBuffer",  L"Check error code");
		return VK_ERROR_UNKNOWN;
	}
	return result;
}

VkExtent2D vkGenerics::SelectIdealImageResolution(const VkSurfaceCapabilitiesKHR& surfaceCapabilities, int width, int height)
{
	if (surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return surfaceCapabilities.currentExtent;
	}
	else
	{
		VkExtent2D newExtent;
		newExtent.width = width;
		newExtent.height = height;

		newExtent.width = std::max(surfaceCapabilities.minImageExtent.width, std::min(surfaceCapabilities.maxImageExtent.width, newExtent.width));
		newExtent.height = std::max(surfaceCapabilities.minImageExtent.height, std::min(surfaceCapabilities.maxImageExtent.height, newExtent.height));
		return newExtent;
	}
}

VkResult vkGenerics::GetSwapchainCapabilities(const VkPhysicalDevice& device, const VkSurfaceKHR& surface , vkSwaphcainCapabilities& swapchainDetails)
{
	vkSwaphcainCapabilities details;

	VkResult result;

	//Capabilities of the surface
	result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &swapchainDetails.m_SurfaceCapabilities);

	//Formats
	uint32_t formatCount = 0;
	result = vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L"vkGenerics::GetSwapchainSurfaceDetails()", L"Unbale to retrieve the amount of formats the physical account supports", L"check the surface and physical device variables");
		return VK_ERROR_UNKNOWN;
	}

	if (formatCount != 0)
	{
		swapchainDetails.m_Formats.resize(formatCount);
		result = vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, swapchainDetails.m_Formats.data());
		if (result != VK_SUCCESS)
		{
			vkDebug::ThrowError(L"vkGenerics::GetSwapchainSurfaceDetails()", L"Unbale to retrieve the formats the physical account supports", L"Check output errors");
			return VK_ERROR_UNKNOWN;
		}
	}
	else
	{
		vkDebug::ThrowWarning(L"vkGenerics::GetSwapchainSurfaceDetails()", L"No formats detected");
	}

	uint32_t presentationCount = 0;
	result = vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentationCount, nullptr);
	if(result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L"vkGenerics::GetSwapchainSurfaceDetails()", L"Unbale to retrieve the amount of presentations the physical account supports", L"check the surface and physical device variables");
		return VK_ERROR_UNKNOWN;
	}
	if (presentationCount != 0)
	{
		swapchainDetails.m_PresentationModes.resize(presentationCount);
		result = vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentationCount, swapchainDetails.m_PresentationModes.data());
		if (result != VK_SUCCESS)
		{
			vkDebug::ThrowError(L"vkGenerics::GetSwapchainSurfaceDetails()", L"Unbale to retrieve the physical account supports", L"Check output errors");
			return VK_ERROR_UNKNOWN;
		}
	}
	else
	{
		vkDebug::ThrowWarning(L"vkGenerics::GetSwapchainSurfaceDetails()", L"No formats detected");
	}
	return result;
}

VkResult vkGenerics::GetVkImages(const VkDevice& device, VkSwapchainKHR& swapchain, VkFormat& format, std::vector<vkSwapchainImages>& images)
{
	VkResult result;

	uint32_t swapChainImageCount;
	result = vkGetSwapchainImagesKHR(device, swapchain, &swapChainImageCount, nullptr);

	std::vector<VkImage> vkImages(swapChainImageCount);
	images = std::vector<vkSwapchainImages>(swapChainImageCount);
	result = vkGetSwapchainImagesKHR(device, swapchain, &swapChainImageCount, vkImages.data());

	for (VkImage image : vkImages)
	{
		vkSwapchainImages swapChainImage = {};
		swapChainImage.m_Image = image;
		vkGenerics::CreateImageView(device, swapChainImage.m_Image, swapChainImage.m_ImageView, format, VK_IMAGE_ASPECT_COLOR_BIT);
		images.push_back(swapChainImage);
	}
	return result;
}

VkResult vkGenerics::CreateImageView(const VkDevice& device, VkImage& image, VkImageView& view, VkFormat format, VkImageAspectFlags aspectFlags)
{
	VkImageViewCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageCreateInfo.image = image;
	imageCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageCreateInfo.format = format;
	imageCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageCreateInfo.subresourceRange.aspectMask = aspectFlags;
	imageCreateInfo.subresourceRange.baseMipLevel = 0;
	imageCreateInfo.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
	imageCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageCreateInfo.subresourceRange.layerCount = 1;

	VkResult result = vkCreateImageView(device, &imageCreateInfo, nullptr, &view);
	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L"vkGenerics::CreateImageView()", L"Unable to create image view", L"Check outputted error.");
	}
	return result;
}

VkResult vkGenerics::CreateSwapchain(const VkDevice& device, const VkPhysicalDevice& physicalDevice, VkSwapchainKHR& swapchain ,vkVsync vsyncMode, VkSurfaceFormatKHR selectedFormat, VkExtent2D extent, VkSurfaceKHR& surface, int graphicsFam, int swapchainFam, vkFrameBufferMode mode)
{
	uint32_t imageCount = (uint32_t)mode;

	vkSwaphcainCapabilities swapchainCap;

	vkGenerics::GetSwapchainCapabilities(physicalDevice, surface, swapchainCap);

	if (swapchainCap.m_SurfaceCapabilities.maxImageCount > 0 && swapchainCap.m_SurfaceCapabilities.maxImageCount < imageCount)
	{
		imageCount = swapchainCap.m_SurfaceCapabilities.maxImageCount;
	}

	// Creation information for swap chain
	VkSwapchainCreateInfoKHR swapChainCreateInfo = {};
	swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapChainCreateInfo.surface = surface;
	swapChainCreateInfo.imageFormat = selectedFormat.format;
	swapChainCreateInfo.imageColorSpace = selectedFormat.colorSpace;
	swapChainCreateInfo.presentMode = (VkPresentModeKHR)vsyncMode;
	swapChainCreateInfo.imageExtent = extent;
	swapChainCreateInfo.minImageCount = imageCount;
	swapChainCreateInfo.imageArrayLayers = 1;
	swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapChainCreateInfo.preTransform = swapchainCap.m_SurfaceCapabilities.currentTransform; 
	swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapChainCreateInfo.clipped = VK_TRUE;

	// If Graphics and Presentation families are different, then swapchain must let images be shared between families
	if (graphicsFam != swapchainFam)
	{
		// Queues to share between
		uint32_t queueFamilyIndices[] = {
			(uint32_t)graphicsFam,
			(uint32_t)swapchainFam
		};

		swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;		// Image share handling
		swapChainCreateInfo.queueFamilyIndexCount = 2;							// Number of queues to share images between
		swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;			// Array of queues to share between
	}
	else
	{
		swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapChainCreateInfo.queueFamilyIndexCount = 0;
		swapChainCreateInfo.pQueueFamilyIndices = nullptr;
	}

	// IF old swap chain been destroyed and this one replaces it, then link old one to quickly hand over responsibilities
	swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

	// Create Swapchain
	VkResult result = vkCreateSwapchainKHR(device, &swapChainCreateInfo, nullptr, &swapchain);
	if (result != VK_SUCCESS)
	{	
		vkDebug::ThrowError(L"vkGenerics::CreateSwapchain()", L"Unable to create swapchain", L"Check output errors");
		return VK_ERROR_UNKNOWN;
	}
	return result;
}

void vkGenerics::CreateQueue(VkDevice device, VkQueue& queue, int queueIndex)
{
	vkGetDeviceQueue(device, queueIndex, 0, &queue);
}

void vkGenerics::CreateQueues(VkDevice device, std::unordered_map<int, VkQueue>& Queues)
{	
	for (	std::pair<int, VkQueue> queueIndex : Queues)
	{
		vkGetDeviceQueue(device, queueIndex.first, 0, &queueIndex.second);
	}
}

VkResult vkGenerics::CreateQueues(VkDevice device, std::wstring deviceNames,  std::unordered_map<std::wstring, VkQueue>& queues, std::unordered_map<vkGenerics::vkQueueFamilyBit, int>& familes, vkQueueFamilyBitAccumulated bits)
{
	//CheckWhat QueueIndex Does, its the Zeros
	switch (bits)
	{
	case vkGenerics::vkQueueFamilyBitAccumulated::BASE:
		vkDebug::ThrowError(L"vkGenerics::CreateQueue", L"bit was specified as base only", L"bit should be replaced by any under vkQueueFamilyBitAccumulated");
		return VK_ERROR_UNKNOWN;
		break;
	case vkGenerics::vkQueueFamilyBitAccumulated::GRAPHICS:
		vkGetDeviceQueue(device, familes[vkGenerics::vkQueueFamilyBit::GRAPHICS], 0, &queues[deviceNames + L"_Graphics"]);
		break;
	case vkGenerics::vkQueueFamilyBitAccumulated::SWAPCHAIN:
		vkGetDeviceQueue(device, familes[vkGenerics::vkQueueFamilyBit::SWAPCHAIN], 0, &queues[deviceNames + L"_Swapchain"]);
		break;
	case vkGenerics::vkQueueFamilyBitAccumulated::COMPUTE:
		vkGetDeviceQueue(device, familes[vkGenerics::vkQueueFamilyBit::COMPUTE], 0, &queues[deviceNames + L"_Compute"]);
		break;
	case vkGenerics::vkQueueFamilyBitAccumulated::SWAPCHAIN_GRAPHICS:
		vkGetDeviceQueue(device, familes[vkGenerics::vkQueueFamilyBit::GRAPHICS], 0, &queues[deviceNames + L"_Graphics"]);
		vkGetDeviceQueue(device, familes[vkGenerics::vkQueueFamilyBit::SWAPCHAIN], 0, &queues[deviceNames + L"_Swapchain"]);
		break;
	case vkGenerics::vkQueueFamilyBitAccumulated::GRAPHICS_COMPUTE:
		vkGetDeviceQueue(device, familes[vkGenerics::vkQueueFamilyBit::GRAPHICS], 0, &queues[deviceNames + L"_Graphics"]);
		vkGetDeviceQueue(device, familes[vkGenerics::vkQueueFamilyBit::COMPUTE], 0, &queues[deviceNames + L"_Compute"]);
		break;
	case vkGenerics::vkQueueFamilyBitAccumulated::SWAPCHAIN_GRAPHICS_COMPUTE:
		vkGetDeviceQueue(device, familes[vkGenerics::vkQueueFamilyBit::GRAPHICS], 0, &queues[deviceNames + L"_Graphics"]);
		vkGetDeviceQueue(device, familes[vkGenerics::vkQueueFamilyBit::SWAPCHAIN], 0, &queues[deviceNames + L"_Swapchain"]);
		vkGetDeviceQueue(device, familes[vkGenerics::vkQueueFamilyBit::COMPUTE], 0, &queues[deviceNames + L"_Compute"]);
		break;
	default:
		vkDebug::ThrowError(L"vkQueueFamilyIndices::CreateDevice", L"No Queue Family was detected, so the logical device wasn't created", L"Check to see which bits are passed into the function");
		return VK_ERROR_UNKNOWN;
		break;
	}

	return VK_SUCCESS;
}

VkResult vkGenerics::AquireFamilyIndices(const vkGenerics::vkQueueFamilyBitAccumulated& bits, const VkPhysicalDevice& device, std::unordered_map<vkGenerics::vkQueueFamilyBit, int>& familyIndicies)
{
	/*
		familyIndicies[vkGenerics::vkQueueFamilyBit::BASE] = -1;
		familyIndicies[vkGenerics::vkQueueFamilyBit::GRAPHICS] = -1;
		familyIndicies[vkGenerics::vkQueueFamilyBit::COMPUTE] = -1;
		familyIndicies[vkGenerics::vkQueueFamilyBit::SWAPCHAIN] = -1;
*/

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilyList(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyList.data());

	int i = 0;

	for (const auto& queuefamily : queueFamilyList)
	{
		switch (bits)
		{
		case vkGenerics::vkQueueFamilyBitAccumulated::BASE:
			break;
		case vkGenerics::vkQueueFamilyBitAccumulated::GRAPHICS:
			if (queuefamily.queueCount > 0 && queuefamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				familyIndicies[vkGenerics::vkQueueFamilyBit::GRAPHICS] = i;
			}
			break;
		case vkGenerics::vkQueueFamilyBitAccumulated::SWAPCHAIN:
			familyIndicies[vkGenerics::vkQueueFamilyBit::SWAPCHAIN] = 2;
			break;
		case vkGenerics::vkQueueFamilyBitAccumulated::COMPUTE:
			if (queuefamily.queueCount > 0 && queuefamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
			{
				familyIndicies[vkGenerics::vkQueueFamilyBit::COMPUTE] = i;
			}
			break;
		case vkGenerics::vkQueueFamilyBitAccumulated::SWAPCHAIN_GRAPHICS:
			if (queuefamily.queueCount > 0 && queuefamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				familyIndicies[vkGenerics::vkQueueFamilyBit::GRAPHICS] = i;
			}
			familyIndicies[vkGenerics::vkQueueFamilyBit::SWAPCHAIN] = 2;
			break;
		case vkGenerics::vkQueueFamilyBitAccumulated::GRAPHICS_COMPUTE:
			if (queuefamily.queueCount > 0 && queuefamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				familyIndicies[vkGenerics::vkQueueFamilyBit::GRAPHICS] = i;
			}
			if (queuefamily.queueCount > 0 && queuefamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
			{
				familyIndicies[vkGenerics::vkQueueFamilyBit::COMPUTE] = i;
			}
			break;
		case vkGenerics::vkQueueFamilyBitAccumulated::SWAPCHAIN_GRAPHICS_COMPUTE:
			if (queuefamily.queueCount > 0 && queuefamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				familyIndicies[vkGenerics::vkQueueFamilyBit::GRAPHICS] = i;
			}
			if (queuefamily.queueCount > 0 && queuefamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
			{
				familyIndicies[vkGenerics::vkQueueFamilyBit::COMPUTE] = i;
			}
			familyIndicies[vkGenerics::vkQueueFamilyBit::SWAPCHAIN] = 2;
			break;
		default:
			vkDebug::ThrowError(L"vkGenerics::AquireQueueIndex", L"The switch failed to hit a vkGenerics::vkQueueFamilyBitAccumulated value", L"Check if you have the correct accumulated value i.e. check if you have assigned it the property of vkGenerics::vkQueueFamilyBit::BASE.");
			return VK_ERROR_UNKNOWN;
			break;
		}

		if (familyIndicies[vkGenerics::vkQueueFamilyBit::GRAPHICS] >= 0 && familyIndicies[vkGenerics::vkQueueFamilyBit::COMPUTE] >= 0 && familyIndicies[vkGenerics::vkQueueFamilyBit::SWAPCHAIN] >= 0)
		{
			vkDebug::ThrowError(L"vkGenerics::AquireQueueIndex", L"One of the Queue families is not valid", L"Check if the physical hardware properties support that family.");
			return VK_ERROR_UNKNOWN;
		}
		i++;
	}
	return VK_SUCCESS;
}

//VkResult vkGenerics::AquireFamilyIndices(const vkQueueFamilyBitAccumulated& bits, const VkPhysicalDevice& device, std::unordered_map<vkQueueFamilyBit, int>& outMap)
//{
//	uint32_t queueFamilyCount = 0;
//	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
//	std::vector<VkQueueFamilyProperties> queueFamilyList(queueFamilyCount);
//	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyList.data());
//
//	for (size_t i = 0; i < queueFamilyList.size(); i++)
//	{
//		switch (bits)
//		{
//		case vkQueueFamilyBitAccumulated::BASE:
//			break;
//		case vkQueueFamilyBitAccumulated::GRAPHICS:
//			if (queueFamilyList[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
//			{
//				outMap[vkQueueFamilyBit::GRAPHICS] = i;
//			}
//			break;
//		case vkQueueFamilyBitAccumulated::SWAPCHAIN:
//			outMap[vkQueueFamilyBit::SWAPCHAIN] = 2;
//			break;
//		case vkQueueFamilyBitAccumulated::COMPUTE:
//			if (queueFamilyList[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
//			{
//				outMap[vkQueueFamilyBit::COMPUTE] = i;
//			}
//			break;
//		case vkQueueFamilyBitAccumulated::SWAPCHAIN_GRAPHICS:
//			if (queueFamilyList[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
//			{
//				outMap[vkQueueFamilyBit::GRAPHICS] = i;
//			}
//			outMap[vkQueueFamilyBit::SWAPCHAIN] = 2;
//			break;
//		case vkQueueFamilyBitAccumulated::GRAPHICS_COMPUTE:
//			if (queueFamilyList[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
//			{
//				outMap[vkQueueFamilyBit::GRAPHICS] = i;
//			}
//			if (queueFamilyList[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
//			{
//				outMap[vkQueueFamilyBit::COMPUTE] = i;
//			}
//			break;
//		case vkQueueFamilyBitAccumulated::SWAPCHAIN_GRAPHICS_COMPUTE:
//			if (queueFamilyList[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
//			{
//				outMap[vkQueueFamilyBit::GRAPHICS] = i;
//			}
//			if (queueFamilyList[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
//			{
//				outMap[vkQueueFamilyBit::COMPUTE] = i;
//			}
//			outMap[vkQueueFamilyBit::SWAPCHAIN] = 2;
//			break;
//		default:
//			vkDebug::ThrowError(L"vkQueueFamilyIndices::Init", L"The switch failed to hit a vkGenerics::vkQueueFamilyBitAccumulated value", L"Check if you have the correct accumulated value i.e. check if you have assigned it the property of vkGenerics::vkQueueFamilyBit::BASE.");
//			return VK_ERROR_UNKNOWN;
//			break;
//		}
//	}
//	return VK_SUCCESS();
//}

VkResult vkGenerics::AquireQueueIndex(const VkPhysicalDevice& device, const vkQueueFamilyBit& bit, int& indexOut)
{

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilyList(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyList.data());
	for (size_t i = 0; i < queueFamilyList.size(); ++i)
	{
		switch (bit)
		{
		case vkGenerics::vkQueueFamilyBit::BASE:
			vkDebug::ThrowError(L"vkGenerics::AquireQueueIndex", L"The switch hit a vkGenerics::vkQueueFamilyBitAccumulated value of BASE", L"Check if you have the correct accumulated value i.e. check if you have assigned it the property of vkGenerics::vkQueueFamilyBit::BASE.");
			indexOut = -1;
			break;
		case vkGenerics::vkQueueFamilyBit::GRAPHICS:
			if (queueFamilyList[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indexOut = i;
			}
			break;
		case vkGenerics::vkQueueFamilyBit::SWAPCHAIN:
			indexOut = 2;
			break;
		case vkGenerics::vkQueueFamilyBit::COMPUTE:
			if (queueFamilyList[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
			{
				indexOut = i;
			}
			break;
		default:
			vkDebug::ThrowError(L"vkQueueFamilyIndices::Init", L"The switch failed to hit a vkGenerics::vkQueueFamilyBitAccumulated value", L"Check if you have the correct accumulated value i.e. check if you have assigned it the property of vkGenerics::vkQueueFamilyBit::BASE.");
			indexOut = -1;
			return VK_ERROR_UNKNOWN;
			break;
		}
	}

	return VK_SUCCESS;
}

VkResult vkGenerics::AquireQueueIndex(const VkPhysicalDevice& device, const VkQueueFlagBits& bit, int& indexOut)
{
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilyList(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyList.data());
	for (size_t i = 0; i < queueFamilyList.size(); ++i)
	{
		switch (bit)
		{
		default:
			vkDebug::ThrowError(L"vkGenerics::AquireQueueIndex", L"The switch failed to hit a vkGenerics::vkQueueFamilyBitAccumulated value", L"Check if you have the correct accumulated value i.e. check if you have assigned it the property of vkGenerics::vkQueueFamilyBit::BASE.");
			return VK_ERROR_UNKNOWN;
		case VK_QUEUE_GRAPHICS_BIT:
			if (queueFamilyList[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indexOut = i;
			}
			break;
		case VK_QUEUE_COMPUTE_BIT:
			if (queueFamilyList[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
			{
				indexOut = i;
			}
			break;
		case VK_QUEUE_TRANSFER_BIT:
			if (queueFamilyList[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
			{
				indexOut = i;
			}
			break;
		}
	}
	return VK_SUCCESS;
}


VkResult vkGenerics::CreateCommandPool(VkDevice& device, VkCommandPool& cmdPool, vkQueueFamilyBit bit, std::map<vkGenerics::vkQueueFamilyBit, int>& familes)
{

	VkResult result;

	VkCommandPoolCreateInfo commadPoolCreateInfo = {};
	commadPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		
	switch (bit)
	{
	case vkGenerics::vkQueueFamilyBit::GRAPHICS:
		commadPoolCreateInfo.queueFamilyIndex = familes[bit];
		result = vkCreateCommandPool(device, &commadPoolCreateInfo, nullptr, &cmdPool);
		break;
	case vkGenerics::vkQueueFamilyBit::SWAPCHAIN:
		commadPoolCreateInfo.queueFamilyIndex = familes[bit];
		result = vkCreateCommandPool(device, &commadPoolCreateInfo, nullptr, &cmdPool);
		break;
	case vkGenerics::vkQueueFamilyBit::COMPUTE:
		commadPoolCreateInfo.queueFamilyIndex = familes[bit];
		result = vkCreateCommandPool(device, &commadPoolCreateInfo, nullptr, &cmdPool);
		break;
	default:
		vkDebug::ThrowError(L"vkGenerics::CreateCommandPool", L"Incorrect bit was provided", L"Check to see which bit are passed into the function and ensure its not BASE");
		return VK_ERROR_UNKNOWN;
		break;
	}

	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L"vkGenerics::CreateCommandPool", L"Unable to create a command pool", L"Check the error code");
	}
	return result;
}

VkResult vkGenerics::CreateCommandPool(VkDevice& device, VkCommandPool& cmdPool, int& queueIndex)
{
	VkResult result;
	VkCommandPoolCreateInfo commadPoolCreateInfo = {};
	commadPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commadPoolCreateInfo.queueFamilyIndex = queueIndex;
	result = vkCreateCommandPool(device, &commadPoolCreateInfo, nullptr, &cmdPool);
	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L"vkGenerics::CreateCommandPool", L"Unable to create a command pool", L"Check the error code");
	}
	return result;
}

VkResult vkGenerics::CreateCommandPool(VkDevice& device, VkCommandPool& cmdPool, int queueFamilyIndex, VkCommandPoolCreateFlags flags)
{
	VkResult result;

	VkCommandPoolCreateInfo commadPoolCreateInfo = {};
	commadPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commadPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;
	commadPoolCreateInfo.pNext = VK_NULL_HANDLE;
	commadPoolCreateInfo.flags = flags;

	result = vkCreateCommandPool(device, &commadPoolCreateInfo, nullptr, &cmdPool);
	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L"vkGenerics::CreateCommandPool", L"Unable to create a command pool", L"Check the error code");
	}
	return result;
}

VkResult vkGenerics::ResetCommandPool(const VkDevice& device, const VkCommandPool& cmdPool)
{
	VkResult result;
	result = vkResetCommandPool(device, cmdPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
	return result;
}

VkResult vkGenerics::ResetCommandPool(const VkDevice& device, const VkCommandPool& cmdPool, VkCommandPoolResetFlagBits flag)
{
	VkResult result;
	result = vkResetCommandPool(device, cmdPool, flag);
	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L" vkGenerics::ResetCommandPool()", L"Failed to reset command pool", L"Check the outputted error message from Vulkan");
	}
	return result;
}

VkResult vkGenerics::CreateCommandBuffer(const VkDevice& device, VkCommandBuffer* buffers, VkCommandBufferAllocateInfo commandBufferInfo)
{
	VkResult result;
	result = vkAllocateCommandBuffers(device, &commandBufferInfo, buffers);
	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L" vkGenerics::CreateCommandBuffer()", L"Failed to allocate the command buffers", L"Check the outputted error message from Vulkan");
	}
	return result;
}

VkResult vkGenerics::CreateCommandBuffer(const VkDevice& device, VkCommandBuffer& buffer, VkCommandPool& commandPool, VkCommandBufferLevel level)
{
	VkResult result;

	VkCommandBufferAllocateInfo comBuffAllocInfo = {};
	comBuffAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	comBuffAllocInfo.commandPool = commandPool;
	comBuffAllocInfo.level = level;
	comBuffAllocInfo.commandBufferCount = 1;
	comBuffAllocInfo.pNext = VK_NULL_HANDLE;

	result = vkAllocateCommandBuffers(device, &comBuffAllocInfo, &buffer);
	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L" vkGenerics::CreateCommandBuffer()", L"Failed to allocate the command buffers", L"Check the outputted error message from Vulkan");
	}
	return result;
}

VkResult vkGenerics::CreateCommandBuffer(const VkDevice& device, std::vector<VkCommandBuffer>& buffers, VkCommandPool& commandPool, VkCommandBufferLevel level)
{
	VkResult result;

	VkCommandBufferAllocateInfo comBuffAllocInfo = {};
	comBuffAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	comBuffAllocInfo.commandPool = commandPool;
	comBuffAllocInfo.level = level;
	comBuffAllocInfo.commandBufferCount = static_cast<uint32_t>(buffers.size());
	comBuffAllocInfo.pNext = VK_NULL_HANDLE;

	result = vkAllocateCommandBuffers(device, &comBuffAllocInfo, buffers.data());

	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L" vkGenerics::CreateCommandBuffer()", L"Failed to allocate the command buffers", L"Check the outputted error message from Vulkan");
	}
	return result;
}

VkResult vkGenerics::BeginCommandBuffer(VkCommandBuffer& commandBuff)
{
	VkResult result;

	VkCommandBufferBeginInfo commandBufferBeginInfo = 
{
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		0,
		VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		0
	};

	result = vkBeginCommandBuffer(commandBuff, &commandBufferBeginInfo);

	return result;

}

VkResult vkGenerics::BeginCommandBuffer(VkCommandBuffer& commandBuff, VkCommandBufferUsageFlagBits bits)
{
	VkResult result;

	VkCommandBufferBeginInfo commandBufferBeginInfo =
	{
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		0,
		bits,
		0
	};

	result = vkBeginCommandBuffer(commandBuff, &commandBufferBeginInfo);

	return result;

}

VkResult vkGenerics::BeginCommandBuffer(VkCommandBuffer& commandBuff, VkCommandBufferBeginInfo* infoType)
{
	VkResult result;

	result = vkBeginCommandBuffer(commandBuff, infoType);

	return result;
}

VkResult vkGenerics::EndCommandBuffer(VkCommandBuffer& commandBuff)
{
	VkResult result;

	result = vkEndCommandBuffer(commandBuff);

	return result;
}

template<std::size_t N>
VkResult vkGenerics::CreateCommandBuffer(const VkDevice& device, std::array<VkCommandBuffer, N> buffers, VkCommandBufferAllocateInfo commandBufferInfo, VkCommandPool& commandPool, VkCommandBufferLevel level)
{
	VkResult result;

	VkCommandBufferAllocateInfo combBuffAllocInfo = {};
	combBuffAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	combBuffAllocInfo.commandPool = commandPool;
	combBuffAllocInfo.level = level;
	combBuffAllocInfo.commandBufferCount = static_cast<uint32_t>(buffers.size());
	combBuffAllocInfo.pNext = VK_NULL_HANDLE;

	result = vkAllocateCommandBuffers(device, &combBuffAllocInfo, buffers.data());

	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L" vkGenerics::CreateCommandBuffer()", L"Failed to allocate the command buffers", L"Check the outputted error message from Vulkan");
	}
	return result;
}

VkDescriptorSetLayoutCreateInfo vkGenerics::PopulateVkDescriptorSetLayoutCreateInfo(uint32_t size, int descriptorCount, VkDescriptorType type, VkShaderStageFlags flags, VkSampler* sampler)
{
	std::vector<VkDescriptorSetLayoutBinding> descSetLayoutBinding(size);

	for (size_t i = 0; i < size; i++)
	{
		descSetLayoutBinding[i].binding = UnsignedRecast<uint32_t, size_t>(i);
		descSetLayoutBinding[i].descriptorCount = descriptorCount;
		descSetLayoutBinding[i].descriptorType = type;
		descSetLayoutBinding[i].stageFlags = flags;
		descSetLayoutBinding[i].pImmutableSamplers = sampler;
	}
	  

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = 
	{
	  VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
	  0,
	  0,
	  size,
	  descSetLayoutBinding.data()
	};

	return descriptorSetLayoutCreateInfo;
}

VkResult vkGenerics::CreateDescriptorSetLayout(const VkDevice& device, VkDescriptorSetLayout& setLayout, VkDescriptorSetLayoutCreateInfo setLayoutInfo)
{
	VkResult result;

	result = vkCreateDescriptorSetLayout(device, &setLayoutInfo, nullptr, &setLayout);
	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L" vkGenerics::CreateDescriptorSetLayout()", L"Failed to create descriptor set layout", L"Check the outputted error message from Vulkan and if the layout settings were created properly i.e. correct bindings");
	}

	return result;
}

VkResult vkGenerics::CreateDescriptorSet(const VkDevice& device, VkDescriptorSet& setLayout, VkDescriptorSetAllocateInfo setLayoutInfo)
{
	VkResult result;
	result = vkAllocateDescriptorSets(device, &setLayoutInfo, &setLayout);
	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L" vkGenerics::CreateDescriptorSet()", L"Failed to allocate descriptor set", L"Check the outputted error message from Vulkan and if the VkDescriptorSetAllocateInfo settings were created properly i.e. correct bindings");
	}
	return result;
}

VkResult vkGenerics::CreateDescriptorSet(const VkDevice& device, VkDescriptorSet& setLayout, VkDescriptorPool& pool, int descLayoutCount, VkDescriptorSetLayout* layouts)
{
	VkResult result;

	VkDescriptorSetAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocateInfo.descriptorPool = pool;
	allocateInfo.descriptorSetCount = descLayoutCount;
	allocateInfo.pSetLayouts = layouts;
	allocateInfo.pNext = VK_NULL_HANDLE;


	result = vkAllocateDescriptorSets(device, &allocateInfo, &setLayout);
	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L" vkGenerics::CreateDescriptorSet()", L"Failed to allocate descriptor tests", L"Check the outputted error message from Vulkan and if the VkDescriptorSetAllocateInfo settings were created properly i.e. correct bindings");
	}
	return result;
}

VkResult vkGenerics::CreateDescriptorPool(const VkDevice& device, VkDescriptorPool& descriptorPool, VkDescriptorPoolCreateInfo descPoolCreateInfo)
{
	VkResult result;

	//VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {
	// VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
	// 0,
	// 0,
	// 1,
	// 1,
	// poolSizes.data()
	//};

	result = vkCreateDescriptorPool(device, &descPoolCreateInfo, nullptr, &descriptorPool);
	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L" vkGenerics::CreateDescriptorPool()", L"Failed to descriptor pool", L"Check the outputted error message from Vulkan and if the VkDescriptorPoolCreateInfo settings were created properly i.e. correct bindings");
	}
	return result;
}

VkResult vkGenerics::CreateDescriptorPool(const VkDevice& device, VkDescriptorPool& descriptorPool, int poolSize, std::vector<int> descriptorCounts, VkDescriptorType type)
{
	VkResult result;

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};


	std::vector<VkDescriptorPoolSize> poolSizes(poolSize);

	for (size_t i = 0; i < poolSize; i++)
	{
		poolSizes[i].descriptorCount = descriptorCounts[i];
		poolSizes[i].type = type;
	}

	//VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {
	// VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
	// 0,
	// 0,
	// 1,
	// 1,
	// poolSizes.data()
	//};

	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.poolSizeCount = poolSize;
	descriptorPoolCreateInfo.pPoolSizes = poolSizes.data();
	descriptorPoolCreateInfo.pNext = VK_NULL_HANDLE;
	descriptorPoolCreateInfo.maxSets = poolSize;
	descriptorPoolCreateInfo.flags = 0;


	result = vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, nullptr, &descriptorPool);
	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L" vkGenerics::CreateDescriptorPool()", L"Failed to create a descriptor pool", L"Check the outputted error message from Vulkan and if the VkDescriptorPoolCreateInfo settings were created properly i.e. correct bindings");
	}
	return result;
}

void vkGenerics::BindDescriptorSetsToCompute(const VkCommandBuffer& commandBuff, const VkPipelineLayout& pipelineLayout, const VkDescriptorSet& descSet)
{
	vkCmdBindDescriptorSets(commandBuff, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &descSet, 0, 0);
}

void vkGenerics::BindDescriptorSetsToGraphics(const VkCommandBuffer& commandBuff, const VkPipelineLayout& pipelineLayout, const VkDescriptorSet& descSet)
{
	vkCmdBindDescriptorSets(commandBuff, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descSet, 0, 0);
}

void vkGenerics::BindDescriptorSetsToCompute(const VkCommandBuffer& commandBuff, const VkPipelineLayout& pipelineLayout, const VkDescriptorSet* descSet, uint32_t setCount, uint32_t firstSet)
{
	vkCmdBindDescriptorSets(commandBuff, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, firstSet, setCount, descSet, 0, 0);
}

void vkGenerics::BindDescriptorSetsToGraphics(const VkCommandBuffer& commandBuff, const VkPipelineLayout& pipelineLayout, const VkDescriptorSet* descSet, uint32_t setCount, uint32_t firstSet)
{
	vkCmdBindDescriptorSets(commandBuff, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, firstSet, setCount, descSet, 0, 0);
}

void vkGenerics::UpdateDescriptorBufferInfo(const UpdateDescData& updateData, VkDevice& device)
{
	VkDescriptorBufferInfo updateInfo = {};
	updateInfo.buffer = *updateData.m_Buffer;
	updateInfo.offset = updateData.m_Offset;
	updateInfo.range = VK_WHOLE_SIZE;

	VkWriteDescriptorSet updateWriteInfo = {};

	updateWriteInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	updateWriteInfo.dstSet = *updateData.m_DstSet;
	updateWriteInfo.descriptorCount = updateData.m_DstCount;
	updateWriteInfo.descriptorType = updateData.m_Type;
	updateWriteInfo.pBufferInfo = &updateInfo;
	updateWriteInfo.dstArrayElement = 0;
	updateWriteInfo.dstBinding = updateData.m_Binding;
	updateWriteInfo.pTexelBufferView = updateData.m_TextelBuff;
	updateWriteInfo.pImageInfo = updateData.m_ImageInfo;
	updateWriteInfo.pNext = updateData.m_pNext;

	vkUpdateDescriptorSets(device, 1, &updateWriteInfo, 0, 0);
}

VkResult vkGenerics::CreateComputePipeline(const VkDevice& device, VkPipeline& pipeline, VkComputePipelineCreateInfo pipelineInfo)
{
	VkResult result;
	result = vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline);
	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L" vkGenerics::CreateComputePipeline()", L"Failed to create pipleine", L"Check the outputted error message from Vulkan");
	}
	return result;
}

VkResult vkGenerics::CreateGraphicsPipeline(const VkDevice& device, VkPipeline& pipeline, VkGraphicsPipelineCreateInfo pipelineInfo)
{
	VkResult result;
	result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline);
	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L" vkGenerics::CreateComputePipeline()", L"Failed to create pipleine", L"Check the outputted error message from Vulkan");
	}
	return result;
}

void vkGenerics::BindGraphicsPipeline(const VkCommandBuffer& commandBuff, const VkPipeline& pipeline)
{
	vkCmdBindPipeline(commandBuff, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

VkResult vkGenerics::CreatePipelineLayout(const VkDevice& device, VkPipelineLayout& layout, VkPipelineLayoutCreateInfo pipelineLayoutInfo)
{
	VkResult result;
	result = vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &layout);
	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L" vkGenerics::CreatePipelineLayout()", L"Failed to create pipleine layout", L"Check the outputted error message from Vulkan");
	}
	return result;
}

VkResult vkGenerics::CreatePipelineLayout(const VkDevice& device, VkPipelineLayout& layout, std::vector<VkDescriptorSetLayout> descSets)
{
	VkResult result;

	VkPipelineLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layoutInfo.setLayoutCount = UnsignedRecast<uint32_t, size_t>(descSets.size());
	layoutInfo.pSetLayouts = descSets.data();
	layoutInfo.pushConstantRangeCount = 0;
	layoutInfo.pPushConstantRanges = nullptr;

	result = vkCreatePipelineLayout(device, &layoutInfo, nullptr, &layout);
	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L" vkGenerics::CreatePipelineLayout()", L"Failed to create pipleine layout", L"Check the outputted error message from Vulkan");
	}
	return result;
}

VkResult vkGenerics::CreateSemaphore(const VkDevice& device, VkSemaphore& semaphore, VkSemaphoreCreateFlags flags)
{
	VkResult result;
	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreCreateInfo.pNext = nullptr;
	semaphoreCreateInfo.flags = flags;
	result = vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphore);
	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L" vkGenerics::CreateSemaphore()", L"Failed to create a semaphore", L"Check the flag inputted");
	}
	return result;
}

VkResult vkGenerics::CreateFence(const VkDevice& device, VkFence& fence, VkFenceCreateFlags flags)
{
	VkResult result;
	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.pNext = nullptr;
	fenceCreateInfo.flags = flags;
	result = vkCreateFence(device, &fenceCreateInfo, nullptr, &fence);
	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L" vkGenerics::CreateFence()", L"Failed to create fence", L"Check the flag inputted");
	}
	return result;
}

VkResult vkGenerics::CreateVkEvent(const VkDevice& device, VkEvent& vkEvent)
{
	VkResult result;
	VkEventCreateInfo eventCreateInfo;
	eventCreateInfo.flags = VK_EVENT_CREATE_DEVICE_ONLY_BIT;
	eventCreateInfo.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
	eventCreateInfo.pNext = VK_NULL_HANDLE;
	result = vkCreateEvent(device, &eventCreateInfo, nullptr , &vkEvent);
	return result;
}

void vkGenerics::SetComputeEvent(VkCommandBuffer& buffer, VkEvent& vkEvent)
{
	vkCmdSetEvent(buffer, vkEvent, VK_PIPELINE_BIND_POINT_COMPUTE);
}

void vkGenerics::WaitOnEvent(VkCommandBuffer& buffer, VkEvent& vkEvent)
{
	VkDependencyInfo depInfo = {};
	depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	depInfo.pNext = VK_NULL_HANDLE;
	depInfo.dependencyFlags = VK_DEPENDENCY_DEVICE_GROUP_BIT;
	depInfo.bufferMemoryBarrierCount = 0;
	depInfo.imageMemoryBarrierCount = 0;
	depInfo.memoryBarrierCount= 0;
	depInfo.pBufferMemoryBarriers = VK_NULL_HANDLE;
	depInfo.pMemoryBarriers = VK_NULL_HANDLE;
	depInfo.pImageMemoryBarriers = VK_NULL_HANDLE;
	vkCmdWaitEvents2(buffer, 0, &vkEvent, &depInfo);
}

uint32_t vkGenerics::FindMemoryIndex(uint32_t allowedTypes, VkMemoryPropertyFlags propertyFlags, const VkPhysicalDevice& device)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(device, &memProperties);
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((allowedTypes & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & propertyFlags) == propertyFlags))
		{
			return i;
		}
	}

	vkDebug::ThrowError(L" vkGenerics::FindMemoryIndex()", L"Unable to find memory index", L"Check the outputted error message if one is generated");

	return -1;
}

void vkGenerics::GetBufferMemoryRequirements(const VkDevice& device, VkBuffer& buffer, VkMemoryRequirements& memRequirments)
{
	vkGetBufferMemoryRequirements(device, buffer, &memRequirments);
}

VkResult vkGenerics::CreateMemoryBuffer(const VkDevice& device, VkBuffer& buffer, VkBufferCreateInfo& bufferInfo)
{
	VkResult result;
	result = vkCreateBuffer(device, &bufferInfo, nullptr, &buffer);
	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L" vkGenerics::CreateMemoryBuffer()", L"Failed to create VkBuffer", L"Check output message");
	}
	return result;
}

VkResult vkGenerics::CreateMemoryBuffer(const VkDevice& device, VkBuffer& buffer, uint32_t size, VkBufferUsageFlags usageMode, VkSharingMode sharingMode, uint32_t queueFamilyIndexCount, uint32_t* queueFamilyIndices)
{
	VkResult result;

	VkBufferCreateInfo bufferInfo = {};

	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usageMode;
	bufferInfo.sharingMode = sharingMode;
	bufferInfo.pQueueFamilyIndices = queueFamilyIndices;
	bufferInfo.pNext = VK_NULL_HANDLE;
	bufferInfo.queueFamilyIndexCount = queueFamilyIndexCount;

	result = vkCreateBuffer(device, &bufferInfo, nullptr, &buffer);
	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L" vkGenerics::CreateMemoryBuffer()", L"Failed to create VkBuffer", L"Check output message");
	}
	return result;
}

VkResult vkGenerics::AllocateMemory(const VkDevice& device, VkDeviceMemory& buffer, VkMemoryAllocateInfo allocateInfo)
{
	VkResult result;
	result = vkAllocateMemory(device, &allocateInfo, nullptr, &buffer);
	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L" vkGenerics::AllocateMemory()", L"Failed to allocate device memory", L"Check output message");
	}
	return result;
}

VkResult vkGenerics::AllocateMemory(const VkDevice& device, VkDeviceMemory& deviceMemory, VkBuffer& buffer, uint32_t memoryIndex)
{
	VkMemoryRequirements memRequirments;
	vkGetBufferMemoryRequirements(device, buffer, &memRequirments);
	VkMemoryAllocateInfo allocateMemInfo = {};
	allocateMemInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateMemInfo.allocationSize = memRequirments.size;
	allocateMemInfo.pNext = nullptr;
	allocateMemInfo.memoryTypeIndex = memoryIndex;

	VkResult result;
	result = vkAllocateMemory(device, &allocateMemInfo, nullptr, &deviceMemory);
	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L" vkGenerics::AllocateMemory()", L"Failed to allocate device memory", L"Check output message");
	}
	return result;
}

VkResult vkGenerics::BindBufferToMemory(const VkDevice& device, VkDeviceMemory& deviceMemory, VkBuffer& buffer)
{
	VkResult result; 
	result = vkBindBufferMemory(device, buffer, deviceMemory, 0);
	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L" vkGenerics::BindBufferToMemory()", L"Failed to bind buffers to memory", L"Check output message");
	}
	return result;
}

VkResult vkGenerics::BindBufferToMemory(const VkDevice& device, VkDeviceMemory& deviceMemory, VkBuffer& buffer, uint32_t offset)
{
	VkResult result;
	result = vkBindBufferMemory(device, buffer, deviceMemory, offset);
	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L" vkGenerics::BindBufferToMemory()", L"Failed to bind buffers to memory", L"Check output message");
	}
	return result;
}

VkResult vkGenerics::MapMemoryToPointer(const VkDevice& device, VkDeviceMemory& deviceMemory, void** address, uint32_t sizeOfBuffer)
{
	VkResult result;
	result = vkMapMemory(device, deviceMemory, 0, sizeOfBuffer, 0,  address);
	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L" vkGenerics::MapMemoryToPointer()", L"Failed to map memory onto a void pointer", L"Check output message");
	}
	return result;
}

VkResult vkGenerics::MapMemoryToPointer(const VkDevice& device, VkDeviceMemory& deviceMemory, VkDeviceSize offset, void** address, uint32_t sizeOfBuffer)
{
	VkResult result;
	result = vkMapMemory(device, deviceMemory, offset, sizeOfBuffer, 0, address);
	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L" vkGenerics::MapMemoryToPointer()", L"Failed to map memory onto a void pointer", L"Check output message");
	}
	return result;
}

VkResult vkGenerics::MapMemoryToPointer(const VkDevice& device, VkDeviceMemory& deviceMemory, void* address, uint32_t sizeOfBuffer)
{
	VkResult result;
	result = vkMapMemory(device, deviceMemory, 0, sizeOfBuffer, 0, &address);
	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L" vkGenerics::MapMemoryToPointer()", L"Failed to map memory onto a void pointer", L"Check output message");
	}
	return result;
}

VkResult vkGenerics::MapMemoryToPointer(const VkDevice& device, VkDeviceMemory& deviceMemory, VkDeviceSize offset, void* address, uint32_t sizeOfBuffer)
{
	VkResult result;
	result = vkMapMemory(device, deviceMemory, offset, sizeOfBuffer, 0, &address);
	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L" vkGenerics::MapMemoryToPointer()", L"Failed to map memory onto a void pointer", L"Check output message");
	}
	return result;
}

void vkGenerics::UnmapMemory(const VkDevice& device, VkDeviceMemory& deviceMemory)
{
	vkUnmapMemory(device, deviceMemory);
}

void vkGenerics::BindComputeMemory(const VkCommandBuffer& commandBuff, const VkPipelineLayout& pipeLayout, const VkDescriptorSet* descSets)
{
	vkCmdBindDescriptorSets(commandBuff, VK_PIPELINE_BIND_POINT_COMPUTE, pipeLayout, 0, 2, descSets, 0, nullptr);
}

VkResult vkGenerics::CreateRenderpass(VkDevice& device, VkRenderPass& renderpass ,VkFormat& format)
{
	VkAttachmentDescription colourAttachment = {};
	colourAttachment.format = format;
	colourAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colourAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colourAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colourAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colourAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colourAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colourAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colourAttachmentReference = {};
	colourAttachmentReference.attachment = 0;
	colourAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colourAttachmentReference;

	//std::vector<VkSubpassDependency> subpassDependancies(2);
	//subpassDependancies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	//subpassDependancies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	//subpassDependancies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;

	//subpassDependancies[0].dstSubpass = 0;
	//subpassDependancies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	//subpassDependancies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	//subpassDependancies[0].dependencyFlags = 0;

	//subpassDependancies[1].srcSubpass = VK_SUBPASS_EXTERNAL;
	//subpassDependancies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	//subpassDependancies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	//subpassDependancies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	//subpassDependancies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	//subpassDependancies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	//subpassDependancies[1].dependencyFlags = 0;


	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.pNext = nullptr;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colourAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	//renderPassInfo.dependencyCount = static_cast<uint32_t>(subpassDependancies.size());
	//renderPassInfo.pDependencies = subpassDependancies.data();

	VkResult result = vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderpass);

	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L" vkGenerics::CreateRenderpass()", L"Failed to create renderpass", L"See outputted error.");
	}

	return result;
}

VkResult vkGenerics::RecordGraphicsCommands(VkPipeline& pipeline, VkCommandBuffer& buffer, VkRenderPass& renderPass, VkFramebuffer& frameBuffer, VkExtent2D& extent, VkBuffer& VertexBuffer, uint32_t vertexCount)
{
	VkResult result;

	VkCommandBufferBeginInfo bufBeginInfo = {};
	bufBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	bufBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	// Information about how to begin a render pass (only needed for graphical applications)
	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = renderPass;							// Render Pass to begin
	renderPassBeginInfo.renderArea.offset = { 0, 0 };						// Start point of render pass in pixels
	renderPassBeginInfo.renderArea.extent = extent;						// Size of region to run render pass on (starting at offset)
	VkClearValue clearValues[] = { {1.f, 1.f, 1.f, 1.f} };
	renderPassBeginInfo.pClearValues = clearValues;							// List of clear values (TODO: Depth Attachment Clear Value)
	renderPassBeginInfo.clearValueCount = 1;
	renderPassBeginInfo.framebuffer = frameBuffer;

	result = vkBeginCommandBuffer(buffer, &bufBeginInfo);
	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L" vkGenerics::RecordCommands()", L"Failed to start recording a Command buffer", L"See outputted error.");
	}

	//Begin and end render pass
	vkCmdBeginRenderPass(buffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	// Bind Pipeline to be used in render pass
	vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	VkBuffer vertexBuffers[] = { VertexBuffer };					// Buffers to bind
	VkDeviceSize offsets[] = { 0 };							
	vkCmdBindVertexBuffers(buffer, 0, 1, vertexBuffers, offsets);	// Command to bind vertex buffer before drawing with 
	vkCmdDraw(buffer, vertexCount, 1, 0, 0);
	vkCmdEndRenderPass(buffer);

	result = vkEndCommandBuffer(buffer);

	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L" vkGenerics::RecordCommands()", L"Failed to stop recording a Command buffer ", L"See outputted error.");
	}

	return result;
}

VkResult vkGenerics::PresentImage(VkQueue& queue, VkSwapchainKHR& swapchain, uint32_t currentFrame , std::vector<VkSemaphore>& semaphore)
{
	VkResult result;

	//Present Image
	VkPresentInfoKHR presentInfo;
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.waitSemaphoreCount = semaphore.size();
	presentInfo.pWaitSemaphores = semaphore.data();
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain;
	presentInfo.pImageIndices = &currentFrame;
	presentInfo.pResults = nullptr;
	result = vkQueuePresentKHR(queue, &presentInfo);

	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L" vkGenerics::PresentImage()", L"Failed to present image", L"See outputted error.");
	}

	return result;
}

VkResult vkGenerics::AquireNextFrame(const VkDevice& device, VkSwapchainKHR& swapchain, VkSemaphore& semaphore, uint32_t newFrameIndex)
{
	VkResult result;
	result = vkAcquireNextImageKHR(device, swapchain, std::numeric_limits<uint64_t>::max(), semaphore, VK_NULL_HANDLE, &newFrameIndex);
	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L" vkGenerics::vkAcquireNextImageKHR()", L"Failed to aquire next frame", L"N/A");
	}
	return result;
}

VkResult vkGenerics::WaitOnFence(const VkDevice& device, VkFence& fence)
{
	return vkWaitForFences(device, 1, &fence, VK_TRUE, std::numeric_limits<uint64_t>::max());
}

VkResult vkGenerics::WaitOnFence(const VkDevice& device, VkFence& fence, VkBool32& waitAll, uint64_t waitTime)
{
	return vkWaitForFences(device, 1, &fence, waitAll, waitTime);
}

VkResult vkGenerics::ResetFence(const VkDevice& device, VkFence& fence)
{
	VkResult result;
	result = vkResetFences(device, 1, &fence);
	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L" vkGenerics::ResetFence()", L"Failed to reset", L"Ensure that VkInstance is a valid pointer");
	}
	return result;
}

void vkGenerics::BindComputePipeline(const VkCommandBuffer& commandBuff, const VkPipeline& pipeline)
{
	vkCmdBindPipeline(commandBuff, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
}

VkResult vkGenerics::CreateInstance(VkInstanceCreateInfo createInfo, VkInstance* instance)
{
	VkResult result = vkCreateInstance(&createInfo, nullptr, instance);
	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L" vkGenerics::CreateInstance()", L"Failed to create instance", L"Ensure that VkInstance is a valid pointer");
	}
	return result;
}

VkResult vkGenerics::CreateDebugInstance(VkInstanceCreateInfo createInfo, VkInstance* instance, VkDebugReportCallbackEXT* callback)
{
	std::vector<const char*> layers(createInfo.enabledLayerCount);
	for (size_t i = 0; i < createInfo.enabledLayerCount; i++)
	{
		layers[i] = createInfo.ppEnabledLayerNames[i];
	}
	std::vector<const char*> extensions(createInfo.enabledExtensionCount);
	for (size_t i = 0; i < createInfo.enabledExtensionCount; i++)
	{
		extensions[i] = createInfo.ppEnabledExtensionNames[i];
	}
	layers.push_back("VK_LAYER_KHRONOS_validation");
	extensions.push_back("VK_EXT_debug_report");
	createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
	createInfo.ppEnabledLayerNames = layers.data();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	//Create Instance
	VkResult result = vkCreateInstance(&createInfo, nullptr, instance);
	if (result != VK_SUCCESS)
	{
		vkDebug::ThrowError(L"vkInstanceContainer::DebugInit()", L"Failed to create instance", L"Ensure that VkInstance is a valid pointer");
	}
	*callback = vkDebug::CreateDebugCallback(instance);
	return result;
}


VkDebugReportCallbackEXT vkDebug::CreateDebugCallback(const VkInstance* instance)
{
	PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(*instance, "vkCreateDebugReportCallbackEXT"));
	PFN_vkDebugReportMessageEXT vkDebugReportMessageEXT = reinterpret_cast<PFN_vkDebugReportMessageEXT>(vkGetInstanceProcAddr(*instance, "vkDebugReportMessageEXT"));
	PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(*instance, "vkDestroyDebugReportCallbackEXT"));

	VkDebugReportCallbackCreateInfoEXT callbackCreateInfo = {};
	callbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	callbackCreateInfo.pNext = nullptr;
	callbackCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT |
		VK_DEBUG_REPORT_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
	callbackCreateInfo.pfnCallback = &DebugCallback;
	callbackCreateInfo.pUserData = nullptr;

	///* Register the callback */
	VkDebugReportCallbackEXT callback;
	VkResult result = vkCreateDebugReportCallbackEXT(*instance, &callbackCreateInfo, nullptr, &callback);

	return callback;
}
