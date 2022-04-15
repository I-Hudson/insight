#pragma once

#include "Graphics/CommandListCommands.h"
#include "Graphics/PipelineStateObject.h"
#include <type_traits>
#include <string>
#include <assert.h>

namespace Insight
{
	namespace Graphics
	{
		class GPUBuffer;
		class RHI_Shader;

		class CommandList
		{
		public:
			CommandList();
			CommandList(const CommandList& other);
			CommandList(CommandList && other);
			~CommandList();

			CommandList& operator=(const CommandList& other);
			CommandList& operator=(CommandList&& other);

			ICommand* GetCurrentCommand();
			void NextCommand();
			u64 GetCommandCount() const { return m_commandCount; }
			
			void Reset();
			void ResetReadHead();
			void SetReadHeadToCommandIndex(int index);

			void SetPipelineStateObject(PipelineStateObject pso);
			void SetPrimitiveTopologyType(PrimitiveTopologyType type);
			void SetPolygonMode(PolygonMode mode);
			void SetCullMode(CullMode mode);
			void SetShader(RHI_Shader* shader);
			void ClearRenderTargets();

			void SetViewport(int width, int height);
			void SetScissor(int width, int height);

			void SetVertexBuffer(GPUBuffer* buffer);
			void SetIndexBuffer(GPUBuffer* buffer);

			void Draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance);
			void DrawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, u32 vertexOffset, u32 firstInstance);

		private:
			template<typename T>
			void AddCommand(T command)
			{
				static_assert(std::is_base_of_v<ICommand, T> && "[CommandList::AddCommand] T must be derived from ICommand.");

				ICommand& baseCommand = static_cast<ICommand&>(command);
				const u64 commandSize = baseCommand.GetSize();
				Byte* endPos = (Byte*)m_commands + m_commandMaxByteSize;
				if (m_pos + commandSize > endPos)
				{
					Resize(m_commandMaxByteSize * 2);
				}
				memcpy((void*)m_pos, (const void*)&command, commandSize);
				m_pos += baseCommand.GetSize();
				++m_commandCount;
			}

			void Resize(u64 newSize);

			u64 GetByteSizeFromStart() const;
			u64 GetBytesSizeReadHeadFromStart() const;

		private:
			u64 m_commandCount = 0;
			unsigned char* m_readHead = 0;
			unsigned char* m_pos = 0;
			ICommand* m_commands = nullptr;
			u64 m_commandMaxByteSize = 0;

			PipelineStateObject m_pso;
		};
	}
}