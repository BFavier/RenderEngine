#version 450
#extension GL_EXT_scalar_block_layout : enable  // for std430 uniform buffer object layouts
//#extension GL_EXT_debug_printf : enable

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec4 vertex_color;

layout(push_constant, std430) uniform MeshParameters
{
    vec4 mesh_position;
	mat3 mesh_rotation;
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
    // mesh coords to world coords
    vec3 position = vec3(mp.mesh_position) + mp.mesh_rotation * (mp.mesh_scale * vertex_position);
    vec3 normal = mp.mesh_rotation * vertex_normal;
    // world coords to camera coords
    position = cp.world_to_camera * (position - vec3(cp.camera_position)) / cp.camera_scale;
    normal = normalize(cp.world_to_camera * normal);
    // camera coords to screen coords
    vec2 xy_coords = vec2(position) * cp.focal_length / (cp.focal_length + abs(position.z)) * 2/cp.camera_aperture_size;
    // outputs
    gl_Position = vec4(xy_coords, 1 - exp(-position.z), 1.0);
    frag_color = vertex_color;

    //debugPrintfEXT("camera position is %f %f %f\n", cp.camera_position.x, cp.camera_position.y, cp.camera_position.z);
    //debugPrintfEXT("camera screen dimension is %f %f, focal_length is %f\n", cp.camera_aperture_size.x, cp.camera_aperture_size.y, cp.focal_length);
    //debugPrintfEXT("camera: scale=%f, aperture size (x=%f, y=%f), focal length=%f\n", cp.camera_scale, cp.camera_aperture_size.x, cp.camera_aperture_size.y, cp.focal_length);
    //debugPrintfEXT("fragment coordinates is (x=%f, y=%f, z=%f) screen coordinates are (x=%f, y=%f, z=%f)\n", position.x, position.y, position.z, gl_Position.x, gl_Position.y, gl_Position.z);
}