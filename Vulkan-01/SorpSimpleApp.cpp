#include "SorpSimpleApp.hpp"

#include <stdexcept>
#include <array>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace sorp_v
{
	const std::string SorpSimpleApp::VERTEX_SHADER = "shaders\\compiled\\simple_shader.vert.spv";
	const std::string SorpSimpleApp::FRAGMENT_SHADER = "shaders\\compiled\\simple_shader.frag.spv";
	const std::string SorpSimpleApp::DEFAULT_TEXTURE = "textures\\0.jpg";

	SorpSimpleApp::SorpSimpleApp()
	{
		createTextureImage();
		createTextureImageView();
		createTextureSampler();
		
		loadModels();
		createUniformBuffers();
		createDescriptorSetLayout();
		createPipelineLayout();
		createDescriptorPool();
		createDescriptorSets();
		recreateSwapChain();
		createCommandBuffers();
	}

	SorpSimpleApp::~SorpSimpleApp() 
	{
		vkDestroySampler(_renderDevice.device(), _textureSampler, nullptr);
		vkDestroyImageView(_renderDevice.device(), _textureImageView, nullptr);
		vkDestroyImage(_renderDevice.device(), _textureImage, nullptr);
		vkFreeMemory(_renderDevice.device(), _textureImageMemory, nullptr);


		for (size_t i = 0; i < _uniformBuffers.size(); i++) {
			vkDestroyBuffer(_renderDevice.device(), _uniformBuffers[i], nullptr);
			vkFreeMemory(_renderDevice.device(), _uniformBuffersMemory[i], nullptr);
		}
		vkDestroyDescriptorPool(_renderDevice.device(), _descriptorPool, nullptr);
		vkDestroyDescriptorSetLayout(_renderDevice.device(), _descriptorSetLayout, nullptr);
		vkDestroyPipelineLayout(_renderDevice.device(), _pipelineLayout, nullptr);
	}

	void SorpSimpleApp::run()
	{
		while (!_sorpWindow.shouldClose())
		{
			glfwPollEvents();
			drawFrame();
		}

		vkDeviceWaitIdle(_renderDevice.device());
	}

	void SorpSimpleApp::loadModels()
	{
		std::vector<SorpModel::Vertex> vertices = {
			{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
			{{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
			{{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
			{{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},

			{{-0.5f, -0.5f, 0.5}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
			{{0.5f, -0.5f, 0.5}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
			{{0.5f, 0.5f, 0.5}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
			{{-0.5f, 0.5f, 0.5}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
		};

		std::vector<uint16_t> indexes = {
			4, 5, 6, 6, 7, 4,
			0, 2, 1, 2, 0, 3,
			0, 5, 4, 0, 1, 5,
			2, 5, 1, 2, 6, 5,
			3, 6, 2, 3, 7, 6,
			0, 4, 7, 0, 7, 3
		};

		_sorpModel = std::make_unique<SorpModel>(_renderDevice, vertices, indexes);
	}

	void SorpSimpleApp::createDescriptorSetLayout()
	{
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		uboLayoutBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.binding = 1;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(_renderDevice.device(), &layoutInfo, nullptr,
			&_descriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}

	void SorpSimpleApp::createPipelineLayout()
	{
		VkPipelineLayoutCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineInfo.setLayoutCount = 1;
		pipelineInfo.pSetLayouts = &_descriptorSetLayout;
		pipelineInfo.pushConstantRangeCount = 0;
		pipelineInfo.pPushConstantRanges = nullptr;

		if (vkCreatePipelineLayout(_renderDevice.device(), &pipelineInfo, nullptr, &_pipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Couldnt create pipeline layout");
		}
	}

	void SorpSimpleApp::createPipeline()
	{
		auto pipelineConfig = SorpPipeline::defaultPipelineConfiguration(_swapChain->width(), _swapChain->height());
		pipelineConfig.renderPass = _swapChain->getRenderPass();
		pipelineConfig.pipelineLayout = _pipelineLayout;
		_sorpPipeline = std::make_unique<SorpPipeline>(
			_renderDevice,
			_sorpPathResolver.resolve(VERTEX_SHADER),
			_sorpPathResolver.resolve(FRAGMENT_SHADER),
			pipelineConfig
		);
	}

	void SorpSimpleApp::createCommandBuffers()
	{
		_commandBuffers.resize(_swapChain->imageCount());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = _renderDevice.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(_commandBuffers.size());

		if (vkAllocateCommandBuffers(_renderDevice.device(), &allocInfo, _commandBuffers.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("failled to allocate command buffer");
		}
	}

	void SorpSimpleApp::drawFrame()
	{
		uint32_t imageIndex;
		auto result = _swapChain->acquireNextImage(&imageIndex);

		if(result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			recreateSwapChain();
			return;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("failed to acquire swap chain image");
		}

		updateUniformBuffer(imageIndex);
		recordCommandBuffer(imageIndex);
		result = _swapChain->submitCommandBuffers(&_commandBuffers[imageIndex], &imageIndex);

		if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || _sorpWindow.wasWindowResized())
		{
			_sorpWindow.resetWindowResizedFlag();
			recreateSwapChain();
			return;
		}

		if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}
	}

	void SorpSimpleApp::recreateSwapChain()
	{
		auto extent = _sorpWindow.getExtent();
		while (extent.width == 0 || extent.height == 0)
		{
			extent = _sorpWindow.getExtent();
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(_renderDevice.device());
		_swapChain.reset(nullptr);
		_swapChain = std::make_unique<SorpSwapChain>(_renderDevice, extent);
		createPipeline();
	}

	void SorpSimpleApp::recordCommandBuffer(int imageIndex)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(_commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("failled to begin recording command buffer: " + imageIndex);
		}

		VkRenderPassBeginInfo renderPassBegin{};
		renderPassBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBegin.renderPass = _swapChain->getRenderPass();
		renderPassBegin.framebuffer = _swapChain->getFrameBuffer(imageIndex);

		renderPassBegin.renderArea.offset = { 0, 0 };
		renderPassBegin.renderArea.extent = _swapChain->getSwapChainExtent();

		std::array<VkClearValue, 2> clearColors{};
		clearColors[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
		clearColors[1].depthStencil = { 1.0f, 0 };
		renderPassBegin.clearValueCount = static_cast<uint32_t>(clearColors.size());
		renderPassBegin.pClearValues = clearColors.data();

		vkCmdBeginRenderPass(_commandBuffers[imageIndex], &renderPassBegin, VK_SUBPASS_CONTENTS_INLINE);

		_sorpPipeline->bind(_commandBuffers[imageIndex]);
		_sorpModel->bind(_commandBuffers[imageIndex]);
		vkCmdBindDescriptorSets(_commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS,
			_pipelineLayout, 0, 1, &_descriptorSets[imageIndex], 0, nullptr);

		_sorpModel->draw(_commandBuffers[imageIndex]);

		vkCmdEndRenderPass(_commandBuffers[imageIndex]);
		if (vkEndCommandBuffer(_commandBuffers[imageIndex]) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer");
		}
	}

	void SorpSimpleApp::createDescriptorSets()
	{
		std::vector<VkDescriptorSetLayout> layouts(SorpSwapChain::MAX_FRAMES_IN_FLIGHT + 1, _descriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = _descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(SorpSwapChain::MAX_FRAMES_IN_FLIGHT + 1);
		allocInfo.pSetLayouts = layouts.data();

		_descriptorSets.resize(SorpSwapChain::MAX_FRAMES_IN_FLIGHT + 1);
		if (vkAllocateDescriptorSets(_renderDevice.device(), &allocInfo, _descriptorSets.data()) !=
			VK_SUCCESS) {
			throw std::runtime_error("не вдалося виділити набори дескрипторів!");
		}

		for (size_t i = 0; i < SorpSwapChain::MAX_FRAMES_IN_FLIGHT + 1; i++) {
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = _uniformBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObject);

			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = _textureImageView;
			imageInfo.sampler = _textureSampler;

			std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = _descriptorSets[i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;

			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = _descriptorSets[i];
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pImageInfo = &imageInfo;

			vkUpdateDescriptorSets(_renderDevice.device(), static_cast<uint32_t>(descriptorWrites.size()), 
				descriptorWrites.data(), 0, nullptr);

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = _descriptorSets[i];
			descriptorWrite.dstBinding = 0;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;
			descriptorWrite.pImageInfo = nullptr;
			descriptorWrite.pTexelBufferView = nullptr;
			vkUpdateDescriptorSets(_renderDevice.device(), 1, &descriptorWrite, 0, nullptr);
		}
	}

	void SorpSimpleApp::createUniformBuffers()
	{
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);

		_uniformBuffers.resize(SorpSwapChain::MAX_FRAMES_IN_FLIGHT + 1);
		_uniformBuffersMemory.resize(SorpSwapChain::MAX_FRAMES_IN_FLIGHT + 1);
		_uniformBuffersMapped.resize(SorpSwapChain::MAX_FRAMES_IN_FLIGHT + 1);
		for (size_t i = 0; i < SorpSwapChain::MAX_FRAMES_IN_FLIGHT + 1; i++) {
			_renderDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				_uniformBuffers[i], _uniformBuffersMemory[i]);
			vkMapMemory(_renderDevice.device(), _uniformBuffersMemory[i], 0, bufferSize, 0,
				&_uniformBuffersMapped[i]);
		}
	}

	void SorpSimpleApp::createDescriptorPool()
	{
		std::array<VkDescriptorPoolSize, 2> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(SorpSwapChain::MAX_FRAMES_IN_FLIGHT + 1);
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = static_cast<uint32_t>(SorpSwapChain::MAX_FRAMES_IN_FLIGHT + 1);

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(SorpSwapChain::MAX_FRAMES_IN_FLIGHT + 1);

		if (vkCreateDescriptorPool(_renderDevice.device(), &poolInfo, nullptr, &_descriptorPool) !=
			VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}

	void SorpSimpleApp::updateUniformBuffer(int imageIndex) {
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float,
			std::chrono::seconds::period>(currentTime - startTime).count();

		auto swapChainExtent = _sorpWindow.getExtent();

		UniformBufferObject ubo{};
		ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f),
			glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f,
			0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width /
			(float)swapChainExtent.height, 0.1f, 10.0f);
		ubo.proj[1][1] *= -1;
		ubo.time = time;
		memcpy(_uniformBuffersMapped[imageIndex], &ubo, sizeof(ubo));
	}

	void SorpSimpleApp::createTextureImage()
	{
		int width, height, channels;
		stbi_uc* pixels = stbi_load(_sorpPathResolver.resolve(DEFAULT_TEXTURE).c_str(), &width, &height, &channels, STBI_rgb_alpha);
		VkDeviceSize imageSize = width * height * 4;

		if(!pixels)
		{
			throw std::runtime_error("failed to load texture image!");
		}

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		_renderDevice.createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);
		void* data;
		vkMapMemory(_renderDevice.device(), stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(_renderDevice.device(), stagingBufferMemory);

		stbi_image_free(pixels);
		
		createImage(static_cast<uint32_t>(width), static_cast<uint32_t>(height), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, 
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _textureImage, _textureImageMemory);

		_renderDevice.transitionImageLayout(_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		_renderDevice.copyBufferToImage(stagingBuffer, _textureImage, static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1);
		_renderDevice.transitionImageLayout(_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		vkDestroyBuffer(_renderDevice.device(), stagingBuffer, nullptr);
		vkFreeMemory(_renderDevice.device(), stagingBufferMemory, nullptr);
	}

	void SorpSimpleApp::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
		VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
	{
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.flags = 0;

		if (vkCreateImage(_renderDevice.device(), &imageInfo, nullptr, &image) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create texture!");
		}

		VkMemoryRequirements memoryRequirements;
		vkGetImageMemoryRequirements(_renderDevice.device(), image, &memoryRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memoryRequirements.size;
		allocInfo.memoryTypeIndex = _renderDevice.findMemoryType(memoryRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(_renderDevice.device(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate image memory!");
		}

		vkBindImageMemory(_renderDevice.device(), image, imageMemory, 0);
	}

	void SorpSimpleApp::createTextureImageView()
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = _textureImage;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(_renderDevice.device(), &viewInfo, nullptr, &_textureImageView) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture image view!");
		}
	}

	void SorpSimpleApp::createTextureSampler()
	{
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = _renderDevice.properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;


		if (vkCreateSampler(_renderDevice.device(), &samplerInfo, nullptr, &_textureSampler) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture sampler!");
		}
	}
}
