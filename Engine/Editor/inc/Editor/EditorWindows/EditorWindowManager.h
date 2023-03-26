#pragma once

#include "Core/Singleton.h"
#include "Core/Delegate.h"

#include "Editor/EditorWindows/IEditorWindow.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace Insight
{
	namespace Editor
	{
		class IEditorWindow;

		struct RegisterWindow
		{
			using RegisterWindowFunc = Core::Action<IEditorWindow*()>;

			RegisterWindow() = default;

			template<typename Lambda>
			RegisterWindow(Lambda func, EditorWindowCategories category)
			{
				RegisterFunc.Bind(func);
				Category = category;
			}

			RegisterWindowFunc RegisterFunc;
			EditorWindowCategories Category;
		};

		class EditorWindowManager : public Core::Singleton<EditorWindowManager>
		{

		public:
			EditorWindowManager();
			~EditorWindowManager();

			void RegisterWindows();
			void AddWindow(const std::string& windowName);
			void RemoveWindow(const std::string& windowName);

			bool IsWindowVisable(const std::string& windowName) const;

			std::vector<std::string> GetAllRegisteredWindowNames(EditorWindowCategories category) const;
			std::vector<std::string> GetAllActiveWindowNames(EditorWindowCategories category) const;
			std::vector<IEditorWindow const*> GetAllActiveWindows(EditorWindowCategories category) const;

			std::vector<std::string> GetAllRegisteredWindowNames() const;
			std::vector<std::string> GetAllActiveWindowNames() const;
			IEditorWindow const* GetActiveWindow(std::string_view windowName) const;
			void RemoveAllWindows();

			void Update();
			void Destroy();

		private:
			void RemoveQueuedWindows();

		private:
			std::unordered_map<std::string, RegisterWindow> m_windowRegistry;
			std::vector<IEditorWindow*> m_activeWindows;
			std::vector<std::string> m_windowsToRemove;
		};
	}
}