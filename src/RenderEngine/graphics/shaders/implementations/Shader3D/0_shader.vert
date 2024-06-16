#version 450
#extension GL_EXT_scalar_block_layout : enable  // for std430 uniform buffer object layouts
//#extension GL_EXT_debug_printf : enable

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec4 vertex_color;

layout(push_constant, std430) uniform MeshDrawParameters
{
    vec4 mesh_position;
	mat3 mesh_rotation;
    vec4 camera_parameters;
    float mesh_scale;
} mp;

layout(set=0, binding=0, std430) uniform CameraParameters
{
    vec4 camera_position;
    mat3 world_to_camera;
    vec2 camera_aperture_size;
    float focal_length;
    float camera_scale;
} cp;

layout(location = 0) out vec4 frag_color;

void main()
{
    float field_of_view = mp.camera_parameters.x;
    float width_to_height_ratio = mp.camera_parameters.y;
    float near_plane = mp.camera_parameters.z;
    float far_plane = mp.camera_parameters.w;
    // mesh coords to world coords
    vec3 position = vec3(mp.mesh_position) + mp.mesh_rotation * (mp.mesh_scale * vertex_position);
    vec3 normal = mp.mesh_rotation * vertex_normal;
    // output in clip coords: normalised device coordinates = (x_clip, y_clip, z_clip) / w_clip
    gl_Position = vec4(position.x / (width_to_height_ratio * tan(field_of_view/2.0)),
                       position.y / tan(field_of_view/2.0),
                       (position.z - near_plane) * far_plane / (far_plane - near_plane),
                       position.z);
    frag_color = vertex_color;

    //debugPrintfEXT("fragment coordinates is (x=%f, y=%f, z=%f) screen coordinates are (x=%f, y=%f, z=%f)\n", position.x, position.y, position.z, gl_Position.x, gl_Position.y, gl_Position.z);
}