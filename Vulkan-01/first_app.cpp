#include "first_app.hpp"

#include <stdexcept>
#include <array>

namespace sorp_v
{
	const std::string SorpSimpleApp::VERTEX_SHADER = "C:\\Users\\daniel.vozovikov\\Documents\\Projects\\Learning\\C++\\Dev\\Vulkan-01\\Vulkan-01\\shaders\\compiled\\simple_shader.vert.spv";
	const std::string SorpSimpleApp::FRAGMENT_SHADER = "C:\\Users\\daniel.vozovikov\\Documents\\Projects\\Learning\\C++\\Dev\\Vulkan-01\\Vulkan-01\\shaders\\compiled\\simple_shader.frag.spv";

	SorpSimpleApp::SorpSimpleApp()
	{
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
		for (size_t i = 0; i < uniformBuffers.size(); i++) {
			vkDestroyBuffer(renderDevice.device(), uniformBuffers[i], nullptr);
			vkFreeMemory(renderDevice.device(), uniformBuffersMemory[i], nullptr);
		}
		vkDestroyDescriptorPool(renderDevice.device(), descriptorPool, nullptr);
		vkDestroyDescriptorSetLayout(renderDevice.device(), descriptorSetLayout, nullptr);
		vkDestroyPipelineLayout(renderDevice.device(), pipelineLayout, nullptr);
	}

	void SorpSimpleApp::run()
	{
		while (!sorpWindow.shouldClose())
		{
			glfwPollEvents();
			drawFrame();
		}

		vkDeviceWaitIdle(renderDevice.device());
	}

	void SorpSimpleApp::loadModels()
	{
		std::vector<SorpModel::Vertex> vertices = {
			{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
			{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
			{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
		};

		std::vector<uint16_t> indexes = {
			0, 1, 2, 2, 3, 0
		};

		sorpModel = std::make_unique<SorpModel>(renderDevice, vertices, indexes);
	}

	void SorpSimpleApp::createDescriptorSetLayout()
	{
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		uboLayoutBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 1;
		layoutInfo.pBindings = &uboLayoutBinding;

		if (vkCreateDescriptorSetLayout(renderDevice.device(), &layoutInfo, nullptr,
			&descriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}

	void SorpSimpleApp::createPipelineLayout()
	{
		VkPipelineLayoutCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineInfo.setLayoutCount = 1;
		pipelineInfo.pSetLayouts = &descriptorSetLayout;
		pipelineInfo.pushConstantRangeCount = 0;
		pipelineInfo.pPushConstantRanges = nullptr;

		if (vkCreatePipelineLayout(renderDevice.device(), &pipelineInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Couldnt create pipeline layout");
		}
	}

	void SorpSimpleApp::createPipeline()
	{
		auto pipelineConfig = SorpPipeline::defaultPipelineConfiguration(swapChain->width(), swapChain->height());
		pipelineConfig.renderPass = swapChain->getRenderPass();
		pipelineConfig.pipelineLayout = pipelineLayout;
		sorpPipeline = std::make_unique<SorpPipeline>(
			renderDevice,
			VERTEX_SHADER,
			FRAGMENT_SHADER,
			pipelineConfig
		);
	}

	void SorpSimpleApp::createCommandBuffers()
	{
		commandBuffers.resize(swapChain->imageCount());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = renderDevice.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(renderDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("failled to allocate command buffer");
		}
	}

	void SorpSimpleApp::drawFrame()
	{
		uint32_t imageIndex;
		auto result = swapChain->acquireNextImage(&imageIndex);

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
		result = swapChain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);

		if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || sorpWindow.wasWindowResized())
		{
			sorpWindow.resetWindowResizedFlag();
			recreateSwapChain();
			return;
		}

		if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}
	}

	void SorpSimpleApp::recreateSwapChain()
	{
		auto extent = sorpWindow.getExtent();
		while (extent.width == 0 || extent.height == 0)
		{
			extent = sorpWindow.getExtent();
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(renderDevice.device());
		swapChain.reset(nullptr);
		swapChain = std::make_unique<SorpSwapChain>(renderDevice, extent);
		createPipeline();
	}

	void SorpSimpleApp::recordCommandBuffer(int imageIndex)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("failled to begin recording command buffer: " + imageIndex);
		}

		VkRenderPassBeginInfo renderPassBegin{};
		renderPassBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBegin.renderPass = swapChain->getRenderPass();
		renderPassBegin.framebuffer = swapChain->getFrameBuffer(imageIndex);

		renderPassBegin.renderArea.offset = { 0, 0 };
		renderPassBegin.renderArea.extent = swapChain->getSwapChainExtent();

		std::array<VkClearValue, 2> clearColors{};
		clearColors[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
		clearColors[1].depthStencil = { 1.0f, 0 };
		renderPassBegin.clearValueCount = static_cast<uint32_t>(clearColors.size());
		renderPassBegin.pClearValues = clearColors.data();

		vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassBegin, VK_SUBPASS_CONTENTS_INLINE);

		sorpPipeline->bind(commandBuffers[imageIndex]);
		sorpModel->bind(commandBuffers[imageIndex]);
		vkCmdBindDescriptorSets(commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout, 0, 1, &descriptorSets[imageIndex], 0, nullptr);

		sorpModel->draw(commandBuffers[imageIndex]);

		vkCmdEndRenderPass(commandBuffers[imageIndex]);
		if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer");
		}
	}

	void SorpSimpleApp::createDescriptorSets()
	{
		std::vector<VkDescriptorSetLayout> layouts(SorpSwapChain::MAX_FRAMES_IN_FLIGHT + 1, descriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(SorpSwapChain::MAX_FRAMES_IN_FLIGHT + 1);
		allocInfo.pSetLayouts = layouts.data();

		descriptorSets.resize(SorpSwapChain::MAX_FRAMES_IN_FLIGHT + 1);
		if (vkAllocateDescriptorSets(renderDevice.device(), &allocInfo, descriptorSets.data()) !=
			VK_SUCCESS) {
			throw std::runtime_error("не вдалося виділити набори дескрипторів!");
		}

		for (size_t i = 0; i < SorpSwapChain::MAX_FRAMES_IN_FLIGHT + 1; i++) {
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = uniformBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObject);

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = descriptorSets[i];
			descriptorWrite.dstBinding = 0;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;
			descriptorWrite.pImageInfo = nullptr;
			descriptorWrite.pTexelBufferView = nullptr;
			vkUpdateDescriptorSets(renderDevice.device(), 1, &descriptorWrite, 0, nullptr);
		}
	}

	void SorpSimpleApp::createUniformBuffers()
	{
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);

		uniformBuffers.resize(SorpSwapChain::MAX_FRAMES_IN_FLIGHT + 1);
		uniformBuffersMemory.resize(SorpSwapChain::MAX_FRAMES_IN_FLIGHT + 1);
		uniformBuffersMapped.resize(SorpSwapChain::MAX_FRAMES_IN_FLIGHT + 1);
		for (size_t i = 0; i < SorpSwapChain::MAX_FRAMES_IN_FLIGHT + 1; i++) {
			renderDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				uniformBuffers[i], uniformBuffersMemory[i]);
			vkMapMemory(renderDevice.device(), uniformBuffersMemory[i], 0, bufferSize, 0,
				&uniformBuffersMapped[i]);
		}
	}

	void SorpSimpleApp::createDescriptorPool()
	{
		VkDescriptorPoolSize poolSize{};
		poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSize.descriptorCount = static_cast<uint32_t>(SorpSwapChain::MAX_FRAMES_IN_FLIGHT + 1);

		VkDescriptorPoolCreateInfo:VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = 1;
		poolInfo.pPoolSizes = &poolSize;
		poolInfo.maxSets = static_cast<uint32_t>(SorpSwapChain::MAX_FRAMES_IN_FLIGHT + 1);

		if (vkCreateDescriptorPool(renderDevice.device(), &poolInfo, nullptr, &descriptorPool) !=
			VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}

	void SorpSimpleApp::updateUniformBuffer(int imageIndex) {
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float,
			std::chrono::seconds::period>(currentTime - startTime).count();

		auto swapChainExtent = sorpWindow.getExtent();

		UniformBufferObject ubo{};
		ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f),
			glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f,
			0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width /
			(float)swapChainExtent.height, 0.1f, 10.0f);
		ubo.proj[1][1] *= -1;
		memcpy(uniformBuffersMapped[imageIndex], &ubo, sizeof(ubo));
	}
}
