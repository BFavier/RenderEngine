#version 450
// RenderEngine.depth_test = false
// RenderEngine.blending = Shader::Blending::OVERWRITE

layout(set=0, binding=0) uniform sampler2D albedo;
layout(set=0, binding=1) uniform sampler2D normal;
layout(set=0, binding=2) uniform sampler2D material;

layout(location = 0) in vec2 vertex_uv;

layout(location = 0) out vec4 color;

void main()
{
    vec4 fragment_albedo = texture(albedo, vertex_uv);
    vec3 fragment_normal = vec3(texture(normal, vertex_uv));
    vec4 fragment_material = texture(material, vertex_uv);
    color = fragment_albedo * 0.5*(abs(dot(vec3(0., 0., -1.), fragment_normal)) + 1.0);
}