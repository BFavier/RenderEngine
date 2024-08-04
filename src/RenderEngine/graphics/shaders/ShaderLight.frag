#version 450
// #extension GL_EXT_debug_printf : enable

// RenderEngine.depth_test = false
// RenderEngine.blending = Blending::ADD

#define NONE             0
#define ORTHOGRAPHIC     1
#define EQUIRECTANGULAR  2
#define PERSPECTIVE      3

#define PI 3.1415926535897932384626433832795


// converts from camera space coordinates to clip space coordinates
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


// converts from clip coordinates to camera space coordinates
vec3 camera_space_coordinates(vec3 clip, vec4 camera_parameters, uint projection_type)
{
    const float aperture_width = camera_parameters.x;
    const float aperture_height = camera_parameters.y;
    const float focal_length = camera_parameters.z;
    const float max_distance = camera_parameters.w;

    if (projection_type == PERSPECTIVE)
    {
        const float Zs = clip.z * (max_distance - focal_length);
        const float z = focal_length * Zs / (max_distance - Zs);
        return vec3(clip.x * 0.5 * aperture_width * (z + focal_length) / focal_length,
                    clip.y * 0.5 * aperture_height * (z + focal_length) / focal_length,
                    z);
    }
    else if (projection_type == ORTHOGRAPHIC)
    {
        return vec3(clip.x * (aperture_width / 2.0),
                    clip.y * (aperture_height / 2.0),
                    clip.z * max_distance);
    }
    else if (projection_type == EQUIRECTANGULAR)
    {
        const float theta = (clip.x + 1) * 0.5 * PI;
        const float phi = clip.y * PI;
        const float r = clip.z;
        return vec3(r * sin(theta) * phi,
                    r * sin(theta) * sin(phi),
                    r * cos(theta));
    }
    else
    {
        return vec3(0., 0., 0.);
    }
}


// returns the fraction of microfacets with normal aligned with halfway vector
float aligned_microfacets(float roughness, vec3 normal, vec3 halfway)
{
    const float a2 = roughness*roughness;
    const float d = dot(normal, halfway);
    const float denom = d*d * (a2 - 1) + 1;
    return a2 / (PI * denom*denom);
}


// returns the fraction of light with given incidence obstructed by microfacets
float microfacets_obstruction(float roughness, vec3 normal, vec3 incidence)
{
    const float k = (roughness + 1)*(roughness + 1)/8;
    const float d = abs(dot(normal, incidence));
    return d / (d*(1 - k) + k);
}


// return the fraction of reflected light that actually reaches the camera aperture
float cook_torrance(float roughness, vec3 normal, vec3 halfway, vec3 view, vec3 light)
{
    const float Distribution = aligned_microfacets(roughness, normal, halfway);
    const float Geometry = microfacets_obstruction(roughness, normal, view) * microfacets_obstruction(roughness, normal, light);
    return Distribution * Geometry;
}


// returns the fraction of light reflected as specular light (for each of the wavelength)
vec4 fresnel_schlick(vec4 albedo, float metalness, vec3 halfway, vec3 view)
{
    const float d = max(dot(halfway, view), 0.);
    const float md = (1 - d);
    const float md2 = md*md;
    const vec3 F0 = mix(vec3(0.04), vec3(albedo), metalness);
    return vec4(F0 + (1 - F0)*md2*md2*md, 1.0);
}


vec4 received_light(vec3 intensity, vec4 albedo, float roughness, float metalness, vec3 normal, vec3 light, vec3 view)
{
    const vec3 halfway = normalize(light + view);
    const vec4 Ks = fresnel_schlick(albedo, metalness, halfway, view);  // specular part
    const vec4 Kd = vec4((1.0 - vec3(Ks)) * (1 - metalness), 1.0); // diffuse part
    return (Kd * albedo/PI * abs(dot(normal, light))
            + Ks * cook_torrance(roughness, normal, halfway, view, light) / (4 * abs(dot(normal, view)) + 1.0E-8)
            ) * vec4(intensity, 1.0);
}


layout(push_constant, std430) uniform LightParameters
{
    vec4 light_position;
    mat3 light_inverse_rotation;
    vec4 light_color_intensity;
    vec4 light_camera_parameters;
    vec4 camera_parameters;
    uint light_projection_type;
    uint camera_projection_type;
    float camera_sensitivity;
} params;


layout(set=0, binding=0) uniform sampler2D albedo;
layout(set=0, binding=1) uniform sampler2D normal;
layout(set=0, binding=2) uniform sampler2D material;
layout(set=0, binding=3) uniform sampler2D depth;

layout(location = 0) in vec2 vertex_uv;

layout(location = 0) out vec4 color;

void main()
{
    const vec4 fragment_albedo = texture(albedo, vertex_uv);
    const vec3 fragment_normal = vec3(texture(normal, vertex_uv));
    const vec4 fragment_material = texture(material, vertex_uv);
    const float fragment_depth = texture(depth, vertex_uv).x;

    const vec3 fragment_clip = vec3(vertex_uv * 2 - 1.0, fragment_depth);
    const vec3 fragment_position = camera_space_coordinates(fragment_clip, params.camera_parameters, params.camera_projection_type);
    const vec3 view = normalize(-fragment_position);

    const float metalness = fragment_material.x;
    const float roughness = fragment_material.y;
    const float ambient_occlusion = fragment_material.z;

    if (params.light_projection_type == NONE)
    {
        color = vec4(vec3(fragment_albedo) * ambient_occlusion * vec3(params.light_color_intensity) * params.light_color_intensity.a / params.camera_sensitivity, fragment_albedo.a);
    }
    else if (params.light_projection_type == ORTHOGRAPHIC)
    {
        const vec3 light = params.light_inverse_rotation * vec3(0., 0., -1.);
        color = received_light(vec3(params.light_color_intensity) * params.light_color_intensity.a,
                               fragment_albedo, roughness, metalness, fragment_normal, light, view
                               ) / params.camera_sensitivity;
    }
    else if (params.light_projection_type == EQUIRECTANGULAR)
    {
        const vec3 light = normalize(vec3(params.light_position) - fragment_position);
        const float d = max(length(fragment_position - vec3(params.light_position)), params.light_camera_parameters.z);
        color = received_light(vec3(params.light_color_intensity) * params.light_color_intensity.a / (d*d),
                               fragment_albedo, roughness, metalness, fragment_normal, light, view
                               ) / params.camera_sensitivity;
    }
    else
    {
        color = vec4(0., 0., 0., 1.0);
    }

    /*
    if (params.light_projection_type == NONE)
    {
        color = fragment_albedo * vec4(vec3(params.light_color_intensity) * params.light_color_intensity.a / params.camera_sensitivity, 1.0);
    }
    else if (params.light_projection_type == ORTHOGRAPHIC)
    {
        float cos_angle = max(dot(vec3(fragment_normal), params.light_inverse_rotation * vec3(0., 0., -1.)), 0.);
        color = vec4(cos_angle, cos_angle, cos_angle, 1.0) * fragment_albedo * vec4(vec3(params.light_color_intensity) * params.light_color_intensity.a / params.camera_sensitivity, 1.0);
    }
    else
    {
        color = vec4(0., 0., 0., 1.0);
    }
    */

    // debugPrintfEXT("fragment normal is (x=%f, y=%f, z=%f)\n", fragment_normal.x, fragment_normal.y, fragment_normal.z);
    // debugPrintfEXT("projection type is %d\n", params.projection_type);
}