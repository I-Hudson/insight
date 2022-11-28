#pragma once

#include "Graphics/Enums.h"
#include "Graphics/PixelFormat.h"
#include "Graphics/RHI/RHI_Resource.h"
#include <vector>

namespace Insight
{
	namespace Graphics
	{
		class RenderContext;
		class RHI_CommandList;

		struct RHI_TextureInfo
		{
			TextureType TextureType = TextureType::Unknown;
			int Width = -1;
			int Height = -1;
			int Depth = -1;
			PixelFormat Format = PixelFormat::Unknown;
			ImageUsageFlags ImageUsage = 0;

			u32 Mip_Count = 1;
			u32 Layer_Count = 1;

			ImageLayout Layout = ImageLayout::Undefined;

			static RHI_TextureInfo Tex2D(int width, int height, PixelFormat format, ImageUsageFlags usage)
			{
				RHI_TextureInfo info = { };
				info.TextureType = TextureType::Tex2D;
				info.Width = width;
				info.Height = height;
				info.Depth = 1;
				info.Format = format;
				info.ImageUsage = usage;
				return info;
			}
			static RHI_TextureInfo Tex2DArray(int width, int height, PixelFormat format, ImageUsageFlags usage, u32 layer_count)
			{
				RHI_TextureInfo info = { };
				info.TextureType = TextureType::Tex2DArray;
				info.Width = width;
				info.Height = height;
				info.Depth = 1;
				info.Format = format;
				info.ImageUsage = usage;
				info.Layer_Count = layer_count;
				return info;
			}
		};

		class IS_GRAPHICS RHI_Texture : public RHI_Resource
		{
		public:
			static RHI_Texture* New();

			void LoadFromFile(std::string filePath);
			void LoadFromData(Byte* data, u32 width, u32 height, u32 depth, u32 channels);

			RHI_TextureInfo GetInfo(u32 mip = 0)					const { return m_infos.at(mip); }
			int				GetWidth                (u32 mip = 0)	const { return m_infos.at(mip).Width; }
			int				GetHeight               (u32 mip = 0)	const { return m_infos.at(mip).Height; }
			int				GetChannels             (u32 mip = 0)	const { return 4; }
			TextureType		GetType					(u32 mip = 0)	const { return m_infos.at(mip).TextureType; }
			PixelFormat		GetFormat				(u32 mip = 0)	const { return m_infos.at(mip).Format; }
			ImageLayout		GetLayout				(u32 mip = 0)	const { return m_infos.at(mip).Layout; }

			void			SetLayout				(ImageLayout newLayout, u32 mip = 0) { m_infos.at(mip).Layout = newLayout; }

			virtual void Create(RenderContext* context, RHI_TextureInfo createInfo) = 0;
			//TODO: Look into a ssytem to batch upload textures. Maybe submit a batch upload struct with a list of textures and data.
			void Upload(void* data, int sizeInBytes);
			virtual void Upload(void* data, int sizeInBytes, RHI_CommandList* cmdList) = 0;
			virtual std::vector<Byte> Download(void* data, int sizeInBytes) = 0;

		protected:
			/// @brief Define the info for all mips of the image.
			std::vector<RHI_TextureInfo> m_infos = { };

			friend class RHI_CommandList;
		};
	}
}