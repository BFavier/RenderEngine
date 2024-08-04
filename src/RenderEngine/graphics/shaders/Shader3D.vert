#version 450
#extension GL_EXT_scalar_block_layout : enable  // for std430 uniform buffer object layouts
//#extension GL_EXT_debug_printf : enable

#define NONE             0
#define ORTHOGRAPHIC     1
#define EQUIRECTANGULAR  2
#define PERSPECTIVE      3

#define PI 3.1415926535897932384626433832795

layout(push_constant, std430) uniform DrawParameters
{
    vec4 mesh_position;
	mat3 mesh_inverse_rotation;
    vec4 camera_parameters;
    uint projection_type;
    float mesh_scale;
} params;

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec4 vertex_color;
layout(location = 3) in vec2 vertex_uv;
layout(location = 4) in vec3 vertex_material;

layout(location = 0) out vec4 frag_color;
layout(location = 1) out vec3 frag_normal;
layout(location = 2) out vec2 frag_uv;
layout(location = 3) out vec3 frag_material;

vec4 clip_space_coordinates(vec3 position, vec4 camera_parameters, uint projection_type)
{
    const float aperture_width = camera_parameters.x;
    const float aperture_height = camera_parameters.y;
    const float focal_length = camera_parameters.z;
    const float max_distance = camera_parameters.w;

    if (projection_type == PERSPECTIVE)
    {
        return vec4(position.x * focal_length / (0.5 * aperture_width),
                    position.y * focal_length / (0.5 * aperture_height),
                    position.z * max_distance / (max_distance - focal_length),
                    position.z + focal_length);
    }
    else if (projection_type == ORTHOGRAPHIC)
    {
        return vec4(position.x / (aperture_width / 2.0),
                    position.y / (aperture_height / 2.0),
                    position.z / max_distance,
                    1.0);
    }
    else if (projection_type == EQUIRECTANGULAR)
    {
        const float r = sqrt(position.x*position.x + position.y*position.y + position.z*position.z);
        const float theta = acos(position.y / (r + 1.0E-10));
        const float phi = sign(position.x) * acos(position.z / sqrt(position.z*position.z + position.x*position.x + 1.0E-10));
        return vec4(2*theta/PI - 1, phi/PI, r, 1.0);
    }
    else
    {
        return vec4(0., 0., 0., 0.);
    }
}

void main()
{
    // mesh coords to world coords
    vec3 position = vec3(params.mesh_position) + params.mesh_inverse_rotation * (params.mesh_scale * vertex_position);
    vec3 normal = params.mesh_inverse_rotation * vertex_normal;

    // output in clip coords: normalised device coordinates = (x_clip, y_clip, z_clip) / w_clip
    gl_Position = clip_space_coordinates(position, params.camera_parameters, params.projection_type);

    // return fragment attributes
    frag_color = vertex_color;
    frag_normal = normal;
    frag_material = vertex_material;

    //debugPrintfEXT("fragment normal is (x=%f, y=%f, z=%f)\n", frag_normal.x, frag_normal.y, frag_normal.z);
}