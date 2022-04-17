#include "Graphics/GraphicsManager.h"
#include "Graphics/PixelFormatExtensions.h"
#include "Graphics/RenderContext.h"
#include "Graphics/Window.h"
//#include "Graphics/GPU/GPUDevice.h"

//#include "Graphics/GPU/RHI/Vulkan/GPUDevice_Vulkan.h"
//#include "Graphics/GPU/RHI/DX12/GPUDevice_DX12.h"

namespace Insight
{
	namespace Graphics
	{
		GraphicsManagerData GraphicsManager::m_sharedData;

		bool GraphicsManager::Init()
		{
			PixelFormatExtensions::Init();

			m_sharedData.GraphicsAPI = GraphicsAPI::Vulkan;
			m_renderContext = RenderContext::New();
			if (!m_renderContext)
			{
				return false;
			}

			if (!m_renderContext->Init())
			{
				return false;
			}

			return true;
		}

		void GraphicsManager::Update(const float deltaTime)
		{
			//GPUBuffer* vBuffer = Renderer::CreateVertexBuffer(128);

			bool show = true;
			//ImGui::ShowDemoWindow(&show);
			ImGui::Begin("Test window");
			ImGui::Text("TEXT");
			ImGui::End();

			ShaderDesc shaderDesc;
			shaderDesc.VertexFilePath = L"Resources/Shaders/hlsl/Swapchain.hlsl";
			shaderDesc.PixelFilePath = L"Resources/Shaders/hlsl/Swapchain.hlsl";
			RHI_Shader* shader = Renderer::GetShader(shaderDesc);

			PipelineStateObject pso{};
			pso.Shader = shader;
			pso.CullMode = CullMode::None;
			pso.RenderTargets.clear();
			pso.Swapchain = true;
			Renderer::SetPipelineStateObject(pso);

			Renderer::SetViewport(Window::Instance().GetWidth(), Window::Instance().GetHeight());
			Renderer::SetScissor(Window::Instance().GetWidth(), Window::Instance().GetHeight());

			Renderer::Draw(3, 1, 0, 0);

			m_renderContext->Render(Renderer::s_FrameCommandList);
			Renderer::s_FrameCommandList.Reset();

			//Renderer::FreeVertexBuffer(vBuffer);
		}

		void GraphicsManager::Destroy()
		{
			if (m_renderContext)
			{
				m_renderContext->Destroy();
				delete m_renderContext;
				m_renderContext = nullptr;
			}
		}
	}
}