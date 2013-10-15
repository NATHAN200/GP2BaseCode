float4x4 matWorld:WORLD;
float4x4 matView:VIEW;
float4x4 matProjection:PROJECTION;
Texture2D lightTexture;
float Time : TIME;
float4 objectColour
<
	string UIName="Object Colour";
	string UIWidget="Color";
> = {1,1,1,1};

struct VS_INPUT
{
	float4 pos:POSITION;
	float2 texCoord:TEXCOORD0;
};

struct PS_INPUT
{
	float4 pos:SV_POSITION;
	float2 texCoord:TEXCOORD0;
};

PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output=(PS_INPUT)0;
	output.texCoord=input.texCoord;
	float4x4 matViewProjection=mul(matView,matProjection);
	float4x4 matWorldViewProjection=mul(matWorld,matViewProjection);
	
	output.pos=mul(input.pos,matWorldViewProjection);
	return output;
}
Texture2D diffuseTexture;

SamplerState WrapPointSampler
{
	Filter = MIN_MAG_LINEAR_MIP_POINT;
	AddressU= Wrap;
	AddressV = Wrap;
};
float4 PS(PS_INPUT input):SV_TARGET
{
	
	float4 diffuseSample = diffuseTexture.Sample(WrapPointSampler,input.texCoord);
	float4 lightSample = lightTexture.Sample(WrapPointSampler,input.texCoord);
	float4 result=diffuseSample*lightSample;
	return result;
}

RasterizerState DisableCulling
{
	CullMode=NONE;
};

technique10 Render
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0,VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0,PS()));
		SetRasterizerState(DisableCulling);
	}
} 
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
