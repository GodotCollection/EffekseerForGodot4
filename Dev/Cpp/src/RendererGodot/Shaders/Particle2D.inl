
const char code[] = 
#if !LIGHTING
R"(
render_mode unshaded;
)"
#endif

#if DISTORTION || LIGHTING
R"(
varying vec4 v_UVTangent;
uniform sampler2D UVTangentTexture : hint_normal;
)"
#endif

#if DISTORTION
R"(
uniform float DistortionIntensity;
uniform sampler2D DistortionTexture : hint_normal, repeat_enable;
uniform sampler2D ScreenTexture : hint_screen_texture;
)"
#elif LIGHTING
R"(
uniform float EmissiveScale;
uniform sampler2D ColorTexture : source_color, repeat_enable;
uniform sampler2D NormalTexture : hint_normal, repeat_enable;
)"
#else
R"(
uniform float EmissiveScale;
uniform sampler2D ColorTexture : source_color, repeat_enable;
)"
#endif

#include "Common2D.inl"

R"(
void vertex() {
)"
#if DISTORTION || LIGHTING
R"(
	v_UVTangent = texture(UVTangentTexture, UV);
)"
#endif
R"(
}
)"

R"(
void fragment() {
)"
#if DISTORTION
R"(
	vec2 distortionUV = DistortionMap(DistortionTexture, v_UVTangent.xy, DistortionIntensity, COLOR.xy, v_UVTangent.zw);
	COLOR = ColorMap(ScreenTexture, SCREEN_UV + distortionUV, vec4(1.0, 1.0, 1.0, COLOR.a));
)"
#elif LIGHTING
R"(
	NORMAL = NormalMap(NormalTexture, v_UVTangent.xy, v_UVTangent.zw);
	COLOR = ColorMap(ColorTexture, v_UVTangent.xy, COLOR);
	COLOR.rgb *= EmissiveScale;
)"
#else
R"(
	COLOR = ColorMap(ColorTexture, UV, COLOR);
	COLOR.rgb *= EmissiveScale;
)"
#endif
R"(
}
)";

const Shader::ParamDecl decl[] = {
#if DISTORTION
	{ "DistortionIntensity", Shader::ParamType::Float, 0, 1, 48 },
	{ "DistortionTexture", Shader::ParamType::Texture, 0, 0, 0 },
	{ "UVTangentTexture", Shader::ParamType::Texture, 0, 1, 0 },
#elif LIGHTING
	{ "EmissiveScale", Shader::ParamType::Float, 0, 1, offsetof(EffekseerRenderer::PixelConstantBuffer, EmmisiveParam) },
	{ "ColorTexture",  Shader::ParamType::Texture, 0, 0, 0 },
	{ "NormalTexture", Shader::ParamType::Texture, 0, 1, 0 },
#else
	{ "EmissiveScale", Shader::ParamType::Float, 0, 1, offsetof(EffekseerRenderer::PixelConstantBuffer, EmmisiveParam) },
	{ "ColorTexture",  Shader::ParamType::Texture, 0, 0, 0 },
#endif
};
