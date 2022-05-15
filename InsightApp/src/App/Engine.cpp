#include "App/Engine.h"

#include "optick.h"

namespace Insight
{
	namespace App
	{
//#define WAIT_FOR_PROFILE_CONNECTION

		bool Engine::Init()
		{
#define RETURN_IF_FALSE(x) if (!x) { return false; }
			
			RETURN_IF_FALSE(Graphics::Window::Instance().Init());
			RETURN_IF_FALSE(m_graphicsManager.Init());

			OnInit();

			return true;
		}

		void Engine::Update()
		{
#ifdef WAIT_FOR_PROFILE_CONNECTION
			while (!Optick::IsActive()) { }
#endif
			while (!Graphics::Window::Instance().ShouldClose())
			{
				OPTICK_FRAME("MainThread");
				m_graphicsManager.Update(0.0f);
				Graphics::Window::Instance().Update();
			}
		}

		void Engine::Destroy()
		{
			OnDestroy();

			m_graphicsManager.Destroy();
			Graphics::Window::Instance().Destroy();
		}
	}
}