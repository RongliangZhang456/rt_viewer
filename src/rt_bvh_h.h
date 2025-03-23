#ifndef BVH_H
#define BVH_H

#include "rt_aabb.h"
#include "rt_hitable.h"
#include "rt_hitable_list.h"
#include "rt_raytracing.h"
#include <algorithm>

namespace rt{
    class Bvh_node : public Hitable {
    public:
        Bvh_node(Hittable_list list) : Bvh_node(list.objects, 0, list.objects.size()) {
            // There's a C++ subtlety here. This constructor (without span indices) creates an
            // implicit copy of the hittable list, which we will modify. The lifetime of the copied
            // list only extends until this constructor exits. That's OK, because we only need to
            // persist the resulting bounding volume hierarchy.
        }

        Bvh_node(std::vector<shared_ptr<Hitable>>& objects, size_t start, size_t end) {
            // To be implemented later.
            int axis = random_int(0, 2);

            auto comparator = (axis == 0) ? box_x_compare
                : (axis == 1) ? box_y_compare
                : box_z_compare;

            size_t object_span = end - start;

            if (object_span == 1) {
                left = right = objects[start];
            }
            else if (object_span == 2) {
                left = objects[start];
                right = objects[start + 1];
            }
            else {
                std::sort(std::begin(objects) + start, std::begin(objects) + end, comparator);

                auto mid = start + object_span / 2;
                left = make_shared<Bvh_node>(objects, start, mid);
                right = make_shared<Bvh_node>(objects, mid, end);
            }

            bbox = AABB(left->bounding_box(), right->bounding_box());
        }

        bool hit(const Ray &r, float min, float max, HitRecord & rec) const override {
			Interval ray_t(min, max);
            if (!bbox.hit(r, ray_t))
                return false;

            bool hit_left = left->hit(r, min, max, rec);
            bool hit_right = right->hit(r, ray_t.min, hit_left ? rec.t : ray_t.max, rec);

            return hit_left || hit_right;
        }

        AABB bounding_box() const override { return bbox; }

    private:
        shared_ptr<Hitable> left;
        shared_ptr<Hitable> right;
        AABB bbox;

        static bool box_compare(
            const shared_ptr<Hitable> a, const shared_ptr<Hitable> b, int axis_index
        ) {
            auto a_axis_interval = a->bounding_box().axis_interval(axis_index);
            auto b_axis_interval = b->bounding_box().axis_interval(axis_index);
            return a_axis_interval.min < b_axis_interval.min;
        }

        static bool box_x_compare(const shared_ptr<Hitable> a, const shared_ptr<Hitable> b) {
            return box_compare(a, b, 0);
        }

        static bool box_y_compare(const shared_ptr<Hitable> a, const shared_ptr<Hitable> b) {
            return box_compare(a, b, 1);
        }

        static bool box_z_compare(const shared_ptr<Hitable> a, const shared_ptr<Hitable> b) {
            return box_compare(a, b, 2);
        }
    };
}


#endif