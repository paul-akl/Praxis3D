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

typedef entt::basic_view<unsigned int, entt::get_t<ModelComponent, SpatialComponent>, entt::exclude_t<ShaderComponent, GraphicsLoadToMemoryComponent, GraphicsLoadToVideoMemoryComponent>> ModelSpatialView;
typedef entt::basic_view<unsigned int, entt::get_t<ModelComponent, ShaderComponent, SpatialComponent>, entt::exclude_t<GraphicsLoadToMemoryComponent, GraphicsLoadToVideoMemoryComponent>> ModelShaderSpatialView;
typedef entt::basic_view<unsigned int, entt::get_t<LightComponent, SpatialComponent>, entt::exclude_t<>> LightSpatialView;

//typedef entt::basic_view<unsigned int, entt::get_t<ModelComponent>, entt::exclude_t<ShaderComponent, GraphicsLoadToMemoryComponent, GraphicsLoadToVideoMemoryComponent>> ModelLoadToVideoMemoryView;
//typedef entt::basic_view<unsigned int, entt::get_t<ModelComponent, ShaderComponent>, entt::exclude_t<GraphicsLoadToMemoryComponent, GraphicsLoadToVideoMemoryComponent>> ModelShaderLoadToVideoMemoryView;

typedef entt::basic_view<unsigned int, entt::get_t<GraphicsLoadToVideoMemoryComponent>, entt::exclude_t<GraphicsLoadToMemoryComponent>> LoadToVideoMemoryView;