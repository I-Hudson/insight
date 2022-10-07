#include "Common.hlsl"

// Define all our textures needed

[[vk::combinedImageSampler]][[vk::binding(0, 1)]]
Texture2D<float4> GBuffer_Colour : register(t0);
[[vk::combinedImageSampler]][[vk::binding(0, 1)]]
SamplerState GBuffer_Colour_Sampler : register(s0);

[[vk::combinedImageSampler]][[vk::binding(1, 1)]]
Texture2D<float4> GBuffer_World_Normal: register(t1);
[[vk::combinedImageSampler]][[vk::binding(1, 1)]]
SamplerState GBuffer_World_Normal_Sampler : register(s1);

[[vk::combinedImageSampler]][[vk::binding(2, 1)]]
Texture2D<float4> GBuffer_Shadow : register(t2);
[[vk::combinedImageSampler]][[vk::binding(2, 1)]]
SamplerState GBuffer_Shadow_Sampler : register(s2);

[[vk::binding(3, 1)]]
Texture2DArray<float> Cascade_Shadow : register(t3);
[[vk::binding(4, 1)]]
SamplerComparisonState Cascade_Shadow_Sampler : register(s3);

/*------------------------------------------------------------------------------
    SETTINGS
------------------------------------------------------------------------------*/
// technique - all
static const uint   g_shadow_samples                 = 3;
static const float  g_shadow_filter_size             = 3.0f;
static const float  g_shadow_cascade_blend_threshold = 0.8f;
// technique - vogel
static const uint   g_penumbra_samples                = 8;
static const float  g_penumbra_filter_size            = 128.0f;
// technique - pre-calculated
static const float g_pcf_filter_size    = (sqrt((float)g_shadow_samples) - 1.0f) / 2.0f;
static const float g_shadow_samples_rpc = 1.0f / (float) g_shadow_samples;

struct PushConstant
{
	int Output_Texture;
	int Cascade_Override;
};
[[vk::push_constant]] PushConstant push_constant;

struct VertexOutput
{
	float4 Position : SV_POSITION;
	float2 UV : TEXCOORD0;
};

VertexOutput VSMain(uint id : SV_VertexID)
{
	VertexOutput o;

	o.UV  		= float2((id << 1) & 2, id & 2);
//#ifdef VULKAN
    o.Position 	= float4(o.UV * 2.0f + -1.0f, 0.0f, 1.0f);
//#else
    //o.Position 	= float4(o.UV * 2.0f + float2(-1.0f, 1.0f), 0.0f, 1.0f);
//#endif

	return o;
}

float Get_Normal_Dot_Light_Direction(float3 surface_normal, float3 light_direction)
{
	return saturate(dot(surface_normal, light_direction));
}

void Apply_Bias(inout float3 position, float3 surface_normal, float3 light_direction, float bias_mul = 1.0)
{
    //// Receiver plane bias (slope scaled basically)
    //float3 du                   = ddx(position);
    //float3 dv                   = ddy(position);
    //float2 receiver_plane_bias  = mul(transpose(float2x2(du.xy, dv.xy)), float2(du.z, dv.z));
    
    //// Static depth biasing to make up for incorrect fractional sampling on the shadow map grid
    //float sampling_error = min(2.0f * dot(g_shadow_texel_size, abs(receiver_plane_bias)), 0.01f);

    // Scale down as the user is interacting with much bigger, non-fractional values (just a UX approach)
    float fixed_factor = 0.0001f;
    
	float n_dot_l = Get_Normal_Dot_Light_Direction(surface_normal, light_direction);

    // Slope scaling
    float slope_factor = (1.0f - saturate(n_dot_l));

    // Apply bias
    position.z += fixed_factor * slope_factor * bias_mul;
}

float3 bias_normal_offset(float3 n_dot_l, float normal_bias, float2 shadow_resolution, float3 normal)
{
	float texel_size = 1.0 / shadow_resolution.x;
    return normal * (1.0f - saturate(n_dot_l)) * normal_bias * texel_size * 10;
}

float SampleShadow(float3 uv, float compare)
{
	return Cascade_Shadow.SampleCmpLevelZero(Cascade_Shadow_Sampler
											, uv
											, compare).r;
}

/*------------------------------------------------------------------------------
    TECHNIQUE - PCF
------------------------------------------------------------------------------*/
float Technique_Pcf(float3 uv, float compare)
{
    float shadow = 0.0f;
	float texel_size = 1.0 / 4096.0;

    for (float y = -g_pcf_filter_size; y <= g_pcf_filter_size; y++)
    {
        for (float x = -g_pcf_filter_size; x <= g_pcf_filter_size; x++)
        {
            float2 offset = float2(x, y) * texel_size;
            shadow        += SampleShadow(uv + float3(offset, 0.0), compare);
        }
    }
    
    return shadow * g_shadow_samples_rpc;
}

float4 PSMain(VertexOutput input) : SV_TARGET
{	
	float4 colour 						= GBuffer_Colour.Sample(GBuffer_Colour_Sampler, input.UV);
	float4 world_normal 				= GBuffer_World_Normal.Sample(GBuffer_World_Normal_Sampler, input.UV);
	float gbuffer_depth 				= GBuffer_Shadow.Sample(GBuffer_Shadow_Sampler, input.UV).r;

	float3 reconstruct_world_position 	= reconstruct_position(input.UV, gbuffer_depth, Main_Camera_Projection_View_Inverted);
	float3 reconstruct_view_position 	= mul(Main_Camera_View_Inverted, float4(reconstruct_world_position, 1)).xyz;

	float4 shadow 						= 1.0;
	float cascade_index 				= 0.0;
	float cascade_split_index 			= 0.0;
	float shadow_npc_z 					= 0.0;
	float3 shadow_ndc 					= 0.0;

{
    for (uint cascade = 0; cascade < 4; cascade++)
    {
		cascade_index = cascade;
		Shadow_Camera shadow_camera = shadow_cameras[cascade];

		if (reconstruct_view_position.z < -shadow_camera.Shadow_CameraSplit_Depth)
		{
			cascade_split_index = cascade;
			break;
		}
	}
	cascade_index = 0;
	Shadow_Camera shadow_camera = shadow_cameras[cascade_split_index];
	// Project into light space
	float4x4 shadow_space_matrix = shadow_camera.Shadow_Camera_ProjView;
    float3 shadow_pos_ndc = world_to_ndc(reconstruct_world_position, shadow_space_matrix);
	float2 shadow_uv = ndc_to_uv(shadow_pos_ndc);

	//float shadow_sample = SampleShadow(float3(shadow_uv, cascade_split_index), shadow_pos_ndc.z);
	float shadow_sample = Technique_Pcf(float3(shadow_uv, cascade_split_index), shadow_pos_ndc.z);
	shadow = shadow * shadow_sample;
}
{
/*
	//uint cascade = push_constant.Cascade_Override;
    for (uint cascade = 0; cascade < 4; cascade++)
    {
		cascade_index = cascade;
		Shadow_Camera shadow_camera = shadow_cameras[cascade];
		// Project into light space
		float4x4 shadow_space_matrix = shadow_camera.Shadow_Camera_ProjView;
        float3 shadow_pos_ndc = world_to_ndc(reconstruct_world_position, shadow_space_matrix);
		float2 shadow_uv = ndc_to_uv(shadow_pos_ndc);

		// Ensure not out of bound
    	if (is_saturated(shadow_uv))
    	{
			float shadow_sample = Cascade_Shadow.SampleCmpLevelZero(Cascade_Shadow_Sampler
																, float3(shadow_uv.xy, cascade)
																, shadow_pos_ndc.z).r;
			shadow = shadow * shadow_sample;
			break;
		}
	}
	*/
}
	float4 result;
	if(push_constant.Output_Texture == 0)
	{
		result = colour;
	}
	else if(push_constant.Output_Texture == 1)
	{
		result = float4(remap_to_0_to_1(world_normal.xyz), 1.0); //Remap normal to 0-1.
	}
	else if(push_constant.Output_Texture == 2)
	{
		result = float4(reconstruct_world_position, 1.0);
	}
	else if(push_constant.Output_Texture == 3)
	{
		result = shadow;
	}
	else if(push_constant.Output_Texture == 4)
	{
		float4 ambient = 0.25 * colour;
		result = colour * (shadow + ambient);
	}
	else if (push_constant.Output_Texture == 5)
	{
		result = float4(reconstruct_view_position, 1);
	}
	else if(push_constant.Output_Texture == 6)
	{
		if (cascade_split_index == 0)
		{
			result = float4(1, 0, 0, 1);
		}
		if (cascade_split_index == 1)
		{
			result = float4(0, 1, 0, 1);
		}
		if (cascade_split_index == 2)
		{
			result = float4(0, 0, 1, 1);
		}
		if (cascade_split_index == 3)
		{
			result = float4(1, 1, 1, 1);
		}
	}
	else if (push_constant.Output_Texture == 7)
	{
		//result = float4(shadow_npc_z, shadow_npc_z, shadow_npc_z, 1);
		result = float4(shadow_ndc, 1);
	}
	return result;
}