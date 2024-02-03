#pragma once

#include "SorpRenderDevice.hpp"

#include <string>
#include <vector>

namespace sorp_v
{
	struct PipelineConfiguration {
		VkViewport viewport;
		VkRect2D scissor;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo rasterizationInfo;
		VkPipelineMultisampleStateCreateInfo multisampleInfo;
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		VkPipelineColorBlendStateCreateInfo colorBlendInfo;
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
		VkPipelineLayout pipelineLayout = nullptr;
		VkRenderPass renderPass = nullptr;
		uint32_t subpass = 0;
	};

	class SorpPipeline
	{
	public:
		SorpPipeline(
			SorpRenderDevice &renderDevice, 
			const std::string vertShader, 
			const std::string fragShader, 
			const PipelineConfiguration& config);
		~SorpPipeline();

		SorpPipeline(const SorpPipeline&) = delete;
		SorpPipeline& operator=(const SorpPipeline&) = delete;
		SorpPipeline() = default;

		static PipelineConfiguration defaultPipelineConfiguration(uint32_t width, uint32_t height);

		void bind(VkCommandBuffer command);

	private:
		SorpRenderDevice& _renderDevice;
		VkPipeline _graphicsPipeline;
		VkShaderModule _vertShaderModel;
		VkShaderModule _fragShaderModel;

		static std::vector<char> readFile(const std::string& filePath);

		void createGraphicsPipeline(const std::string vertShader, const std::string fragShader, const PipelineConfiguration& config);
	
		void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModel);
	};
}