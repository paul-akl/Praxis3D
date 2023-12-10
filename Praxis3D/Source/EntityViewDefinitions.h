#pragma once

#include <entt/entt.hpp>

#include "CameraComponent.h"
#include "GraphicsLoadComponents.h"
#include "LightComponent.h"
#include "ModelComponent.h"
#include "ShaderComponent.h"
#include "SpatialComponent.h"

//class ModelComponent;
//class SpatialComponent;
//class ShaderComponent;

//typedef entt::basic_view<unsigned int, entt::get_t<ModelComponent, SpatialComponent>, entt::exclude_t<ShaderComponent, GraphicsLoadToMemoryComponent, GraphicsLoadToVideoMemoryComponent>> ModelSpatialView;
//typedef entt::basic_view<entt::get_t<entt::basic_sigh_mixin<entt::basic_storage<Type, Entity, Allocator, void>, entt::basic_registry<Entity, std::allocator<std::seed_seq::result_type>>>, entt::basic_sigh_mixin<entt::basic_storage<SpatialComponent, Entity, std::allocator<SpatialComponent>, void>, entt::basic_registry<Entity, std::allocator<std::seed_seq::result_type>>>>, entt::exclude_t<entt::basic_sigh_mixin<entt::basic_storage<ShaderComponent, Entity, std::allocator<ShaderComponent>, void>, entt::basic_registry<Entity, std::allocator<std::seed_seq::result_type>>>, entt::basic_sigh_mixin<entt::basic_storage<GraphicsLoadToMemoryComponent, Entity, std::allocator<GraphicsLoadToMemoryComponent>, void>, entt::basic_registry<Entity, std::allocator<std::seed_seq::result_type>>>, entt::basic_sigh_mixin<entt::basic_storage<GraphicsLoadToVideoMemoryComponent, Entity, std::allocator<GraphicsLoadToVideoMemoryComponent>, void>, entt::basic_registry<Entity, std::allocator<std::seed_seq::result_type>>>>, void> ModelSpatialView;
typedef entt::basic_view<unsigned int, entt::get_t<ModelComponent, ShaderComponent, SpatialComponent>, entt::exclude_t<GraphicsLoadToMemoryComponent, GraphicsLoadToVideoMemoryComponent>> ModelShaderSpatialView;
typedef entt::basic_view<unsigned int, entt::get_t<LightComponent, SpatialComponent>, entt::exclude_t<>> LightSpatialView;

//typedef entt::basic_view<unsigned int, entt::get_t<ModelComponent>, entt::exclude_t<ShaderComponent, GraphicsLoadToMemoryComponent, GraphicsLoadToVideoMemoryComponent>> ModelLoadToVideoMemoryView;
//typedef entt::basic_view<unsigned int, entt::get_t<ModelComponent, ShaderComponent>, entt::exclude_t<GraphicsLoadToMemoryComponent, GraphicsLoadToVideoMemoryComponent>> ModelShaderLoadToVideoMemoryView;

typedef entt::basic_view<unsigned int, entt::get_t<GraphicsLoadToVideoMemoryComponent>, entt::exclude_t<GraphicsLoadToMemoryComponent>> LoadToVideoMemoryView;