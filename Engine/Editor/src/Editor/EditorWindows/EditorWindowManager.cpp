#include "Editor/EditorWindows/EditorWindowManager.h"
#include "Editor/EditorWindows/IEditorWindow.h"

#include "EditorWindows.gen.h"

#include "Editor/EditorWindows/BuildSettingsWindow.h"
#include "Editor/EditorWindows/ContentWindow.h"
#include "Editor/EditorWindows/ResourceWindow.h"
#include "Editor/EditorWindows/EntitiyDescriptionWindow.h"
#include "Editor/EditorWindows/EntitiesWindow.h"
#include "Editor/EditorWindows/InputWindow.h"
#include "Editor/EditorWindows/MemoryWindow.h"
#include "Editor/EditorWindows/ProjectWindow.h"
#include "Editor/EditorWindows/SystemInformationWindow.h"

namespace Insight
{
	namespace Editor
	{
		EditorWindowManager::EditorWindowManager()
		{
		}

		EditorWindowManager::~EditorWindowManager()
		{
			Destroy();
		}

		void EditorWindowManager::RegisterWindows()
		{
#define REGISTER_NEW_WINDOW(window) m_windowRegistry[window::WINDOW_NAME] = RegisterWindow([]() { return static_cast<IEditorWindow*>(New<window, Core::MemoryAllocCategory::Editor>()); }, window::WINDOW_CATEGORY)

			//REGISTER_NEW_WINDOW(BuildSettingsWindow);
			//REGISTER_NEW_WINDOW(ContentWindow);
			//REGISTER_NEW_WINDOW(EntitiyDescriptionWindow);
			//REGISTER_NEW_WINDOW(EntitiesWindow);
			//REGISTER_NEW_WINDOW(InputWindow);
			//REGISTER_NEW_WINDOW(MemoryWindow);
			//REGISTER_NEW_WINDOW(ResourceWindow);
			//REGISTER_NEW_WINDOW(ProjectWindow);
			//REGISTER_NEW_WINDOW(SystemInformationWindow);

			RegisterAllEditorWindows();

#undef REGISTER_NEW_WINDOW
		}

		void EditorWindowManager::AddWindow(const std::string& windowName)
		{
			for (size_t i = 0; i < m_activeWindows.size(); ++i)
			{
				if (m_activeWindows.at(i)->GetWindowName() == windowName)
				{
					return;
				}
			}

			auto itr = m_windowRegistry.find(windowName);
			if (itr != m_windowRegistry.end())
			{
				IEditorWindow* newWindow = itr->second.RegisterFunc();
				m_activeWindows.push_back(newWindow);
			}
		}

		void EditorWindowManager::RemoveWindow(const std::string& windowName)
		{
			m_windowsToRemove.push_back(windowName);
		}

		bool EditorWindowManager::IsWindowVisable(const std::string& windowName) const
		{
			for (size_t i = 0; i < m_activeWindows.size(); ++i)
			{
				if (windowName == m_activeWindows.at(i)->GetWindowName())
				{
					return true;
				}
			}
			return false;
		}

		std::vector<std::string> EditorWindowManager::GetAllRegisteredWindowNames(EditorWindowCategories category) const
		{
			std::vector<std::string> result;
			for (auto const& pair : m_windowRegistry)
			{
				if (pair.second.Category == category)
				{
					result.push_back(pair.first);
				}
			}
			return result;
		}

		std::vector<std::string> EditorWindowManager::GetAllActiveWindowNames(EditorWindowCategories category) const
		{
			std::vector<std::string> result;
			for (IEditorWindow const* window : m_activeWindows)
			{
				if (window->GetCategory() == category)
				{
					result.push_back(window->GetWindowName());
				}
			}
			return result;
		}

		std::vector<IEditorWindow const*> EditorWindowManager::GetAllActiveWindows(EditorWindowCategories category) const
		{
			std::vector< IEditorWindow const*> result;
			for (IEditorWindow const* window : m_activeWindows)
			{
				if (window->GetCategory() == category)
				{
					result.push_back(window);
				}
			}
			return result;
		}

		std::vector<std::string> EditorWindowManager::GetAllRegisteredWindowNames() const
		{
			std::vector<std::string> result;
			for (auto const& pair : m_windowRegistry)
			{
				result.push_back(pair.first);
			}
			return result;
		}

		std::vector<std::string> EditorWindowManager::GetAllActiveWindowNames() const
		{
			std::vector<std::string> result;
			for (IEditorWindow const* window : m_activeWindows)
			{
				result.push_back(window->GetWindowName());
			}
			return result;
		}

		IEditorWindow const* EditorWindowManager::GetActiveWindow(std::string_view windowName) const
		{
			for (IEditorWindow const* window : m_activeWindows)
			{
				if (window->GetWindowName() == windowName)
				{
					return window;
				}
			}
			return nullptr;
		}

		void EditorWindowManager::RemoveAllWindows()
		{
			std::vector<std::string> activeWindows = GetAllActiveWindowNames();
			for (size_t i = 0; i < activeWindows.size(); ++i)
			{
				RemoveWindow(activeWindows.at(i));
			}
		}

		void EditorWindowManager::Update()
		{
			RemoveQueuedWindows();
			for (size_t i = 0; i < m_activeWindows.size(); ++i)
			{
				m_activeWindows.at(i)->Draw();
			}
		}

		void EditorWindowManager::Destroy()
		{
			m_windowRegistry.clear();
			for (size_t i = 0; i < m_activeWindows.size(); ++i)
			{
				DeleteTracked(m_activeWindows.at(i));
			}
			m_activeWindows.resize(0);
		}

		void EditorWindowManager::RemoveQueuedWindows()
		{
			for (size_t i = 0; i < m_windowsToRemove.size(); ++i)
			{
				std::string_view windowToRemove = m_windowsToRemove.at(i);
				if (auto itr = std::find_if(m_activeWindows.begin(), m_activeWindows.end(), [windowToRemove](const IEditorWindow* window)
					{
						return window->GetWindowName() == windowToRemove;
					}); itr != m_activeWindows.end())
				{
					DeleteTracked(*itr);
					m_activeWindows.erase(itr);
				}
			}
			m_windowsToRemove.clear();
		}
	}
}