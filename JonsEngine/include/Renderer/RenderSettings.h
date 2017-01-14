#pragma once

namespace JonsEngine
{
	struct RenderSettings
	{
		typedef float AutoExposureRate;
		constexpr AutoExposureRate static DefaultAutoExposureRate() { return 1.0f; }


		enum class ShadowResolution
		{
			Resolution_1024,
			Resolution_2048,
			Resolution_4092
		} mShadowResolution;

		enum class ShadowReadbackLatency
		{
			Latency_0,
			Latency_1,
			Latency_2,
			Latency_3
		} mShadowReadbackLatency;

		enum class ShadowFiltering
		{
			PCF_2x2,
			PCF_3x3,
			PCF_5x5,
			PCF_7x7
		} mShadowFiltering;

		enum class Anisotropic
		{
			X1,
			X2,
			X4,
			X8,
			X16
		} mAnisotropicFiltering;

		enum class AntiAliasing
		{
			None,
			Fxaa
		} mAntiAliasing;

		enum class ToneMappingAlghorithm
		{
			None,
			FilmicU2
		} mToneMapping;

		AutoExposureRate mAutoExposureRate;
		bool mBloomEnabled;
		bool mSSAOEnabled;


		RenderSettings();
	};

	inline RenderSettings::RenderSettings() :
		mShadowResolution(ShadowResolution::Resolution_2048),
		mShadowReadbackLatency(ShadowReadbackLatency::Latency_2),
		mShadowFiltering(ShadowFiltering::PCF_2x2),
		mAnisotropicFiltering(Anisotropic::X16),
		mAntiAliasing(AntiAliasing::Fxaa),
		mToneMapping(ToneMappingAlghorithm::FilmicU2),
		mAutoExposureRate(DefaultAutoExposureRate()),
		mBloomEnabled(true),
		mSSAOEnabled(true)
	{
	}
}