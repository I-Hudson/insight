#include "Editor/EditorWindows/EditorWindowManager.h"
#include "Editor/EditorWindows/IEditorWindow.h"

#include "Editor/EditorWindows/ResourceWindow.h"
#include "Editor/EditorWindows/EntitiesWindow.h"
#include "Editor/EditorWindows/InputWindow.h"

#include "Core/Memory.h"

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
			m_windowRegistry[ResourceWindow::WINDOW_NAME].Bind([]() { return static_cast<IEditorWindow*>(NewTracked(ResourceWindow)); });
			m_windowRegistry[EntitiesWindow::WINDOW_NAME].Bind([]() { return static_cast<IEditorWindow*>(NewTracked(EntitiesWindow)); });
			m_windowRegistry[InputWindow::WINDOW_NAME].Bind([]() { return static_cast<IEditorWindow*>(NewTracked(InputWindow)); });
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
				IEditorWindow* newWindow = itr->second();
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