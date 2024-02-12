
#include "AtmScatteringPass.h"
#include "AtmScatteringShaderPass.h"
#include "RendererScene.h"

void AtmScatteringPass::initModel()
{
	// Values from "Reference Solar Spectral Irradiance: ASTM G-173", ETR column
	// (see http://rredc.nrel.gov/solar/spectra/am1.5/ASTMG173/ASTMG173.html),
	// summed and averaged in each bin (e.g. the value for 360nm is the average
	// of the ASTM G-173 values for all wavelengths between 360 and 370nm).
	// Values in W.m^-2.
	constexpr int kLambdaMin = 360;
	constexpr int kLambdaMax = 830;
	constexpr double kSolarIrradiance[48] = {
		1.11776, 1.14259, 1.01249, 1.14716, 1.72765, 1.73054, 1.6887, 1.61253,
		1.91198, 2.03474, 2.02042, 2.02212, 1.93377, 1.95809, 1.91686, 1.8298,
		1.8685, 1.8931, 1.85149, 1.8504, 1.8341, 1.8345, 1.8147, 1.78158, 1.7533,
		1.6965, 1.68194, 1.64654, 1.6048, 1.52143, 1.55622, 1.5113, 1.474, 1.4482,
		1.41018, 1.36775, 1.34188, 1.31429, 1.28303, 1.26758, 1.2367, 1.2082,
		1.18737, 1.14683, 1.12362, 1.1058, 1.07124, 1.04992
	};
	// Values from http://www.iup.uni-bremen.de/gruppen/molspec/databases/
	// referencespectra/o3spectra2011/index.html for 233K, summed and averaged in
	// each bin (e.g. the value for 360nm is the average of the original values
	// for all wavelengths between 360 and 370nm). Values in m^2.
	constexpr double kOzoneCrossSection[48] = {
		1.18e-27, 2.182e-28, 2.818e-28, 6.636e-28, 1.527e-27, 2.763e-27, 5.52e-27,
		8.451e-27, 1.582e-26, 2.316e-26, 3.669e-26, 4.924e-26, 7.752e-26, 9.016e-26,
		1.48e-25, 1.602e-25, 2.139e-25, 2.755e-25, 3.091e-25, 3.5e-25, 4.266e-25,
		4.672e-25, 4.398e-25, 4.701e-25, 5.019e-25, 4.305e-25, 3.74e-25, 3.215e-25,
		2.662e-25, 2.238e-25, 1.852e-25, 1.473e-25, 1.209e-25, 9.423e-26, 7.455e-26,
		6.566e-26, 5.105e-26, 4.15e-26, 4.228e-26, 3.237e-26, 2.451e-26, 2.801e-26,
		2.534e-26, 1.624e-26, 1.465e-26, 2.078e-26, 1.383e-26, 7.105e-27
	};
	// From https://en.wikipedia.org/wiki/Dobson_unit, in molecules.m^-2.
	constexpr double kDobsonUnit = 2.687e20;
	// Maximum number density of ozone molecules, in m^-3 (computed so at to get
	// 300 Dobson units of ozone - for this we divide 300 DU by the integral of
	// the ozone density profile defined below, which is equal to 15km).
	constexpr double kMaxOzoneNumberDensity = 300.0 * kDobsonUnit / 15000.0;
	// Wavelength independent solar irradiance "spectrum" (not physically
	// realistic, but was used in the original implementation).
	constexpr double kConstantSolarIrradiance = 1.5;
	constexpr double kBottomRadius = 6360000.0;
	constexpr double kTopRadius = 6420000.0;
	constexpr double kRayleigh = 1.24062e-6;
	constexpr double kRayleighScaleHeight = 8000.0;
	constexpr double kMieScaleHeight = 1200.0;
	constexpr double kMieAngstromAlpha = 0.0;
	constexpr double kMieAngstromBeta = 5.328e-3;
	constexpr double kMieSingleScatteringAlbedo = 0.9;
	constexpr double kMiePhaseFunctionG = 0.8;
	constexpr double kGroundAlbedo = 0.1;
	const double max_sun_zenith_angle =
		(m_useHalfPrecision ? 102.0 : 120.0) / 180.0 * Math::PI_DOUBLE;

	DensityProfileLayer
		rayleigh_layer(0.0, 1.0, -1.0 / kRayleighScaleHeight, 0.0, 0.0);
	DensityProfileLayer mie_layer(0.0, 1.0, -1.0 / kMieScaleHeight, 0.0, 0.0);
	// Density profile increasing linearly from 0 to 1 between 10 and 25km, and
	// decreasing linearly from 1 to 0 between 25 and 40km. This is an approximate
	// profile from http://www.kln.ac.lk/science/Chemistry/Teaching_Resources/
	// Documents/Introduction%20to%20atmospheric%20chemistry.pdf (page 10).
	std::vector<DensityProfileLayer> ozone_density;
	ozone_density.push_back(
		DensityProfileLayer(25000.0, 0.0, 0.0, 1.0 / 15000.0, -2.0 / 3.0));
	ozone_density.push_back(
		DensityProfileLayer(0.0, 0.0, 0.0, -1.0 / 15000.0, 8.0 / 3.0));

	std::vector<double> wavelengths;
	std::vector<double> solar_irradiance;
	std::vector<double> rayleigh_scattering;
	std::vector<double> mie_scattering;
	std::vector<double> mie_extinction;
	std::vector<double> absorption_extinction;
	std::vector<double> ground_albedo;
	for(int l = kLambdaMin; l <= kLambdaMax; l += 10)
	{
		double lambda = static_cast<double>(l) * 1e-3;  // micro-meters
		double mie =
			kMieAngstromBeta / kMieScaleHeight * pow(lambda, -kMieAngstromAlpha);
		wavelengths.push_back(l);
		if(m_useConstantSolarSpectrum)
		{
			solar_irradiance.push_back(kConstantSolarIrradiance);
		}
		else
		{
			solar_irradiance.push_back(kSolarIrradiance[(l - kLambdaMin) / 10]);
		}
		rayleigh_scattering.push_back(kRayleigh * pow(lambda, -4));
		mie_scattering.push_back(mie * kMieSingleScatteringAlbedo);
		mie_extinction.push_back(mie);
		absorption_extinction.push_back(m_useOzone ?
			kMaxOzoneNumberDensity * kOzoneCrossSection[(l - kLambdaMin) / 10] :
			0.0);
		ground_albedo.push_back(kGroundAlbedo);
	}

	m_atmScatteringModel.reset(new AtmScatteringModel(wavelengths, solar_irradiance, m_sunAngularRadius,
		kBottomRadius, kTopRadius, { rayleigh_layer }, rayleigh_scattering,
		{ mie_layer }, mie_scattering, mie_extinction, kMiePhaseFunctionG,
		ozone_density, absorption_extinction, ground_albedo, max_sun_zenith_angle,
		m_lengthUnitInMeters, m_luminanceType == PRECOMPUTED ? 15 : 3,
		m_useCombinedTextures, m_useHalfPrecision));
	m_atmScatteringModel->Init();

	double whitePointR = 1.0f;
	double whitePointG = 1.0f;
	double whitePointB = 1.0f;

	if(m_useWhiteBalance)
	{
		AtmScatteringModel::ConvertSpectrumToLinearSrgb(wavelengths, solar_irradiance,
			&whitePointR, &whitePointG, &whitePointB);
		double whitePointAvg = (whitePointR + whitePointG + whitePointB) / 3.0;
		whitePointR /= whitePointAvg;
		whitePointG /= whitePointAvg;
		whitePointB /= whitePointAvg;
	}

	m_atmScatteringParam.m_whitePoint = glm::vec3((float)whitePointR, (float)whitePointG, (float)whitePointB);

	/*
	<p>Then, it creates and compiles the vertex and fragment shaders used to render
	our demo scene, and link them with the <code>AtmScatteringModel</code>'s atmosphere shader
	to get the final scene rendering program:
	*/
	/*
	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	const char* const vertex_shader_source = m_vertexShaderSource.c_str();
	glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
	glCompileShader(vertex_shader);

	const std::string fragment_shader_str =
		"#version 330\n" +
		std::string(m_luminanceType != NONE ? "#define USE_LUMINANCE\n" : "") +
		"const float kLengthUnitInMeters = " +
		std::to_string(kLengthUnitInMeters) + ";\n" +
		demo_glsl;
	const char* fragment_shader_source = fragment_shader_str.c_str();
	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
	glCompileShader(fragment_shader);

	if(program_ != 0)
	{
		glDeleteProgram(program_);
	}
	program_ = glCreateProgram();
	glAttachShader(program_, vertex_shader);
	glAttachShader(program_, fragment_shader);
	glAttachShader(program_, m_atmScatteringModel->GetShader());
	glLinkProgram(program_);
	glDetachShader(program_, vertex_shader);
	glDetachShader(program_, fragment_shader);
	glDetachShader(program_, m_atmScatteringModel->GetShader());
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	/*
	<p>Finally, it sets the uniforms of this program that can be set once and for
	all (in our case this includes the <code>AtmScatteringModel</code>'s texture uniforms,
	because our demo app does not have any texture of its own):
	*/
	/*
	glUseProgram(program_);
	model_->SetProgramUniforms(program_, 0, 1, 2, 3);
	double white_point_r = 1.0;
	double white_point_g = 1.0;
	double white_point_b = 1.0;
	if(m_useWhiteBalance)
	{
		AtmScatteringModel::ConvertSpectrumToLinearSrgb(wavelengths, solar_irradiance,
			&white_point_r, &white_point_g, &white_point_b);
		double white_point = (white_point_r + white_point_g + white_point_b) / 3.0;
		white_point_r /= white_point;
		white_point_g /= white_point;
		white_point_b /= white_point;
	}
	glUniform3f(glGetUniformLocation(program_, "white_point"),
		white_point_r, white_point_g, white_point_b);
	glUniform3f(glGetUniformLocation(program_, "earth_center"),
		0.0, 0.0, -kBottomRadius / kLengthUnitInMeters);
	glUniform2f(glGetUniformLocation(program_, "sun_size"),
		tan(kSunAngularRadius),
		cos(kSunAngularRadius));
	*/
	// This sets 'view_from_clip', which only depends on the window size.
	//HandleReshapeEvent(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
}

void AtmScatteringPass::updateAtmScatteringData(const AtmosphericScatteringData &p_atmScatteringData)
{
	m_sunAngularRadius = p_atmScatteringData.m_sunSize / 2.0;

	m_atmosphereParam.m_sunAngularRadius = (float)m_sunAngularRadius;
	m_atmosphereParam.m_bottomRadius = p_atmScatteringData.m_atmosphereBottomRadius;
	m_atmosphereParam.m_topRadius = p_atmScatteringData.m_atmosphereTopRadius;
	m_atmosphereParam.m_groundAlbedo = p_atmScatteringData.m_planetGroundColor;

	m_atmScatteringParam.m_atmosphereParam = m_atmosphereParam;
	m_atmScatteringParam.m_earthCenter = p_atmScatteringData.m_planetCenterPosition / (float)m_lengthUnitInMeters;
	m_atmScatteringParam.m_sunSize = glm::vec2(tan((float)m_sunAngularRadius), cos((float)m_sunAngularRadius));

	//initModel();
}
