#include "GenerateFiles/GenerateProjectInitialise.h"
#include "Utils.h"

#include "Editor/HotReload/HotReloadExportFunctions.h"

#include <filesystem>

namespace InsightReflectTool
{
    bool GenerateProjectInitialise::Generate(const Reflect::Parser::FileParser& fileParser, std::string_view outFilePath, const Reflect::ReflectAddtionalOptions& options) const
    {
        std::fstream file;
        std::string absPath = std::filesystem::absolute(outFilePath).string();
        Utils::ValidateOutputPath(absPath);

        file.open(absPath, std::ios::out | std::ios::trunc);
        if (file.is_open())
        {
            Utils::WriteGeneratedFileHeader(file);

            file << "#include \"Core/ImGuiSystem.h\"\n";

            Utils::WriteIncludeFile(file, "EditorWindows.gen.h");
            Utils::WriteIncludeFile(file, "RegisterComponents.gen.h");

            Utils::WriteIncludeFile(file, "Core/Logger.h");
            Utils::WriteIncludeFile(file, "Editor/HotReload/HotReloadMetaData.h");

            file << "\n";

            file << "#ifndef IS_MONOLITH\n";
            file << "#ifdef IS_EXPORT_PROJECT_DLL\n";
            file << "#define IS_PROJECT __declspec(dllexport)\n";
            file << "#else\n";
            file << "#define IS_PROJECT __declspec(dllimport)\n";
            file << "#endif\n";
            file << "#else\n";
            file << "#define IS_PROJECT\n";
            file << "#endif\n\n";

            file << "namespace Insight\n";
            file << "{\n";

            Utils::WriteSourceFunctionDefinition(file, "extern \"C\" IS_PROJECT void ", ProjectModule::c_Initialise,
                { "Core::ImGuiSystem* imguiSystem" }, [&](std::fstream& file)
                {
                    const int indent = 2;
                    TAB_N(indent);
                    file << "SET_IMGUI_CURRENT_CONTEXT();" << NEW_LINE;
                    TAB_N(indent);
                    file << "Editor::RegisterAllEditorWindows();" << NEW_LINE;                    
                    TAB_N(indent);
                    file << "ECS::RegisterAllComponents();" << NEW_LINE;
                    TAB_N(indent);
                    file << "IS_INFO(\"Project DLL module initialised\");" << NEW_LINE;
                }, 1);

            Utils::WriteSourceFunctionDefinition(file, "extern \"C\" IS_PROJECT void", ProjectModule::c_Uninitialise, { }, [&](std::fstream& file)
                {
                    const int indent = 2;
                    TAB_N(indent);
                    file << "Editor::UnregisterAllEditorWindows();" << NEW_LINE;
                    TAB_N(indent);
                    file << "ECS::UnregisterAllComponents();" << NEW_LINE;
                    TAB_N(indent);
                    file << "IS_INFO(\"Project DLL module uninitialised\");" << NEW_LINE;
                }, 1);

            Utils::WriteSourceFunctionDefinition(file, "extern \"C\" IS_PROJECT std::vector<std::string>", ProjectModule::c_GetEditorWindowNames, { }, [&](std::fstream& file)
                {
                    const int indent = 2;
                    TAB_N(indent);
                    file << "return Editor::GetAllEditorWindowNames();" << NEW_LINE;
                }, 1);

            Utils::WriteSourceFunctionDefinition(file, "extern \"C\" IS_PROJECT std::vector<std::string>", ProjectModule::c_GetComponentNames, { }, [&](std::fstream& file)
                {
                    const int indent = 2;
                    TAB_N(indent);
                    file << "return ECS::GetAllComponentNames();" << NEW_LINE;
                }, 1);

            Utils::WriteSourceFunctionDefinition(file, "extern \"C\" IS_PROJECT ::Insight::Editor::HotReloadMetaData", ProjectModule::c_GetMetaData, { }, [&](std::fstream& file)
            {
                const int indent = 2;
                TAB_N(indent); file << "::Insight::Editor::HotReloadMetaData metaData;" << NEW_LINE;
                file << NEW_LINE;

                TAB_N(indent); file << "metaData.EditorWindowNames = std::move(Editor::" << EditorWindowRegister::c_GetAllEditorWindowNames << "()); " << NEW_LINE;
                TAB_N(indent); file << "metaData.ComponentNames = std::move(ECS::" << ComponentRegister::c_GetAllComponentNames << "()); " << NEW_LINE;
                file << NEW_LINE;

                TAB_N(indent); file << "metaData.EditorWindowTypeInfos = std::move(Editor::" << EditorWindowRegister::c_GetAllEditorWindowsTypeInfos << "());" << NEW_LINE;
                TAB_N(indent); file << "metaData.ComponentTypeInfos = std::move(ECS::" << ComponentRegister::c_GetAllComponentTypeInfos << "());" << NEW_LINE;
                file << NEW_LINE;

                TAB_N(indent); file << "return metaData;" << NEW_LINE;
            }, 1);


            file << "}\n";

            return true;
        }
        else
        {
            return false;
        }
    }
}