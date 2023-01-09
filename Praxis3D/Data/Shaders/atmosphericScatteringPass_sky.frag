#version 430 core

/*const float kLengthUnitInMeters = 1000.0;
const float PI = 3.14159265;
const vec3 kSphereCenter = vec3(0.0, 0.0, 1000.0) / kLengthUnitInMeters;
const float kSphereRadius = 1000.0 / kLengthUnitInMeters;
const vec3 kSphereAlbedo = vec3(0.8);
const vec3 kGroundAlbedo = vec3(0.0, 0.0, 0.04);

#ifdef USE_LUMINANCE
#define GetSolarRadiance GetSolarLuminance
#define GetSkyRadiance GetSkyLuminance
#define GetSkyRadianceToPoint GetSkyLuminanceToPoint
#define GetSunAndSkyIrradiance GetSunAndSkyIlluminance
#endif*/

#define TEMPLATE(x)
#define TEMPLATE_ARGUMENT(x)
#define assert(x)

in vec3 viewRay;

out vec4 color;

uniform float exposure;

uniform ivec2 screenSize;
uniform vec3 cameraPosVec;

uniform sampler2D atmIrradianceTexture;
uniform sampler3D atmScatteringTexture;
uniform sampler3D atmSingleMieTexture;
uniform sampler2D atmTransmitTexture;

const float kLengthUnitInMeters = 1000.0;
const vec3 kGroundAlbedo = vec3(0.0, 0.0, 0.04);

const float m = 1.0;
const float nm = 1.0;
const float rad = 1.0;
const float sr = 1.0;
const float watt = 1.0;
const float lm = 1.0;
const float PI = 3.14159265358979323846;
const float km = 1000.0 * m;
const float m2 = m * m;
const float m3 = m * m * m;
const float pi = PI * rad;
const float deg = pi / 180.0;
const float watt_per_square_meter = watt / m2;
const float watt_per_square_meter_per_sr = watt / (m2 * sr);
const float watt_per_square_meter_per_nm = watt / (m2 * nm);
const float watt_per_square_meter_per_sr_per_nm = watt / (m2 * sr * nm);
const float watt_per_cubic_meter_per_sr_per_nm = watt / (m3 * sr * nm);
const float cd = lm / sr;
const float kcd = 1000.0 * cd;
const float cd_per_square_meter = cd / m2;
const float kcd_per_square_meter = kcd / m2;
const vec3 SKY_SPECTRAL_RADIANCE_TO_LUMINANCE = vec3(114974.916437,71305.954816,65310.548555);
const vec3 SUN_SPECTRAL_RADIANCE_TO_LUMINANCE = vec3(98242.786222,69954.398112,66475.012354);
const int TRANSMITTANCE_TEXTURE_WIDTH = 256;
const int TRANSMITTANCE_TEXTURE_HEIGHT = 64;
const int SCATTERING_TEXTURE_R_SIZE = 32;
const int SCATTERING_TEXTURE_MU_SIZE = 128;
const int SCATTERING_TEXTURE_MU_S_SIZE = 32;
const int SCATTERING_TEXTURE_NU_SIZE = 8;
const int IRRADIANCE_TEXTURE_WIDTH = 64;
const int IRRADIANCE_TEXTURE_HEIGHT = 16;

struct DirectionalLight
{
    vec3 m_color;
    vec3 m_direction;
    float m_intensity;
};

// An atmosphere layer of width 'width', and whose density is defined as
//   'exp_term' * exp('exp_scale' * h) + 'linear_term' * h + 'constant_term',
// clamped to [0,1], and where h is the altitude.
struct DensityProfileLayer 
{
	float width;
	float exp_term;
	float exp_scale;
	float linear_term;
	float constant_term;
};

// An atmosphere density profile made of several layers on top of each other
// (from bottom to top). The width of the last layer is ignored, i.e. it always
// extend to the top atmosphere boundary. The profile values vary between 0
// (null density) to 1 (maximum density).
struct DensityProfile 
{
	DensityProfileLayer layers[2];
};

struct AtmosphereParameters 
{
	// The solar irradiance at the top of the atmosphere.
	vec3 solar_irradiance;
	// The sun's angular radius. Warning: the implementation uses approximations
	// that are valid only if this angle is smaller than 0.1 radians.
	float sun_angular_radius;
	// The density profile of air molecules, i.e. a function from altitude to
	// dimensionless values between 0 (null density) and 1 (maximum density).
	DensityProfile rayleigh_density;
	// The density profile of aerosols, i.e. a function from altitude to
	// dimensionless values between 0 (null density) and 1 (maximum density).
	DensityProfile mie_density;
	// The density profile of air molecules that absorb light (e.g. ozone), i.e.
	// a function from altitude to dimensionless values between 0 (null density)
	// and 1 (maximum density).
	DensityProfile absorption_density;
	// The scattering coefficient of air molecules at the altitude where their
	// density is maximum (usually the bottom of the atmosphere), as a function of
	// wavelength. The scattering coefficient at altitude h is equal to
	// 'rayleigh_scattering' times 'rayleigh_density' at this altitude.
	vec3 rayleigh_scattering;
	// The distance between the planet center and the bottom of the atmosphere.
	float bottom_radius;
	// The scattering coefficient of aerosols at the altitude where their density
	// is maximum (usually the bottom of the atmosphere), as a function of
	// wavelength. The scattering coefficient at altitude h is equal to
	// 'mie_scattering' times 'mie_density' at this altitude.
	vec3 mie_scattering;
	// The distance between the planet center and the top of the atmosphere.
	float top_radius;
	// The extinction coefficient of aerosols at the altitude where their density
	// is maximum (usually the bottom of the atmosphere), as a function of
	// wavelength. The extinction coefficient at altitude h is equal to
	// 'mie_extinction' times 'mie_density' at this altitude.
	vec3 mie_extinction;
	// The asymetry parameter for the Cornette-Shanks phase function for the
	// aerosols.
	float mie_phase_function_g;
	// The extinction coefficient of molecules that absorb light (e.g. ozone) at
	// the altitude where their density is maximum, as a function of wavelength.
	// The extinction coefficient at altitude h is equal to
	// 'absorption_extinction' times 'absorption_density' at this altitude.
	vec3 absorption_extinction;
	// The cosine of the maximum Sun zenith angle for which atmospheric scattering
	// must be precomputed (for maximum precision, use the smallest Sun zenith
	// angle yielding negligible sky light radiance values. For instance, for the
	// Earth case, 102 degrees is a good choice - yielding mu_s_min = -0.2).
	float mu_s_min;
	// The average albedo of the ground.
	vec3 ground_albedo;
};

struct AtmScatteringParameters
{
	vec3 m_whitePoint;
	vec3 m_earthCenter;
	vec2 m_sunSize;
	AtmosphereParameters m_atmosphereParam;
};

uniform DirectionalLight directionalLight;

layout (std140) uniform AtmScatParametersBuffer
{
	AtmScatteringParameters atmScatteringParam;
};
	
vec2 calcTexCoord(void)
{
	return gl_FragCoord.xy / screenSize;
}
float ClampCosine(float p_mu) 
{
	return clamp(p_mu, float(-1.0), float(1.0));
}

float ClampDistance(float p_d) 
{
	return max(p_d, 0.0 * m);
}

float ClampRadius(const AtmosphereParameters p_atmospherePar, float p_r) 
{
	return clamp(p_r, p_atmospherePar.bottom_radius, p_atmospherePar.top_radius);
}

float SafeSqrt(float p_a) 
{
	return sqrt(max(p_a, 0.0 * m2));
}

float GetTextureCoordFromUnitRange(float p_x, int p_textureSize) 
{
	return 0.5 / float(p_textureSize) + p_x * (1.0 - 1.0 / float(p_textureSize));
}

float RayleighPhaseFunction(float p_nu) 
{
	float k = 3.0 / (16.0 * PI * sr);
	return k * (1.0 + p_nu * p_nu);
}

float MiePhaseFunction(float p_g, float p_nu) 
{
	float k = 3.0 / (8.0 * PI * sr) * (1.0 - p_g * p_g) / (2.0 + p_g * p_g);
	return k * (1.0 + p_nu * p_nu) / pow(1.0 + p_g * p_g - 2.0 * p_g * p_nu, 1.5);
}

float DistanceToTopAtmosphereBoundary(const AtmosphereParameters p_atmospherePar, float p_r, float p_mu) 
{
	assert(p_r <= p_atmospherePar.top_radius);
	assert(p_mu >= -1.0 && p_mu <= 1.0);
	float discriminant = p_r * p_r * (p_mu * p_mu - 1.0) + p_atmospherePar.top_radius * p_atmospherePar.top_radius;
	return ClampDistance(-p_r * p_mu + SafeSqrt(discriminant));
}

vec2 GetTransmittanceTextureUvFromRMu(const AtmosphereParameters p_atmospherePar, float p_r, float p_mu) 
{
	assert(p_r >= p_atmospherePar.bottom_radius && p_r <= p_atmospherePar.top_radius);
	assert(p_mu >= -1.0 && p_mu <= 1.0);
	// Distance to top atmosphere boundary for a horizontal ray at ground level.
	float H = sqrt(p_atmospherePar.top_radius * p_atmospherePar.top_radius - p_atmospherePar.bottom_radius * p_atmospherePar.bottom_radius);
	// Distance to the horizon.
	float rho = SafeSqrt(p_r * p_r - p_atmospherePar.bottom_radius * p_atmospherePar.bottom_radius);
	// Distance to the top atmosphere boundary for the ray (r,mu), and its minimum
	// and maximum values over all mu - obtained for (r,1) and (r,mu_horizon).
	float d = DistanceToTopAtmosphereBoundary(p_atmospherePar, p_r, p_mu);
	float d_min = p_atmospherePar.top_radius - p_r;
	float d_max = rho + H;
	float x_mu = (d - d_min) / (d_max - d_min);
	float x_r = rho / H;
	return vec2(GetTextureCoordFromUnitRange(x_mu, TRANSMITTANCE_TEXTURE_WIDTH), GetTextureCoordFromUnitRange(x_r, TRANSMITTANCE_TEXTURE_HEIGHT));
}

vec2 GetIrradianceTextureUvFromRMuS(const AtmosphereParameters p_atmospherePar, float p_r, float p_muS) 
{
	assert(p_r >= p_atmospherePar.bottom_radius && p_r <= p_atmospherePar.top_radius);
	assert(p_muS >= -1.0 && p_muS <= 1.0);
	float x_r = (p_r - p_atmospherePar.bottom_radius) / (p_atmospherePar.top_radius - p_atmospherePar.bottom_radius);
	float x_mu_s = p_muS * 0.5 + 0.5;
	return vec2(GetTextureCoordFromUnitRange(x_mu_s, IRRADIANCE_TEXTURE_WIDTH), GetTextureCoordFromUnitRange(x_r, IRRADIANCE_TEXTURE_HEIGHT));
}

vec3 GetIrradiance(const AtmosphereParameters p_atmospherePar, sampler2D p_atmIrradianceTex, float p_r, float p_muS) 
{
	vec2 uv = GetIrradianceTextureUvFromRMuS(p_atmospherePar, p_r, p_muS);
	return vec3(texture(p_atmIrradianceTex, uv).xyz);
}

vec3 GetTransmittanceToTopAtmosphereBoundary(const AtmosphereParameters p_atmospherePar, sampler2D p_atmTransmittanceTex, float p_r, float p_muS) 
{
	assert(p_r >= p_atmospherePar.bottom_radius && p_r <= p_atmospherePar.top_radius);
	vec2 uv = GetTransmittanceTextureUvFromRMu(p_atmospherePar, p_r, p_muS);
	return vec3(texture(p_atmTransmittanceTex, uv));
}

vec3 GetTransmittanceToSun(const AtmosphereParameters p_atmospherePar, sampler2D p_atmTransmittanceTex, float p_r, float p_muS) 
{
	float sin_theta_h = p_atmospherePar.bottom_radius / p_r;
	float cos_theta_h = -sqrt(max(1.0 - sin_theta_h * sin_theta_h, 0.0));
	return GetTransmittanceToTopAtmosphereBoundary(p_atmospherePar, p_atmTransmittanceTex, p_r, p_muS) *
		smoothstep(-sin_theta_h * p_atmospherePar.sun_angular_radius / rad, sin_theta_h * p_atmospherePar.sun_angular_radius / rad, p_muS - cos_theta_h);
}

vec3 GetSunAndSkyIrradiance(const AtmosphereParameters p_atmospherePar, in vec3 p_point, in vec3 p_normal, in vec3 p_sunDirection, out vec3 p_skyIrradiance) 
{
	float r = length(p_point);
	float mu_s = dot(p_point, p_sunDirection) / r;

	// Indirect irradiance (approximated if the surface is not horizontal).
	p_skyIrradiance = GetIrradiance(p_atmospherePar, atmIrradianceTexture, r, mu_s) * (1.0 + dot(p_normal, p_point) / r) * 0.5;

	// Direct irradiance.
	return p_atmospherePar.solar_irradiance * GetTransmittanceToSun(p_atmospherePar, atmTransmitTexture, r, mu_s) * max(dot(p_normal, p_sunDirection), 0.0);
}

bool RayIntersectsGround(const AtmosphereParameters p_atmospherePar, float p_r, float p_mu) 
{
	assert(p_r >= p_atmospherePar.bottom_radius);
	assert(p_mu >= -1.0 && p_mu <= 1.0);
	return p_mu < 0.0 && p_r * p_r * (p_mu * p_mu - 1.0) + p_atmospherePar.bottom_radius * p_atmospherePar.bottom_radius >= 0.0 * m2;
}

vec3 GetTransmittance(const AtmosphereParameters p_atmospherePar, sampler2D p_atmTransmittanceTex,
    float p_r, float p_mu, float p_d, bool p_ray_r_mu_intersects_ground) 
{
	assert(p_r >= p_atmospherePar.bottom_radius && p_r <= p_atmospherePar.top_radius);
	assert(p_mu >= -1.0 && p_mu <= 1.0);
	assert(p_d >= 0.0 * m);

	float r_d = ClampRadius(p_atmospherePar, sqrt(p_d * p_d + 2.0 * p_r * p_mu * p_d + p_r * p_r));
	float mu_d = ClampCosine((p_r * p_mu + p_d) / r_d);

	if(p_ray_r_mu_intersects_ground)
	{
		return min(GetTransmittanceToTopAtmosphereBoundary(p_atmospherePar, p_atmTransmittanceTex, r_d, -mu_d) /
			GetTransmittanceToTopAtmosphereBoundary(p_atmospherePar, p_atmTransmittanceTex, p_r, -p_mu), 
			vec3(1.0));
	} 
	else
	{
		return min(GetTransmittanceToTopAtmosphereBoundary(p_atmospherePar, p_atmTransmittanceTex, p_r, p_mu) /
			GetTransmittanceToTopAtmosphereBoundary(p_atmospherePar, p_atmTransmittanceTex, r_d, mu_d),
			vec3(1.0));
	}
}

vec4 GetScatteringTextureUvwzFromRMuMuSNu(const AtmosphereParameters p_atmospherePar, float p_r, float p_mu, float p_muS, float p_nu, bool p_ray_r_mu_intersects_ground) 
{
	assert(p_r >= p_atmospherePar.bottom_radius && p_r <= p_atmospherePar.top_radius);
	assert(p_mu >= -1.0 && p_mu <= 1.0);
	assert(p_muS >= -1.0 && p_muS <= 1.0);
	assert(p_nu >= -1.0 && p_nu <= 1.0);

	// Distance to top atmosphere boundary for a horizontal ray at ground level.
	float H = sqrt(p_atmospherePar.top_radius * p_atmospherePar.top_radius - p_atmospherePar.bottom_radius * p_atmospherePar.bottom_radius);
	// Distance to the horizon.
	float rho = SafeSqrt(p_r * p_r - p_atmospherePar.bottom_radius * p_atmospherePar.bottom_radius);
	float u_r = GetTextureCoordFromUnitRange(rho / H, SCATTERING_TEXTURE_R_SIZE);

	// Discriminant of the quadratic equation for the intersections of the ray
	// (r,mu) with the ground (see RayIntersectsGround).
	float r_mu = p_r * p_mu;
	float discriminant = r_mu * r_mu - p_r * p_r + p_atmospherePar.bottom_radius * p_atmospherePar.bottom_radius;
	float u_mu;
	if(p_ray_r_mu_intersects_ground)
	{
		// Distance to the ground for the ray (r,mu), and its minimum and maximum
		// values over all mu - obtained for (r,-1) and (r,mu_horizon).
		float d = -r_mu - SafeSqrt(discriminant);
		float d_min = p_r - p_atmospherePar.bottom_radius;
		float d_max = rho;
		u_mu = 0.5 - 0.5 * GetTextureCoordFromUnitRange(d_max == d_min ? 0.0 : (d - d_min) / (d_max - d_min), SCATTERING_TEXTURE_MU_SIZE / 2);
	} 
	else 
	{
		// Distance to the top atmosphere boundary for the ray (r,mu), and its
		// minimum and maximum values over all mu - obtained for (r,1) and
		// (r,mu_horizon).
		float d = -r_mu + SafeSqrt(discriminant + H * H);
		float d_min = p_atmospherePar.top_radius - p_r;
		float d_max = rho + H;
		u_mu = 0.5 + 0.5 * GetTextureCoordFromUnitRange((d - d_min) / (d_max - d_min), SCATTERING_TEXTURE_MU_SIZE / 2);
	}

	float d = DistanceToTopAtmosphereBoundary(p_atmospherePar, p_atmospherePar.bottom_radius, p_muS);
	float d_min = p_atmospherePar.top_radius - p_atmospherePar.bottom_radius;
	float d_max = H;
	float a = (d - d_min) / (d_max - d_min);
	float A = -2.0 * p_atmospherePar.mu_s_min * p_atmospherePar.bottom_radius / (d_max - d_min);
	float u_mu_s = GetTextureCoordFromUnitRange(max(1.0 - a / A, 0.0) / (1.0 + a), SCATTERING_TEXTURE_MU_S_SIZE);

	float u_nu = (p_nu + 1.0) / 2.0;
	return vec4(u_nu, u_mu_s, u_mu, u_r);
}

vec3 GetCombinedScattering(
    const AtmosphereParameters p_atmospherePar,
    sampler3D p_atmScatteringTex,
    sampler3D p_atmSingleMieScatteringTex,
    float p_r, 
	float p_mu, 
	float p_muS, 
	float p_nu,
    bool p_ray_r_mu_intersects_ground,
    out vec3 p_single_mie_scattering) 
	{
	vec4 uvwz = GetScatteringTextureUvwzFromRMuMuSNu(p_atmospherePar, p_r, p_mu, p_muS, p_nu, p_ray_r_mu_intersects_ground);
	float tex_coord_x = uvwz.x * float(SCATTERING_TEXTURE_NU_SIZE - 1);
	float tex_x = floor(tex_coord_x);
	float lerp = tex_coord_x - tex_x;
	vec3 uvw0 = vec3((tex_x + uvwz.y) / float(SCATTERING_TEXTURE_NU_SIZE), uvwz.z, uvwz.w);
	vec3 uvw1 = vec3((tex_x + 1.0 + uvwz.y) / float(SCATTERING_TEXTURE_NU_SIZE), uvwz.z, uvwz.w);
	
#ifdef COMBINED_SCATTERING_TEXTURES
	vec4 combined_scattering =texture(p_atmScatteringTex, uvw0) * (1.0 - lerp) + texture(p_atmScatteringTex, uvw1) * lerp;
	vec3 scattering = vec3(combined_scattering);
	p_single_mie_scattering = GetExtrapolatedSingleMieScattering(p_atmospherePar, combined_scattering);
#else
	vec3 scattering = vec3(texture(p_atmScatteringTex, uvw0) * (1.0 - lerp) + texture(p_atmScatteringTex, uvw1) * lerp);
	p_single_mie_scattering = vec3(texture(p_atmSingleMieScatteringTex, uvw0) * (1.0 - lerp) + texture(p_atmSingleMieScatteringTex, uvw1) * lerp);
#endif

	return scattering;
}

vec3 GetSkyRadianceToPoint(
    const AtmosphereParameters p_atmospherePar,
    sampler2D p_atmTransmittanceTex,
    sampler3D p_atmScatteringTex,
    sampler3D p_atmSingleMieScatteringTex,
    vec3 p_cameraPos, 
	in vec3 p_point, 
	in float p_shadowLength,
    in vec3 p_sunDirection, 
	out vec3 p_transmittance) 
	{
	// Compute the distance to the top atmosphere boundary along the view ray,
	// assuming the viewer is in space (or NaN if the view ray does not intersect
	// the atmosphere).
	vec3 view_ray = normalize(p_point - p_cameraPos);
	float r = length(p_cameraPos);
	float rmu = dot(p_cameraPos, view_ray);
	float distance_to_top_atmosphere_boundary = -rmu -sqrt(rmu * rmu - r * r + p_atmospherePar.top_radius * p_atmospherePar.top_radius);
	// If the viewer is in space and the view ray intersects the atmosphere, move
	// the viewer to the top atmosphere boundary (along the view ray):
	if(distance_to_top_atmosphere_boundary > 0.0 * m) 
	{
		p_cameraPos = p_cameraPos + view_ray * distance_to_top_atmosphere_boundary;
		r = p_atmospherePar.top_radius;
		rmu += distance_to_top_atmosphere_boundary;
	}

	// Compute the r, mu, mu_s and nu parameters for the first texture lookup.
	float mu = rmu / r;
	float mu_s = dot(p_cameraPos, p_sunDirection) / r;
	float nu = dot(view_ray, p_sunDirection);
	float d = length(p_point - p_cameraPos);
	bool ray_r_mu_intersects_ground = RayIntersectsGround(p_atmospherePar, r, mu);

	p_transmittance = GetTransmittance(p_atmospherePar, p_atmTransmittanceTex, r, mu, d, ray_r_mu_intersects_ground);

	vec3 single_mie_scattering;
	vec3 scattering = GetCombinedScattering(
		p_atmospherePar, p_atmScatteringTex, p_atmSingleMieScatteringTex,
		r, mu, mu_s, nu, ray_r_mu_intersects_ground,
		single_mie_scattering);

	// Compute the r, mu, mu_s and nu parameters for the second texture lookup.
	// If shadow_length is not 0 (case of light shafts), we want to ignore the
	// scattering along the last shadow_length meters of the view ray, which we
	// do by subtracting shadow_length from d (this way scattering_p is equal to
	// the S|x_s=x_0-lv term in Eq. (17) of our paper).
	d = max(d - p_shadowLength, 0.0 * m);
	float r_p = ClampRadius(p_atmospherePar, sqrt(d * d + 2.0 * r * mu * d + r * r));
	float mu_p = (r * mu + d) / r_p;
	float mu_s_p = (r * mu_s + d * nu) / r_p;

	vec3 single_mie_scattering_p;
	vec3 scattering_p = GetCombinedScattering(
		p_atmospherePar, p_atmScatteringTex, p_atmSingleMieScatteringTex,
		r_p, mu_p, mu_s_p, nu, ray_r_mu_intersects_ground,
		single_mie_scattering_p);

	// Combine the lookup results to get the scattering between camera and point.
	vec3 shadow_transmittance = p_transmittance;
	if(p_shadowLength > 0.0 * m) 
	{
		// This is the T(x,x_s) term in Eq. (17) of our paper, for light shafts.
		shadow_transmittance = GetTransmittance(p_atmospherePar, p_atmTransmittanceTex, r, mu, d, ray_r_mu_intersects_ground);
	}
	scattering = scattering - shadow_transmittance * scattering_p;
	single_mie_scattering = single_mie_scattering - shadow_transmittance * single_mie_scattering_p;
#ifdef COMBINED_SCATTERING_TEXTURES
	single_mie_scattering = GetExtrapolatedSingleMieScattering(p_atmospherePar, vec4(scattering, single_mie_scattering.r));
#endif

	// Hack to avoid rendering artifacts when the sun is below the horizon.
	single_mie_scattering = single_mie_scattering * smoothstep(float(0.0), float(0.01), mu_s);

	return scattering * RayleighPhaseFunction(nu) + single_mie_scattering * MiePhaseFunction(p_atmospherePar.mie_phase_function_g, nu);
}

vec3 GetSkyRadiance(
    const AtmosphereParameters p_atmospherePar,
    sampler2D p_atmTransmittanceTex,
    sampler3D p_atmScatteringTex,
    sampler3D p_atmSingleMieScatteringTex,
    vec3 p_camera, 
	in vec3 p_viewRay, 
	float p_shadowLength,
    in vec3 p_sunDirection, 
	out vec3 p_transmittance)
{
	// Compute the distance to the top atmosphere boundary along the view ray,
	// assuming the viewer is in space (or NaN if the view ray does not intersect
	// the atmosphere).
	float r = length(p_camera);
	float rmu = dot(p_camera, p_viewRay);
	float distance_to_top_atmosphere_boundary = -rmu - sqrt(rmu * rmu - r * r + p_atmospherePar.top_radius * p_atmospherePar.top_radius);
	// If the viewer is in space and the view ray intersects the atmosphere, move
	// the viewer to the top atmosphere boundary (along the view ray):
	if(distance_to_top_atmosphere_boundary > 0.0 * m)
	{
		p_camera = p_camera + p_viewRay * distance_to_top_atmosphere_boundary;
		r = p_atmospherePar.top_radius;
		rmu += distance_to_top_atmosphere_boundary;
	} 
	else if(r > p_atmospherePar.top_radius) 
	{
		// If the view ray does not intersect the atmosphere, simply return 0.
		p_transmittance = vec3(1.0);
		return vec3(0.0 * watt_per_square_meter_per_sr_per_nm);
	}
	// Compute the r, mu, mu_s and nu parameters needed for the texture lookups.
	float mu = rmu / r;
	float mu_s = dot(p_camera, p_sunDirection) / r;
	float nu = dot(p_viewRay, p_sunDirection);
	bool ray_r_mu_intersects_ground = RayIntersectsGround(p_atmospherePar, r, mu);

	p_transmittance = ray_r_mu_intersects_ground ? vec3(0.0) : GetTransmittanceToTopAtmosphereBoundary(p_atmospherePar, p_atmTransmittanceTex, r, mu);
	vec3 single_mie_scattering;
	vec3 scattering;
	if(p_shadowLength == 0.0 * m)
	{
		scattering = GetCombinedScattering(
			p_atmospherePar, p_atmScatteringTex, p_atmSingleMieScatteringTex,
			r, mu, mu_s, nu, ray_r_mu_intersects_ground,
			single_mie_scattering);
	}
	else
	{
		// Case of light shafts (shadow_length is the total length noted l in our
		// paper): we omit the scattering between the camera and the point at
		// distance l, by implementing Eq. (18) of the paper (shadow_transmittance
		// is the T(x,x_s) term, scattering is the S|x_s=x+lv term).
		float d = p_shadowLength;
		float r_p = ClampRadius(p_atmospherePar, sqrt(d * d + 2.0 * r * mu * d + r * r));
		float mu_p = (r * mu + d) / r_p;
		float mu_s_p = (r * mu_s + d * nu) / r_p;

		scattering = GetCombinedScattering(
			p_atmospherePar, p_atmScatteringTex, p_atmSingleMieScatteringTex,
			r_p, mu_p, mu_s_p, nu, ray_r_mu_intersects_ground,
			single_mie_scattering);
		vec3 shadow_transmittance =
			GetTransmittance(p_atmospherePar, p_atmTransmittanceTex,
			r, mu, p_shadowLength, ray_r_mu_intersects_ground);
		scattering = scattering * shadow_transmittance;
		single_mie_scattering = single_mie_scattering * shadow_transmittance;
	}
	
	return scattering * RayleighPhaseFunction(nu) + single_mie_scattering * MiePhaseFunction(p_atmospherePar.mie_phase_function_g, nu);
}

float GetSunVisibility(vec3 p_point, vec3 p_sunDirection)
{
	vec3 p = p_point;// - kSphereCenter;
	float p_dot_v = dot(p, p_sunDirection);
	float p_dot_p = dot(p, p);
	float ray_sphere_center_squared_distance = p_dot_p - p_dot_v * p_dot_v;
	//float distance_to_intersection = -p_dot_v - sqrt(kSphereRadius * kSphereRadius - ray_sphere_center_squared_distance);
	
	//if(distance_to_intersection > 0.0)
	{
		// Compute the distance between the view ray and the sphere, and the
		// corresponding (tangent of the) subtended angle. Finally, use this to
		// compute an approximate sun visibility.
		//float ray_sphere_distance = kSphereRadius - sqrt(ray_sphere_center_squared_distance);
		//float ray_sphere_angular_distance = -ray_sphere_distance / p_dot_v;
		//return smoothstep(1.0, 0.0, ray_sphere_angular_distance / atmScatteringParam.m_sunSize.x);
	}
	
	return 1.0;
}

float GetSkyVisibility(vec3 p_point)
{
	vec3 p = p_point;// - kSphereCenter;
	float p_dot_p = dot(p, p);
	return 1.0;// + p.z / sqrt(p_dot_p) * kSphereRadius * kSphereRadius / p_dot_p;
} 
vec3 GetSolarRadiance(const AtmosphereParameters p_atmospherePar)
{
	return p_atmospherePar.solar_irradiance / (PI * p_atmospherePar.sun_angular_radius * p_atmospherePar.sun_angular_radius);
}

void main() 
{
	//vec2 texCoord = calcTexCoord();
	vec3 cameraPosition = cameraPosVec.xyz / kLengthUnitInMeters;
	
	// Normalized view direction vector.
	vec3 viewDirection = normalize(viewRay);
	
	// Tangent of the angle subtended by this fragment.
	float fragment_angular_size = length(dFdx(viewRay) + dFdy(viewRay)) / length(viewRay);
	
	float shadow_length = 0.0;
	
	// Compute the distance between the view ray line and the Earth center,
	// and the distance between the camera and the intersection of the view
	// ray with the ground (or NaN if there is no intersection).
	vec3 p = cameraPosition - atmScatteringParam.m_earthCenter;
	float p_dot_v = dot(p, viewDirection);
	float p_dot_p = dot(p, p);
	float ray_earth_center_squared_distance = p_dot_p - p_dot_v * p_dot_v;
	float distance_to_intersection = -p_dot_v - sqrt(atmScatteringParam.m_earthCenter.z * atmScatteringParam.m_earthCenter.z - ray_earth_center_squared_distance);

	// Compute the radiance reflected by the ground, if the ray intersects it.
	float ground_alpha = 0.0;
	vec3 ground_radiance = vec3(0.0);
	if(distance_to_intersection > 0.0)
	{
		vec3 point = cameraPosition + viewDirection * distance_to_intersection;
		vec3 normal = normalize(point - atmScatteringParam.m_earthCenter);

		// Compute the radiance reflected by the ground.
		vec3 sky_irradiance;
		vec3 sun_irradiance = GetSunAndSkyIrradiance(
			atmScatteringParam.m_atmosphereParam,
			point - atmScatteringParam.m_earthCenter,
			normal,
			directionalLight.m_direction,
			sky_irradiance);
			
		ground_radiance = kGroundAlbedo * (1.0 / PI) * (sun_irradiance * GetSunVisibility(point, directionalLight.m_direction) + sky_irradiance * GetSkyVisibility(point));

		vec3 transmittance;
		vec3 in_scatter = GetSkyRadianceToPoint(
			atmScatteringParam.m_atmosphereParam,
			atmTransmitTexture,
			atmScatteringTexture,
			atmSingleMieTexture,
			cameraPosition - atmScatteringParam.m_earthCenter,
			point - atmScatteringParam.m_earthCenter,
			shadow_length,
			directionalLight.m_direction,
			transmittance);
			
		ground_radiance = ground_radiance * transmittance + in_scatter;
		ground_alpha = 1.0;
	}
	
	// Compute the radiance of the sky.
	vec3 transmittance;
	vec3 radiance = GetSkyRadiance(
		atmScatteringParam.m_atmosphereParam,
		atmTransmitTexture,
		atmScatteringTexture,
		atmSingleMieTexture,
		cameraPosition - atmScatteringParam.m_earthCenter,
		viewDirection,
		shadow_length,
		directionalLight.m_direction,
		transmittance);

	// If the view ray intersects the Sun, add the Sun radiance.
	if(dot(viewDirection, directionalLight.m_direction) > atmScatteringParam.m_sunSize.y)
	{
		radiance = radiance + transmittance * GetSolarRadiance(atmScatteringParam.m_atmosphereParam);
	}
	
	radiance = mix(radiance, ground_radiance, ground_alpha);
	
	//color = vec4(vec3(1.0) - exp(-radiance / atmScatteringParam.m_whitePoint * directionalLight.m_intensity), 1.0);
	color = vec4(radiance / atmScatteringParam.m_whitePoint * directionalLight.m_intensity * 1.0, 1.0);
	//color = vec4(radiance / atmScatteringParam.m_whitePoint, 1.0);
}
