#pragma once
#include <RenderEngine/scene/Camera.hpp>
#include <RenderEngine/graphics/Color.hpp>

namespace RenderEngine
{
    class Light : public Camera
    {
        public:
            Light() = delete;
            ~Light();
        protected:
            Light(Color color, float intensity,
                  float aperture_width, float focal_length, float max_distance, ProjectionType projection_type,
                  Vector position, Quaternion orientation, double scale, Referential* parent);
        public:
            Color color;
            float intensity;
    };


    class AmbientLight : public Light
    {
        public:
            AmbientLight() = delete;
            AmbientLight(Color color, float intensity, Referential* parent = nullptr);
            ~AmbientLight();
    };


    class DirectionalLight : public Light
    {
        public:
            DirectionalLight() = delete;
            DirectionalLight(Color color, float intensity, float aperture_width = 100.0, float max_distance = 1000.0,
                             Vector position = {0., 0., 0.}, Quaternion orientation = {}, double scale=1.0, Referential* parent = nullptr);
            ~DirectionalLight();
    };


    class PointLight : public Light
    {
        public:
            PointLight() = delete;
            PointLight(Color color, float intensity, float constant_intensity_radius = 1.0, float max_shadow_distance = 1000.0,
                       Vector position = {0., 0., 0.}, Quaternion orientation = {}, double scale=1.0, Referential* parent = nullptr);
            ~PointLight();
    };
}
