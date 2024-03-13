#version 450
//#extension GL_EXT_debug_printf : enable

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec4 vertex_color;

layout(location = 0) out vec4 frag_color;

layout(push_constant, std430) uniform PushConstants
{
	mat3 mesh_rotation;
} pc;

void main()
{
    //debugPrintfEXT("My float is %f %f %f %f %f %f %f %f %f\n", m[0][0], m[0][1], m[0][2], m[1][0], m[1][1], m[1][2], m[2][0], m[2][1], m[2][2]);
    frag_color = vertex_color;
    gl_Position = vec4(pc.mesh_rotation*vertex_position, 1.0);
}