﻿#include <godot_cpp/classes/resource_loader.hpp>
#include "EffekseerGodot.MaterialLoader.h"
#include "../RendererGodot/EffekseerGodot.ShaderGenerator.h"
#include "../RendererGodot/EffekseerGodot.Shader.h"
#include "../RendererGodot/EffekseerGodot.RenderResources.h"
#include "../Utils/EffekseerGodot.Utils.h"
#include "../EffekseerResource.h"

namespace EffekseerGodot
{

::Effekseer::MaterialRef MaterialLoader::Load(const char16_t* path)
{
	// Load by Godot
	auto loader = godot::ResourceLoader::get_singleton();
	auto resource = loader->load(ToGdString(path), "");
	if (!resource.is_valid())
	{
		return nullptr;
	}

	auto efkres = godot::Ref<godot::EffekseerResource>(resource);
	auto& data = efkres->get_data_ref();
	if (data.size() == 0)
	{
		return nullptr;
	}

	return Load(data.ptr(), data.size(), Effekseer::MaterialFileType::Code);
}

::Effekseer::MaterialRef MaterialLoader::LoadAcutually(const ::Effekseer::MaterialFile& materialFile)
{
	using namespace EffekseerRenderer;

	auto material = ::Effekseer::MakeRefPtr<::Effekseer::Material>();
	material->IsSimpleVertex = materialFile.GetIsSimpleVertex();
	material->IsRefractionRequired = materialFile.GetHasRefraction();

	ShaderGenerator shaderGenerator;
	auto shaderList = shaderGenerator.Generate(materialFile);

	{
		auto shader = Shader::Create("Sprite_Material", RendererShaderType::Material);
		shader->SetVertexConstantBufferSize(std::max(shaderList[(size_t)ShaderType::Sprite2D].VertexConstantBufferSize, shaderList[(size_t)ShaderType::Sprite3D].VertexConstantBufferSize));
		shader->SetPixelConstantBufferSize(std::max(shaderList[(size_t)ShaderType::Sprite2D].PixelConstantBufferSize, shaderList[(size_t)ShaderType::Sprite3D].PixelConstantBufferSize));
		shader->SetCode(Shader::RenderType::SpatialLightweight, shaderList[(size_t)ShaderType::Sprite3D].ShaderCode.c_str(), std::move(shaderList[(size_t)ShaderType::Sprite3D].ParamDecls));
		shader->SetCode(Shader::RenderType::CanvasItem, shaderList[(size_t)ShaderType::Sprite2D].ShaderCode.c_str(), std::vector<Shader::ParamDecl>(shaderList[0].ParamDecls));
		shader->SetCustomData1Count(materialFile.GetCustomData1Count());
		shader->SetCustomData2Count(materialFile.GetCustomData2Count());
		material->UserPtr = shader.release();
	}
	{
		auto shader = Shader::Create("Model_Material", RendererShaderType::Material);
		shader->SetVertexConstantBufferSize(std::max(shaderList[(size_t)ShaderType::Model2D].VertexConstantBufferSize, shaderList[(size_t)ShaderType::Model3D].VertexConstantBufferSize));
		shader->SetPixelConstantBufferSize(std::max(shaderList[(size_t)ShaderType::Model2D].PixelConstantBufferSize, shaderList[(size_t)ShaderType::Model3D].PixelConstantBufferSize));
		shader->SetCode(Shader::RenderType::SpatialLightweight, shaderList[(size_t)ShaderType::Model3D].ShaderCode.c_str(), std::move(shaderList[(size_t)ShaderType::Model3D].ParamDecls));
		shader->SetCode(Shader::RenderType::CanvasItem, shaderList[(size_t)ShaderType::Model2D].ShaderCode.c_str(), std::move(shaderList[(size_t)ShaderType::Model2D].ParamDecls));
		shader->SetCustomData1Count(materialFile.GetCustomData1Count());
		shader->SetCustomData2Count(materialFile.GetCustomData2Count());
		material->ModelUserPtr = shader.release();
	}

	if (materialFile.GetHasRefraction())
	{
	}

	material->CustomData1 = materialFile.GetCustomData1Count();
	material->CustomData2 = materialFile.GetCustomData2Count();
	material->TextureCount = std::min(materialFile.GetTextureCount(), Effekseer::UserTextureSlotMax);
	material->UniformCount = materialFile.GetUniformCount();
	material->ShadingModel = materialFile.GetShadingModel();

	for (int32_t i = 0; i < material->TextureCount; i++)
	{
		material->TextureWrapTypes.at(i) = materialFile.GetTextureWrap(i);
	}

	return material;
}

::Effekseer::MaterialRef MaterialLoader::Load(const void* data, int32_t size, Effekseer::MaterialFileType fileType)
{
	Effekseer::MaterialFile materialFile;

	if (materialFile.Load((const uint8_t*)data, size))
	{
		return LoadAcutually(materialFile);
	}

	return nullptr;
}

void MaterialLoader::Unload(::Effekseer::MaterialRef data)
{
	if (data == nullptr)
		return;

	auto shader = reinterpret_cast<Shader*>(data->UserPtr);
	auto modelShader = reinterpret_cast<Shader*>(data->ModelUserPtr);
	auto refractionShader = reinterpret_cast<Shader*>(data->RefractionUserPtr);
	auto refractionModelShader = reinterpret_cast<Shader*>(data->RefractionModelUserPtr);

	ES_SAFE_DELETE(shader);
	ES_SAFE_DELETE(modelShader);
	ES_SAFE_DELETE(refractionShader);
	ES_SAFE_DELETE(refractionModelShader);

	data->UserPtr = nullptr;
	data->ModelUserPtr = nullptr;
	data->RefractionUserPtr = nullptr;
	data->RefractionModelUserPtr = nullptr;
}

} // namespace EffekseerGodot
