#pragma once

#include "Graphics/Defines.h"
#include "Graphics/Enums.h"
#include "Graphics/GPUDeferedManager.h"
#include "imgui.h"

#include "Graphics/RHI/RHI_Buffer.h"
#include "Graphics/RHI/RHI_Texture.h"
#include "Graphics/RHI/RHI_CommandList.h"
#include "Graphics/RHI/RHI_Shader.h"
#include "Graphics/RHI/RHI_Descriptor.h"
#include "Graphics/RHI/RHI_Renderpass.h"
#include "Graphics/RHI/RHI_Sampler.h"

#include "Graphics/RenderGraph/RenderGraph.h"

#include "Graphics/RenderStats.h"

#include "Core/Collections/FactoryMap.h"

#include <mutex>

namespace Insight
{
	class Renderer;
	
	namespace Graphics
	{
		class RenderContext;
		class RHI_Texture;
		class RenderTarget;

		//// <summary>
		//// Store any and all details about the physical device being used.
		//// </summary>
		struct PhysicalDeviceInformation : public Core::Singleton<PhysicalDeviceInformation>
		{
			std::wstring Device_Name;
			std::wstring Vendor;
			u32 Vendor_Id = 0;
			u64 VRam_Size;

			u64 MinUniformBufferAlignment = 0;

			void SetVendorName()
			{
				ASSERT(Vendor_Id != 0);
				switch (Vendor_Id)
				{
				case 0x1002: Vendor = L"AMD"; break;
				case 0x8086: Vendor = L"INTEL"; break;
				case 0x10DE: Vendor = L"NVIDIA"; break;
				case 0x1414: Vendor = L"MICROSOFT"; break;
				default:
					break;
				}
			}
		};

		class RenderContext
		{
		public:
			static RenderContext* New();

			virtual bool Init() = 0;
			virtual void Destroy() = 0;

			virtual void InitImGui() = 0;
			virtual void DestroyImGui() = 0;

			virtual bool PrepareRender() = 0;
			virtual void PreRender(RHI_CommandList* cmdList) = 0;
			virtual void PostRender(RHI_CommandList* cmdList) = 0;

			virtual void GpuWaitForIdle() = 0;
			virtual void SubmitCommandListAndWait(RHI_CommandList* cmdList) = 0;

			virtual RHI_Texture* GetSwaphchainIamge() const = 0;

			bool HasExtension(DeviceExtension extension) const;
			bool IsExtensionEnabled(DeviceExtension extension) const;
			void EnableExtension(DeviceExtension extension);
			void DisableExtension(DeviceExtension extension);

			CommandListManager& GetCommandListManager() { return m_commandListManager.Get(); }
			RHI_DescriptorLayoutManager& GetDescriptorLayoutManager() { return m_descriptorLayoutManager; }
			RHI_ShaderManager& GetShaderManager() { return m_shaderManager; }
			RHI_RenderpassManager& GetRenderpassManager() { return m_renderpassManager; }
			RHI_DescriptorSetManager& GetDescriptorSetManager() { return m_descriptorSetManager.Get(); }
			RHI_SamplerManager& GetSamplerManager() { return *m_samplerManager; }

		protected:
			void ImGuiBeginFrame();
			void ImGuiRender();

			virtual void WaitForGpu() = 0;

			void BaseDestroy();

		private:
			RHI_Buffer* CreateBuffer(BufferType bufferType, u64 sizeBytes, int stride, Graphics::RHI_Buffer_Overrides buffer_overrides);
			void FreeBuffer(RHI_Buffer* buffer);
			int GetBufferCount(BufferType bufferType) const;

			RHI_Texture* CreateTextre();
			void FreeTexture(RHI_Texture* texture);

		protected:
			///const static int c_FrameCount = 3;

			std::mutex m_lock;

			std::array<u8, (size_t)DeviceExtension::DeviceExtensionCount> m_deviceExtensions;
			std::array<u8, (size_t)DeviceExtension::DeviceExtensionCount> m_enabledDeviceExtensions;

			std::map<BufferType, RHI_ResourceManager<RHI_Buffer>> m_buffers;
			RHI_ResourceManager<RHI_Texture> m_textures;
			RHI_ShaderManager m_shaderManager;
			RHI_RenderpassManager m_renderpassManager;
			RHI_SamplerManager* m_samplerManager;
			GPUDeferedManager m_gpu_defered_manager;

			FrameResource<CommandListManager> m_commandListManager;
			FrameResource<RHI_DescriptorSetManager> m_descriptorSetManager;

			RHI_DescriptorLayoutManager m_descriptorLayoutManager;

			PhysicalDeviceInformation m_physical_device_info;
			RenderStats m_renderStats;

			RHI_Texture* m_font_texture = nullptr;

			friend class Renderer;
		};
	}

	/// Utility class for all other engine systems to call into.
	class IS_GRAPHICS Renderer
	{
	public:
		static void SetImGUIContext(ImGuiContext*& context);

		static Graphics::RHI_Buffer* CreateVertexBuffer(u64 sizeBytes, int stride, Graphics::RHI_Buffer_Overrides buffer_overrides = { });
		static Graphics::RHI_Buffer* CreateIndexBuffer(u64 sizeBytes, Graphics::RHI_Buffer_Overrides buffer_overrides = { });
		static Graphics::RHI_Buffer* CreateUniformBuffer(u64 sizeBytes, Graphics::RHI_Buffer_Overrides buffer_overrides = { });
		static Graphics::RHI_Buffer* CreateRawBuffer(u64 sizeBytes, Graphics::RHI_Buffer_Overrides buffer_overrides = { });

		static void FreeVertexBuffer(Graphics::RHI_Buffer* buffer);
		static void FreeIndexBuffer(Graphics::RHI_Buffer* buffer);
		static void FreeUniformBuffer(Graphics::RHI_Buffer* buffer);
		static void FreeRawBuffer(Graphics::RHI_Buffer* buffer);

		static int GetVertexBufferCount();
		static int GetIndexBufferCount();
		static int GetUniformBufferCount();
		static int GetBufferCount(Graphics::BufferType bufferType);

		static Graphics::RHI_Texture* CreateTexture();
		static void FreeTexture(Graphics::RHI_Texture* texture);

	private:
		static Graphics::RenderContext* s_context;
		friend class Graphics::RenderContext;
	};
}