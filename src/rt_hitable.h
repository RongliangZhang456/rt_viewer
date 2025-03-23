#pragma once

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/component_wise.hpp>
#include <memory>
using std::shared_ptr;
namespace rt {

class material;
struct HitRecord {
    float t;
    glm::vec3 p;
    glm::vec3 normal;
    shared_ptr<material> mat;

    bool front_face;

    void set_face_normal(const Ray& r, const glm::vec3& outward_normal) {
        front_face = glm::dot(r.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }

};

class Hitable {
public:
    virtual bool hit(const Ray &r, float t_min, float t_max, HitRecord &rec) const = 0;
};

} // namespace rt
