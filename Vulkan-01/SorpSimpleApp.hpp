#pragma once

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>

#include "SorpPathResolver.h"
#include "SorpWindow.hpp"
#include "SorpPipeline.hpp"
#include "SorpRenderDevice.hpp"
#include "SorpSwapChain.hpp"
#include "SorpModel.hpp"

#include <memory>
#include <vector>

namespace sorp_v {
	class SorpSimpleApp
	{
	public:
		struct UniformBufferObject {
			glm::mat4 model;
			glm::mat4 view;
			glm::mat4 proj;
			float time;
			glm::vec3 _padding;
		};

		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		static const std::string VERTEX_SHADER;
		static const std::string FRAGMENT_SHADER;
		static const std::string DEFAULT_TEXTURE;

		SorpSimpleApp();
		~SorpSimpleApp();

		SorpSimpleApp(const SorpSimpleApp&) = delete;
		SorpSimpleApp& operator=(const SorpSimpleApp&) = delete;

		void run();

	private:
		SorpWindow _sorpWindow{ WIDTH, HEIGHT, "SorpSimpleApp" };
		SorpPathResolver _sorpPathResolver;
		SorpRenderDevice _renderDevice{ _sorpWindow };
		std::unique_ptr<SorpSwapChain> _swapChain;
		std::unique_ptr<SorpPipeline> _sorpPipeline;
		VkDescriptorPool _descriptorPool;
		VkDescriptorSetLayout _descriptorSetLayout;
		VkPipelineLayout _pipelineLayout;
		std::vector<VkCommandBuffer> _commandBuffers;
		std::unique_ptr<SorpModel> _sorpModel;

		std::vector<VkBuffer> _uniformBuffers;
		std::vector<VkDeviceMemory> _uniformBuffersMemory;
		std::vector<void*> _uniformBuffersMapped;
		
		std::vector<VkDescriptorSet> _descriptorSets;

		VkImage _textureImage;
		VkImageView _textureImageView;
		VkSampler _textureSampler;
		VkDeviceMemory _textureImageMemory;

		void loadModels();
		void createDescriptorSetLayout();
		void createPipelineLayout();
		void createPipeline();
		void createCommandBuffers();
		void drawFrame();
		void recreateSwapChain();
		void recordCommandBuffer(int imageIndex);
		void createUniformBuffers();
		void createDescriptorSets();
		void createDescriptorPool();
		void updateUniformBuffer(int imageIndex);
		void createTextureImage();
		void createTextureImageView();
		void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, 
			VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
		void createTextureSampler();
	};
}
