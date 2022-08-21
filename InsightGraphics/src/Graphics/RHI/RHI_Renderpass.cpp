#include "Graphics/RHI/RHI_Renderpass.h"
#include "Graphics/GraphicsManager.h"

#include "Graphics/RHI/RHI_Texture.h"

#ifdef IS_VULKAN_ENABLED
#include "Graphics/RHI/Vulkan/RenderContext_Vulkan.h"
#include "Graphics/RHI/Vulkan/VulkanUtils.h"
#endif // #ifdef IS_VULKAN_ENABLED
#ifdef IS_DX12_ENABLED
#include "Graphics/RHI/DX12/RenderContext_DX12.h"
#endif // #ifdef IS_DX12_ENABLED

#include "Core/Profiler.h"

#include <tuple>

namespace Insight
{
	namespace Graphics
	{
		void RHI_RenderpassManager::SetRenderContext(RenderContext* context)
		{
			m_context = context;
		}

		RHI_Renderpass RHI_RenderpassManager::GetOrCreateRenderpass(RenderpassDescription description)
		{
			if (auto itr = m_renderpasses.find(description.GetHash()); itr != m_renderpasses.end())
			{
				return itr->second;
			}
			return AddRenderpass(description);
		}

		RHI_Renderpass RHI_RenderpassManager::GetRenderpass(u64 hash) const
		{
			if (auto itr = m_renderpasses.find(hash); itr != m_renderpasses.end())
			{
				return itr->second;
			}
			return { };
		}


		void RHI_RenderpassManager::Release(u64 hash, bool remove)
		{
			if (auto itr = m_renderpasses.find(hash); itr != m_renderpasses.end())
			{
#ifdef IS_VULKAN_ENABLED
				if (GraphicsManager::IsVulkan())
				{
					RHI::Vulkan::RenderContext_Vulkan* contextVulkan = static_cast<RHI::Vulkan::RenderContext_Vulkan*>(m_context);
					vk::RenderPass renderpassVulkan = *reinterpret_cast<vk::RenderPass*>(&itr->second);
					contextVulkan->GetDevice().destroyRenderPass(renderpassVulkan);
					itr->second.Resource = nullptr;
				}
#endif

				if (remove)
				{
					m_renderpasses.erase(itr);
				}
			}
		}

		void RHI_RenderpassManager::Release(RenderpassDescription description, bool remove)
		{
			Release(description.GetHash(), remove);
		}

		void RHI_RenderpassManager::ReleaseAll()
		{
			for (const auto& pair : m_renderpasses)
			{
				Release(pair.first, false);
			}
			m_renderpasses.clear();
		}

		RHI_Renderpass RHI_RenderpassManager::AddRenderpass(RenderpassDescription description)
		{
			IS_PROFILE_FUNCTION();
			const u64 hash = description.GetHash();
			ASSERT(m_renderpasses.find(hash) == m_renderpasses.end());

#ifdef IS_VULKAN_ENABLED
			if (GraphicsManager::IsVulkan())
			{
				RHI::Vulkan::RenderContext_Vulkan* contextVulkan = static_cast<RHI::Vulkan::RenderContext_Vulkan*>(m_context);

				struct AttachmentCreation
				{
					static vk::AttachmentDescription CreateCustom(const RHI_Texture* texture, const AttachmentDescription* attachment)
					{
						return vk::AttachmentDescription(
							vk::AttachmentDescriptionFlags(),
							texture != nullptr ? 
							PixelFormatToVulkan(texture->GetFormat())
							: PixelFormatToVulkan(attachment->Format),
							vk::SampleCountFlagBits::e1,
							AttachmentLoadOpToVulkan(attachment->LoadOp),
							vk::AttachmentStoreOp::eStore,
							AttachmentLoadOpToVulkan(attachment->StencilLoadOp),
							vk::AttachmentStoreOp::eDontCare,
							ImageLayoutToVulkan(attachment->InitalLayout),
							ImageLayoutToVulkan(attachment->FinalLayout));
					}

					static vk::AttachmentDescription CreateColour(const RHI_Texture* texture, const AttachmentDescription* attachment)
					{
						if (attachment == nullptr)
						{
							return vk::AttachmentDescription(
								vk::AttachmentDescriptionFlags(),
								PixelFormatToVulkan(texture->GetFormat()),
								vk::SampleCountFlagBits::e1,
								vk::AttachmentLoadOp::eClear,
								vk::AttachmentStoreOp::eStore,
								vk::AttachmentLoadOp::eDontCare,
								vk::AttachmentStoreOp::eDontCare,
								vk::ImageLayout::eUndefined,
								vk::ImageLayout::eColorAttachmentOptimal);
						}
						vk::AttachmentDescription desc = CreateCustom(texture, attachment);
						return desc;
					}

					static vk::AttachmentDescription CreateDepthStencil(const RHI_Texture* texture, const AttachmentDescription* attachment)
					{
						if (attachment == nullptr)
						{
							return vk::AttachmentDescription(
								vk::AttachmentDescriptionFlags(),
								PixelFormatToVulkan(texture->GetFormat()),
								vk::SampleCountFlagBits::e1,								// Sample count
								vk::AttachmentLoadOp::eClear,								// load op
								vk::AttachmentStoreOp::eStore,								// store op
								vk::AttachmentLoadOp::eDontCare,							// stencil load op
								vk::AttachmentStoreOp::eDontCare,							// stencil store op
								vk::ImageLayout::eUndefined,								// initial layout
								vk::ImageLayout::eDepthAttachmentStencilReadOnlyOptimal);	// final layout
						}
						vk::AttachmentDescription desc = CreateCustom(texture, attachment);
						desc.finalLayout = vk::ImageLayout::eDepthAttachmentStencilReadOnlyOptimal;
						return desc;
					}

					static std::pair<std::vector<vk::AttachmentDescription>, std::vector<vk::AttachmentReference>>
						CreateAllColour(std::vector<RHI_Texture*> const& textures, std::vector<AttachmentDescription> const& descriptions)
					{
						std::vector<vk::AttachmentDescription> resultAttachments;
						std::vector<vk::AttachmentReference> resultReferences;

						for (size_t i = 0; i < textures.size(); ++i)
						{
							const AttachmentDescription* attachment = nullptr;
							if (i >= 0 && i < descriptions.size())
							{
								attachment = &descriptions.at(i);
							}
							resultAttachments.push_back(CreateColour(textures.at(i), attachment));
							resultReferences.push_back(vk::AttachmentReference(i, vk::ImageLayout::eColorAttachmentOptimal));
						}

						if (textures.size() == 0 && descriptions.size() > 0)
						{
							for (size_t i = 0; i < descriptions.size(); ++i)
							{
								resultAttachments.push_back(CreateColour(nullptr, &descriptions.at(i)));
								resultAttachments.at(i).format = resultAttachments.at(i).format != vk::Format::eUndefined ?
									resultAttachments.at(i).format
									: static_cast<RHI::Vulkan::RenderContext_Vulkan*>(GraphicsManager::Instance().GetRenderContext())->GetSwapchainColourFormat();
								resultReferences.push_back(vk::AttachmentReference(i, vk::ImageLayout::eColorAttachmentOptimal));
							}
						}

						return { resultAttachments, resultReferences };
					}

					static vk::AttachmentReference CreateDepthStencil(std::vector<vk::AttachmentDescription>& attachmentDescriptions
						, RHI_Texture* texture
						, AttachmentDescription attachmentDescription)
					{
						vk::AttachmentReference result = { };

						if (texture)
						{
							result.attachment = static_cast<u32>(attachmentDescriptions.size());
							result.layout = vk::ImageLayout::eDepthAttachmentStencilReadOnlyOptimal;
							attachmentDescriptions.push_back(CreateDepthStencil(texture, &attachmentDescription));
						}

						return result;
					}
				};

				struct SubpassCreation
				{
					static std::vector<vk::SubpassDependency> CreateDependencies(bool hasDepthStencil)
					{
						std::vector<vk::SubpassDependency> result;

						result.push_back(
							vk::SubpassDependency(
								VK_SUBPASS_EXTERNAL,
								0u,
								vk::PipelineStageFlagBits::eColorAttachmentOutput,
								vk::PipelineStageFlagBits::eColorAttachmentOutput,
								vk::AccessFlagBits::eNoneKHR,
								vk::AccessFlagBits::eColorAttachmentWrite,
								{ }
						));

						if (hasDepthStencil)
						{
							result.push_back(
								vk::SubpassDependency(
									VK_SUBPASS_EXTERNAL,
									0u,
									vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests,
									vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests,
									vk::AccessFlagBits::eNoneKHR,
									vk::AccessFlagBits::eDepthStencilAttachmentWrite,
									{ }
							));
						}

						return result;
					}
				};

				std::pair<std::vector<vk::AttachmentDescription>, std::vector<vk::AttachmentReference>> colourAttachmentsAndRefs
					= AttachmentCreation::CreateAllColour(description.ColourAttachments, description.Attachments);
				std::vector<vk::AttachmentDescription>& renderpassAttachments = colourAttachmentsAndRefs.first;
				std::vector<vk::AttachmentReference>& colourReferences = colourAttachmentsAndRefs.second;

				if (description.SwapchainPass)
				{
					// If there are no render attachments added and we have a custom attachment add custom attachment.
					if (renderpassAttachments.size() == 0 && description.Attachments.size() > 0)
					{
						renderpassAttachments.push_back(AttachmentCreation::CreateCustom(nullptr, &description.Attachments.at(0)));
						colourReferences.push_back(vk::AttachmentReference(renderpassAttachments.size() - 1, vk::ImageLayout::eColorAttachmentOptimal));
					}
					else if (renderpassAttachments.size() == 0)
					{
						renderpassAttachments.push_back(AttachmentCreation::CreateColour(nullptr, 
							&AttachmentDescription::Default(VkFormatToPixelFormat[(int)contextVulkan->GetSwapchainColourFormat()], ImageLayout::PresentSrc)));
						colourReferences.push_back(vk::AttachmentReference(renderpassAttachments.size() - 1, vk::ImageLayout::eColorAttachmentOptimal));
					}
				}


				vk::AttachmentReference depthReference = AttachmentCreation::CreateDepthStencil(renderpassAttachments, description.DepthStencil, { });

				std::vector<vk::SubpassDependency> subpassDependencies = SubpassCreation::CreateDependencies(description.DepthStencil != nullptr);

				vk::SubpassDescription  subpassDescription = { };
				subpassDescription.setColorAttachments(colourReferences);
				if (description.DepthStencil)
				{
					subpassDescription.setPDepthStencilAttachment(&depthReference);
				}

				vk::RenderPassCreateInfo createInfo = { };
				createInfo.setAttachments(renderpassAttachments);
				createInfo.setSubpasses(subpassDescription);
				createInfo.setDependencies(subpassDependencies);

				vk::RenderPass renderpass = contextVulkan->GetDevice().createRenderPass(createInfo);
				RHI_Renderpass newPass = *reinterpret_cast<RHI_Renderpass*>(&renderpass);

				m_renderpasses[hash] = newPass;
			}
#endif // #ifdef IS_VULKAN_ENABLED

			return m_renderpasses[hash];
		}
	}
}