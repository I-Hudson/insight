#include "Graphics/GPU/RHI/Vulkan/GPUCommandList_Vulkan.h"
#include "Graphics/GPU/RHI/Vulkan/GPUPipelineStateObject_Vulkan.h"
#include "Graphics/GPU/RHI/Vulkan/GPUSwapchain_Vulkan.h"
#include "Graphics/GPU/RHI/Vulkan/GPUBuffer_Vulkan.h"
#include "Graphics/GPU/RHI/Vulkan/GPUSemaphore_Vulkan.h"
#include "Graphics/GPU/RHI/Vulkan/GPUFence_Vulkan.h"
#include "Graphics/GPU/RHI/Vulkan/VulkanUtils.h"
#include "Graphics/PixelFormatExtensions.h"
#include "Graphics/Window.h"
#include "Graphics/RenderTarget.h"
#include "Core/Logger.h"

#include <iostream>

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::Vulkan
		{
			GPUCommandList_Vulkan::GPUCommandList_Vulkan()
			{
				m_signalSemaphore = GPUSemaphoreManager::Instance().GetOrCreateSemaphore();
				m_fence = GPUFenceManager::Instance().GetFence();
			}

			GPUCommandList_Vulkan::~GPUCommandList_Vulkan()
			{
				if (m_framebuffer)
				{
					GetDevice()->GetDevice().destroyFramebuffer(m_framebuffer);
					m_framebuffer = nullptr;
				}

				if (m_signalSemaphore)
				{
					GPUSemaphoreManager::Instance().ReturnSemaphore(m_signalSemaphore);
				}

				if (m_fence)
				{
					GPUFenceManager::Instance().ReturnFence(m_fence);
				}
			}

			void GPUCommandList_Vulkan::CopyBufferToBuffer(GPUBuffer* src, GPUBuffer* dst, u64 srcOffset, u64 dstOffset, u64 size)
			{
				if (!m_commandList) { IS_CORE_ERROR("[GPUCommandList_Vulkan::CopyBufferToBuffer] CommandList is null."); return; }
				if (m_state != GPUCommandListState::Recording) { IS_CORE_ERROR("[GPUCommandList_Vulkan::CopyBufferToBuffer] CommandList is not recording."); return; }

				GPUBuffer_Vulkan* srcBufferVulkan = dynamic_cast<GPUBuffer_Vulkan*>(src);
				GPUBuffer_Vulkan* dstBufferVulkan = dynamic_cast<GPUBuffer_Vulkan*>(dst);
				std::array<vk::BufferCopy, 1> regions =
				{ vk::BufferCopy(srcOffset, dstOffset, size) };
				m_commandList.copyBuffer(srcBufferVulkan->GetBuffer(), dstBufferVulkan->GetBuffer(), regions);
				++m_recordCommandCount;
			}

			void GPUCommandList_Vulkan::SetViewport(int width, int height)
			{
				if (!m_commandList) { IS_CORE_ERROR("[GPUCommandList_Vulkan::SetViewport] CommandList is null."); return; }
				if (m_state != GPUCommandListState::Recording) { IS_CORE_ERROR("[GPUCommandList_Vulkan::SetViewport] CommandList is not recording."); return; }

				vk::Viewport viewports[1] = { vk::Viewport(0, 0, static_cast<float>(width),  static_cast<float>(-height), 0, 1) }; // Inverse height as vulkan is from top left, not bottom left.
				if (m_pso.Swapchain)
				{
					viewports[0].height = static_cast<float>(height);
				}
				m_commandList.setViewport(0, 1, viewports);
				++m_recordCommandCount;
			}

			void GPUCommandList_Vulkan::SetScissor(int width, int height)
			{
				if (!m_commandList) { IS_CORE_ERROR("[GPUCommandList_Vulkan::SetScissor] CommandList is null."); return; }
				if (m_state != GPUCommandListState::Recording) { IS_CORE_ERROR("[GPUCommandList_Vulkan::SetScissor] CommandList is not recording."); return; }

				vk::Rect2D scissors[1] = { vk::Rect2D({0, 0}, {static_cast<u32>(width), static_cast<u32>(height)}) };
				m_commandList.setScissor(0, 1, scissors);
				++m_recordCommandCount;
			}

			void GPUCommandList_Vulkan::SetVertexBuffer(GPUBuffer* buffer)
			{
				if (!m_commandList) { IS_CORE_ERROR("[GPUCommandList_Vulkan::SetVertexBuffer] CommandList is null."); return; }
				if (m_state != GPUCommandListState::Recording) { IS_CORE_ERROR("[GPUCommandList_Vulkan::SetVertexBuffer] CommandList is not recording."); return; }

				if (!buffer || m_activeItems.VertexBuffer == buffer)
				{
					return;
				}
				const GPUBuffer_Vulkan* bufferVulkan = dynamic_cast<const GPUBuffer_Vulkan*>(buffer);
				std::array<vk::Buffer, 1> buffersVulkan = { bufferVulkan->GetBuffer()};
				std::array<vk::DeviceSize, 1> offsets = { 0 };
				m_commandList.bindVertexBuffers(0, buffersVulkan, offsets);
				++m_recordCommandCount;
				m_activeItems.VertexBuffer = buffer;
			}

			void GPUCommandList_Vulkan::SetIndexBuffer(GPUBuffer* buffer)
			{
				if (!m_commandList) { IS_CORE_ERROR("[GPUCommandList_Vulkan::SetIndexBuffer] CommandList is null."); return; }
				if (m_state != GPUCommandListState::Recording) { IS_CORE_ERROR("[GPUCommandList_Vulkan::SetIndexBuffer] CommandList is not recording."); return; }

				if (!buffer || m_activeItems.IndexBuffer == buffer)
				{
					return;
				}
				const GPUBuffer_Vulkan* bufferVulkan = dynamic_cast<const GPUBuffer_Vulkan*>(buffer);
				m_commandList.bindIndexBuffer(bufferVulkan->GetBuffer(), 0, vk::IndexType::eUint32);
				++m_recordCommandCount;
				m_activeItems.IndexBuffer = buffer;
			}

			void GPUCommandList_Vulkan::Draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance)
			{
				if (!m_commandList) { IS_CORE_ERROR("[GPUCommandList_Vulkan::Draw] CommandList is null."); return; }
				if (!CanDraw()) { IS_CORE_ERROR("[GPUCommandList_Vulkan::DrawIndexed] Unable to draw."); return; }
				m_commandList.draw(vertexCount, instanceCount, firstVertex, firstInstance);
				++m_recordCommandCount;
			}

			void GPUCommandList_Vulkan::DrawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, u32 vertexOffset, u32 firstInstance)
			{
				if (!m_commandList) { IS_CORE_ERROR("[GPUCommandList_Vulkan::DrawIndexed] CommandList is null."); return; }
				if (!CanDraw()) { IS_CORE_ERROR("[GPUCommandList_Vulkan::DrawIndexed] Unable to draw."); return; }
				m_commandList.drawIndexed(indexCount, instanceCount, firstIndex,vertexOffset, firstInstance);
				++m_recordCommandCount;
			}

			void GPUCommandList_Vulkan::Submit(GPUQueue queue, std::vector<GPUSemaphore*> waitSemaphores, std::vector<GPUSemaphore*> signalSemaphores)
			{
				if (!m_commandList) { IS_CORE_ERROR("[GPUCommandList_Vulkan::Submit] CommandList is null."); return; }
				if (m_recordCommandCount == 0) { IS_CORE_ERROR("[GPUCommandList_Vulkan::Submit] Record command count is 0. Nothing is submitted."); return; }

				if (m_activeItems.Renderpass)
				{
					EndRenderpass();
				}

				std::vector<vk::PipelineStageFlags> pipelineStageFlags;
				std::vector<vk::Semaphore> waitSemaphoresVulkan;
				for (const GPUSemaphore* smeaphore : waitSemaphores)
				{
					const GPUSemaphore_Vulkan* semaphroeVulkan = dynamic_cast<const GPUSemaphore_Vulkan*>(smeaphore);
					waitSemaphoresVulkan.push_back(semaphroeVulkan->GetSemaphore());
					pipelineStageFlags.push_back(vk::PipelineStageFlagBits::eColorAttachmentOutput);
				}
				// Get the swapchain image acquired semaphore for this frame.
				GPUSwapchain_Vulkan* swapchainVulkan = dynamic_cast<GPUSwapchain_Vulkan*>(GetDevice()->GetSwapchain());
				GPUSemaphore_Vulkan* swaphcainImage = swapchainVulkan->GetImageAcquiredSemaphore();
				waitSemaphoresVulkan.push_back(swaphcainImage->GetSemaphore());
				pipelineStageFlags.push_back(vk::PipelineStageFlagBits::eColorAttachmentOutput);

				std::vector<vk::Semaphore> signalSemaphoresVulkan;
				for (const GPUSemaphore* smeaphore : signalSemaphores)
				{
					const GPUSemaphore_Vulkan* semaphroeVulkan = dynamic_cast<const GPUSemaphore_Vulkan*>(smeaphore);
					signalSemaphoresVulkan.push_back(semaphroeVulkan->GetSemaphore());
				}

				// Signal our command list semaphore so other GPU work know this command lists has completed.
				const GPUSemaphore_Vulkan* signalSemaphore = dynamic_cast<const GPUSemaphore_Vulkan*>(m_signalSemaphore);
				signalSemaphoresVulkan.push_back(signalSemaphore->GetSemaphore());

				// Signal our command lists fence so the CPU can wait if needed.
				const GPUFence_Vulkan* fenceVulkan = dynamic_cast<const GPUFence_Vulkan*>(m_fence);
				vk::Fence waitFence = fenceVulkan->GetFence();

				std::vector<vk::CommandBuffer> commandListVulkan = { m_commandList };	
				vk::SubmitInfo submitInfo = vk::SubmitInfo(waitSemaphoresVulkan, pipelineStageFlags, commandListVulkan, signalSemaphoresVulkan);
				GetDevice()->GetQueue(queue).submit(submitInfo, waitFence);
			}

			void GPUCommandList_Vulkan::BeginRecord()
			{
				Reset();
				if (!m_commandList) { IS_CORE_ERROR("[GPUCommandList_Vulkan::BeginRecord] CommandList is null."); return; }
				if (m_state != GPUCommandListState::Idle) { IS_CORE_ERROR("[GPUCommandList_Vulkan::BeginRecord] CommandList is already recording."); return; }
				m_state = GPUCommandListState::Recording;

				vk::CommandBufferBeginInfo info = vk::CommandBufferBeginInfo();
				m_commandList.begin(info);
				++m_recordCommandCount;
			}

			void GPUCommandList_Vulkan::EndRecord()
			{
				if (!m_commandList) { IS_CORE_ERROR("[GPUCommandList_Vulkan::EndRecord] CommandList is null."); return; }
				if (m_state != GPUCommandListState::Recording) { IS_CORE_ERROR("[GPUCommandList_Vulkan::EndRecord] CommandList is not recording."); return; }
				
				if (m_activeItems.Renderpass)
				{
					EndRenderpass();
				}

				m_state = GPUCommandListState::Idle;
				m_commandList.end();
				++m_recordCommandCount;
			}

			void GPUCommandList_Vulkan::BeginRenderpass()
			{
				if (!m_commandList) { IS_CORE_ERROR("[GPUCommandList_Vulkan::BeginRenderpass] CommandList is null."); return; }
				if (m_state != GPUCommandListState::Recording) { IS_CORE_ERROR("[GPUCommandList_Vulkan::BeginRenderpass] CommandList is not recording."); return; }
				if (m_activeItems.Renderpass) { IS_CORE_ERROR("[GPUCommandList_Vulkan::BeginRenderpass] Renderpass must call End before Begin can be called."); return; }

				vk::RenderPass renderpass = GPURenderpassManager_Vulkan::Instance().GetOrCreateRenderpass({ m_pso.RenderTargets });
				vk::Rect2D rect = vk::Rect2D({}, { static_cast<u32>(Window::Instance().GetWidth()), static_cast<u32>(Window::Instance().GetHeight()) });
				std::vector<vk::ImageView> imageViews;
				std::vector<vk::ClearValue> clearColours;

				if (m_pso.Swapchain)
				{
					GPUSwapchain_Vulkan* swapchainVulkan = dynamic_cast<GPUSwapchain_Vulkan*>(GetDevice()->GetSwapchain());
					imageViews.push_back(swapchainVulkan->GetImageView());

					vk::ClearValue clearValue;
					clearValue.color.float32[0] = 0;
					clearValue.color.float32[1] = 0;
					clearValue.color.float32[2] = 0;
					clearValue.color.float32[3] = 1;
					clearColours.push_back(clearValue);
				}
				else
				{
					for (size_t i = 0; i < m_pso.RenderTargets.size(); ++i)
					{
						const RenderTarget* rt = m_pso.RenderTargets[i];
						const RenderTargetDesc rtDesc = rt->GetDesc();
						const glm::vec4 cc = rtDesc.ClearColour;
						vk::ClearValue clearValue;
						if (PixelFormatExtensions::IsDepthStencil(rtDesc.Format))
						{
							clearValue.depthStencil.depth = cc.x;
							clearValue.depthStencil.stencil = static_cast<u32>(cc.y);
							clearColours.push_back(clearValue);
						}
						else
						{
							clearValue.color.float32[0] = cc.x;
							clearValue.color.float32[1] = cc.y;
							clearValue.color.float32[2] = cc.z;
							clearValue.color.float32[3] = cc.w;
							clearColours.push_back(clearValue);
						}
						// Get image views.
					}
				}

				if (m_framebuffer)
				{
					GetDevice()->GetDevice().destroyFramebuffer(m_framebuffer);
					m_framebuffer = nullptr;
				}

				vk::FramebufferCreateInfo frameBufferInfo = vk::FramebufferCreateInfo({}, renderpass, imageViews, rect.extent.width, rect.extent.height, 1);
				m_framebuffer = GetDevice()->GetDevice().createFramebuffer(frameBufferInfo);

				vk::RenderPassBeginInfo info = vk::RenderPassBeginInfo(renderpass, m_framebuffer, rect, clearColours);
				m_commandList.beginRenderPass(info, vk::SubpassContents::eInline);
				++m_recordCommandCount;
				m_activeItems.Renderpass = true;
			}

			void GPUCommandList_Vulkan::EndRenderpass()
			{
				if (!m_commandList) { IS_CORE_ERROR("[GPUCommandList_Vulkan::EndRenderpass] CommandList is null."); return; }
				if (m_state != GPUCommandListState::Recording) { IS_CORE_ERROR("[GPUCommandList_Vulkan::EndRenderpass] CommandList is not recording."); return; }
				if (!m_activeItems.Renderpass) { IS_CORE_ERROR("[GPUCommandList_Vulkan::BeginRenderpass] Renderpass must call Begin before End can be called."); return; }

				m_commandList.endRenderPass();
				++m_recordCommandCount;
				m_activeItems.Renderpass = false;
			}

			void GPUCommandList_Vulkan::BindPipeline(GPUPipelineStateObject* pipeline)
			{
				if (!m_commandList) { IS_CORE_ERROR("[GPUCommandList_Vulkan::DrawIndexed] CommandList is null."); return; }
				const GPUPipelineStateObject_Vulkan* pipelineVulkan = dynamic_cast<GPUPipelineStateObject_Vulkan*>(pipeline);

				// Check if we need to also begin a render pass?
				m_commandList.bindPipeline(GPUQueueToVulkanBindPoint(pipelineVulkan->GetPSO().Queue), pipelineVulkan->GetPipeline());
				++m_recordCommandCount;
			}



			GPUComamndListAllocator_Vulkan::GPUComamndListAllocator_Vulkan()
			{ }

			GPUComamndListAllocator_Vulkan::~GPUComamndListAllocator_Vulkan()
			{ 
				FreeAllCommandLists();
				Destroy();
			}

			GPUCommandList* GPUComamndListAllocator_Vulkan::AllocateCommandList(GPUCommandListType type)
			{
				vk::CommandPool& commandPool = m_commandPools[type];
				if (!commandPool)
				{
					vk::CommandPoolCreateFlags flags;
					if (type == GPUCommandListType::Transient)
					{
						flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
					}
					vk::CommandPoolCreateInfo poolCreateInfo = vk::CommandPoolCreateInfo( flags, GetDevice()->GetQueueFamilyIndex(m_queue));
					commandPool = GetDevice()->GetDevice().createCommandPool(poolCreateInfo);
				}

				vk::CommandBufferAllocateInfo allocInfo = vk::CommandBufferAllocateInfo(commandPool, vk::CommandBufferLevel::ePrimary, 1);
				std::vector<vk::CommandBuffer> commandBuffers = GetDevice()->GetDevice().allocateCommandBuffers(allocInfo);

				GPUCommandList_Vulkan* cmdList = new GPUCommandList_Vulkan();
				cmdList->m_commandList = commandBuffers.front();
				cmdList->m_queue = m_queue;
				cmdList->m_type = type;
				m_allocatedCommandLists.push_back(cmdList);

				return cmdList;
			}

			void GPUComamndListAllocator_Vulkan::ResetCommandLists(std::list<GPUCommandList*> cmdLists)
			{
				// TOOD:
				IS_CORE_ERROR("[GPUComamndListAllocator_Vulkan::ResetCommandPool] Vulkan does not support this.");
			}

			void GPUComamndListAllocator_Vulkan::ResetCommandPool(GPUCommandListType type)
			{
				vk::CommandPool& cmdPool = m_commandPools[type];
				if (cmdPool)
				{
					GetDevice()->GetDevice().resetCommandPool(cmdPool);
				}
			}

			void GPUComamndListAllocator_Vulkan::FreeCommandList(GPUCommandList* cmdList)
			{
				FreeCommandLists({ m_allocatedCommandLists });
			}

			void GPUComamndListAllocator_Vulkan::FreeCommandLists(const std::list<GPUCommandList*>& cmdLists)
			{
				if (m_allocatedCommandLists.size() == 0)
				{
					return;
				}

				std::unordered_map<GPUCommandListType, std::vector<vk::CommandBuffer>> commndBuffersVulkan;
				for (const std::list<GPUCommandList*>::iterator::value_type& ptr : cmdLists)
				{
					std::list<GPUCommandList*>::iterator cmdListItr = std::find(m_allocatedCommandLists.begin(), m_allocatedCommandLists.end(), ptr);
					if (cmdListItr == m_allocatedCommandLists.end())
					{
						continue;
					}
					GPUCommandList_Vulkan* cmdListVulkan = dynamic_cast<GPUCommandList_Vulkan*>(*cmdListItr);

					commndBuffersVulkan[cmdListVulkan->GetType()].push_back(cmdListVulkan->GetCommandBufferVulkan());
					delete cmdListVulkan;
					m_allocatedCommandLists.erase(cmdListItr);
				}

				for (const auto& kvp : commndBuffersVulkan)
				{
					vk::CommandPool& cmdPool = m_commandPools[kvp.first];
					if (!cmdPool)
					{
						IS_CORE_ERROR("[GPUComamndListAllocator_Vulkan::FreeCommandLists] CommandPool is null.");
						continue;
					}
					GetDevice()->GetDevice().freeCommandBuffers(cmdPool, kvp.second);
				}
			}

			void GPUComamndListAllocator_Vulkan::FreeAllCommandLists()
			{
				std::list<GPUCommandList*> list = m_allocatedCommandLists;
				FreeCommandLists(list);
			}

			void GPUComamndListAllocator_Vulkan::Destroy()
			{
				for (const auto& pair : m_commandPools)
				{
					GetDevice()->GetDevice().destroyCommandPool(pair.second);
				}
				m_commandPools.clear();
			}
		}
	}
}