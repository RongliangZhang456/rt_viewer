#pragma once
#ifndef MATERIAL_H
#define MATERIAL_H
#include "rt_hitable.h"

namespace rt
{
    class material {
    public:
        virtual ~material() = default;

        virtual bool scatter(
            const Ray &r_in, const HitRecord &rec, Color &attenuation, Ray &scattered
        ) const {
            return false;
        }
    };
} // namespace rt

