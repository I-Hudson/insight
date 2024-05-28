#include "InputModule.h"
#include "Core/ImGuiSystem.h"
#include "Core/Logger.h"

namespace Insight
{
	void InputModule::Initialise(Core::ImGuiSystem* imguiSystem)
	{
		SET_IMGUI_CURRENT_CONTEXT();
		SET_SPDLOG_LOGGERS();
	}
}