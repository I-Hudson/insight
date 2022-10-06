#pragma once

#if defined(IS_VULKAN_ENABLED)

#include "Graphics/RHI/RHI_Buffer.h"
#include "Graphics/RHI/Vulkan/RenderContext_Vulkan.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::Vulkan
		{
			class RenderContext_Vulkan;

			class RHI_Buffer_Vulkan : public RHI_Buffer
			{
			public:
				virtual ~RHI_Buffer_Vulkan() override;

				virtual void Create(RenderContext* context, BufferType bufferType, u64 sizeBytes, u64 stride, RHI_Buffer_Overrides overrides) override;
				virtual RHI_BufferView Upload(const void* data, u64 sizeInBytes, u64 offset) override;
				virtual std::vector<Byte> Download() override;
				virtual void Resize(u64 newSizeBytes) override;

				/// RHI_Resource
				virtual void Release() override;
				virtual bool ValidResouce() override;
				virtual void SetName(std::wstring name) override;

				vk::Buffer GetBuffer() const { return m_buffer; }

			private:
				RenderContext_Vulkan* m_context = nullptr;
				void* m_mappedData = nullptr;
				vk::Buffer m_buffer;
				VmaAllocation m_vmaAllocation;
			};
		}
	}
}

#endif ///#if defined(IS_VULKAN_ENABLED)