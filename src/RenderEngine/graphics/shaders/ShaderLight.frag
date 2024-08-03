#version 450
// RenderEngine.depth_test = false
// RenderEngine.blending = Blending::ADD
// #extension GL_EXT_debug_printf : enable

#define NONE             0
#define ORTHOGRAPHIC     1
#define EQUIRECTANGULAR  2
#define PERSPECTIVE      3

layout(push_constant, std430) uniform LightParameters
{
    vec4 light_position;
	mat3 light_inverse_rotation;
    vec4 light_color_intensity;
    uint projection_type;
    float camera_sensitivity;
} params;

layout(set=0, binding=0) uniform sampler2D albedo;
layout(set=0, binding=1) uniform sampler2D normal;
layout(set=0, binding=2) uniform sampler2D material;

layout(location = 0) in vec2 vertex_uv;

layout(location = 0) out vec4 color;

void main()
{
    vec4 fragment_albedo = texture(albedo, vertex_uv);
    vec4 fragment_normal = texture(normal, vertex_uv);
    vec4 fragment_material = texture(material, vertex_uv);

    if (params.projection_type == NONE)
    {
        color = fragment_albedo * vec4(vec3(params.light_color_intensity) * params.light_color_intensity.a / params.camera_sensitivity, 1.0);
    }
    else if (params.projection_type == ORTHOGRAPHIC)
    {
        float cos_angle = max(dot(vec3(fragment_normal), vec3(0., 0., -1.)), 0.);
        color = vec4(cos_angle, cos_angle, cos_angle, 1.0) * fragment_albedo * vec4(vec3(params.light_color_intensity) * params.light_color_intensity.a / params.camera_sensitivity, 1.0);
    }
    // debugPrintfEXT("fragment normal is (x=%f, y=%f, z=%f)\n", fragment_normal.x, fragment_normal.y, fragment_normal.z);
}