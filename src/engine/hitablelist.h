// ======================================================================
// File: world.h
// Revision: 1.0
// Creation: 05/22/2018 - jsberbel
// Notice: Copyright � 2018 by Jordi Serrano Berbel. All Rights Reserved.
// ======================================================================

#pragma once

#include "core/utils.h"

#include "engine/hitable.h"
#include "core/math/aabb.h"

class HitableList : public Hitable
{
public:
    inline HitableList(u32 _capacity);
    constexpr HitableList(const HitableList&) noexcept = delete;
    constexpr HitableList& operator=(const HitableList&) noexcept = delete;
    inline HitableList(HitableList&& _other) noexcept;
    constexpr HitableList& operator=(HitableList&& _other) noexcept;
    virtual inline ~HitableList();

    constexpr Hitable*& add(Hitable* _h);
    constexpr Hitable*& operator[](usize _idx) const;
    constexpr Hitable** get_buffer() const;
    constexpr u32 get_size() const;

    inline b32 hit(const Ray& _ray, f32 _time, f32 _zmin, f32 _zmax, Hit* hit_) const override;
    inline b32 compute_aabb(f32 _time, AABB* aabb_) const override;
    inline b32 compute_aabb(f32 _t0, f32 _t1, AABB* aabb_) const override;

private:
    Hitable** m_hitables = nullptr;
    u32 m_size = 0u;
    u32 m_capacity = 0u;
};

inline HitableList::HitableList(u32 _capacity)
    : m_hitables(new Hitable*[_capacity])
    , m_size(0u)
    , m_capacity(_capacity)
{
}

inline HitableList::HitableList(HitableList&& _other) noexcept
    : m_hitables(std::exchange(_other.m_hitables, nullptr))
    , m_size(_other.m_size)
    , m_capacity(_other.m_capacity)
{
}

inline constexpr HitableList& HitableList::operator=(HitableList&& _other) noexcept
{
    if (this != std::addressof(_other))
    {
        m_hitables = std::exchange(_other.m_hitables, nullptr);
        m_size = _other.m_size;
        m_capacity = _other.m_capacity;
    }
    return *this;
}

inline HitableList::~HitableList()
{
    for (usize idx = 0; idx < m_size; ++idx)
        util::safe_del(m_hitables[idx]);
}

constexpr Hitable*& HitableList::add(Hitable* _h)
{
    sws_assert(m_size < m_capacity);
    return (m_hitables[m_size++] = _h);
}

inline constexpr Hitable*& HitableList::operator[](usize _idx) const
{
    sws_assert(_idx < m_size);
    return m_hitables[_idx];
}

inline constexpr Hitable** HitableList::get_buffer() const
{
    return m_hitables;
}

inline constexpr u32 HitableList::get_size() const
{
    return m_size;
}

inline b32 HitableList::hit(const Ray& _ray, f32 _time, f32 _zmin, f32 _zmax, Hit* hit_) const
{
    sws_assert(hit_);

    Hit tmp_hit;
    b32 has_hit_anything = false;
    f32 closest_dist = _zmax;

    for (usize idx = 0; idx < m_size; ++idx)
    {
        if (m_hitables[idx]->hit(_ray, _time, _zmin, closest_dist, &tmp_hit))
        {
            has_hit_anything = true;
            closest_dist = tmp_hit.distance;
            *hit_ = std::move(tmp_hit);
        }
    }

    return has_hit_anything;
}

inline b32 HitableList::compute_aabb(f32 _time, AABB* aabb_) const
{
    return false;
};

inline b32 HitableList::compute_aabb(f32 _t0, f32 _t1, AABB* aabb_) const
{
    if (m_size == 0)
        return false;
    
    AABB tmp_aabb;
    if (!m_hitables[0]->compute_aabb(_t0, _t1, &tmp_aabb))
        return false;

    *aabb_ = tmp_aabb;
    for (usize idx = 1u; idx < m_size; ++idx)
    {
        if (!m_hitables[idx]->compute_aabb(_t0, _t1, &tmp_aabb))
            return false;

        *aabb_ = AABB::get_surrounding_box(*aabb_, tmp_aabb);
    }
    return true;
}