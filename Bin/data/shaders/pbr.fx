#include "common.fx"

#define PI 3.14159265359f

//--------------------------------------------------------------------------------------
// GBuffer generation pass. Vertex
//--------------------------------------------------------------------------------------
void VS_GBuffer(
	in float4 iPos     : POSITION
	, in float3 iNormal : NORMAL0
	, in float2 iTex0 : TEXCOORD0
	, in float2 iTex1 : TEXCOORD1
	, in float4 iTangent : NORMAL1

	, out float4 oPos : SV_POSITION
	, out float3 oNormal : NORMAL0
	, out float4 oTangent : NORMAL1
	, out float2 oTex0 : TEXCOORD0
	, out float2 oTex1 : TEXCOORD1
	, out float3 oWorldPos : TEXCOORD2
)
{
	float4 world_pos = mul(iPos, obj_world);
	oPos = mul(world_pos, camera_view_proj);

	// Rotar la normal segun la transform del objeto
	oNormal = mul(iNormal, (float3x3)obj_world);
	oTangent.xyz = mul(iTangent.xyz, (float3x3)obj_world);
	oTangent.w = iTangent.w;

	// Las uv's se pasan directamente al ps
	oTex0 = iTex0;
	oTex1 = iTex1;
	oWorldPos = world_pos.xyz;
}

//--------------------------------------------------------------------------------------
// GBuffer generation pass. Pixel shader
//--------------------------------------------------------------------------------------
void PS_GBuffer(
	float4 Pos       : SV_POSITION
	, float3 iNormal : NORMAL0
	, float4 iTangent : NORMAL1
	, float2 iTex0 : TEXCOORD0
	, float2 iTex1 : TEXCOORD1
	, float3 iWorldPos : TEXCOORD2
	, out float4 o_albedo : SV_Target0
	, out float4 o_normal : SV_Target1
	, out float1 o_depth : SV_Target2
	, out float4 o_selfIllum : SV_Target3
)
{
	// Store in the Alpha channel of the albedo texture, the 'metallic' amount of
	// the material
	o_albedo = txAlbedo.Sample(samLinear, iTex0);
	o_albedo.a = txMetallic.Sample(samLinear, iTex0).r;
	o_selfIllum = color_emission * txEmissive.Sample(samLinear, iTex0);

	// Save roughness in the alpha coord of the N render target
	float roughness = txRoughness.Sample(samLinear, iTex0).r;
	float3 N = computeNormalMap(iNormal, iTangent, iTex0);
	o_normal = encodeNormal(N, roughness);

	// Si el material lo pide, sobreescribir los valores de la textura
	// por unos escalares uniformes. Only to playtesting...
	if (scalar_metallic >= 0.f)
		o_albedo.a = scalar_metallic;
	if (scalar_roughness >= 0.f)
		o_normal.a = scalar_roughness;

	// Compute the Z in linear space, and normalize it in the range 0...1
	// In the range z=0 to z=zFar of the camera (not zNear)
	float3 camera2wpos = iWorldPos - camera_pos;
	o_depth = dot(camera_front.xyz, camera2wpos) / camera_zfar;
}

//--------------------------------------------------------------------------------------
void decodeGBuffer(
	in float2 iPosition          // Screen coords
	, out float3 wPos
	, out float3 N
	, out float3 real_albedo
	, out float3 real_specular_color
	, out float  roughness
	, out float3 reflected_dir
	, out float3 view_dir
) {

	int3 ss_load_coords = uint3(iPosition.xy, 0);

	// Recover world position coords
	float  zlinear = txGBufferLinearDepth.Load(ss_load_coords).x;
	wPos = getWorldCoords(iPosition.xy, zlinear);

	// Recuperar la normal en ese pixel. Sabiendo que se
	// guardó en el rango 0..1 pero las normales se mueven
	// en el rango -1..1
	float4 N_rt = txGBufferNormals.Load(ss_load_coords);
	N = decodeNormal(N_rt.xyz);
	N = normalize(N);

	// Get other inputs from the GBuffer
	float4 albedo = txGBufferAlbedos.Load(ss_load_coords);
	// In the alpha of the albedo, we stored the metallic value
	// and in the alpha of the normal, we stored the roughness
	float  metallic = albedo.a;
	roughness = N_rt.a;

	// Apply gamma correction to albedo to bring it back to linear.
	albedo.rgb = pow(albedo.rgb, 2.2f);

	// Lerp with metallic value to find the good diffuse and specular.
	// If metallic = 0, albedo is the albedo, if metallic = 1, the
	// used albedo is almost black
	real_albedo = albedo.rgb * (1. - metallic);

	// 0.03 default specular value for dielectric.
	real_specular_color = lerp(0.03f, albedo.rgb, metallic);

	// Eye to object
	float3 incident_dir = normalize(wPos - camera_pos.xyz);
	reflected_dir = normalize(reflect(incident_dir, N));
	view_dir = -incident_dir;
}

// -------------------------------------------------
// Gloss = 1 - rough*rough
float3 Specular_F_Roughness(float3 specularColor, float gloss, float3 h, float3 v)
{
	// Sclick using roughness to attenuate fresnel.
	return (specularColor + (max(gloss, specularColor) - specularColor) * pow((1 - saturate(dot(v, h))), 5));
}

float NormalDistribution_GGX(float a, float NdH)
{
	// Isotropic ggx.
	float a2 = a * a;
	float NdH2 = NdH * NdH;

	float denominator = NdH2 * (a2 - 1.0f) + 1.0f;
	denominator *= denominator;
	denominator *= PI;

	return a2 / denominator;
}

float Geometric_Smith_Schlick_GGX(float a, float NdV, float NdL)
{
	// Smith schlick-GGX.
	float k = a * 0.5f;
	float GV = NdV / (NdV * (1 - k) + k);
	float GL = NdL / (NdL * (1 - k) + k);

	return GV * GL;
}

float Specular_D(float a, float NdH)
{
	return NormalDistribution_GGX(a, NdH);
}

float Specular_G(float a, float NdV, float NdL, float NdH, float VdH, float LdV)
{
	return Geometric_Smith_Schlick_GGX(a, NdV, NdL);
}

float3 Fresnel_Schlick(float3 specularColor, float3 h, float3 v)
{
	return (specularColor + (1.0f - specularColor) * pow((1.0f - saturate(dot(v, h))), 5));
}

float3 Specular_F(float3 specularColor, float3 h, float3 v)
{
	return Fresnel_Schlick(specularColor, h, v);
}

float3 Specular(float3 specularColor, float3 h, float3 v, float3 l, float a, float NdL, float NdV, float NdH, float VdH, float LdV)
{
	return ((Specular_D(a, NdH) * Specular_G(a, NdV, NdL, NdH, VdH, LdV)) * Specular_F(specularColor, v, h)) / (4.0f * NdL * NdV + 0.0001f);
}

//--------------------------------------------------------------------------------------
// Ambient pass, to compute the ambient light of each pixel
float4 PS_ambient(in float4 iPosition : SV_Position) : SV_Target
{
	// Declare some float3 to store the values from the GBuffer
	float  roughness;
	float3 wPos, N, albedo, specular_color, reflected_dir, view_dir;
	decodeGBuffer(iPosition.xy, wPos, N, albedo, specular_color, roughness, reflected_dir, view_dir);

	// if roughness = 0 -> I want to use the miplevel 0, the all-detailed image
	// if roughness = 1 -> I will use the most blurred image, the 8-th mipmap, If image was 256x256 => 1x1
	float mipIndex = roughness * roughness * 8.0f;
	float3 env = txEnvironmentMap.SampleLevel(samLinear, reflected_dir, mipIndex).xyz;
	env = pow(env, 2.2f);	// Convert the color to linear also.

	// The irrandiance, is read using the N direction.
	// Here we are sampling using the cubemap-miplevel 4, and the already blurred txIrradiance texture
	// and mixing it in base to the scalar_irradiance_vs_mipmaps which comes from the ImGui.
	// Remove the interpolation in the final version!!!
	float3 irradiance_mipmaps = txEnvironmentMap.SampleLevel(samLinear, N, 4).xyz;
	float3 irradiance_texture = txIrradianceMap.Sample(samLinear, N).xyz;
	float3 irradiance = irradiance_texture * scalar_irradiance_vs_mipmaps + irradiance_mipmaps * (1. - scalar_irradiance_vs_mipmaps);

	// How much the environment we see
	float3 env_fresnel = Specular_F_Roughness(specular_color, 1. - roughness * roughness, N, view_dir);

	float g_ReflectionIntensity = 0.5;
	float g_AmbientLightIntensity = 1.0;
	float4 self_illum = scalar_emission * txSelfIllum.Load(uint3(iPosition.xy,0)); // temp 

	float4 final_color = float4(env_fresnel * env * g_ReflectionIntensity + albedo.xyz * irradiance * g_AmbientLightIntensity, 1.0f) + self_illum;

	return final_color * global_ambient_adjustment;
}

//--------------------------------------------------------------------------------------
// The geometry that approximates the light volume uses this shader
void VS_pass(
	in float4 iPos : POSITION
	, in float3 iNormal : NORMAL0
	, in float2 iTex0 : TEXCOORD0
	, in float2 iTex1 : TEXCOORD0
	, in float4 iTangent : NORMAL1
	, out float4 oPos : SV_POSITION
) {
	float4 world_pos = mul(iPos, obj_world);
	oPos = mul(world_pos, camera_view_proj);
}

// --------------------------------------------------------
float3 Diffuse(float3 pAlbedo)
{
	return pAlbedo / PI;
}

// --------------------------------------------------------
float4 shade(float4 iPosition, bool use_shadows)
{
	// Decode GBuffer information
	float3 wPos, N, albedo, specular_color, reflected_dir, view_dir;
	float  roughness;
	decodeGBuffer(iPosition.xy, wPos, N, albedo, specular_color, roughness, reflected_dir, view_dir);
	N = normalize(N);

	// From wPos to Light
	float3 light_dir_full = light_pos.xyz - wPos;
	float  distance_to_light = length(light_dir_full);
	float3 light_dir = light_dir_full / distance_to_light;

	float  NdL = saturate(dot(N, light_dir));
	float  NdV = saturate(dot(N, view_dir));
	float3 h = normalize(light_dir + view_dir); // half vector

	float  NdH = saturate(dot(N, h));
	float  VdH = saturate(dot(view_dir, h));
	float  LdV = saturate(dot(light_dir, view_dir));
	float  a = max(0.001f, roughness * roughness);
	float3 cDiff = Diffuse(albedo);
	float3 cSpec = Specular(specular_color, h, view_dir, light_dir, a, NdL, NdV, NdH, VdH, LdV);

	float  att = (1. - smoothstep(0.90, 0.98, distance_to_light / light_radius)); // Att, point light
	//float  att_spot = (1. - smoothstep(light_angle_spot, 1, dot(-light_dir, light_color.xyz)));
	//att *= 1 / distance_to_light;

	// Shadow factor entre 0 (totalmente en sombra) y 1 (no ocluido)
	float shadow_factor = use_shadows ? computeShadowFactor(wPos) : 1.;

	// Determine spot angle, temp condition
	float lightToSurfaceAngle = degrees(acos(dot(-light_dir, light_color.xyz)));
	if (lightToSurfaceAngle < 22.5) {
		float3 final_color = NdL * (cDiff * (1.0f - cSpec) + cSpec) * att * light_intensity * shadow_factor;// *projectColor(wPos).xyz;
		return float4(final_color, 1);
	}

	return float4(0, 0, 0, 1);
}

float4 PS_point_lights(in float4 iPosition : SV_Position) : SV_Target
{
	return shade(iPosition, false);
}

float4 PS_dir_lights(in float4 iPosition : SV_Position) : SV_Target
{
	return shade(iPosition, true);
}

float4 PS_spot_lights(in float4 iPosition : SV_Position) : SV_Target
{
	return shade(iPosition, false);
}

// ----------------------------------------
void VS_skybox(in float4 iPosition : POSITION, in float4 iColor : COLOR0, out float4 oPosition : SV_Position)
{
	oPosition = float4(iPosition.x * 2 - 1., 1 - iPosition.y * 2, 1, 1);
}

// --------------------------------------------------------
float4 PS_skybox(in float4 iPosition : SV_Position) : SV_Target
{
	float3 view_dir = mul(float4(iPosition.xy, 1, 1), camera_screen_to_world).xyz;
	float4 skybox_color = txEnvironmentMap.Sample(samLinear, view_dir);
	return float4(skybox_color.xyz, 1);// *global_ambient_adjustment;
}