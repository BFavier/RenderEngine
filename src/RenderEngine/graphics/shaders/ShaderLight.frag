#version 450
// RenderEngine.depth_test = false
// RenderEngine.blending = Shader::Blending::ALPHA
//#extension GL_EXT_debug_printf : enable

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
    float cos_angle = max(dot(vec3(fragment_normal), vec3(0., 0., -1.)), 0.);
    color = (0.8 * vec4(cos_angle, cos_angle, cos_angle, 1.0) + 0.2) * fragment_albedo;

    /*
    if (vec3(fragment_normal) != vec3(0., 0., 0.))
    {
        debugPrintfEXT("fragment normal is (x=%f, y=%f, z=%f)\n", fragment_normal.x, fragment_normal.y, fragment_normal.z);
    }
    */
}