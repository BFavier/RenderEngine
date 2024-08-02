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
    //color = vec4(vec3(fragment_material), 1.0);
    float angle = max(dot(vec3(fragment_normal), vec3(0., 0., -1.)), 0.);
    color = vec4(angle, angle, angle, 1.0);
    //color = fragment_albedo * 0.5*(abs(dot(vec3(0., 0., -1.), fragment_normal)) + 1.0);

    /*
    if (vec3(fragment_normal) != vec3(0., 0., 0.))
    {
        debugPrintfEXT("fragment normal is (x=%f, y=%f, z=%f)\n", fragment_normal.x, fragment_normal.y, fragment_normal.z);
    }
    */
}