#ifndef HITTABLE_LIST_H
#define HITTABLE_LIST_H

#include "rt_hitable.h"

#include <memory>
#include <vector>

using std::make_shared;
using std::shared_ptr;

namespace rt {

    class Hittable_list : public Hitable {
    public:
        std::vector<shared_ptr<Hitable>> objects;

        Hittable_list() {}
        Hittable_list(shared_ptr<Hitable> object) { add(object); }

        void clear() { objects.clear(); }

        void add(shared_ptr<Hitable> object) {
            objects.push_back(object);
            bbox = AABB(bbox, object->bounding_box());
        }

        bool hit(const Ray &r, double ray_tmin, double ray_tmax, HitRecord &rec) const override {
            HitRecord temp_rec;
            bool hit_anything = false;
            auto closest_so_far = ray_tmax;

            for (const auto& object : objects) {
                if (object->hit(r, ray_tmin, closest_so_far, temp_rec)) {
                    hit_anything = true;
                    closest_so_far = temp_rec.t;
                    rec = temp_rec;
                }
            }

            return hit_anything;
        }

        AABB bounding_box() const override { return bbox; }

    private:
        AABB bbox;
    };

}
#endif