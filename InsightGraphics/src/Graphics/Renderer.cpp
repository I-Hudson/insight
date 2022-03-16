#include "Graphics/Renderer.h"
#include "Graphics/Window.h"
#include "Graphics/GPU/GPUDevice.h"
#include "Graphics/GPU/GPUSwapchain.h"

namespace Insight
{
	namespace Graphics
	{
		void Renderer::Init(GPUDevice* gpuDevice)
		{
			m_gpuDevice = gpuDevice;
			m_swapchain = GPUSwapchain::Create();
			if (m_swapchain)
			{
				m_swapchain->Prepare();
				m_swapchain->Build(GPUSwapchainDesc(Window::Instance().GetWidth(), Window::Instance().GetHeight()));
				m_swapchain->Build(GPUSwapchainDesc(Window::Instance().GetWidth(), Window::Instance().GetHeight()));
			}
			m_presentCompleteSemaphore = m_gpuDevice->GetSemaphoreManager().GetOrCreateSemaphore();
			m_commandListManager.Create();
		}

		void Renderer::Destroy()
		{
			if (m_presentCompleteSemaphore)
			{
				m_gpuDevice->GetSemaphoreManager().ReturnSemaphore(m_presentCompleteSemaphore);
			}

			m_commandListManager.Destroy();

			if (m_swapchain)
			{
				m_swapchain->Destroy();
				delete m_swapchain;
			}
		}

		void Renderer::Render()
		{
			m_commandListManager.ResetCommandPool();
			GPUCommandList* cmdList = m_commandListManager.GetOrCreateCommandList();
			Prepare(cmdList);
			Submit(cmdList);
		}

		void Renderer::Prepare(GPUCommandList* cmdList)
		{
			m_swapchain->AcquireNextImage(m_presentCompleteSemaphore);

			cmdList->SetPrimitiveTopologyType(PrimitiveTopologyType::TriangleList);
			cmdList->SetPolygonMode(PolygonMode::Fill);
			cmdList->SetCullMode(CullMode::Back);

			// Render to a GBuffer.
			//cmdList->SetShader(nullptr);
			//cmdList->AddRenderTarget(nullptr /*Colour*/);
			//cmdList->AddRenderTarget(nullptr /*Normal*/);
			//cmdList->AddRenderTarget(nullptr /*Depth*/);
			//cmdList->Draw(3, 1, 0, 0);

			// Render straight to swapchain.
			cmdList->SetShader(nullptr);
			cmdList->ClearRenderTargets();
			cmdList->SetSwapchainSubmit(true);
			cmdList->Draw(3, 1, 0, 0);

			cmdList->SubmitAndWait(GPUQueue_Graphics);
		}

		void Renderer::Submit(GPUCommandList* cmdList)
		{
			m_swapchain->Present(GPUQueue::GPUQueue_Graphics, { m_presentCompleteSemaphore });
		}
	}
}