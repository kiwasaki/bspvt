#ifndef _TGALOADER_HPP_
#define _TGALOADER_HPP_

#include <iostream>
#include <fstream>
#include <bitset>
#include <vector>
#include <array>
#include <atomic>
#include <cmath>
#include"../parallel/parallel.hpp"

#undef LoadImage

//////////////////////////////////////////////////////////////////////////
//　　TGALoader class
//////////////////////////////////////////////////////////////////////////
class TGAImage
{	
public:
    //unsigned int ID;
	TGAImage() = default;
    ~TGAImage() = default;
    void LoadImage(const char *filename);
	void PassGaussianFilter(const float sigma_s, const float sigma_e, const uint8_t div);
	void PassGaussianFilter_with_margin(const float sigma_s, const float sigma_e, const uint8_t div, const float margin_scale);
	void PassGaussianFilterLOD_with_margin(const float margin_scale, const std::string filename);
	void LoadTex_with_margin(const char *filename);
	const std::array<float, 3> sampleLOD(const float sigma, const float u, const float v) const;
	const std::array<float, 3> sampleLOD_with_margin(const float LOD, float u, float v) const;
	inline const unsigned int w() const { return m_width; }
	inline const unsigned int h() const { return m_height; }
	inline const unsigned int margin_w() const { return m_margin_width; }
	inline const unsigned int margin_h() const { return m_margin_height; }
	inline const float &LOD(const int i, const int j) const {
		if(m_imageDataLOD[i] == nullptr){
			std::cerr << "m_imageDataLOD[" << i << "] is null!\n";
			exit(-1);
		}
		return m_imageDataLOD[i][j];
	}
	inline const float &LOD_with_margin(const int i, const int j) const {
		if(m_imageDataLOD_with_margin[i] == nullptr){
			std::cerr << "m_imageDataLOD_with_margin[" << i << "] is null!\n";
			exit(-1);
		}
		return m_imageDataLOD_with_margin[i][j];
	}

	const int operator[](const int i) const { return int(m_imageData[i]); }
	const std::unique_ptr< unsigned char [] >& data() const { return m_imageData; }

private:
    std::unique_ptr<unsigned char []> m_imageData;
    std::vector<std::unique_ptr<float []>> m_imageDataLOD;
	std::vector<std::unique_ptr<float []>> m_imageDataLOD_with_margin;
    unsigned int m_width;
    unsigned int m_height;
    unsigned int m_margin_width;
    unsigned int m_margin_height;
	unsigned int m_imageSize;
	float m_margin_min;
	float m_margin_max;
	unsigned char m_bytePerPixel;
	std::unique_ptr<float []> m_sigmaArray;
	std::unique_ptr<float []> m_sigmaArray_with_margin;
};

#include"tgaloader-impl.hpp"

#endif