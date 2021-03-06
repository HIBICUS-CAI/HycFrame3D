#include "color_utility.hlsli"

// TEMP-----------------------------
#define gScreenWidth (1280)
#define gScreenHeight (720)
// TEMP-----------------------------

struct VS_OUTPUT
{
    float4 PosH : SV_POSITION;
};

Buffer<float4> gParticleRenderBuffer : register(t0);

float4 Main(float4 posH : SV_POSITION) : SV_Target
{
	float4 color = (float4)0.f;

	float x = posH.x - (gScreenWidth / 2);
	float y = posH.y;
	uint pixelIndex = x + (y * gScreenWidth);
	float4 particleValue = gParticleRenderBuffer.Load(pixelIndex);
	color = particleValue;
	color.rgb = sRGBToACES(color.rgb);

	return color;
}
