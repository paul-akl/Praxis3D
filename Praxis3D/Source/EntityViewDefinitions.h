#pragma once

#include <entt/entt.hpp>

#include "ModelComponent.h"
#include "ShaderComponent.h"
#include "SpatialComponent.h"

//class ModelComponent;
//class SpatialComponent;
//class ShaderComponent;

typedef entt::basic_view<unsigned int, entt::get_t<ModelComponent, SpatialComponent>, entt::exclude_t<ShaderComponent>> ModelSpatialView;
typedef entt::basic_view<unsigned int, entt::get_t<ModelComponent, ShaderComponent, SpatialComponent>, entt::exclude_t<>> ModelShaderSpatialView;