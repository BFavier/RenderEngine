#version 450
#extension GL_EXT_scalar_block_layout : enable  // for std430 uniform buffer object layouts
//#extension GL_EXT_debug_printf : enable

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec4 vertex_color;
layout(location = 3) in vec2 vertex_uv;
layout(location = 4) in vec3 vertex_material;

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
layout(location = 1) out vec3 frag_normal;
layout(location = 2) out vec3 frag_material;

void main()
{
    // mesh coords to world coords
    vec3 position = vec3(mp.mesh_position) + mp.mesh_rotation * (mp.mesh_scale * vertex_position);
    vec3 normal = mp.mesh_rotation * vertex_normal;

    // output in clip coords: normalised device coordinates = (x_clip, y_clip, z_clip) / w_clip
    /*
    float field_of_view = mp.camera_parameters.x;
    */
    float near_plane_width = mp.camera_parameters.x;
    float near_plane_height = mp.camera_parameters.y;
    float near_plane_distance = mp.camera_parameters.z;
    float far_plane_distance = mp.camera_parameters.w;
    if (near_plane_distance > 0.)
    {
        gl_Position = vec4(position.x * near_plane_distance * 2 / near_plane_width,
                           position.y * near_plane_distance * 2 / near_plane_height,
                           (position.z - near_plane_distance) * far_plane_distance / (far_plane_distance - near_plane_distance),
                           position.z);
    }
    else
    {
        gl_Position = vec4(position.x / (near_plane_width / 2.0),
                           position.y / (near_plane_height / 2.0),
                           position.z / far_plane_distance,
                           1.0);
    }

    // return fragment attributes
    frag_color = vertex_color;
    frag_normal = normal;
    frag_material = vertex_material;

    //debugPrintfEXT("fragment coordinates is (x=%f, y=%f, z=%f) screen coordinates are (x=%f, y=%f, z=%f)\n", position.x, position.y, position.z, gl_Position.x, gl_Position.y, gl_Position.z);
}