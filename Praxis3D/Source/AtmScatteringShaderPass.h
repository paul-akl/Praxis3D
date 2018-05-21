#pragma once

const char* demo_glsl = \
"uniform vec3 cameraPosVec;\r\n"\
"uniform float exposure;\r\n"\
"uniform vec3 white_point;\r\n"\
"uniform vec3 earth_center;\r\n"\
"uniform vec3 sun_direction;\r\n"\
"uniform vec2 sun_size;\r\n"\
"in vec3 view_ray;\r\n"\
"out vec4 color;\r\n"\
"const float PI = 3.14159265;\r\n"\
"const vec3 kSphereCenter = vec3(0.0, 0.0, 1000.0) / kLengthUnitInMeters;\r\n"\
"const float kSphereRadius = 1000.0 / kLengthUnitInMeters;\r\n"\
"const vec3 kSphereAlbedo = vec3(0.8);\r\n"\
"const vec3 kGroundAlbedo = vec3(0.0, 0.0, 0.04);\r\n"\
"#ifdef USE_LUMINANCE\r\n"\
"#define GetSolarRadiance GetSolarLuminance\r\n"\
"#define GetSkyRadiance GetSkyLuminance\r\n"\
"#define GetSkyRadianceToPoint GetSkyLuminanceToPoint\r\n"\
"#define GetSunAndSkyIrradiance GetSunAndSkyIlluminance\r\n"\
"#endif\r\n"\
"vec3 GetSolarRadiance();\r\n"\
"vec3 GetSkyRadiance(vec3 camera, vec3 view_ray, float shadow_length,\r\n"\
"    vec3 sun_direction, out vec3 transmittance);\r\n"\
"vec3 GetSkyRadianceToPoint(vec3 camera, vec3 point, float shadow_length,\r\n"\
"    vec3 sun_direction, out vec3 transmittance);\r\n"\
"vec3 GetSunAndSkyIrradiance(\r\n"\
"    vec3 p, vec3 normal, vec3 sun_direction, out vec3 sky_irradiance);\r\n"\
"float GetSunVisibility(vec3 point, vec3 sun_direction) {\r\n"\
"  vec3 p = point - kSphereCenter;\r\n"\
"  float p_dot_v = dot(p, sun_direction);\r\n"\
"  float p_dot_p = dot(p, p);\r\n"\
"  float ray_sphere_center_squared_distance = p_dot_p - p_dot_v * p_dot_v;\r\n"\
"  float distance_to_intersection = -p_dot_v - sqrt(\r\n"\
"      kSphereRadius * kSphereRadius - ray_sphere_center_squared_distance);\r\n"\
"  if (distance_to_intersection > 0.0) {\r\n"\
"    float ray_sphere_distance =\r\n"\
"        kSphereRadius - sqrt(ray_sphere_center_squared_distance);\r\n"\
"    float ray_sphere_angular_distance = -ray_sphere_distance / p_dot_v;\r\n"\
"    return smoothstep(1.0, 0.0, ray_sphere_angular_distance / sun_size.x);\r\n"\
"  }\r\n"\
"  return 1.0;\r\n"\
"}\r\n"\
"float GetSkyVisibility(vec3 point) {\r\n"\
"  vec3 p = point - kSphereCenter;\r\n"\
"  float p_dot_p = dot(p, p);\r\n"\
"  return\r\n"\
"      1.0 + p.z / sqrt(p_dot_p) * kSphereRadius * kSphereRadius / p_dot_p;\r\n"\
"}\r\n"\
"void GetSphereShadowInOut(vec3 view_direction, vec3 sun_direction,\r\n"\
"    out float d_in, out float d_out) {\r\n"\
"  vec3 pos = cameraPosVec - kSphereCenter;\r\n"\
"  float pos_dot_sun = dot(pos, sun_direction);\r\n"\
"  float view_dot_sun = dot(view_direction, sun_direction);\r\n"\
"  float k = sun_size.x;\r\n"\
"  float l = 1.0 + k * k;\r\n"\
"  float a = 1.0 - l * view_dot_sun * view_dot_sun;\r\n"\
"  float b = dot(pos, view_direction) - l * pos_dot_sun * view_dot_sun -\r\n"\
"      k * kSphereRadius * view_dot_sun;\r\n"\
"  float c = dot(pos, pos) - l * pos_dot_sun * pos_dot_sun -\r\n"\
"      2.0 * k * kSphereRadius * pos_dot_sun - kSphereRadius * kSphereRadius;\r\n"\
"  float discriminant = b * b - a * c;\r\n"\
"  if (discriminant > 0.0) {\r\n"\
"    d_in = max(0.0, (-b - sqrt(discriminant)) / a);\r\n"\
"    d_out = (-b + sqrt(discriminant)) / a;\r\n"\
"    float d_base = -pos_dot_sun / view_dot_sun;\r\n"\
"    float d_apex = -(pos_dot_sun + kSphereRadius / k) / view_dot_sun;\r\n"\
"    if (view_dot_sun > 0.0) {\r\n"\
"      d_in = max(d_in, d_apex);\r\n"\
"      d_out = a > 0.0 ? min(d_out, d_base) : d_base;\r\n"\
"    } else {\r\n"\
"      d_in = a > 0.0 ? max(d_in, d_base) : d_base;\r\n"\
"      d_out = min(d_out, d_apex);\r\n"\
"    }\r\n"\
"  } else {\r\n"\
"    d_in = 0.0;\r\n"\
"    d_out = 0.0;\r\n"\
"  }\r\n"\
"}\r\n"\
"void main() {\r\n"\
"  vec3 camera = cameraPosVec;\r\n"\
"  vec3 view_direction = normalize(view_ray);\r\n"\
"  float fragment_angular_size =\r\n"\
"      length(dFdx(view_ray) + dFdy(view_ray)) / length(view_ray);\r\n"\
"  float shadow_in;\r\n"\
"  float shadow_out;\r\n"\
"  GetSphereShadowInOut(view_direction, sun_direction, shadow_in, shadow_out);\r\n"\
"  float lightshaft_fadein_hack = smoothstep(\r\n"\
"      0.02, 0.04, dot(normalize(camera - earth_center), sun_direction));\r\n"\
"  vec3 p = camera - kSphereCenter;\r\n"\
"  float p_dot_v = dot(p, view_direction);\r\n"\
"  float p_dot_p = dot(p, p);\r\n"\
"  float ray_sphere_center_squared_distance = p_dot_p - p_dot_v * p_dot_v;\r\n"\
"  float distance_to_intersection = -p_dot_v - sqrt(\r\n"\
"      kSphereRadius * kSphereRadius - ray_sphere_center_squared_distance);\r\n"\
"  float sphere_alpha = 0.0;\r\n"\
"  vec3 sphere_radiance = vec3(0.0);\r\n"\
"  if (distance_to_intersection > 0.0) {\r\n"\
"    float ray_sphere_distance =\r\n"\
"        kSphereRadius - sqrt(ray_sphere_center_squared_distance);\r\n"\
"    float ray_sphere_angular_distance = -ray_sphere_distance / p_dot_v;\r\n"\
"    sphere_alpha =\r\n"\
"        min(ray_sphere_angular_distance / fragment_angular_size, 1.0);\r\n"\
"    vec3 point = camera + view_direction * distance_to_intersection;\r\n"\
"    vec3 normal = normalize(point - kSphereCenter);\r\n"\
"    vec3 sky_irradiance;\r\n"\
"    vec3 sun_irradiance = GetSunAndSkyIrradiance(\r\n"\
"        point - earth_center, normal, sun_direction, sky_irradiance);\r\n"\
"    sphere_radiance =\r\n"\
"        kSphereAlbedo * (1.0 / PI) * (sun_irradiance + sky_irradiance);\r\n"\
"    float shadow_length =\r\n"\
"        max(0.0, min(shadow_out, distance_to_intersection) - shadow_in) *\r\n"\
"        lightshaft_fadein_hack;\r\n"\
"    vec3 transmittance;\r\n"\
"    vec3 in_scatter = GetSkyRadianceToPoint(camera - earth_center,\r\n"\
"        point - earth_center, shadow_length, sun_direction, transmittance);\r\n"\
"    sphere_radiance = sphere_radiance * transmittance + in_scatter;\r\n"\
"  }\r\n"\
"  p = camera - earth_center;\r\n"\
"  p_dot_v = dot(p, view_direction);\r\n"\
"  p_dot_p = dot(p, p);\r\n"\
"  float ray_earth_center_squared_distance = p_dot_p - p_dot_v * p_dot_v;\r\n"\
"  distance_to_intersection = -p_dot_v - sqrt(\r\n"\
"      earth_center.z * earth_center.z - ray_earth_center_squared_distance);\r\n"\
"  float ground_alpha = 0.0;\r\n"\
"  vec3 ground_radiance = vec3(0.0);\r\n"\
"  if (distance_to_intersection > 0.0) {\r\n"\
"    vec3 point = camera + view_direction * distance_to_intersection;\r\n"\
"    vec3 normal = normalize(point - earth_center);\r\n"\
"    vec3 sky_irradiance;\r\n"\
"    vec3 sun_irradiance = GetSunAndSkyIrradiance(\r\n"\
"        point - earth_center, normal, sun_direction, sky_irradiance);\r\n"\
"    ground_radiance = kGroundAlbedo * (1.0 / PI) * (\r\n"\
"        sun_irradiance * GetSunVisibility(point, sun_direction) +\r\n"\
"        sky_irradiance * GetSkyVisibility(point));\r\n"\
"    float shadow_length =\r\n"\
"        max(0.0, min(shadow_out, distance_to_intersection) - shadow_in) *\r\n"\
"        lightshaft_fadein_hack;\r\n"\
"    vec3 transmittance;\r\n"\
"    vec3 in_scatter = GetSkyRadianceToPoint(camera - earth_center,\r\n"\
"        point - earth_center, shadow_length, sun_direction, transmittance);\r\n"\
"    ground_radiance = ground_radiance * transmittance + in_scatter;\r\n"\
"    ground_alpha = 1.0;\r\n"\
"  }\r\n"\
"  float shadow_length = max(0.0, shadow_out - shadow_in) *\r\n"\
"      lightshaft_fadein_hack;\r\n"\
"  vec3 transmittance;\r\n"\
"  vec3 radiance = GetSkyRadiance(\r\n"\
"      camera - earth_center, view_direction, shadow_length, sun_direction,\r\n"\
"      transmittance);\r\n"\
"  if (dot(view_direction, sun_direction) > sun_size.y) {\r\n"\
"    radiance = radiance + transmittance * GetSolarRadiance();\r\n"\
"  }\r\n"\
"  radiance = mix(radiance, ground_radiance, ground_alpha);\r\n"\
"  radiance = mix(radiance, sphere_radiance, sphere_alpha);\r\n"\
"  color =\r\n"\
"     vec4(sun_direction, 1.0);\r\n"\
"}\r\n"\
"";

//"     vec4(pow(vec3(1.0) - exp(-radiance / white_point * exposure), vec3(1.0 / 2.2)), 1.0);\r\n"\