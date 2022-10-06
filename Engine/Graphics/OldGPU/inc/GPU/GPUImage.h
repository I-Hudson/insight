#pragma once

#include "Core/TypeAlias.h"
#include "Graphics/PixelFormat.h"
#include "Graphics/GPU/Enums.h"
#include <string>
#include <vector>
#include <list>
#include <map>

namespace Insight
{
	namespace Graphics
	{
		class GPUImageManager;

		struct GPUImageCreateInfo
		{
			std::vector<Byte> Data;
			PixelFormat Format;
			ImageUsageFlags Usage;
		};

		class GPUImage
		{
		public:
			virtual ~GPUImage() { }

			virtual void LoadFromData(GPUImageCreateInfo info) = 0;

			int GetWidth() const { return m_width; }
			int GetHeight() const { return m_height; }
			int GetChannels() const { return m_channels; }
			bool LoadedFromFile() const { return m_loadedFromFile; }

		private:
			static GPUImage* New();
			virtual void Destroy() = 0;

		protected:
			int m_width{ 0 };
			int m_height{ 0 };
			int m_channels{ 0 };
			bool m_loadedFromFile{ false };
			std::string m_fileName;

			friend class GPUImageManager;
		};

		class GPUImageManager
		{
		public:
			GPUImageManager();
			~GPUImageManager();

			static GPUImageManager& Instance()
			{
				static GPUImageManager ins;
				return ins;
			}

			GPUImage* CreateOrGetImage(std::string key);

			void DestroyImage(std::string key);
			void DestroyImage(GPUImage* image);
			void Destroy();

		private:
			std::list<GPUImage*> m_images; // Owner of all images.
			std::map<std::string, GPUImage*> m_keyToImageLookup; // String to GPUImage lookup.
		};
	}
}