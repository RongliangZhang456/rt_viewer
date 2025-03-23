#pragma once
#include "rt_hitable.h"
#include "rt_raytracing.h"

namespace rt
{
    class Material {
    public:
        virtual ~Material() = default;

        virtual bool scatter(
            const Ray& r_in, const HitRecord& rec, glm::vec3& attenuation, Ray& scattered
        ) const {
            return false;
        }


    };
    class Metal : public Material {

    public:
        Metal(const glm::vec3& albedo) : albedo(albedo) {}

        bool scatter(const Ray& r_in, const HitRecord& rec, glm::vec3& attenuation, Ray& scattered)
            const override {
            glm::vec3 reflected = reflect(r_in.direction(), rec.normal);
            scattered = Ray(rec.p, reflected);
            attenuation = albedo;
            return true;
        }

    private:
        glm::vec3 albedo;
    };

    class Lambertian : public Material {
    public:
        Lambertian(const glm::vec3& albedo) : albedo(albedo) {}

        bool scatter(const Ray& r_in, const HitRecord &rec, glm::vec3 & attenuation, Ray& scattered)
            const override {
            glm::vec3 scatter_direction = rec.normal + random_unit_vector();
            scattered = Ray(rec.p, scatter_direction);
            attenuation = albedo;
            return true;
        }

    private:
        glm::vec3 albedo;
    };
} // namespace rt

