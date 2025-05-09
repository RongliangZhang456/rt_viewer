#include "rt_raytracing.h"
#include "rt_ray.h"
#include "rt_hitable.h"
#include "rt_sphere.h"
#include "rt_triangle.h"
#include "rt_box.h"
#include "rt_material.h"
#include "rt_hitable_list.h"
#include "rt_bvh_h.h"
#include <memory>

#include <random>
#include "cg_utils2.h"  // Used for OBJ-mesh loading
#include <stdlib.h>     // Needed for drand48()

namespace rt {

    // Store scene (world) in a global variable for convenience
    struct Scene {
        Sphere ground;
        std::vector<Sphere> spheres;
        std::vector<Box> boxes;
        std::vector<Triangle> mesh;
        Hittable_list world;
    } g_scene;

    inline double random_double() {
        static std::uniform_real_distribution<double> distribution(0.0, 1.0);
        static std::mt19937 generator;
        return distribution(generator);
    }

    inline double random_double(double min, double max) {
        // Returns a random real in [min,max).
        return min + (max - min) * random_double();
    }

    inline int random_int(int min, int max) {
        // Returns a random integer in [min,max].
        return int(random_double(min, max + 1));
    }

    inline glm::vec3 random_unit_vector() {
        while (true) {
            auto p = glm::vec3(random_double(-1, 1), random_double(-1, 1), random_double(-1, 1));
            auto lensq = glm::dot(p, p);
            if (1e-160 < lensq && lensq <= 1)
                return p / sqrt(lensq);
        }
    }

    bool hit_world(const Ray& r, float t_min, float t_max, HitRecord& rec)
    {
        HitRecord temp_rec;
        bool hit_anything = false;
        float closest_so_far = t_max;

        
        hit_anything = g_scene.world.hit(r, t_min, closest_so_far, temp_rec);
        rec = temp_rec;
        // if (g_scene.mesh[i].hit(r, t_min, closest_so_far, temp_rec)) {
        //     hit_anything = true;
        //     closest_so_far = temp_rec.t;
        //     rec = temp_rec;
        // }

        return hit_anything;
    }

    inline glm::vec3 reflect(const glm::vec3 &v, const glm::vec3 &n) {
        return v - 2 * glm::dot(v, n) * n;
    }

    inline glm::vec3 random_on_hemisphere(const glm::vec3& normal) {
        glm::vec3 on_unit_sphere = random_unit_vector();
        if (glm::dot(on_unit_sphere, normal) > 0.0) // In the same hemisphere as the normal
            return on_unit_sphere;
        else
            return -on_unit_sphere;
    }
    // This function should be called recursively (inside the function) for
    // bouncing rays when you compute the lighting for materials, like this
    //
    // if (hit_world(...)) {
    //     ...
    //     return color(rtx, r_bounce, max_bounces - 1);
    // }
    //
    // See Chapter 7 in the "Ray Tracing in a Weekend" book
    glm::vec3 color(RTContext& rtx, const Ray& r, int max_bounces)
    {
        if (max_bounces < 0) return glm::vec3(0.0f);

        HitRecord rec;
        if (hit_world(r, 0.001f, 9999.0f, rec)) {
            rec.normal = glm::normalize(rec.normal);  // Always normalise before use!
            if (rtx.show_normals) { return rec.normal * 0.5f + 0.5f; }

            // Implement lighting for materials here
            // ...
            glm::vec3 direction;
            if (rtx.enable_Material) {
                Ray scattered;
                glm::vec3 attenuation;
                if (rec.mat->scatter(r, rec, attenuation, scattered))
                    return attenuation * color(rtx, scattered, max_bounces - 1);
                return glm::vec3(0, 0, 0);
            }
            else 
            {
                if (rtx.enable_Lambertian_Reflection) {
                    direction = rec.normal + random_unit_vector();
                }
                else {
                    direction = random_on_hemisphere(rec.normal);
                }
                Ray scattered(rec.p, direction);
                return 0.5f * color(rtx, scattered, max_bounces - 1);
            }
            
        }

        // If no hit, return sky color
        glm::vec3 unit_direction = glm::normalize(r.direction());
        float t = 0.5f * (unit_direction.y + 1.0f);
        return (1.0f - t) * rtx.ground_color + t * rtx.sky_color;
    }


    // MODIFY THIS FUNCTION!
    // Called when initializing the program.

    void setupScene(RTContext& rtx, const char* filename)
    {
        auto metal = std::make_shared<Metal>(glm::vec3(0.7f, 0.6f, 0.5f));
        auto lambertian = std::make_shared<Lambertian>(glm::vec3(0.3f, 0.6f, 0.5f));
        g_scene.ground = Sphere(glm::vec3(0.0f, -1000.5f, 0.0f), 1000.0f, lambertian);
        g_scene.spheres = {
            Sphere(glm::vec3(0.0f, 0.0f, 0.0f), 0.5f, metal),
            Sphere(glm::vec3(1.0f, 0.0f, 0.0f), 0.5f, lambertian),
            Sphere(glm::vec3(-1.0f, 0.0f, 0.0f), 0.5f, metal),
        };
        //g_scene.boxes = {
        //    Box(glm::vec3(0.0f, -0.25f, 0.0f), glm::vec3(0.25f)),
        //    Box(glm::vec3(1.0f, -0.25f, 0.0f), glm::vec3(0.25f)),
        //    Box(glm::vec3(-1.0f, -0.25f, 0.0f), glm::vec3(0.25f)),
        //};

        cg::OBJMesh mesh;
        cg::objMeshLoad(mesh, filename);
        g_scene.mesh.clear();
        for (int i = 0; i < mesh.indices.size(); i += 3) {
            int i0 = mesh.indices[i + 0];
            int i1 = mesh.indices[i + 1];
            int i2 = mesh.indices[i + 2];
            glm::vec3 v0 = mesh.vertices[i0] + glm::vec3(0.0f, 0.135f, 0.0f);
            glm::vec3 v1 = mesh.vertices[i1] + glm::vec3(0.0f, 0.135f, 0.0f);
            glm::vec3 v2 = mesh.vertices[i2] + glm::vec3(0.0f, 0.135f, 0.0f);
            g_scene.mesh.push_back(Triangle(v0, v1, v2, metal));
        }

        g_scene.world.add(std::make_shared<Sphere>(g_scene.ground));
        for (int i = 0; i < g_scene.spheres.size(); ++i) {
            g_scene.world.add(std::make_shared<Sphere>(g_scene.spheres[i]));
        }
        for (int i = 0; i < g_scene.boxes.size(); ++i) {
            g_scene.world.add(std::make_shared<Box>(g_scene.boxes[i]));
        }
        for (int i = 0; i < g_scene.mesh.size(); ++i) {
            g_scene.world.add(std::make_shared<Triangle>(g_scene.mesh[i]));
        }
        g_scene.world = Hittable_list(std::make_shared<Bvh_node>(g_scene.world));
    }

    // MODIFY THIS FUNCTION!
    void updateLine(RTContext& rtx, int y)
    {
        int nx = rtx.width;
        int ny = rtx.height;
        float aspect = float(nx) / float(ny);
        glm::vec3 lower_left_corner(-1.0f * aspect, -1.0f, -1.0f);
        glm::vec3 horizontal(2.0f * aspect, 0.0f, 0.0f);
        glm::vec3 vertical(0.0f, 2.0f, 0.0f);
        glm::vec3 origin(0.0f, 0.0f, 0.0f);
        glm::mat4 world_from_view = glm::inverse(rtx.view);
        int samples_per_pixel = 2;

        // You can try parallelising this loop by uncommenting this line:
        //#pragma omp parallel for schedule(dynamic)
        for (int x = 0; x < nx; ++x) {
            glm::vec3 c;

            if (rtx.enable_antialiasing) {
                glm::vec3 color_sum(0.0f);
                for (int s = 0; s < samples_per_pixel; ++s) {
                    float u = (float(x) + random_double()) / float(nx);
                    float v = (float(y) + random_double()) / float(ny);
                    Ray r(origin, lower_left_corner + u * horizontal + v * vertical);
                    r.A = glm::vec3(world_from_view * glm::vec4(r.A, 1.0f));
                    r.B = glm::vec3(world_from_view * glm::vec4(r.B, 0.0f));
                    color_sum += color(rtx, r, rtx.max_bounces);
                }
                c = color_sum / float(samples_per_pixel);
            }
            else {
                float u = (float(x) + 0.5f) / float(nx);
                float v = (float(y) + 0.5f) / float(ny);
                Ray r(origin, lower_left_corner + u * horizontal + v * vertical);
                r.A = glm::vec3(world_from_view * glm::vec4(r.A, 1.0f));
                r.B = glm::vec3(world_from_view * glm::vec4(r.B, 0.0f));
                c = color(rtx, r, rtx.max_bounces);
            }

            // Note: in the RTOW book, they have an inner loop for the number of
            // samples per pixel. Here, you do not need this loop, because we want
            // some interactivity and accumulate samples over multiple frames
            // instead (until the camera moves or the rendering is reset).

            if (rtx.current_frame <= 0) {
                // Here we make the first frame blend with the old image,
                // to smoothen the transition when resetting the accumulation
                glm::vec4 old = rtx.image[y * nx + x];
                rtx.image[y * nx + x] = glm::clamp(old / glm::max(1.0f, old.a), 0.0f, 1.0f);
            }
            rtx.image[y * nx + x] += glm::vec4(c, 1.0f);
        }
    }

    void updateImage(RTContext& rtx)
    {
        if (rtx.freeze) return;                    // Skip update
        rtx.image.resize(rtx.width * rtx.height);  // Just in case...

        updateLine(rtx, rtx.current_line % rtx.height);

        if (rtx.current_frame < rtx.max_frames) {
            rtx.current_line += 1;
            if (rtx.current_line >= rtx.height) {
                rtx.current_frame += 1;
                rtx.current_line = rtx.current_line % rtx.height;
            }
        }
    }

    void resetImage(RTContext& rtx)
    {
        rtx.image.clear();
        rtx.image.resize(rtx.width * rtx.height);
        rtx.current_frame = 0;
        rtx.current_line = 0;
        rtx.freeze = false;
    }

    void resetAccumulation(RTContext& rtx)
    {
        rtx.current_frame = -1;
    }

}  // namespace rt
