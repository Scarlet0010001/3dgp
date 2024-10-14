// UNIT.99
cbuffer SCENE_CONSTANTS : register(b3)
{
    row_major float4x4 view;
    row_major float4x4 projection;
    row_major float4x4 view_projection;
    row_major float4x4 inverse_projection;
    row_major float4x4 inverse_view_projection;
    float4 directional_light_direction[1];
    float4 directional_light_color[1]; // The w component is 'intensity'.
    float4 omni_light_position[1];
    float4 omni_light_color[1]; // The w component is 'intensity'.
    float4 rimlight_color;
    float4 camera_position;
    float4 camera_focus;
    float4 avatar_position;
    float4 avatar_direction;
    float time;
    float delta_time;
    float wind_frequency;
    float wind_strength;
    float rimlight_factor;
    float snow_factor;
}

cbuffer GRASS_CONSTANTS : register(b4)
{
    float grass_height_factor;
    float grass_width_factor;
    float grass_curvature;
    float grass_withered_factor;
    float grass_height_variance;
    float perlin_noise_distribution_factor;
    float tesselation_max_subdivision;
    float tesselation_max_distance;
    float4 grass_specular_color;
}

cbuffer SHADOW_CONSTANTS : register(b5)
{
    float shadow_depth_bias;
    float shadow_color;
    float shadow_filter_radius;
    uint shadow_sample_count;
    int colorize_cascaded_layer;
}

cbuffer BLOOM_CONSTANTS : register(b6)
{
    float bloom_extraction_threshold;
    float blur_convolution_intensity;
}

// Mist Density : This is the global density factor, which can be thought of as the mist layer's thickness.
// Mist Height Falloff : Height density factor, controls how the density increases as height decreases.Smaller values make the transition larger.
// Height Mist Offset : This controls the height offset of the mist layer relative to the Actor's placed Z (height).
// Mist Cutoff Distance : Scene elements past this distance will not have mist applied.This is useful for excluding skyboxes which already have mist baked into them.
cbuffer ATMOSPHERE_CONSTANTS : register(b7)
{
    float4 mist_color;
    float2 mist_density; // x:extinction, y:inscattering
    float2 mist_height_falloff; // x:extinction, y:inscattering
    float2 height_mist_offset; // x:extinction, y:inscattering

    float mist_cutoff_distance;

    float mist_flow_speed;
    float mist_flow_noise_scale_factor;
    float mist_flow_density_lower_limit;

    float distance_to_sun;
    float sun_highlight_exponential_factor; // Affects the area of influence of the sun's highlights.
    float sun_highlight_intensity;
}

cbuffer POST_EFFECT_CONSTANTS : register(b8)
{
    float brightness;
    float contrast;
    float hue;
    float saturation;

    float bokeh_aperture;
    float bokeh_focus;

    float exposure;
    float post_effect_options;

    // Projection Texture Mapping
    float4 projection_texture_intensity; // The w component is 'intensity'.
    row_major float4x4 projection_texture_transforms;

    float3 colorize;
}

struct cascade_shadow_map
{
    row_major float4x4 view_projection_matrices[4];
    float4 cascade_plane_distances;
};
cbuffer BLUR_CONSTANTS : register(b10)
{
    float smoothstep_minEdge;
    float smoothstep_maxEdge;

    float gaussian_sigma;
    float bloom_intensity;
}

cbuffer CASCADE_SHADOW_MAP_CONSTANTS : register(b13)
{
    cascade_shadow_map csm_data;
}



/*
static const int MAX_BONES = 256;
cbuffer OBJECT_CONSTANT_BUFFER : register(b0)
{
    row_major float4x4 world;
    float4 material_color;
    row_major float4x4 bone_transforms[MAX_BONES];

};

cbuffer SCENE_CONSTANT_BUFFER : register(b1)
{
    row_major float4x4 view;
    row_major float4x4 projection;
    row_major float4x4 view_projection;
    float4 light_color;
    float4 light_direction;
    float4 camera_position;
    float4 avatar_position;
    float4 avatar_direction;
    float2 resolution;
    float time;
    float delta_time;

    row_major float4x4 view_projection;
    float4 light_direction;
    float4 camera_position;

    //float smoothstep_minEdge;
    //float smoothstep_maxEdge;
    //
    //float gaussian_sigma;
    //float bloom_intensity;

    //float exposure;
    //float dummy1;
    //float dummy2;
    //float dummy3;

}

cbuffer SCENE_CONSTANT : register(b10)
{
    float smoothstep_minEdge;
    float smoothstep_maxEdge;

    float gaussian_sigma;
    float bloom_intensity;

    float exposure;
    float dummy1;
    float dummy2;
    float dummy3;


}
*/

