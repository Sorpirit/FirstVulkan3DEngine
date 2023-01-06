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
		createPipelineLayout();
		recreateSwapChain();
		createCommandBuffers();
	}

	SorpSimpleApp::~SorpSimpleApp() 
	{
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

	void SorpSimpleApp::createPipelineLayout()
	{
		VkPipelineLayoutCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineInfo.setLayoutCount = 0;
		pipelineInfo.pSetLayouts = nullptr;
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
		sorpModel->draw(commandBuffers[imageIndex]);

		vkCmdEndRenderPass(commandBuffers[imageIndex]);
		if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer");
		}
	}

	void SorpSimpleApp::createIndexBuffer()
	{
		VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		renderDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
			stagingBufferMemory);
		void* data;
		vkMapMemory(renderDevice.device(), stagingBufferMemory, 0, bufferSize, 0,
			&data);
		memcpy(data, indices.data(), (size_t)bufferSize);
		vkUnmapMemory(renderDevice.device(), stagingBufferMemory);

		renderDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer,
			indexBufferMemory);

		renderDevice.copyBuffer(stagingBuffer, indexBuffer, bufferSize);
		vkDestroyBuffer(renderDevice.device(), stagingBuffer, nullptr);
		vkFreeMemory(renderDevice.device(), stagingBufferMemory, nullptr);
	}
}
