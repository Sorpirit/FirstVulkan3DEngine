#include "first_app.hpp"

#include <stdexcept>
#include <array>

namespace sorp_v
{
	const std::string SorpSimpleApp::VERTEX_SHADER = "C:\\Users\\daniel.vozovikov\\Documents\\Projects\\Learning\\C++\\Dev\\Vulkan-01\\Vulkan-01\\shaders\\simple_shader.vert.spv";
	const std::string SorpSimpleApp::FRAGMENT_SHADER = "C:\\Users\\daniel.vozovikov\\Documents\\Projects\\Learning\\C++\\Dev\\Vulkan-01\\Vulkan-01\\shaders\\simple_shader.frag.spv";

	SorpSimpleApp::SorpSimpleApp()
	{
		createPipelineLayout();
		createPipeline();
		createCommanBuffers();
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
		auto pipelineConfig = SorpPipeline::defaultPipelineConfiguration(swapChain.width(), swapChain.height());
		pipelineConfig.renderPass = swapChain.getRenderPass();
		pipelineConfig.pipelineLayout = pipelineLayout;
		sorpPipeline = std::make_unique<SorpPipeline>(
			renderDevice,
			VERTEX_SHADER,
			FRAGMENT_SHADER,
			pipelineConfig
		);
	}

	void SorpSimpleApp::createCommanBuffers()
	{
		commandBuffers.resize(swapChain.imageCount());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = renderDevice.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(renderDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("failled to allocate command buffer");
		}

		for (int i = 0; i < commandBuffers.size(); i++)
		{
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

			if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) 
			{
				throw std::runtime_error("failled to begin recording command buffer: " + i);
			}

			VkRenderPassBeginInfo renderPassBegin{};
			renderPassBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassBegin.renderPass = swapChain.getRenderPass();
			renderPassBegin.framebuffer = swapChain.getFrameBuffer(i);

			renderPassBegin.renderArea.offset = { 0, 0 };
			renderPassBegin.renderArea.extent = swapChain.getSwapChainExtent();

			std::array<VkClearValue, 2> clearColors{};
			clearColors[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
			clearColors[1].depthStencil = { 1.0f, 0 };
			renderPassBegin.clearValueCount = static_cast<uint32_t>(clearColors.size());
			renderPassBegin.pClearValues = clearColors.data();

			vkCmdBeginRenderPass(commandBuffers[i], &renderPassBegin, VK_SUBPASS_CONTENTS_INLINE);

			sorpPipeline->bind(commandBuffers[i]);
			vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

			vkCmdEndRenderPass(commandBuffers[i]);
			if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to record command buffer");
			}
		}
	}

	void SorpSimpleApp::drawFrame()
	{
		uint32_t imageIndex;
		auto result = swapChain.acquireNextImage(&imageIndex);
	
		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("failed to acquire swap chain image");
		}

		result = swapChain.submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
		if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}
	}
}