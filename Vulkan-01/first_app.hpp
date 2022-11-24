#pragma once

#include "sorp_v_window.hpp"
#include "sorp_v_pipeline.hpp"
#include "sorp_v_device.hpp"
#include "sorp_v_swap_chain.hpp"

#include <memory>
#include <vector>

namespace sorp_v {
	class SorpSimpleApp
	{
	public:
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
		SorpSwapChain swapChain{ renderDevice, sorpWindow.getExtent() };
		std::unique_ptr<SorpPipeline> sorpPipeline;
		VkPipelineLayout pipelineLayout;
		std::vector<VkCommandBuffer> commandBuffers;

		void createPipelineLayout();
		void createPipeline();
		void createCommanBuffers();
		void drawFrame();
	};
}