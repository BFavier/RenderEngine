#version 450
//#extension GL_EXT_debug_printf : enable

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec4 vertex_color;

layout(push_constant, std430) uniform MeshParameters
{
    vec4 mesh_position;
    vec4 mesh_scale;
	mat3 mesh_rotation;
} mp;

layout(set=0, binding=0) uniform CameraParameters
{
    vec4 camera_position;
    mat3 world_to_camera;
    vec2 camera_screen_dim;
    float focal_length;
} cp;

layout(location = 0) out vec4 frag_color;

void main()
{
    // mesh coords to world coords
    vec3 position = vec3(mp.mesh_position) + mp.mesh_rotation*(vec3(mp.mesh_scale) * vertex_position);
    vec3 normal = normalize(mp.mesh_rotation*(vec3(mp.mesh_scale) * vertex_normal));
    // world coords to camera coords
    position = cp.world_to_camera * (position - vec3(cp.camera_position));
    normal = cp.world_to_camera * normal;
    // camera coords to screen coords
    vec2 xy_coords = vec2(position) * cp.focal_length / (cp.focal_length + position.z) * 2/cp.camera_screen_dim;
    // outputs
    gl_Position = vec4(xy_coords, 1 - exp(-position.z), 1.0);
    frag_color = vertex_color;

    //debugPrintfEXT("camera position is %f %f %f\n", cp.camera_position.x, cp.camera_position.y, cp.camera_position.z);
    //debugPrintfEXT("camera screen dimension is %f %f, focal_length is %f\n", cp.camera_screen_dim.x, cp.camera_screen_dim.y, cp.focal_length);
    //debugPrintfEXT("Vertex position is %f %f %f\n", xy_coords.x, xy_coords.y, 1 - exp(-position.z));
}