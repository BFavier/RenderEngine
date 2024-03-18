#include <RenderEngine/graphics/Camera.hpp>
#include <cmath>  // for tan
using namespace RenderEngine;

Camera::Camera(const std::shared_ptr<GPU>& gpu, float _width, float _height, float _field_of_view, Referential* _parent, Vector _position, Quaternion _orientation) :
    Referential(_parent, _position, _orientation), width(_width), height(_height), field_of_view(_field_of_view), _parameters(gpu, sizeof(CameraParameters), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
{
}

Camera::~Camera()
{
}

float Camera::focal_length() const
{
    if (field_of_view > 0.0 && field_of_view < 180.0)
    {
        return std::max(width, height) / 2.0 / tan(field_of_view/2.0 * PI/180.);
    }
    else
    {
        return 0.f;
    }
}

VkDescriptorBufferInfo Camera::get_projection() const
{
    std::pair<Vector, Quaternion> coordinates = absolute_coordinates();
    CameraParameters params{};
    params.camera_position = {static_cast<float>(coordinates.first.x), static_cast<float>(coordinates.first.y), static_cast<float>(coordinates.first.z), 0.f};
    params.world_to_camera = Matrix(coordinates.second.inverse()).to_mat3();
    params.focal_length = focal_length();
    params.camera_screen_dim = {width, height};
    _parameters.upload(&params);
    return VkDescriptorBufferInfo({*_parameters._vk_buffer, 0, VK_WHOLE_SIZE});
}