#version 450
// RenderEngine.depth_test = false
// RenderEngine.blending = Shader::Blending::OVERWRITE

const vec2 positions[6] = vec2[]
(
    vec2(-1.0, 1.0),
    vec2(1.0, -1.0),
    vec2(-1.0, -1.0),
    vec2(-1.0, 1.0),
    vec2(1.0, -1.0),
    vec2(1.0, 1.0)
);

layout(location = 0) out vec2 vertex_uv;

void main()
{
    vec2 position = positions[gl_VertexIndex];
    gl_Position = vec4(position, 0.0, 1.0);
    
}