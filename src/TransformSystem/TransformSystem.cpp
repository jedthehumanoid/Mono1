
#include "TransformSystem.h"
#include "System/Hash.h"
#include <limits>
#include <cstdint>

using namespace mono;

namespace
{
    constexpr uint32_t no_parent = std::numeric_limits<uint32_t>::max();
}

TransformSystem::TransformSystem(size_t n_components)
{
    m_transforms.resize(n_components);

    for(uint32_t index = 0; index < m_transforms.size(); ++index)
        ResetTransformComponent(index);
}

math::Matrix TransformSystem::GetWorld(uint32_t id) const
{
    math::Matrix transform;

    while(id != no_parent)
    {
        const Component& transform_component = m_transforms[id];
        transform = transform_component.transform * transform;
        id = transform_component.parent;
    }

    return transform;
}

math::Vector TransformSystem::GetWorldPosition(uint32_t id) const
{
    return math::GetPosition(GetWorld(id));
}

const math::Matrix& TransformSystem::GetTransform(uint32_t id) const
{
    return m_transforms[id].transform;
}

math::Matrix& TransformSystem::GetTransform(uint32_t id)
{
    return m_transforms[id].transform;
}

void TransformSystem::SetTransform(uint32_t id, const math::Matrix& new_transform)
{
    m_transforms[id].transform = new_transform;
}

math::Quad TransformSystem::GetWorldBoundingBox(uint32_t id) const
{
    return math::Transform(GetWorld(id), GetBoundingBox(id));
}

const math::Quad& TransformSystem::GetBoundingBox(uint32_t id) const
{
    return m_transforms[id].bounding_box;
}

math::Quad& TransformSystem::GetBoundingBox(uint32_t id)
{
    return m_transforms[id].bounding_box;
}

uint32_t TransformSystem::GetParent(uint32_t id) const
{
    return m_transforms[id].parent;
}

void TransformSystem::ChildTransform(uint32_t id, uint32_t parent_id)
{
    m_transforms[id].parent = parent_id;
}

void TransformSystem::UnchildTransform(uint32_t id)
{
    m_transforms[id].parent = no_parent;
}

TransformState TransformSystem::GetTransformState(uint32_t id) const
{
    return m_transforms[id].state;
}

void TransformSystem::SetTransformState(uint32_t id, TransformState new_state)
{
    m_transforms[id].state = new_state;
}

void TransformSystem::ResetTransformComponent(uint32_t id)
{
    Component& component = m_transforms[id];

    math::Identity(component.transform);
    component.bounding_box = math::Quad(-0.5f, -0.5f, 0.5f, 0.5f);
    component.parent = no_parent;
    component.state = TransformState::NONE;
}

uint32_t TransformSystem::Id() const
{
    return hash::Hash(Name());
}

const char* TransformSystem::Name() const
{
    return "transformsystem";
}

uint32_t TransformSystem::Capacity() const
{
    return m_transforms.size();
}

void TransformSystem::Update(const UpdateContext& update_context)
{

}
