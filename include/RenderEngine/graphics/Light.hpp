#pragma once
#include <RenderEngine/graphics/Camera.hpp>
#include <RenderEngine/graphics/Color.hpp>

namespace RenderEngine
{
    class Light
    {
        public:
            Light() = delete;
            ~Light();
        protected:
            Light(Color _color, double _luminance);
        public:
            Color color;
            double luminance = 0;
    };

    class PointLight : public Light, public PerspectiveCamera
    {
    public:
        PointLight() = delete;
        PointLight(Color color, double luminance,
                   float horizontal_field_of_view, float near_plane_distance, float far_plane_distance,
                   Vector position = {0., 0., 0.}, Quaternion orientation = {}, double scale=1.0, Referential* parent = nullptr);
        ~PointLight();
    };

    class DirectionalLight : public Light, public OrthographicCamera
    {
    public:
        DirectionalLight() = delete;
        DirectionalLight(Color color, double luminance,
                         float horizontal_length, float far_plane_distance,
                         Vector position = {0., 0., 0.}, Quaternion orientation = {}, double scale=1.0, Referential* parent = nullptr);
        ~DirectionalLight();
    };
}
