#version 450

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
    vec2 xy = positions[gl_VertexIndex];
    vertex_uv = (xy + 1.0) * 0.5;
    gl_Position = vec4(xy, 0.0, 1.0);
}