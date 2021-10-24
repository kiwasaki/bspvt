#pragma once
#ifndef _LTC_HPP_
#define _LTC_HPP_

#define GLM_ENABLE_EXPERIMENTAL
#define _USE_MATH_DEFINES
#define print_ltc(x) std::cout << #x << ": "  << x << std::endl;

namespace ltc
{
	template < class T >
	class LTC
	{
	public:
		static constexpr uint16_t m_tabSize = 64;

		//コンストラクタ
		LTC( const glm::mat3 (&tabM)[ m_tabSize * m_tabSize ], const glm::mat3 (&tabMinv)[m_tabSize * m_tabSize], const T (&tabAmplitude)[m_tabSize * m_tabSize], const T alpha) : m_tabM( tabM ), m_tabMinv(tabMinv), m_tabAmplitude(tabAmplitude), m_alpha(alpha), m_alpha_index(uint16_t(std::sqrt(m_alpha) * T(m_tabSize - 1)))
		{
		}

		//set関数
		void setAlpha(const T alpha)
		{
			m_alpha = alpha;
			m_alpha_index = uint16_t(std::sqrt(m_alpha) * T(m_tabSize - 1));
		}

		//単位ベクトルwのθ角からm_tabMinvとm_tabAmplitudeの線形補間された値を得る
		const std::tuple< glm::mat3, T > interpolate_Minv_and_Amplitude( const T theta, const T alpha ) const
		{
			const uint16_t max_ind = uint16_t(m_tabSize - 1);

			const T alpha_ind = std::clamp(T(std::sqrt(alpha) * T(max_ind)), T(0), T(max_ind));
			const uint16_t alpha0 = std::clamp(uint16_t(alpha_ind), uint16_t(0), uint16_t(max_ind));
			const uint16_t alpha1 = std::clamp(uint16_t(alpha_ind + 1.f), uint16_t(0), uint16_t(max_ind));
			const T alpha1_w = alpha_ind - T(alpha0);
			const T alpha0_w = T(1.f - alpha1_w);

			const T theta_ind = std::clamp(T(theta * (2.f / T(M_PI)) * T(max_ind)), T(0), T(max_ind));
			const uint16_t theta0 = std::clamp(uint16_t(theta_ind), uint16_t(0), uint16_t(max_ind));
			const uint16_t theta1 = std::clamp(uint16_t(theta_ind + 1.f), uint16_t(0), uint16_t(max_ind));
			const T theta1_w = theta_ind - T(theta0);
			const T theta0_w = T(1.f - theta1_w);

			const uint16_t t0a0 = theta0 * m_tabSize + alpha0;
			const uint16_t t1a0 = theta1 * m_tabSize + alpha0;
			const uint16_t t0a1 = theta0 * m_tabSize + alpha1;
			const uint16_t t1a1 = theta1 * m_tabSize + alpha1;

			const glm::mat3 Minv_alpha0 = m_tabMinv[t0a0] * theta0_w + m_tabMinv[t1a0] * theta1_w;
			const glm::mat3 Minv_alpha1 = m_tabMinv[t0a1] * theta0_w + m_tabMinv[t1a1] * theta1_w;
			const glm::mat3 Minv = Minv_alpha0 * alpha0_w + Minv_alpha1 * alpha1_w;

			const T Amplitude_alpha0 = m_tabAmplitude[t0a0] * theta0_w + m_tabAmplitude[t1a0] * theta1_w;
			const T Amplitude_alpha1 = m_tabAmplitude[t0a1] * theta0_w + m_tabAmplitude[t1a1] * theta1_w;
			const T Amplitude = Amplitude_alpha0 * alpha0_w + Amplitude_alpha1 * alpha1_w;

			return std::make_tuple(Minv, Amplitude);
		}

		//単位ベクトルwのθ角からm_tabMinvの線形補間された値を得る
		const glm::mat3 interpolate_Minv( const T theta, const T alpha ) const
		{
			const uint16_t max_ind = uint16_t(m_tabSize - 1);

			const T alpha_ind = std::clamp(T(std::sqrt(alpha) * T(max_ind)), T(0), T(max_ind));
			const uint16_t alpha0 = std::clamp(uint16_t(alpha_ind), uint16_t(0), uint16_t(max_ind));
			const uint16_t alpha1 = std::clamp(uint16_t(alpha_ind + 1.f), uint16_t(0), uint16_t(max_ind));
			const T alpha1_w = alpha_ind - T(alpha0);
			const T alpha0_w = T(1.f - alpha1_w);

			const T theta_ind = std::clamp(T(theta * (2.f / T(M_PI)) * T(max_ind)), T(0), T(max_ind));
			const uint16_t theta0 = std::clamp(uint16_t(theta_ind), uint16_t(0), uint16_t(max_ind));
			const uint16_t theta1 = std::clamp(uint16_t(theta_ind + 1.f), uint16_t(0), uint16_t(max_ind));
			const T theta1_w = theta_ind - T(theta0);
			const T theta0_w = T(1.f - theta1_w);

			const uint16_t t0a0 = theta0 * m_tabSize + alpha0;
			const uint16_t t1a0 = theta1 * m_tabSize + alpha0;
			const uint16_t t0a1 = theta0 * m_tabSize + alpha1;
			const uint16_t t1a1 = theta1 * m_tabSize + alpha1;

			const glm::mat3 Minv_alpha0 = m_tabMinv[t0a0] * theta0_w + m_tabMinv[t1a0] * theta1_w;
			const glm::mat3 Minv_alpha1 = m_tabMinv[t0a1] * theta0_w + m_tabMinv[t1a1] * theta1_w;

			return Minv_alpha0 * alpha0_w + Minv_alpha1 * alpha1_w;
		}

		//
		const glm::mat3 interpolate_M( const T& theta, const T& alpha ) const
		{
			const uint16_t max_ind = uint16_t(m_tabSize - 1);

			const T alpha_ind = std::clamp(T(std::sqrt(alpha) * T(max_ind)), T(0), T(max_ind));
			const uint16_t alpha0 = std::clamp(uint16_t(alpha_ind), uint16_t(0), uint16_t(max_ind));
			const uint16_t alpha1 = std::clamp(uint16_t(alpha_ind + 1.f), uint16_t(0), uint16_t(max_ind));
			const T alpha1_w = alpha_ind - T(alpha0);
			const T alpha0_w = T(1.f - alpha1_w);

			const T theta_ind = std::clamp(T(theta * (2.f / T(M_PI)) * T(max_ind)), T(0), T(max_ind));
			const uint16_t theta0 = std::clamp(uint16_t(theta_ind), uint16_t(0), uint16_t(max_ind));
			const uint16_t theta1 = std::clamp(uint16_t(theta_ind + 1.f), uint16_t(0), uint16_t(max_ind));
			const T theta1_w = theta_ind - T(theta0);
			const T theta0_w = T(1.f - theta1_w);

			const uint16_t t0a0 = theta0 * m_tabSize + alpha0;
			const uint16_t t1a0 = theta1 * m_tabSize + alpha0;
			const uint16_t t0a1 = theta0 * m_tabSize + alpha1;
			const uint16_t t1a1 = theta1 * m_tabSize + alpha1;

			const glm::mat3 Minv_alpha0 = m_tabM[ t0a0 ] * theta0_w + m_tabM[ t1a0 ] * theta1_w;
			const glm::mat3 Minv_alpha1 = m_tabM[ t0a1 ] * theta0_w + m_tabM[ t1a1 ] * theta1_w;

			return Minv_alpha0 * alpha0_w + Minv_alpha1 * alpha1_w;
		}

	private:
		const glm::mat3 ( &m_tabM )[ m_tabSize * m_tabSize ];
		const glm::mat3 (&m_tabMinv)[m_tabSize * m_tabSize];
		const T (&m_tabAmplitude)[m_tabSize * m_tabSize];
		T m_alpha; //0 < alpha <= 1
		uint16_t m_alpha_index;
	};
}


#endif //_LTC_HPP_
