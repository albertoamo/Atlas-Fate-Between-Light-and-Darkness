//--------------------------------------------------------------------------------------
#include "ctes.h"

//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
  float4 Pos : SV_POSITION;
  float3 N : NORMAL;
  float2 UV : UV;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS(
   float4 Pos : POSITION
  , float3 N : NORMAL
  , float2 UV : TEXCOORD0
)

{
  VS_OUTPUT output = (VS_OUTPUT)0;
  output.Pos = mul(Pos, obj_world);
  output.Pos = mul(output.Pos, camera_view);
  output.Pos = mul(output.Pos, camera_proj);
  // Rotate the normal
  output.N = mul(N, (float3x3)obj_world);
  output.UV = UV;
  return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
Texture2D    txDiffuse      : register(t0);
SamplerState samLinear      : register(s0);

float4 PS(VS_OUTPUT input) : SV_Target
{

	float3 Light = float3(1,1,1);
	Light = normalize(Light);
	float diffuseAmount = dot(input.N, Light);
	diffuseAmount = saturate(0.2 + diffuseAmount);
	diffuseAmount = 0.3 + diffuseAmount * 0.7;

	float4 texture_color = txDiffuse.Sample(samLinear, input.UV);
	return texture_color * obj_color * diffuseAmount;
}