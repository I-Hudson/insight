#include "Graphics/RHI/RHI_Shader.h"
#include "Graphics/GraphicsManager.h"

#include "Graphics/RHI/Vulkan/RHI_Shader_Vulkan.h"
#include "Graphics/RHI/DX12/RHI_Shader_DX12.h"

namespace Insight
{
	namespace Graphics
	{
		RHI_Shader* RHI_Shader::New()
		{
			if (GraphicsManager::IsVulkan()) { return new RHI::Vulkan::RHI_Shader_Vulkan(); }
			else if (GraphicsManager::IsDX12()) { return new RHI::DX12::RHI_Shader_DX12(); }
			return nullptr;
		}


		RHI_ShaderManager::RHI_ShaderManager()
		{
		}

		RHI_ShaderManager::~RHI_ShaderManager()
		{
		}

		RHI_Shader* RHI_ShaderManager::GetOrCreateShader(ShaderDesc desc)
		{
			if (!desc.IsValid())
			{
				return nullptr;
			}
			const u64 hash = desc.GetHash();
			auto itr = m_shaders.find(hash);
			if (itr != m_shaders.end())
			{
				return itr->second;
			}

			RHI_Shader* shader = RHI_Shader::New();
			shader->Create(m_context, desc);
			m_shaders[hash] = shader;

			return shader;
		}

		void RHI_ShaderManager::Destroy()
		{
			for (const auto& pair : m_shaders)
			{
				pair.second->Destroy();
				delete pair.second;
			}
			m_shaders.clear();
		}



		ShaderCompiler::ShaderCompiler()
		{
			HRESULT hres;
			// Initialize DXC library
			hres = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&s_dxUtils));
			if (FAILED(hres)) 
			{
				throw std::runtime_error("Could not init DXC Library");
			}

			// Initialize the DXC compiler
			hres = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&s_dxCompiler));
			if (FAILED(hres))
			{
				throw std::runtime_error("Could not init DXC Compiler");
			}
		}

		ShaderCompiler::~ShaderCompiler()
		{
		}

		RHI::DX12::ComPtr<IDxcBlob> ShaderCompiler::Compile(ShaderStageFlagBits stage, std::wstring_view filePath, std::vector<std::wstring> additionalArgs)
		{
			HRESULT hres;
			// Create default include handler. (You can create your own...)
			RHI::DX12::ComPtr<IDxcIncludeHandler> pIncludeHandler;
			s_dxUtils->CreateDefaultIncludeHandler(&pIncludeHandler);

			// Load the HLSL text shader from disk
			uint32_t codePage = CP_UTF8;
			RHI::DX12::ComPtr<IDxcBlobEncoding> sourceBlob;
			hres = s_dxUtils->LoadFile(filePath.data(), nullptr, &sourceBlob);
			if (FAILED(hres))
			{
				throw std::runtime_error("Could not load shader file");
			}

			DxcBuffer Source;
			Source.Ptr = sourceBlob->GetBufferPointer();
			Source.Size = sourceBlob->GetBufferSize();
			Source.Encoding = DXC_CP_ACP; // Assume BOM says UTF8 or UTF16 or this is ANSI text.


			// Set up arguments to be passed to the shader compiler

			// Tell the compiler to output SPIR-V
			std::vector<LPCWSTR> arguments;
			for (const std::wstring& arg : additionalArgs)
			{
				arguments.push_back(arg.c_str());
			}

			std::wstring mainFunc = StageToFuncName(stage);
			std::wstring targetProfile = StageToProfileTarget(stage);

			// Entry point
			arguments.push_back(L"-E");
			arguments.push_back(mainFunc.c_str());

			// Select target profile based on shader file extension
			arguments.push_back(L"-T");
			arguments.push_back(targetProfile.c_str());

			// Compile shader
			RHI::DX12::ComPtr<IDxcResult> pResults;
			hres = s_dxCompiler->Compile(
				&Source,                // Source buffer.
				arguments.data(),       // Array of pointers to arguments.
				arguments.size(),		// Number of arguments.
				pIncludeHandler.Get(),	// User-provided interface to handle #include directives (optional).
				IID_PPV_ARGS(&pResults) // Compiler output status, buffer, and errors.
			);

			// Print errors if present.
			RHI::DX12::ComPtr<IDxcBlobUtf8> pErrors = nullptr;
			pResults->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), nullptr);
			// Note that d3dcompiler would return null if no errors or warnings are present.  
			// IDxcCompiler3::Compile will always return an error buffer, but its length will be zero if there are no warnings or errors.
			if (pErrors != nullptr && pErrors->GetStringLength() != 0)
			{
				IS_CORE_ERROR(fmt::format("Shader compilation failed : \n\n{}", pErrors->GetStringPointer()));
			}

			// Get compilation result
			RHI::DX12::ComPtr<IDxcBlob> code;
			pResults->GetResult(&code);

			//std::vector<uint8_t> dxil_data;
			//for (size_t i = 0; i < code->GetBufferSize(); ++i)
			//{
			//	dxil_data.push_back(*((uint8_t*)code->GetBufferPointer() + i));
			//}

			//RHI::DX12::ComPtr<IDxcBlobEncoding> container_blob;

			//s_dxUtils->CreateBlobFromPinned(
			//	(BYTE*)dxil_data.data(),
			//	(UINT32)dxil_data.size(),
			//	0 /* binary, no code page */,
			//	container_blob.GetAddressOf());


			return code;
		}

		std::wstring ShaderCompiler::StageToFuncName(ShaderStageFlagBits stage)
		{
			switch (stage)
			{
			case ShaderStageFlagBits::ShaderStage_Vertex: return L"VSMain";
			case ShaderStageFlagBits::ShaderStage_TessControl: return L"TSMain";
			case ShaderStageFlagBits::ShaderStage_TessEval: return L"TEMain";
			case ShaderStageFlagBits::ShaderStage_Geometry: return L"GSMain";
			case ShaderStageFlagBits::ShaderStage_Pixel: return L"PSMain";
				break;
			}
			return L"";
		}

		std::wstring ShaderCompiler::StageToProfileTarget(ShaderStageFlagBits stage)
		{
			switch (stage)
			{
			case ShaderStageFlagBits::ShaderStage_Vertex: return L"vs_6_1";
			case ShaderStageFlagBits::ShaderStage_TessControl: return L"hs_6_1";
			case ShaderStageFlagBits::ShaderStage_TessEval: return L"te_6_1";
			case ShaderStageFlagBits::ShaderStage_Geometry: return L"gs_6_1";
			case ShaderStageFlagBits::ShaderStage_Pixel: return L"ps_6_1";
				break;
			}
			return L"";
		}
	}
}