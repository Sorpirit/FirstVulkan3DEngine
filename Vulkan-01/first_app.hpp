#pragma once

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>

#include "sorp_v_window.hpp"
#include "sorp_v_pipeline.hpp"
#include "sorp_v_device.hpp"
#include "sorp_v_swap_chain.hpp"
#include "sorp_v_model.hpp"

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
		};

		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		static const std::string VERTEX_SHADER;
		static const std::string FRAGMENT_SHADER;

		SorpSimpleApp();
		~SorpSimpleApp();

		SorpSimpleApp(const SorpSimpleApp&) = delete;
		SorpSimpleApp& operator=(const SorpSimpleApp&) = delete;

		void run();

	private:
		SorpWindow sorpWindow{ WIDTH, HEIGHT, "SorpSimpleApp" };
		SorpRenderDevice renderDevice{ sorpWindow };
		std::unique_ptr<SorpSwapChain> swapChain;
		std::unique_ptr<SorpPipeline> sorpPipeline;
		VkDescriptorPool descriptorPool;
		VkDescriptorSetLayout descriptorSetLayout;
		VkPipelineLayout pipelineLayout;
		std::vector<VkCommandBuffer> commandBuffers;
		std::unique_ptr<SorpModel> sorpModel;

		std::vector<VkBuffer> uniformBuffers;
		std::vector<VkDeviceMemory> uniformBuffersMemory;
		std::vector<void*> uniformBuffersMapped;
		
		std::vector<VkDescriptorSet> descriptorSets;

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
	};
}