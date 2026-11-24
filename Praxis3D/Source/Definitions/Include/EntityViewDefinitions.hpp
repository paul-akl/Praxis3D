#pragma once

#include <entt/entt.hpp>

#include "Systems/RendererSystem/Components/Include/CameraComponent.hpp"
#include "Systems/RendererSystem/Components/Include/GraphicsLoadComponents.hpp"
#include "Systems/RendererSystem/Components/Include/LightComponent.hpp"
#include "Systems/RendererSystem/Components/Include/ModelComponent.hpp"
#include "Systems/RendererSystem/Components/Include/ShaderComponent.hpp"
#include "Systems/WorldSystem/Components/Include/SpatialComponent.hpp"

//typedef entt::basic_view<entt::get_t<entt::basic_sigh_mixin<entt::basic_storage<Type, Entity, Allocator, void>, entt::basic_registry<Entity, std::allocator<std::seed_seq::result_type>>>, entt::basic_sigh_mixin<entt::basic_storage<SpatialComponent, Entity, std::allocator<SpatialComponent>, void>, entt::basic_registry<Entity, std::allocator<std::seed_seq::result_type>>>>, entt::exclude_t<entt::basic_sigh_mixin<entt::basic_storage<ShaderComponent, Entity, std::allocator<ShaderComponent>, void>, entt::basic_registry<Entity, std::allocator<std::seed_seq::result_type>>>, entt::basic_sigh_mixin<entt::basic_storage<GraphicsLoadToMemoryComponent, Entity, std::allocator<GraphicsLoadToMemoryComponent>, void>, entt::basic_registry<Entity, std::allocator<std::seed_seq::result_type>>>, entt::basic_sigh_mixin<entt::basic_storage<GraphicsLoadToVideoMemoryComponent, Entity, std::allocator<GraphicsLoadToVideoMemoryComponent>, void>, entt::basic_registry<Entity, std::allocator<std::seed_seq::result_type>>>>, void> ModelSpatialView;
typedef entt::basic_view<unsigned int, entt::get_t<ModelComponent, ShaderComponent, SpatialComponent>, entt::exclude_t<GraphicsLoadToMemoryComponent, GraphicsLoadToVideoMemoryComponent>> ModelShaderSpatialView;
typedef entt::basic_view<unsigned int, entt::get_t<LightComponent, SpatialComponent>, entt::exclude_t<>> LightSpatialView;

typedef entt::basic_view<unsigned int, entt::get_t<GraphicsLoadToVideoMemoryComponent>, entt::exclude_t<GraphicsLoadToMemoryComponent>> LoadToVideoMemoryView;