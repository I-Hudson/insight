#pragma once

#if defined(IS_VULKAN_ENABLED)

#include "Graphics/RenderContext.h"
#include "Graphics/RHI/RHI_Buffer.h"
#include "Graphics/RHI/Vulkan/PipelineStateObject_Vulkan.h"
#include "Graphics/RHI/Vulkan/RHI_Descriptor_Vulkan.h"
#include "Graphics/RHI/RHI_GPUCrashTracker.h"

#include "VmaUsage.h"
#include "Graphics/RenderGraph/RenderGraph.h"

#include <glm/vec2.hpp>
#include <vulkan/vulkan.hpp>
#include <unordered_map>
#include <set>
#include <string>

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::Vulkan
		{
			struct QueueInfo
			{
				int FamilyQueueIndex;
				GPUQueue Queue;
			};

			/// <summary>
			/// Store all the objects needed for a single frame submit.
			/// </summary>
			struct FrameSubmitContext
			{
				std::vector<RHI_CommandList*> CommandLists;
				vk::Fence SubmitFences;
				vk::Semaphore SwapchainAcquires;
				vk::Semaphore SignalSemaphores;
			};

			class RenderContext_Vulkan : public RenderContext
			{
			public:
				virtual bool Init() override;
				virtual void Destroy() override;

				virtual void InitImGui() override;
				virtual void DestroyImGui() override;

				virtual bool PrepareRender() override;
				virtual void PreRender(RHI_CommandList* cmdList) override;
				virtual void PostRender(RHI_CommandList* cmdList) override;

				virtual void SetSwaphchainResolution(glm::ivec2 resolution) override;
				virtual glm::ivec2 GetSwaphchainResolution() const override;

				virtual void GpuWaitForIdle() override;
				virtual void SubmitCommandListAndWait(RHI_CommandList* cmdList) override;

				void SetObejctName(std::wstring_view name, u64 handle, vk::ObjectType objectType);
				void BeginDebugMarker(std::string_view block_name);
				void EndDebugMarker();

				vk::Device GetDevice() const { return m_device; }
				vk::PhysicalDevice GetPhysicalDevice() const { return m_adapter; }
				VmaAllocator GetVMA() const { return m_vmaAllocator; }

				u32 GetFamilyQueueIndex(GPUQueue queue) const { return m_queueFamilyLookup.at(queue); }

				virtual RHI_Texture* GetSwaphchainIamge() const override;
				vk::ImageView GetSwapchainImageView() const;
				vk::Format GetSwapchainColourFormat() const { return m_swapchainFormat; }
				vk::SwapchainKHR GetSwapchain() const { return m_swapchain; }

				PipelineLayoutManager_Vulkan& GetPipelineLayoutManager() { return m_pipelineLayoutManager; }
				PipelineStateObjectManager_Vulkan& GetPipelineStateObjectManager() { return m_pipelineStateObjectManager; }

			protected:
				virtual void WaitForGpu() override;

			private:
				vk::Instance CreateInstance();
				vk::PhysicalDevice FindAdapter();
				std::vector<vk::DeviceQueueCreateInfo> GetDeviceQueueCreateInfos(std::vector<QueueInfo>& queueInfo);
				void GetDeviceExtensionAndLayers(std::set<std::string>& extensions, std::set<std::string>& layers, bool includeAll = false);
				void CreateSwapchain(u32 width, u32 height);

				void SetDeviceExtensions();
				bool CheckInstanceExtension(const char* extension);
				bool CheckForDeviceExtension(const char* extension);

			private:
				vk::Instance m_instnace{ nullptr };
				vk::Device m_device{ nullptr };
				vk::PhysicalDevice m_adapter{ nullptr };

				VmaAllocator m_vmaAllocator{ nullptr };

				vk::SurfaceKHR m_surface{ nullptr };
				vk::SwapchainKHR m_swapchain{ nullptr };
				vk::Format m_swapchainFormat;
				std::vector<RHI_Texture*> m_swapchainImages;
				glm::ivec2 m_swapchainBufferSize;

				std::unordered_map<GPUQueue, vk::Queue> m_commandQueues;
				std::unordered_map<GPUQueue, std::mutex> m_command_queue_mutexs;
				std::unordered_map<GPUQueue, u32> m_queueFamilyLookup;

				PipelineLayoutManager_Vulkan m_pipelineLayoutManager;
				PipelineStateObjectManager_Vulkan m_pipelineStateObjectManager;

				vk::DescriptorPool m_imguiDescriptorPool;
				vk::RenderPass m_imguiRenderpass;
				std::array<vk::Framebuffer, 0> m_imguiFramebuffers;

				u32 m_currentFrame = 0;
				u32 m_availableSwapchainImage = 0;

				Insight::Graphics::RHI_GPUCrashTracker* m_gpuCrashTracker = nullptr;

			public:
				void* m_descriptor_pool = nullptr;

			private:

				FrameResource<FrameSubmitContext> m_submitFrameContexts;
			};
		}
	}
}

#endif ///#if defined(IS_VULKAN_ENABLED)