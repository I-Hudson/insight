#include "Editor/EditorWindows/AssetAuditWindow.h"

#include "Asset/AssetRegistry.h"
#include "Asset/AssetPackage.h"

#include "FileSystem/FileSystem.h"

#include "Resource/Loaders/ResourceLoaderRegister.h"

#include "Serialisation/Serialisers/BinarySerialiser.h"

#include "Core/Profiler.h"

#include <imgui.h>

namespace Insight
{
    namespace Editor
    {
        AssetAuditWindow::AssetAuditWindow()
            : IEditorWindow()
        { }
        AssetAuditWindow::AssetAuditWindow(u32 minWidth, u32 minHeight)
            : IEditorWindow(minWidth, minHeight)
        { }
        AssetAuditWindow::AssetAuditWindow(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight)
            : IEditorWindow(minWidth, minHeight, maxWidth, maxHeight)
        { }
        AssetAuditWindow::~AssetAuditWindow()
        { }

        void AssetAuditWindow::OnDraw()
        {
            IS_PROFILE_FUNCTION();

            std::vector<Runtime::AssetPackage*> assetPackages = Runtime::AssetRegistry::Instance().GetAllAssetPackages();

            for (int i = 0; i < assetPackages.size(); ++i)
            {
                Runtime::AssetPackage* package = assetPackages.at(i);
                if (ImGui::Button(package->GetName().data()))
                {
                    m_selectedAssetPackage = package;
                }

                if (i < assetPackages.size() - 1)
                {
                    ImGui::SameLine();
                }
            }

            const int tableFlags = ImGuiTableFlags_Resizable
                | ImGuiTableFlags_RowBg
                | ImGuiTableFlags_BordersInner
                ;

            if (m_selectedAssetPackage 
                && ImGui::BeginTable("Asset Audit", 3, tableFlags))
            {
                IS_PROFILE_SCOPE("Table");
                ImGui::TableSetupColumn("Folder Path", ImGuiTableColumnFlags_None);
                ImGui::TableSetupColumn("File Name", ImGuiTableColumnFlags_None);
                ImGui::TableSetupColumn("Resource Type Id", ImGuiTableColumnFlags_None);
                ImGui::TableHeadersRow();

                std::vector<const Runtime::AssetInfo*> assetInfos = m_selectedAssetPackage->GetAllAssetInfos();
                {
                    IS_PROFILE_SCOPE("Sort asset infos");
                    std::sort(assetInfos.begin(), assetInfos.end(), [](const Runtime::AssetInfo* a, const Runtime::AssetInfo* b)
                        {
                                if (a->GetFullFilePath().size() != b->GetFullFilePath().size())
                                {
                                    return a->GetFullFilePath().size() < b->GetFullFilePath().size();
                                }
                                return a->GetFullFilePath() < b->GetFullFilePath();
                        });
                }

                ImGuiListClipper clipper;
                clipper.Begin(static_cast<int>(assetInfos.size()));
                
                while (clipper.Step())
                {
                    for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
                    {
                        IS_PROFILE_SCOPE("AssetInfo entry");
                        const Runtime::AssetInfo* info = assetInfos.at(row);

                        {
                            ImGui::TableNextColumn();
                            ImGui::Text("%s", info->FilePath.c_str());
                        }
                        {
                            ImGui::TableNextColumn();
                            ImGui::Text("%s", info->FileName.c_str());
                        }
                        {
                            IS_PROFILE_SCOPE("Find Resource Type Id");
                            ImGui::TableNextColumn();

                            std::string_view fileExtension = FileSystem::GetExtension(info->FileName);
                            const Runtime::IResourceLoader* loader = Runtime::ResourceLoaderRegister::GetLoaderFromExtension(fileExtension);
                            if (loader != nullptr)
                            {
                                Runtime::ResourceTypeId typeId = loader->GetResourceTypeId();
                                ImGui::Text("%s", typeId.GetTypeName().c_str());
                            }
                            else
                            {
                                ImGui::Text("Unknown");
                            }
                        }
                    }  
                }
                ImGui::EndTable();

                if (ImGui::Button("Build Selected Asset Package"))
                {
                    PlatformFileDialog dialog;
                    std::string path;
                    if (dialog.ShowSave(&path, { FileDialogFilter(L"AssetPackage", L"*.isassetpackage") }))
                    {
                        Serialisation::BinarySerialiser serialiser(false);
                        m_selectedAssetPackage->Serialise(&serialiser);
                        std::vector<Byte> data = serialiser.GetRawData();
                        FileSystem::SaveToFile(data, path, FileType::Binary, true);
                    }
                }
            }
            else
            {
                ImGui::Text("No package has been selected.");
            }
        }
    }
}