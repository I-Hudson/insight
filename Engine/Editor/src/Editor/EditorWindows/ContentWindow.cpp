#include "Editor/EditorWindows/ContentWindow.h"

#include "Runtime/ProjectSystem.h"
#include "Resource/ResourceManager.h"

#include "Resource/Texture2D.h"

#include "FileSystem/FileSystem.h"

#include "Core/EnginePaths.h"

#include <filesystem>
#include <imgui.h>
#include <imgui_internal.h>

namespace Insight
{
    namespace Editor
    {
        ContentWindow::ContentWindow()
            : IEditorWindow()
        {
            Setup();
        }

        ContentWindow::ContentWindow(u32 minWidth, u32 minHeight)
            : IEditorWindow(minWidth, minHeight)
        {
            Setup();
        }

        ContentWindow::ContentWindow(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight)
            : IEditorWindow(minWidth, minHeight, maxWidth, maxHeight)
        {
            Setup();
        }

        ContentWindow::~ContentWindow()
        {
        }

        void ContentWindow::OnDraw()
        {
            // Top bar
            TopBar();
            // Centre thumbnails
            CentreArea();
            // Bottom bar
            BottomBar();
        }

        void ContentWindow::Setup()
        {
            m_currentDirectory = Runtime::ProjectSystem::Instance().GetProjectInfo().GetContentPath();
            SplitDirectory();

            m_thumbnailToTexture[ContentWindowThumbnailType::Folder] = 
                Runtime::ResourceManager::LoadSync(Runtime::ResourceId(EnginePaths::GetResourcePath() + "/Editor/Icons/Folder.png", Runtime::Texture2D::GetStaticResourceTypeId())).CastTo<Runtime::Texture2D>().Get();
            m_thumbnailToTexture[ContentWindowThumbnailType::File] =
                Runtime::ResourceManager::LoadSync(Runtime::ResourceId(EnginePaths::GetResourcePath() + "/Editor/Icons/File.png", Runtime::Texture2D::GetStaticResourceTypeId())).CastTo<Runtime::Texture2D>().Get();
        }

        void ContentWindow::TopBar()
        {
            bool contentFolderFound = false;
            std::string currentPath;
            for (size_t i = 0; i < m_currentDirectoryParents.size(); ++i)
            {
                if (!contentFolderFound)
                {
                    currentPath += m_currentDirectoryParents[i] + "/";
                    std::filesystem::path contentRelativePath = std::filesystem::relative(currentPath, Runtime::ProjectSystem::Instance().GetProjectInfo().GetContentPath());
                    if (contentRelativePath == ".")
                    {
                        contentFolderFound = true;
                    }
                }

                if (!contentFolderFound)
                {
                    continue;
                }

                std::string buttonTitle = m_currentDirectoryParents[i] + "/";
                if (ImGui::Button(buttonTitle.c_str()))
                {
                    SetDirectoryFromParent((u32)i);
                    return;
                }
                ImGui::SameLine();
            }
            ImGui::Separator();
        }
        
        void ContentWindow::CentreArea()
        {
            // Compute some useful stuff
            const auto window = ImGui::GetCurrentWindowRead();
            const auto content_width = ImGui::GetContentRegionAvail().x;
            const auto content_height = ImGui::GetContentRegionAvail().y - 20.0f;
            ImGuiContext& g = *GImGui;
            ImGuiStyle& style = ImGui::GetStyle();
            const float font_height = g.FontSize;
            const float label_height = font_height;
            const float text_offset = 3.0f;
            float pen_x_min = 0.0f;
            float pen_x = 0.0f;
            bool new_line = true;
            static u32 displayed_item_count = 0;
            ImRect rect_button;
            ImRect rect_label;

            const ImVec2 itemSize = ImVec2(128.0f, 128.0f);

            if (ImGui::BeginTable("Content Browser", 4))
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                // DrawTreeView();

                for (auto iter : std::filesystem::directory_iterator(m_currentDirectory))
                {
                    ImGui::TableNextColumn();

                    std::string path = iter.path().string();
                    std::string fileName = iter.path().filename().string();
                    std::string fileExtension = iter.path().extension().string();

                    ImGui::BeginGroup();
                    {
                        // Compute rectangles for elements that make up an item
                        {
                            rect_button = ImRect
                            (
                                ImGui::GetCursorScreenPos().x,
                                ImGui::GetCursorScreenPos().y,
                                ImGui::GetCursorScreenPos().x + itemSize.x,
                                ImGui::GetCursorScreenPos().y + itemSize.y
                            );

                            rect_label = ImRect
                            (
                                rect_button.Min.x,
                                rect_button.Max.y - label_height - style.FramePadding.y,
                                rect_button.Max.x,
                                rect_button.Max.y
                            );
                        }

                        // Drop shadow effect
                        if (true)
                        {
                            static const float shadow_thickness = 2.0f;
                            ImVec4 color = ImGui::GetStyle().Colors[ImGuiCol_BorderShadow];
                            ImGui::GetWindowDrawList()->AddRectFilled(rect_button.Min, ImVec2(rect_label.Max.x + shadow_thickness, rect_label.Max.y + shadow_thickness), IM_COL32(color.x * 255, color.y * 255, color.z * 255, color.w * 255));
                        }

                        // THUMBNAIL
                        {
                            ImGui::PushID(fileName.c_str());
                            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 0.25f));

                            if (ImGui::Button("##dummy", itemSize))
                            {
                                m_lastClickTimer.Stop();
                                float elapsedTime = m_lastClickTimer.GetElapsedTimeMillFloat();
                                m_lastClickTimer.Start();
                                const bool isSingleClick = elapsedTime > 0.5f;

                                if (isSingleClick)
                                {
                                    m_currentItemSelected = path;
                                }
                                else
                                {
                                    if (iter.is_directory())
                                    {
                                        m_currentDirectory = path;
                                        FileSystem::FileSystem::PathToUnix(m_currentDirectory);
                                        SplitDirectory();
                                    }
                                    else
                                    {

                                    }
                                }
                            }

                            // Item functionality
                            {
                                // Manually detect some useful states
                                if (ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly))
                                {
                                    //m_is_hovering_item = true;
                                    //m_hovered_item_path = item.GetPath();
                                }

                                //ItemClick(&item);
                                //ItemContextMenu(&item);
                                //ItemDrag(&item);
                            }

                            // Image
                            {
                                // Compute thumbnail size
                                Graphics::RHI_Texture* texture = PathToThumbnail(path)->GetRHITexture();
                                ImVec2 image_size_max = ImVec2(rect_button.Max.x - rect_button.Min.x - style.FramePadding.x * 2.0f, rect_button.Max.y - rect_button.Min.y - style.FramePadding.y - label_height - 5.0f);
                                ImVec2 image_size = texture ? ImVec2(static_cast<float>(texture->GetWidth()), static_cast<float>(texture->GetHeight())) : image_size_max;
                                ImVec2 image_size_delta = ImVec2(0.0f, 0.0f);

                                // Scale the image size to fit the max available size while respecting it's aspect ratio
                                {
                                    // Clamp width
                                    if (image_size.x != image_size_max.x)
                                    {
                                        float scale = image_size_max.x / image_size.x;
                                        image_size.x = image_size_max.x;
                                        image_size.y = image_size.y * scale;
                                    }
                                    // Clamp height
                                    if (image_size.y != image_size_max.y)
                                    {
                                        float scale = image_size_max.y / image_size.y;
                                        image_size.x = image_size.x * scale;
                                        image_size.y = image_size_max.y;
                                    }

                                    image_size_delta.x = image_size_max.x - image_size.x;
                                    image_size_delta.y = image_size_max.y - image_size.y;
                                }

                                // Position the image within the square border
                                ImGui::SetCursorScreenPos(ImVec2(rect_button.Min.x + style.FramePadding.x + image_size_delta.x * 0.5f, rect_button.Min.y + style.FramePadding.y + image_size_delta.y * 0.5f));

                                // Draw the image
                                ImGui::Image(texture, image_size);
                            }

                            ImGui::PopStyleColor(2);
                            ImGui::PopID();
                        }

                        // LABEL
                        {
                            const char* label_text = fileName.c_str();
                            const ImVec2 label_size = ImGui::CalcTextSize(label_text, nullptr, true);

                            // Draw text background
                            ImGui::GetWindowDrawList()->AddRectFilled(rect_label.Min, rect_label.Max, IM_COL32(51, 51, 51, 190));
                            //ImGui::GetWindowDrawList()->AddRect(rect_label.Min, rect_label.Max, IM_COL32(255, 0, 0, 255)); // debug

                            // Draw text
                            ImGui::SetCursorScreenPos(ImVec2(rect_label.Min.x + text_offset, rect_label.Min.y + text_offset));
                            if (label_size.x <= itemSize.x && label_size.y <= itemSize.y)
                            {
                                ImGui::TextUnformatted(label_text);
                            }
                            else
                            {
                                ImGui::RenderTextClipped(rect_label.Min, rect_label.Max, label_text, nullptr, &label_size, ImVec2(0, 0), &rect_label);
                            }
                        }

                        ImGui::EndGroup();
                    }
                }
                ImGui::EndTable();
            }
        }
        
        void ContentWindow::BottomBar()
        {
        }

        void ContentWindow::SplitDirectory()
        {
            m_currentDirectoryParents.clear();
            std::string directory = m_currentDirectory;
            u64 splitIndex = directory.find('/');
            while (splitIndex != std::string::npos)
            {
                std::string splitDirectory = directory.substr(0, splitIndex);
                m_currentDirectoryParents.push_back(splitDirectory);
                directory = directory.substr(splitIndex + 1);
                splitIndex = directory.find('/');
            }

            m_currentDirectoryParents.push_back(directory);
        }

        void ContentWindow::SetDirectoryFromParent(u32 parentIndex)
        {
            std::string newDirectory;
            for (size_t i = 0; i <= parentIndex; ++i)
            {
                newDirectory += m_currentDirectoryParents[i] + "/";
            }

            m_currentDirectory = newDirectory;
            SplitDirectory();
        }

        Runtime::Texture2D* ContentWindow::PathToThumbnail(std::string const& path)
        {
            if (std::filesystem::is_directory(path))
            {
                return m_thumbnailToTexture[ContentWindowThumbnailType::Folder];
            }
            return m_thumbnailToTexture[ContentWindowThumbnailType::File];
        }
    }
}