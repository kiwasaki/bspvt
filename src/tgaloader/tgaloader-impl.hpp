#include"tgaloader.hpp"

#define print(x) std::cout << #x << " = " << x << std::endl

//コンストラクタ
//TGAファイルの読み込み
void TGAImage::LoadImage(const char *filename)
{
    FILE *fp;
    unsigned char header[18]; 
    unsigned int temp;

    //　ファイルを開く
    //if ( (fopen_s(&fp, filename, "rb")) != 0 )
    fp = fopen( filename, "rb" );
    if( not( fp ) )
    {
        std::cout << "Error : 指定したファイルを開けませんでした\n";
        std::cout << "File Name : " << filename << std::endl;
        std::exit(-1);
    }

    //　ヘッダー情報の読み込み
    fread(header, 1, sizeof(header), fp);
    
    //　幅と高さを決める
    m_width = header[13] * 256 + header[12];
    m_height = header[15] * 256 + header[14];
    
    //　ビットの深さ
    unsigned int bpp = header[16];

	std::bitset<8> discripter(header[17]);
	bool bit4 = discripter[4];
	bool bit5 = discripter[5];
	
    //　1ピクセル当たりのバイト数を決定
    m_bytePerPixel = bpp / 8;

    //　データサイズの決定
    m_imageSize = m_width * m_height * m_bytePerPixel;

	m_imageData = std::make_unique<unsigned char []>(m_imageSize);

	if(bit4 == 0 && bit5 == 1){
		//　テクセルデータを一気に読み取り
		fread(m_imageData.get(), 1, m_imageSize, fp);

		//　BGR(A)をRGB(A)にコンバート
		for(int i = 0; i < int(m_imageSize); i += m_bytePerPixel){
		    temp = m_imageData[i];
		    m_imageData[i + 0] = m_imageData[i + 2];
		    m_imageData[i + 2] = temp;
		}
	}
	else{
		//　メモリを確保
		std::unique_ptr<unsigned char []> imageData = std::make_unique<unsigned char []>(m_imageSize);

		//　テクセルデータを一気に読み取り
		fread(imageData.get(), 1, m_imageSize, fp);

		//　BGR(A)をRGB(A)にコンバート
		for(int i = 0; i < int(m_imageSize); i += m_bytePerPixel){
		    temp = imageData[i];
		    imageData[i + 0] = imageData[i + 2];
		    imageData[i + 2] = temp;
		}

		//	左上原点のUV座標にデータを並び替え
		const int u_term = (bit4) ? (m_width - 1) : 0;
		const int u_coef = (bit4) ? -1 : 1;
		const int v_term = (bit5) ? 0 : (m_height - 1);
		const int v_coef = (bit5) ? 1 : -1;

		for(int v = 0; v < m_height; ++v){
			for(int u = 0; u < m_width; ++u){
				m_imageData[(u_term + u_coef * u + (v_term + v_coef * v) * m_width) * 3 + 0] = imageData[(u + v * m_width) * 3 + 0];
				m_imageData[(u_term + u_coef * u + (v_term + v_coef * v) * m_width) * 3 + 1] = imageData[(u + v * m_width) * 3 + 1];
				m_imageData[(u_term + u_coef * u + (v_term + v_coef * v) * m_width) * 3 + 2] = imageData[(u + v * m_width) * 3 + 2];
			}
		}
	}

    //　ファイルを閉じる
    fclose(fp);
}

void TGAImage::PassGaussianFilter(const float sigma_s, const float sigma_e, const uint8_t div)
{
	if(div <= 1){
		std::cerr << "div is not less than 2!\n";
		exit(-1);
	}

	m_sigmaArray = std::make_unique<float []>(div);
	
	for(int level = 0; level < div; ++level){
		std::cout << "now filtering... level = " << level << " / " << int(div - 1) << std::endl;
		const float sigma = sigma_s + ((sigma_e - sigma_s) / float(div - 1)) * float(level);
		const float sigma2 = sigma * sigma;
		int size = int(sigma * 3.f);
		if(size < 3) size = 3;
		const int offset = (size + 1) / 2;
		m_sigmaArray[level] = sigma;
		std::unique_ptr<float []> filter = std::make_unique<float []>(offset);
		for(int i = 0; i < offset; ++i){//calc filter coef
			filter[i] = 1.f / sqrt(2.f * M_PI * sigma2) * std::exp(-(i * i) / (2.f * sigma2));
		}

		std::unique_ptr<float []> image0 = std::make_unique<float []>(m_imageSize);
		for(int h = 0; h < m_height; ++h){//x方向のblur
			for(int w = 0; w < m_width; ++w){
				image0[(w + h * m_width) * 3 + 0] = 0.f;
				image0[(w + h * m_width) * 3 + 1] = 0.f;
				image0[(w + h * m_width) * 3 + 2] = 0.f;
				float sum = 0.f;
				for(int dx = -(offset - 1); dx <= offset - 1; ++dx){//フィルタリング
					const int x = w + dx;
					if(0 <= x && x < m_width){
						const int index = std::abs(dx);
						image0[(w + h * m_width) * 3 + 0] += float(m_imageData[(x + h * m_width) * 3 + 0]) * filter[index];
						image0[(w + h * m_width) * 3 + 1] += float(m_imageData[(x + h * m_width) * 3 + 1]) * filter[index];
						image0[(w + h * m_width) * 3 + 2] += float(m_imageData[(x + h * m_width) * 3 + 2]) * filter[index];
						sum += filter[index];
					}
				}
				image0[(w + h * m_width) * 3 + 0] /= sum;
				image0[(w + h * m_width) * 3 + 1] /= sum;
				image0[(w + h * m_width) * 3 + 2] /= sum;
			}
		}

		std::unique_ptr<float []> image1 = std::make_unique<float []>(m_imageSize);
		for(int h = 0; h < m_height; ++h){//y方向のblur
			for(int w = 0; w < m_width; ++w){
				image1[(w + h * m_width) * 3 + 0] = 0.f;
				image1[(w + h * m_width) * 3 + 1] = 0.f;
				image1[(w + h * m_width) * 3 + 2] = 0.f;
				float sum = 0.f;
				for(int dy = -(offset - 1); dy <= offset - 1; ++dy){//フィルタリング
					const int y = h + dy;
					if(0 <= y && y < m_height){
						const int index = std::abs(dy);
						image1[(w + h * m_width) * 3 + 0] += float(image0[(w + y * m_width) * 3 + 0]) * filter[index];
						image1[(w + h * m_width) * 3 + 1] += float(image0[(w + y * m_width) * 3 + 1]) * filter[index];
						image1[(w + h * m_width) * 3 + 2] += float(image0[(w + y * m_width) * 3 + 2]) * filter[index];
						sum += filter[index];
					}
				}
				image1[(w + h * m_width) * 3 + 0] /= sum;
				image1[(w + h * m_width) * 3 + 1] /= sum;
				image1[(w + h * m_width) * 3 + 2] /= sum;
			}
		}

		m_imageDataLOD.emplace_back(std::move(image1));
	}
}

//void TGAImage::PassGaussianFilter_with_margin(const float sigma_s, const float sigma_e, const uint8_t div, const float margin_scale)
//{
//	if(div <= 1){
//		std::cerr << "div is not less than 2!\n";
//		exit(-1);
//	}
//
//	std::ofstream fout( "PassGaussianFilter_with_margin.tex", std::ios::binary );
//    if( fout.fail() ) {
//        std::cerr << "cannot open file " << "PassGaussianFilter_with_margin.tex" << "\n";
//        std::exit( EXIT_FAILURE );
//    }
//
//	int tex_size = int(margin_scale * m_height);
//	if(tex_size % 2 == 1) tex_size += 1;//margin後のテクスチャのサイズ
//	m_margin_width  = tex_size;
//	m_margin_height = tex_size;
//	const float scale = float(tex_size) / float(m_height);
//	const float margin = (scale - 1.f) / 2.f;//0 - margin <= (u, v) <= 1 + margin
//	m_margin_min = -margin;
//	m_margin_max = 1.f + margin;
//
//	m_sigmaArray_with_margin = std::make_unique<float []>(div);
//
//    //save header information
//	const int texture_size = tex_size * tex_size * 3;
//    fout.write( ( char* ) &texture_size, sizeof( int ) );//テクスチャのデータサイズ
//    fout.write( ( char* ) &m_margin_width , sizeof( unsigned int ) );//テクスチャのwidth
//    fout.write( ( char* ) &m_margin_height, sizeof( unsigned int ) );//テクスチャのheight
//    fout.write( ( char* ) &div, sizeof( uint8_t ) );//テクスチャの数
//    fout.write( ( char* ) &m_margin_min, sizeof( float ) );//
//    fout.write( ( char* ) &m_margin_max, sizeof( float ) );//
//	
//	for(int level = 0; level < div; ++level){
//		std::cout << "now filtering with margin... level = " << level << " / " << int(div - 1) << std::endl;
//		const float sigma = sigma_s + ((sigma_e - sigma_s) / float(div - 1)) * float(level);
//		const float sigma2 = sigma * sigma;
//		const float gauss_coef = 1.f / sqrt(2.f * M_PI * sigma2);
//		const float gauss_coef2 = 1.f / (2.f * M_PI * sigma2);
//		int size = int(sigma * 3.f);
//		if(size < 3) size = 3;
//		const int offset = (size + 1) / 2;
//		m_sigmaArray_with_margin[level] = sigma;
//		std::unique_ptr<float []> filter = std::make_unique<float []>(offset);
//		for(int i = 0; i < offset; ++i){//calc filter coef
//			filter[i] = gauss_coef * std::exp(-(i * i) / (2.f * sigma2));
//		}
//
//		std::unique_ptr<float []> image0 = std::make_unique<float []>(tex_size * tex_size * int(m_bytePerPixel));
//		for(int h = 0; h < tex_size; ++h){//x方向のblur
//			const float v = (float(h) * (2.f * margin + 1.f)) / float(tex_size - 1) - margin;
//			for(int w = 0; w < tex_size; ++w){
//				image0[(w + h * tex_size) * 3 + 0] = 0.f;
//				image0[(w + h * tex_size) * 3 + 1] = 0.f;
//				image0[(w + h * tex_size) * 3 + 2] = 0.f;
//				float sum = 0.f;
//				const float u = (float(w) * (2.f * margin + 1.f)) / float(tex_size - 1) - margin;
//				if(0.f <= u && u <= 1.f && 0.f <= v && v <= 1.f){//普通にフィルタリング
//					const int oh = int(v * float(m_height - 1) + 0.5f);//0 ~ m_height - 1
//					const int ow = int(u * float(m_width  - 1) + 0.5f);
//					for(int dx = -(offset - 1); dx <= offset - 1; ++dx){//フィルタリング
//						const int x = ow + dx;
//						if(0 <= x && x < m_width){
//							const int index = std::abs(dx);
//							image0[(w + h * tex_size) * 3 + 0] += float(m_imageData[(x + oh * m_width) * 3 + 0]) * filter[index];
//							image0[(w + h * tex_size) * 3 + 1] += float(m_imageData[(x + oh * m_width) * 3 + 1]) * filter[index];
//							image0[(w + h * tex_size) * 3 + 2] += float(m_imageData[(x + oh * m_width) * 3 + 2]) * filter[index];
//							sum += filter[index];
//						}
//					}
//				}
//				else{//margin領域について
//					int ow;
//					int oh;
//					int kernel_size;
//					int offset_min_x;
//					int offset_max_x;
//					int offset_min_y;
//					int offset_max_y;
//					if(u < 0.f) ow = -int(-u * float(m_width - 1) + 0.5f);
//					else ow = int(u * float(m_width - 1) + 0.5f);
//					if(v < 0.f) oh = -int(-v * float(m_height - 1) + 0.5f);
//					else oh = int(v * float(m_height - 1) + 0.5f);
//					int kernel_w;
//					int kernel_h;
//
//					if(ow < 0) kernel_w = std::abs(ow);
//					else if(m_width - 1 < ow) kernel_w = ow - (m_width - 1);
//					else kernel_w = 0;
//
//					if(oh < 0) kernel_h = std::abs(oh);
//					else if(m_height - 1 < oh) kernel_h = oh - (m_height - 1);
//					else kernel_h = 0;
//
//					if(kernel_w < kernel_h) kernel_size = kernel_h;
//					else kernel_size = kernel_w;
//
//					if(kernel_size < 1) kernel_size = 1;
//
//					if(ow < 0){
//						offset_min_x = -ow;
//						offset_max_x = kernel_size;
//					}
//					else{
//						offset_min_x = -kernel_size;
//						offset_max_x = m_width - 1 - ow;
//					}
//
//					if(oh < 0){
//						offset_min_y = -oh;
//						offset_max_y = kernel_size;
//					}
//					else{
//						offset_min_y = -kernel_size;
//						offset_max_y = m_height - 1 - oh;
//					}
//					/*std::cout << "kernel_size: " << kernel_size << std::endl;
//					std::cout << "ow: " << ow << std::endl;
//					std::cout << "oh: " << oh << std::endl;
//					std::cout << "offset_min_x: " << offset_min_x << std::endl;
//					std::cout << "offset_max_x: " << offset_max_x << std::endl;
//					std::cout << "offset_min_y: " << offset_min_y << std::endl;
//					std::cout << "offset_max_y: " << offset_max_y << std::endl;
//					std::cout << std::endl;*/
//
//					for(int dy = offset_min_y; dy <= offset_max_y; ++dy){//フィルタリング
//						const int y = oh + dy;
//						for(int dx = offset_min_x; dx <= offset_max_x; ++dx){
//							const int x = ow + dx;
//							if(0 <= x && x < m_width && 0 <= y && y < m_height){
//								float filter_coef = gauss_coef2 * std::exp(-(dx * dx + dy * dy) / (2.f * sigma2));
//								if(filter_coef == 0.f) filter_coef = 1e-8f;
//								image0[(w + h * tex_size) * 3 + 0] += float(m_imageData[(x + y * m_width) * 3 + 0]) * filter_coef;
//								image0[(w + h * tex_size) * 3 + 1] += float(m_imageData[(x + y * m_width) * 3 + 1]) * filter_coef;
//								image0[(w + h * tex_size) * 3 + 2] += float(m_imageData[(x + y * m_width) * 3 + 2]) * filter_coef;
//								sum += filter_coef;
//								//std::cout << "sum: " << sum << std::endl;
//							}
//						}
//					}
//					/*std::cout << "image0[(w + h * tex_size) * 3 + 0]: " << image0[(w + h * tex_size) * 3 + 0] / sum << std::endl;
//					std::cout << "image0[(w + h * tex_size) * 3 + 1]: " << image0[(w + h * tex_size) * 3 + 1] / sum << std::endl;
//					std::cout << "image0[(w + h * tex_size) * 3 + 2]: " << image0[(w + h * tex_size) * 3 + 2] / sum << std::endl;
//					std::cout << std::endl;*/
//				}
//				if(sum != 0.f){
//					image0[(w + h * tex_size) * 3 + 0] /= sum;
//					image0[(w + h * tex_size) * 3 + 1] /= sum;
//					image0[(w + h * tex_size) * 3 + 2] /= sum;
//				}
//			}
//		}
//
//		std::unique_ptr<float []> image1 = std::make_unique<float []>(tex_size * tex_size * int(m_bytePerPixel));
//		for(int h = 0; h < tex_size; ++h){//y方向のblur
//			const float v = (float(h) * (2.f * margin + 1.f)) / float(tex_size - 1) - margin;
//			for(int w = 0; w < tex_size; ++w){
//				image1[(w + h * tex_size) * 3 + 0] = 0.f;
//				image1[(w + h * tex_size) * 3 + 1] = 0.f;
//				image1[(w + h * tex_size) * 3 + 2] = 0.f;
//				float sum = 0.f;
//				const float u = (float(w) * (2.f * margin + 1.f)) / float(tex_size - 1) - margin;
//				if(0.f <= u && u <= 1.f && 0.f <= v && v <= 1.f){//普通にフィルタリング
//					for(int dy = -(offset - 1); dy <= offset - 1; ++dy){//フィルタリング
//						const int y = h + dy;
//						const float yy = (float(y) * (2.f * margin + 1.f)) / float(tex_size - 1) - margin;
//						if(0.f <= yy && yy <= 1.f){
//							const int index = std::abs(dy);
//							image1[(w + h * tex_size) * 3 + 0] += float(image0[(w + y * tex_size) * 3 + 0]) * filter[index];
//							image1[(w + h * tex_size) * 3 + 1] += float(image0[(w + y * tex_size) * 3 + 1]) * filter[index];
//							image1[(w + h * tex_size) * 3 + 2] += float(image0[(w + y * tex_size) * 3 + 2]) * filter[index];
//							sum += filter[index];
//						}
//					}
//					if(sum != 0.f){
//						image1[(w + h * tex_size) * 3 + 0] /= sum;
//						image1[(w + h * tex_size) * 3 + 1] /= sum;
//						image1[(w + h * tex_size) * 3 + 2] /= sum;
//					}
//				}
//				else{
//					image1[(w + h * tex_size) * 3 + 0] = float(image0[(w + h * tex_size) * 3 + 0]);
//					image1[(w + h * tex_size) * 3 + 1] = float(image0[(w + h * tex_size) * 3 + 1]);
//					image1[(w + h * tex_size) * 3 + 2] = float(image0[(w + h * tex_size) * 3 + 2]);
//				}
//			}
//		}
//
//		m_imageDataLOD_with_margin.emplace_back(std::move(image1));
//	}
//
//	for(const auto & tex : m_imageDataLOD_with_margin){
//		for(int i = 0, roop = tex_size * tex_size * 3; i < roop; i += 3){
//			const float r = tex[i + 0];
//			const float g = tex[i + 1];
//			const float b = tex[i + 2];
//			 fout.write( ( char* ) &r, sizeof( float ) );
//			 fout.write( ( char* ) &g, sizeof( float ) );
//			 fout.write( ( char* ) &b, sizeof( float ) );
//		}
//	}
//	for(int i = 0; i < div; ++i){
//		const float sigma_tmp = m_sigmaArray_with_margin[i];
//			fout.write( ( char* ) &sigma_tmp, sizeof( float ) );
//	}
//	fout.close();
//}

void TGAImage::PassGaussianFilter_with_margin(const float sigma_s, const float sigma_e, const uint8_t div, const float margin_scale)
{
	if(div <= 1){
		std::cerr << "div is not less than 2!\n";
		exit(-1);
	}

	std::ofstream fout( "PassGaussianFilter_with_margin.tex", std::ios::binary );
    if( fout.fail() ) {
        std::cerr << "cannot open file " << "PassGaussianFilter_with_margin.tex" << "\n";
        std::exit( EXIT_FAILURE );
    }

	//margin用
	const float max_sigma = (sigma_e < 30.f) ? 30.f : sigma_e;
	const float max_sigma2 = max_sigma * max_sigma;
	const float gauss_coef2 = 1.f / (2.f * M_PI * max_sigma2);
	int kernel_size_offset = int(max_sigma * 3.f);
	if(kernel_size_offset >= m_width) kernel_size_offset = m_width - 1;

	int tex_size = int(margin_scale * m_height);
	if(tex_size % 2 == 1) tex_size += 1;//margin後のテクスチャのサイズ
	m_margin_width  = tex_size;
	m_margin_height = tex_size;
	const float scale = float(tex_size) / float(m_height);
	const float margin = (scale - 1.f) / 2.f;//0 - margin <= (u, v) <= 1 + margin
	m_margin_min = -margin;
	m_margin_max = 1.f + margin;

	m_sigmaArray_with_margin = std::make_unique<float []>(div);

    //save header information
	const int texture_size = tex_size * tex_size * 3;
    fout.write( ( char* ) &texture_size, sizeof( int ) );//テクスチャのデータサイズ
    fout.write( ( char* ) &m_margin_width , sizeof( unsigned int ) );//テクスチャのwidth
    fout.write( ( char* ) &m_margin_height, sizeof( unsigned int ) );//テクスチャのheight
    fout.write( ( char* ) &div, sizeof( uint8_t ) );//テクスチャの数
    fout.write( ( char* ) &m_margin_min, sizeof( float ) );//
    fout.write( ( char* ) &m_margin_max, sizeof( float ) );//
	
	for(int level = 0; level < div; ++level){
		std::cout << "now filtering with margin... level = " << level << " / " << int(div - 1) << std::endl;
		const float sigma = sigma_s + ((sigma_e - sigma_s) / float(div - 1)) * float(level);
		const float sigma2 = sigma * sigma;
		const float gauss_coef = 1.f / sqrt(2.f * M_PI * sigma2);
		int size = int(sigma * 3.f * 2.f);
		if(size < 3) size = 3;
		if(size % 2 == 0) size += 1;
		const int offset = (size + 1) / 2;
		m_sigmaArray_with_margin[level] = sigma;
		std::unique_ptr<float []> filter = std::make_unique<float []>(offset);
		for(int i = 0; i < offset; ++i){//calc filter coef
			filter[i] = gauss_coef * std::exp(-(i * i) / (2.f * sigma2));
		}

		std::unique_ptr<float []> image0 = std::make_unique<float []>(tex_size * tex_size * int(m_bytePerPixel));
		for(int h = 0; h < tex_size; ++h){//x方向のblur
			const float v = (float(h) * (2.f * margin + 1.f)) / float(tex_size - 1) - margin;
			for(int w = 0; w < tex_size; ++w){
				image0[(w + h * tex_size) * 3 + 0] = 0.f;
				image0[(w + h * tex_size) * 3 + 1] = 0.f;
				image0[(w + h * tex_size) * 3 + 2] = 0.f;
				float sum = 0.f;
				const float u = (float(w) * (2.f * margin + 1.f)) / float(tex_size - 1) - margin;
				if(0.f <= u && u <= 1.f && 0.f <= v && v <= 1.f){//普通にフィルタリング
					const int oh = int(v * float(m_height - 1) + 0.5f);//0 ~ m_height - 1
					const int ow = int(u * float(m_width  - 1) + 0.5f);
					for(int dx = -(offset - 1); dx <= offset - 1; ++dx){//フィルタリング
						const int x = ow + dx;
						if(0 <= x && x < m_width){
							const int index = std::abs(dx);
							image0[(w + h * tex_size) * 3 + 0] += float(m_imageData[(x + oh * m_width) * 3 + 0]) * filter[index];
							image0[(w + h * tex_size) * 3 + 1] += float(m_imageData[(x + oh * m_width) * 3 + 1]) * filter[index];
							image0[(w + h * tex_size) * 3 + 2] += float(m_imageData[(x + oh * m_width) * 3 + 2]) * filter[index];
							sum += filter[index];
						}
					}
				}
				else{//margin領域について
					int ow;
					int oh;
					int kernel_size;
					int offset_min_x;
					int offset_max_x;
					int offset_min_y;
					int offset_max_y;
					if(u < 0.f) ow = -int(-u * float(m_width - 1) + 0.5f);
					else ow = int(u * float(m_width - 1) + 0.5f);
					if(v < 0.f) oh = -int(-v * float(m_height - 1) + 0.5f);
					else oh = int(v * float(m_height - 1) + 0.5f);
					int kernel_w;
					int kernel_h;

					if(ow < 0) kernel_w = std::abs(ow);
					else if(m_width - 1 < ow) kernel_w = ow - (m_width - 1);
					else kernel_w = 0;

					if(oh < 0) kernel_h = std::abs(oh);
					else if(m_height - 1 < oh) kernel_h = oh - (m_height - 1);
					else kernel_h = 0;

					if(kernel_w < kernel_h) kernel_size = kernel_h;
					else kernel_size = kernel_w;

					if(kernel_size < 1) kernel_size = 1;
					kernel_size += kernel_size_offset;
					
					if(ow < 0){
						offset_min_x = -ow;
						offset_max_x = kernel_size;
					}
					else{
						offset_min_x = -kernel_size;
						offset_max_x = m_width - 1 - ow;
					}

					if(oh < 0){
						offset_min_y = -oh;
						offset_max_y = kernel_size;
					}
					else{
						offset_min_y = -kernel_size;
						offset_max_y = m_height - 1 - oh;
					}

					for(int dy = offset_min_y; dy <= offset_max_y; ++dy){//フィルタリング
						const int y = oh + dy;
						for(int dx = offset_min_x; dx <= offset_max_x; ++dx){
							const int x = ow + dx;
							if(0 <= x && x < m_width && 0 <= y && y < m_height){
								float filter_coef = gauss_coef2 * std::exp(-(dx * dx + dy * dy) / (2.f * max_sigma2));
								//if(filter_coef == 0.f) filter_coef = 1e-8f;
								image0[(w + h * tex_size) * 3 + 0] += float(m_imageData[(x + y * m_width) * 3 + 0]) * filter_coef;
								image0[(w + h * tex_size) * 3 + 1] += float(m_imageData[(x + y * m_width) * 3 + 1]) * filter_coef;
								image0[(w + h * tex_size) * 3 + 2] += float(m_imageData[(x + y * m_width) * 3 + 2]) * filter_coef;
								sum += filter_coef;
							}
						}
					}
				}
				if(sum != 0.f){
					image0[(w + h * tex_size) * 3 + 0] /= sum;
					image0[(w + h * tex_size) * 3 + 1] /= sum;
					image0[(w + h * tex_size) * 3 + 2] /= sum;
				}
			}
		}

		std::unique_ptr<float []> image1 = std::make_unique<float []>(tex_size * tex_size * int(m_bytePerPixel));
		for(int h = 0; h < tex_size; ++h){//y方向のblur
			const float v = (float(h) * (2.f * margin + 1.f)) / float(tex_size - 1) - margin;
			for(int w = 0; w < tex_size; ++w){
				image1[(w + h * tex_size) * 3 + 0] = 0.f;
				image1[(w + h * tex_size) * 3 + 1] = 0.f;
				image1[(w + h * tex_size) * 3 + 2] = 0.f;
				float sum = 0.f;
				const float u = (float(w) * (2.f * margin + 1.f)) / float(tex_size - 1) - margin;
				if(0.f <= u && u <= 1.f && 0.f <= v && v <= 1.f){//普通にフィルタリング
					for(int dy = -(offset - 1); dy <= offset - 1; ++dy){//フィルタリング
						const int y = h + dy;
						const float yy = (float(y) * (2.f * margin + 1.f)) / float(tex_size - 1) - margin;
						if(0.f <= yy && yy <= 1.f){
							const int index = std::abs(dy);
							image1[(w + h * tex_size) * 3 + 0] += float(image0[(w + y * tex_size) * 3 + 0]) * filter[index];
							image1[(w + h * tex_size) * 3 + 1] += float(image0[(w + y * tex_size) * 3 + 1]) * filter[index];
							image1[(w + h * tex_size) * 3 + 2] += float(image0[(w + y * tex_size) * 3 + 2]) * filter[index];
							sum += filter[index];
						}
					}
					if(sum != 0.f){
						image1[(w + h * tex_size) * 3 + 0] /= sum;
						image1[(w + h * tex_size) * 3 + 1] /= sum;
						image1[(w + h * tex_size) * 3 + 2] /= sum;
					}
				}
				else{
					image1[(w + h * tex_size) * 3 + 0] = float(image0[(w + h * tex_size) * 3 + 0]);
					image1[(w + h * tex_size) * 3 + 1] = float(image0[(w + h * tex_size) * 3 + 1]);
					image1[(w + h * tex_size) * 3 + 2] = float(image0[(w + h * tex_size) * 3 + 2]);
				}
			}
		}

		m_imageDataLOD_with_margin.emplace_back(std::move(image1));
	}

	for(const auto & tex : m_imageDataLOD_with_margin){
		for(int i = 0, roop = tex_size * tex_size * 3; i < roop; i += 3){
			const float r = tex[i + 0];
			const float g = tex[i + 1];
			const float b = tex[i + 2];
			 fout.write( ( char* ) &r, sizeof( float ) );
			 fout.write( ( char* ) &g, sizeof( float ) );
			 fout.write( ( char* ) &b, sizeof( float ) );
		}
	}
	for(int i = 0; i < div; ++i){
		const float sigma_tmp = m_sigmaArray_with_margin[i];
			fout.write( ( char* ) &sigma_tmp, sizeof( float ) );
	}
	fout.close();
}

//
//void TGAImage::PassGaussianFilterLOD_with_margin(const float margin_scale)
//{
//	std::ofstream fout( "PassGaussianFilterLOD_with_margin.tex", std::ios::binary );
//    if( fout.fail() ) {
//        std::cerr << "cannot open file " << "PassGaussianFilter_with_margin.tex" << "\n";
//        std::exit( EXIT_FAILURE );
//    }
//
//	int tex_size = int(margin_scale * m_height);
//	if(tex_size % 2 == 1) tex_size += 1;//margin後のテクスチャのサイズ
//	m_margin_width  = tex_size;
//	m_margin_height = tex_size;
//	const float scale = float(tex_size) / float(m_height);
//	const float margin = (scale - 1.f) / 2.f;//0 - margin <= (u, v) <= 1 + margin
//	m_margin_min = -margin;
//	m_margin_max = 1.f + margin;
//	
//	const int max_LOD = int(log2(int(m_width)));
//
//	m_sigmaArray_with_margin = std::make_unique<float []>(max_LOD);
//
//    //save header information
//	const int texture_size = tex_size * tex_size * 3;
//    fout.write( ( char* ) &texture_size, sizeof( int ) );//テクスチャのデータサイズ
//    fout.write( ( char* ) &m_margin_width , sizeof( unsigned int ) );//テクスチャのwidth
//    fout.write( ( char* ) &m_margin_height, sizeof( unsigned int ) );//テクスチャのheight
//    fout.write( ( char* ) &max_LOD, sizeof( uint8_t ) );//テクスチャの数
//    fout.write( ( char* ) &m_margin_min, sizeof( float ) );//
//    fout.write( ( char* ) &m_margin_max, sizeof( float ) );//
//	
//	for(int level = 0; level < max_LOD; ++level){
//		std::cout << "now filtering with margin... LOD = " << level << " / " << int(max_LOD - 1) << std::endl;
//		const int size = int(pow(2, (level + 1))) + 1;
//		const int offset = (size + 1) / 2;
//		const float sigma = float(size) / 6.f;
//		const float sigma2 = sigma * sigma;
//		const float gauss_coef = 1.f / sqrt(2.f * M_PI * sigma2);
//		const float gauss_coef2 = 1.f / (2.f * M_PI * sigma2);
//		m_sigmaArray_with_margin[level] = sigma;
//		std::unique_ptr<float []> filter = std::make_unique<float []>(offset);
//		for(int i = 0; i < offset; ++i){//calc filter coef
//			filter[i] = gauss_coef * std::exp(-(i * i) / (2.f * sigma2));
//		}
//
//		std::unique_ptr<float []> image0 = std::make_unique<float []>(tex_size * tex_size * int(m_bytePerPixel));
//		for(int h = 0; h < tex_size; ++h){//x方向のblur
//			const float v = (float(h) * (2.f * margin + 1.f)) / float(tex_size - 1) - margin;
//			for(int w = 0; w < tex_size; ++w){
//				image0[(w + h * tex_size) * 3 + 0] = 0.f;
//				image0[(w + h * tex_size) * 3 + 1] = 0.f;
//				image0[(w + h * tex_size) * 3 + 2] = 0.f;
//				float sum = 0.f;
//				const float u = (float(w) * (2.f * margin + 1.f)) / float(tex_size - 1) - margin;
//				if(0.f <= u && u <= 1.f && 0.f <= v && v <= 1.f){//普通にフィルタリング
//					const int oh = int(v * float(m_height - 1) + 0.5f);//0 ~ m_height - 1
//					const int ow = int(u * float(m_width  - 1) + 0.5f);
//					for(int dx = -(offset - 1); dx <= offset - 1; ++dx){//フィルタリング
//						const int x = ow + dx;
//						if(0 <= x && x < m_width){
//							const int index = std::abs(dx);
//							image0[(w + h * tex_size) * 3 + 0] += float(m_imageData[(x + oh * m_width) * 3 + 0]) * filter[index];
//							image0[(w + h * tex_size) * 3 + 1] += float(m_imageData[(x + oh * m_width) * 3 + 1]) * filter[index];
//							image0[(w + h * tex_size) * 3 + 2] += float(m_imageData[(x + oh * m_width) * 3 + 2]) * filter[index];
//							sum += filter[index];
//						}
//					}
//				}
//				else{//margin領域について
//					int ow;
//					int oh;
//					int kernel_offset;
//					int offset_min_x;
//					int offset_max_x;
//					int offset_min_y;
//					int offset_max_y;
//					if(u < 0.f) ow = -int(-u * float(m_width - 1) + 0.5f);
//					else ow = int(u * float(m_width - 1) + 0.5f);
//					if(v < 0.f) oh = -int(-v * float(m_height - 1) + 0.5f);
//					else oh = int(v * float(m_height - 1) + 0.5f);
//
//					int kernel_w;
//					int kernel_h;
//
//					if(ow < 0) kernel_w = std::abs(ow);
//					else if(m_width - 1 < ow) kernel_w = ow - (m_width - 1);
//					else kernel_w = 0;
//
//					if(oh < 0) kernel_h = std::abs(oh);
//					else if(m_height - 1 < oh) kernel_h = oh - (m_height - 1);
//					else kernel_h = 0;
//
//					if(kernel_w < kernel_h) kernel_offset = kernel_h;
//					else kernel_offset = kernel_w;
//
//					if(kernel_offset < offset - 1) kernel_offset = offset - 1;
//					
//					if(ow < 0){
//						offset_min_x = -ow;
//						offset_max_x = +kernel_offset;
//					}
//					else if(m_width - 1 < ow){
//						offset_min_x = -kernel_offset;
//						offset_max_x = (m_width - 1) - ow;
//					}
//					else{
//						offset_min_x = (ow - kernel_offset < 0) ? -ow : -kernel_offset;
//						offset_max_x = (ow + kernel_offset > m_width - 1) ? (m_width - 1 - ow) : +kernel_offset;
//					}
//
//					if(oh < 0){
//						offset_min_y = -oh;
//						offset_max_y = +kernel_offset;
//					}
//					else if(m_height - 1 < oh){
//						offset_min_y = -kernel_offset;
//						offset_max_y = (m_height - 1) - oh;
//					}
//					else{
//						offset_min_y = (oh - kernel_offset < 0) ? -oh : -kernel_offset;
//						offset_max_y = (oh + kernel_offset > m_height - 1) ? (m_height - 1 - oh) : +kernel_offset;
//					}
//
//					/*if(ow < 0){
//						offset_min_x = 0;
//						offset_max_x = +kernel_offset;
//						ow = 0;
//					}
//					else if(m_width - 1 < ow){
//						offset_min_x = -kernel_offset;
//						offset_max_x = 0;
//						ow = m_width - 1;
//					}
//					else{
//						offset_min_x = (ow - kernel_offset < 0) ? -ow : -kernel_offset;
//						offset_max_x = (ow + kernel_offset > m_width - 1) ? (m_width - 1 - ow) : +kernel_offset;
//					}
//
//					if(oh < 0){
//						offset_min_y = 0;
//						offset_max_y = +kernel_offset;
//						oh = 0;
//					}
//					else if(m_height - 1 < oh){
//						offset_min_y = -kernel_offset;
//						offset_max_y = 0;
//						oh = m_height - 1;
//					}
//					else{
//						offset_min_y = (oh - kernel_offset < 0) ? -oh : -kernel_offset;
//						offset_max_y = (oh + kernel_offset > m_height - 1) ? (m_height - 1 - oh) : +kernel_offset;
//					}*/
//
//					for(int dy = offset_min_y; dy <= offset_max_y; ++dy){//フィルタリング
//						const int y = oh + dy;
//						for(int dx = offset_min_x; dx <= offset_max_x; ++dx){
//							const int x = ow + dx;
//							if(0 <= x && x < m_width && 0 <= y && y < m_height){
//								float filter_coef = gauss_coef2 * std::exp(-(dx * dx + dy * dy) / (2.f * sigma2));
//								//if(filter_coef == 0.f) filter_coef = 1e-8f;
//								image0[(w + h * tex_size) * 3 + 0] += float(m_imageData[(x + y * m_width) * 3 + 0]) * filter_coef;
//								image0[(w + h * tex_size) * 3 + 1] += float(m_imageData[(x + y * m_width) * 3 + 1]) * filter_coef;
//								image0[(w + h * tex_size) * 3 + 2] += float(m_imageData[(x + y * m_width) * 3 + 2]) * filter_coef;
//								sum += filter_coef;
//							}
//						}
//					}
//				}
//				if(sum != 0.f){
//					image0[(w + h * tex_size) * 3 + 0] /= sum;
//					image0[(w + h * tex_size) * 3 + 1] /= sum;
//					image0[(w + h * tex_size) * 3 + 2] /= sum;
//				}
//			}
//		}
//
//		std::unique_ptr<float []> image1 = std::make_unique<float []>(tex_size * tex_size * int(m_bytePerPixel));
//		for(int h = 0; h < tex_size; ++h){//y方向のblur
//			const float v = (float(h) * (2.f * margin + 1.f)) / float(tex_size - 1) - margin;
//			for(int w = 0; w < tex_size; ++w){
//				image1[(w + h * tex_size) * 3 + 0] = 0.f;
//				image1[(w + h * tex_size) * 3 + 1] = 0.f;
//				image1[(w + h * tex_size) * 3 + 2] = 0.f;
//				float sum = 0.f;
//				const float u = (float(w) * (2.f * margin + 1.f)) / float(tex_size - 1) - margin;
//				if(0.f <= u && u <= 1.f && 0.f <= v && v <= 1.f){//普通にフィルタリング
//					for(int dy = -(offset - 1); dy <= offset - 1; ++dy){//フィルタリング
//						const int y = h + dy;
//						const float yy = (float(y) * (2.f * margin + 1.f)) / float(tex_size - 1) - margin;
//						if(0.f <= yy && yy <= 1.f){
//							const int index = std::abs(dy);
//							image1[(w + h * tex_size) * 3 + 0] += float(image0[(w + y * tex_size) * 3 + 0]) * filter[index];
//							image1[(w + h * tex_size) * 3 + 1] += float(image0[(w + y * tex_size) * 3 + 1]) * filter[index];
//							image1[(w + h * tex_size) * 3 + 2] += float(image0[(w + y * tex_size) * 3 + 2]) * filter[index];
//							sum += filter[index];
//						}
//					}
//					if(sum != 0.f){
//						image1[(w + h * tex_size) * 3 + 0] /= sum;
//						image1[(w + h * tex_size) * 3 + 1] /= sum;
//						image1[(w + h * tex_size) * 3 + 2] /= sum;
//					}
//				}
//				else{
//					image1[(w + h * tex_size) * 3 + 0] = float(image0[(w + h * tex_size) * 3 + 0]);
//					image1[(w + h * tex_size) * 3 + 1] = float(image0[(w + h * tex_size) * 3 + 1]);
//					image1[(w + h * tex_size) * 3 + 2] = float(image0[(w + h * tex_size) * 3 + 2]);
//				}
//			}
//		}
//
//		m_imageDataLOD_with_margin.emplace_back(std::move(image1));
//	}
//
//	for(const auto & tex : m_imageDataLOD_with_margin){
//		for(int i = 0, roop = tex_size * tex_size * 3; i < roop; i += 3){
//			const float r = tex[i + 0];
//			const float g = tex[i + 1];
//			const float b = tex[i + 2];
//			 fout.write( ( char* ) &r, sizeof( float ) );
//			 fout.write( ( char* ) &g, sizeof( float ) );
//			 fout.write( ( char* ) &b, sizeof( float ) );
//		}
//	}
//	for(int i = 0; i < max_LOD; ++i){
//		const float sigma_tmp = m_sigmaArray_with_margin[i];
//			fout.write( ( char* ) &sigma_tmp, sizeof( float ) );
//	}
//	fout.close();
//}

void TGAImage::PassGaussianFilterLOD_with_margin(const float margin_scale, const std::string filename)
{
	std::ofstream fout( filename, std::ios::binary );
    if( fout.fail() ) {
        std::cerr << "cannot open file " << "PassGaussianFilter_with_margin.tex" << "\n";
        std::exit( EXIT_FAILURE );
    }

	m_margin_height = int(margin_scale * m_height);
	m_margin_width = int(margin_scale * m_width);
	if(m_margin_height % 2 == 1) m_margin_height += 1;//margin後のテクスチャのサイズ
	if(m_margin_width % 2 == 1) m_margin_width += 1;//margin後のテクスチャのサイズ
	const float scale = margin_scale;
	const float margin = (scale - 1.f) / 2.f;//0 - margin <= (u, v) <= 1 + margin
	m_margin_min = -margin;
	m_margin_max = 1.f + margin;
	
	const int max_LOD = int(log2(int(m_width)));

	m_sigmaArray_with_margin = std::make_unique<float []>(max_LOD);

    //save header information
	const int texture_size = m_margin_height * m_margin_width * 3;
    fout.write( ( char* ) &texture_size, sizeof( int ) );//テクスチャのデータサイズ
    fout.write( ( char* ) &m_margin_width , sizeof( unsigned int ) );//テクスチャのwidth
    fout.write( ( char* ) &m_margin_height, sizeof( unsigned int ) );//テクスチャのheight
    fout.write( ( char* ) &max_LOD, sizeof( uint8_t ) );//テクスチャの数
    fout.write( ( char* ) &m_margin_min, sizeof( float ) );//
    fout.write( ( char* ) &m_margin_max, sizeof( float ) );//
	
	for(int level = 0; level < max_LOD; ++level){
		std::cout << "now filtering with margin... LOD = " << level << " / " << int(max_LOD - 1) << std::endl;
		const int size = int(pow(2, (level + 1))) + 1;
		const int offset = (size + 1) / 2;
		const float sigma = float(size) / 6.f;
		const float sigma2 = sigma * sigma;
		const float gauss_coef = 1.f / sqrt(2.f * M_PI * sigma2);
		const float gauss_coef2 = 1.f / (2.f * M_PI * sigma2);
		m_sigmaArray_with_margin[level] = sigma;
		std::unique_ptr<float []> filter = std::make_unique<float []>(offset);
		for(int i = 0; i < offset; ++i){//calc filter coef
			filter[i] = gauss_coef * std::exp(-(i * i) / (2.f * sigma2));
		}

		std::unique_ptr<float []> image0 = std::make_unique<float []>(m_margin_height * m_margin_width * int(m_bytePerPixel));
		std::atomic<size_t> roop0(0);
		std::vector<std::thread> thread0(std::thread::hardware_concurrency());
		for(auto &thread : thread0){
			auto f0 = [&]()
			{
				for(size_t h; (h = roop0.fetch_add(1)) < m_margin_height; ){//x方向のblur
					const float v = (float(h) * (2.f * margin + 1.f)) / float(m_margin_height - 1) - margin;
					for(int w = 0; w < m_margin_width; ++w){
						image0[(w + h * m_margin_width) * 3 + 0] = 0.f;
						image0[(w + h * m_margin_width) * 3 + 1] = 0.f;
						image0[(w + h * m_margin_width) * 3 + 2] = 0.f;
						float sum = 0.f;
						const float u = (float(w) * (2.f * margin + 1.f)) / float(m_margin_width - 1) - margin;
						if(0.f <= u && u <= 1.f && 0.f <= v && v <= 1.f){//普通にフィルタリング
							const int oh = int(v * float(m_height - 1) + 0.5f);//0 ~ m_height - 1
							const int ow = int(u * float(m_width  - 1) + 0.5f);
							for(int dx = -(offset - 1); dx <= offset - 1; ++dx){//フィルタリング
								const int x = ow + dx;
								if(0 <= x && x < m_width){
									const int index = std::abs(dx);
									image0[(w + h * m_margin_width) * 3 + 0] += float(m_imageData[(x + oh * m_width) * 3 + 0]) * filter[index];
									image0[(w + h * m_margin_width) * 3 + 1] += float(m_imageData[(x + oh * m_width) * 3 + 1]) * filter[index];
									image0[(w + h * m_margin_width) * 3 + 2] += float(m_imageData[(x + oh * m_width) * 3 + 2]) * filter[index];
									sum += filter[index];
								}
							}
						}
						else{//margin領域について
							int ow;
							int oh;
							int kernel_offset;
							int offset_min_x;
							int offset_max_x;
							int offset_min_y;
							int offset_max_y;
							if(u < 0.f) ow = -int(-u * float(m_width - 1) + 0.5f);
							else ow = int(u * float(m_width - 1) + 0.5f);
							if(v < 0.f) oh = -int(-v * float(m_height - 1) + 0.5f);
							else oh = int(v * float(m_height - 1) + 0.5f);

							int kernel_w;
							int kernel_h;

							if(ow < 0) kernel_w = std::abs(ow);
							else if(m_width - 1 < ow) kernel_w = ow - (m_width - 1);
							else kernel_w = 0;

							if(oh < 0) kernel_h = std::abs(oh);
							else if(m_height - 1 < oh) kernel_h = oh - (m_height - 1);
							else kernel_h = 0;

							if(kernel_w < kernel_h) kernel_offset = kernel_h;
							else kernel_offset = kernel_w;

							if(kernel_offset < offset - 1) kernel_offset = offset - 1;
							
							if(ow < 0){
								offset_min_x = -ow;
								offset_max_x = +kernel_offset;
							}
							else if(m_width - 1 < ow){
								offset_min_x = -kernel_offset;
								offset_max_x = (m_width - 1) - ow;
							}
							else{
								offset_min_x = (ow - kernel_offset < 0) ? -ow : -kernel_offset;
								offset_max_x = (ow + kernel_offset > m_width - 1) ? (m_width - 1 - ow) : +kernel_offset;
							}

							if(oh < 0){
								offset_min_y = -oh;
								offset_max_y = +kernel_offset;
							}
							else if(m_height - 1 < oh){
								offset_min_y = -kernel_offset;
								offset_max_y = (m_height - 1) - oh;
							}
							else{
								offset_min_y = (oh - kernel_offset < 0) ? -oh : -kernel_offset;
								offset_max_y = (oh + kernel_offset > m_height - 1) ? (m_height - 1 - oh) : +kernel_offset;
							}

							/*if(ow < 0){
								offset_min_x = 0;
								offset_max_x = +kernel_offset;
								ow = 0;
							}
							else if(m_width - 1 < ow){
								offset_min_x = -kernel_offset;
								offset_max_x = 0;
								ow = m_width - 1;
							}
							else{
								offset_min_x = (ow - kernel_offset < 0) ? -ow : -kernel_offset;
								offset_max_x = (ow + kernel_offset > m_width - 1) ? (m_width - 1 - ow) : +kernel_offset;
							}

							if(oh < 0){
								offset_min_y = 0;
								offset_max_y = +kernel_offset;
								oh = 0;
							}
							else if(m_height - 1 < oh){
								offset_min_y = -kernel_offset;
								offset_max_y = 0;
								oh = m_height - 1;
							}
							else{
								offset_min_y = (oh - kernel_offset < 0) ? -oh : -kernel_offset;
								offset_max_y = (oh + kernel_offset > m_height - 1) ? (m_height - 1 - oh) : +kernel_offset;
							}*/

							for(int dy = offset_min_y; dy <= offset_max_y; ++dy){//フィルタリング
								const int y = oh + dy;
								for(int dx = offset_min_x; dx <= offset_max_x; ++dx){
									const int x = ow + dx;
									if(0 <= x && x < m_width && 0 <= y && y < m_height){
										float filter_coef = gauss_coef2 * std::exp(-(dx * dx + dy * dy) / (2.f * sigma2));
										//if(filter_coef == 0.f) filter_coef = 1e-8f;
										image0[(w + h * m_margin_width) * 3 + 0] += float(m_imageData[(x + y * m_width) * 3 + 0]) * filter_coef;
										image0[(w + h * m_margin_width) * 3 + 1] += float(m_imageData[(x + y * m_width) * 3 + 1]) * filter_coef;
										image0[(w + h * m_margin_width) * 3 + 2] += float(m_imageData[(x + y * m_width) * 3 + 2]) * filter_coef;
										sum += filter_coef;
									}
								}
							}
						}
						if(sum != 0.f){
							image0[(w + h * m_margin_width) * 3 + 0] /= sum;
							image0[(w + h * m_margin_width) * 3 + 1] /= sum;
							image0[(w + h * m_margin_width) * 3 + 2] /= sum;
						}
					}
				}
			};
			thread = std::thread(f0);
		}
		for(auto &thread : thread0){
			thread.join();
		}

		std::unique_ptr<float []> image1 = std::make_unique<float []>(m_margin_height * m_margin_width * int(m_bytePerPixel));
		std::atomic<size_t> roop1(0);
		std::vector<std::thread> thread1(std::thread::hardware_concurrency());
		for(auto &thread : thread1){
			auto f1 = [&]()
			{
				for(size_t h; (h = roop1.fetch_add(1)) < m_margin_height; ){//y方向のblur
					const float v = (float(h) * (2.f * margin + 1.f)) / float(m_margin_height - 1) - margin;
					for(int w = 0; w < m_margin_width; ++w){
						image1[(w + h * m_margin_width) * 3 + 0] = 0.f;
						image1[(w + h * m_margin_width) * 3 + 1] = 0.f;
						image1[(w + h * m_margin_width) * 3 + 2] = 0.f;
						float sum = 0.f;
						const float u = (float(w) * (2.f * margin + 1.f)) / float(m_margin_width - 1) - margin;
						if(0.f <= u && u <= 1.f && 0.f <= v && v <= 1.f){//普通にフィルタリング
							for(int dy = -(offset - 1); dy <= offset - 1; ++dy){//フィルタリング
								const int y = h + dy;
								const float yy = (float(y) * (2.f * margin + 1.f)) / float(m_margin_height - 1) - margin;
								if(0.f <= yy && yy <= 1.f){
									const int index = std::abs(dy);
									image1[(w + h * m_margin_width) * 3 + 0] += float(image0[(w + y * m_margin_width) * 3 + 0]) * filter[index];
									image1[(w + h * m_margin_width) * 3 + 1] += float(image0[(w + y * m_margin_width) * 3 + 1]) * filter[index];
									image1[(w + h * m_margin_width) * 3 + 2] += float(image0[(w + y * m_margin_width) * 3 + 2]) * filter[index];
									sum += filter[index];
								}
							}
							if(sum != 0.f){
								image1[(w + h * m_margin_width) * 3 + 0] /= sum;
								image1[(w + h * m_margin_width) * 3 + 1] /= sum;
								image1[(w + h * m_margin_width) * 3 + 2] /= sum;
							}
						}
						else{
							image1[(w + h * m_margin_width) * 3 + 0] = float(image0[(w + h * m_margin_width) * 3 + 0]);
							image1[(w + h * m_margin_width) * 3 + 1] = float(image0[(w + h * m_margin_width) * 3 + 1]);
							image1[(w + h * m_margin_width) * 3 + 2] = float(image0[(w + h * m_margin_width) * 3 + 2]);
						}
					}
				}
			};
			thread = std::thread(f1);
		}
		for(auto &thread : thread1){
			thread.join();
		}

		m_imageDataLOD_with_margin.emplace_back(std::move(image1));
	}

	for(const auto & tex : m_imageDataLOD_with_margin){
		for(int i = 0, roop = m_margin_height * m_margin_width * 3; i < roop; i += 3){
			const float r = tex[i + 0];
			const float g = tex[i + 1];
			const float b = tex[i + 2];
			 fout.write( ( char* ) &r, sizeof( float ) );
			 fout.write( ( char* ) &g, sizeof( float ) );
			 fout.write( ( char* ) &b, sizeof( float ) );
		}
	}
	for(int i = 0; i < max_LOD; ++i){
		const float sigma_tmp = m_sigmaArray_with_margin[i];
			fout.write( ( char* ) &sigma_tmp, sizeof( float ) );
	}
	fout.close();
}

//sigmaの値に応じて適切なLODのテクスチャから線形補間されたRGB値を返す(0~1)
//sigma=LODを決めるためのガウシアンフィルタにおけるσ, (u, v)=UV座標(0~1)
const std::array<float, 3> TGAImage::sampleLOD(const float sigma, const float u, const float v) const
{
	if(u < 0.f || v < 0.f || 1.f < u || 1.f < v){
		//std::cerr << "u or v is not 0 ~ 1\n";
		//exit(-1);
		return std::array<float, 3>{0.f, 0.f, 0.f};
	}

	const int w = int(u * (m_width  - 1) + 0.5f);
	const int h = int(v * (m_height - 1) + 0.5f);
	const int index = int(w + h * m_width) * 3;

	if(sigma <= m_sigmaArray[0]){
		const float r = m_imageDataLOD[0][index + 0] / 255.f;
		const float g = m_imageDataLOD[0][index + 1] / 255.f;
		const float b = m_imageDataLOD[0][index + 2] / 255.f;
		return std::array<float, 3>{r, g, b};
	}
	else if(m_sigmaArray[m_imageDataLOD.size() - 1] <= sigma){
		const int level = m_imageDataLOD.size() - 1;
		const float r = m_imageDataLOD[level][index + 0] / 255.f;
		const float g = m_imageDataLOD[level][index + 1] / 255.f;
		const float b = m_imageDataLOD[level][index + 2] / 255.f;
		return std::array<float, 3>{r, g, b};
	}
	else{
		const int level = m_imageDataLOD.size();
		int id0, id1;
		float s_min, s_max;
		for(int i = 0; i < level - 1; ++i){
			if(m_sigmaArray[i] < sigma && sigma <= m_sigmaArray[i + 1]){
				id0 = i;
				id1 = i + 1;
				s_min = m_sigmaArray[i];
				s_max = m_sigmaArray[i + 1];
				break;
			}
		}
		const float a1 = (sigma - s_min) / (s_max - s_min);
		const float a0 = 1.f - a1;

		const float r0 = m_imageDataLOD[id0][index + 0] * a0;
		const float g0 = m_imageDataLOD[id0][index + 1] * a0;
		const float b0 = m_imageDataLOD[id0][index + 2] * a0;
		
		const float r1 = m_imageDataLOD[id1][index + 0] * a1;
		const float g1 = m_imageDataLOD[id1][index + 1] * a1;
		const float b1 = m_imageDataLOD[id1][index + 2] * a1;

		return std::array<float, 3>{(r0 + r1) / 255.f, (g0 + g1) / 255.f, (b0 + b1) / 255.f};
	}
	
}

//適切なLODのテクスチャから線形補間されたRGB値を返す(0~1)
//(u, v)=UV座標(m_margin_min~m_margin_max)
//margin ver
const std::array<float, 3> TGAImage::sampleLOD_with_margin(const float LOD, float u, float v) const
{
	if(u < m_margin_min) u = m_margin_min;
	if(v < m_margin_min) v = m_margin_min;
	if(u > m_margin_max) u = m_margin_max;
	if(v > m_margin_max) v = m_margin_max;
		
	const float w = float((u - m_margin_min) / (m_margin_max - m_margin_min)) * float(m_margin_width  - 1);
	const float h = float((v - m_margin_min) / (m_margin_max - m_margin_min)) * float(m_margin_height - 1);
	const int w0 = int(w);
	const int h0 = int(h);
	const int w1 = (w0 < m_margin_width  - 1) ? int(w0 + 1) : int(m_margin_width  - 1);
	const int h1 = (h0 < m_margin_height - 1) ? int(h0 + 1) : int(m_margin_height - 1);
	const float wa1 = w - w0;
	const float ha1 = h - h0;
	const float wa0 = 1.f - wa1;
	const float ha0 = 1.f - ha1;
	const int ind00 = int(w0 + h0 * m_margin_width) * 3;
	const int ind10 = int(w1 + h0 * m_margin_width) * 3;
	const int ind01 = int(w0 + h1 * m_margin_width) * 3;
	const int ind11 = int(w1 + h1 * m_margin_width) * 3;

	auto calc_rgb_interpolate = [&](const int level)
	{
		const float r00 = m_imageDataLOD_with_margin[level][ind00 + 0];
		const float g00 = m_imageDataLOD_with_margin[level][ind00 + 1];
		const float b00 = m_imageDataLOD_with_margin[level][ind00 + 2];
		
		const float r10 = m_imageDataLOD_with_margin[level][ind10 + 0];
		const float g10 = m_imageDataLOD_with_margin[level][ind10 + 1];
		const float b10 = m_imageDataLOD_with_margin[level][ind10 + 2];

		const float r0 = r00 * wa0 + r10 * wa1;
		const float g0 = g00 * wa0 + g10 * wa1;
		const float b0 = b00 * wa0 + b10 * wa1;
		
		const float r01 = m_imageDataLOD_with_margin[level][ind01 + 0];
		const float g01 = m_imageDataLOD_with_margin[level][ind01 + 1];
		const float b01 = m_imageDataLOD_with_margin[level][ind01 + 2];
		
		const float r11 = m_imageDataLOD_with_margin[level][ind11 + 0];
		const float g11 = m_imageDataLOD_with_margin[level][ind11 + 1];
		const float b11 = m_imageDataLOD_with_margin[level][ind11 + 2];

		const float r1 = r01 * wa0 + r11 * wa1;
		const float g1 = g01 * wa0 + g11 * wa1;
		const float b1 = b01 * wa0 + b11 * wa1;

		const float r = r0 * ha0 + r1 * ha1;
		const float g = g0 * ha0 + g1 * ha1;
		const float b = b0 * ha0 + b1 * ha1;

		return std::array<float, 3>{r, g, b};
	};

	if(LOD <= 0.f){
		const auto rgb = calc_rgb_interpolate(0);
		return std::array<float, 3>{rgb[0] / 255.f, rgb[1] / 255.f, rgb[2] / 255.f};
	}
	else if(m_imageDataLOD_with_margin.size() - 1 <= LOD){
		const int level = m_imageDataLOD_with_margin.size() - 1;
		const auto rgb = calc_rgb_interpolate(level);
		return std::array<float, 3>{rgb[0] / 255.f, rgb[1] / 255.f, rgb[2] / 255.f};
	}
	else{
		const int level0 = int(LOD);
		const int level1 = level0 + 1;
		const float a1 = LOD - float(level0);
		const float a0 = 1.f - a1;

		const auto rgb0 = calc_rgb_interpolate(level0);
		const auto rgb1 = calc_rgb_interpolate(level1);

		const float r = rgb0[0] * a0 + rgb1[0] * a1;
		const float g = rgb0[1] * a0 + rgb1[1] * a1;
		const float b = rgb0[2] * a0 + rgb1[2] * a1;
		
		return std::array<float, 3>{r / 255.f, g / 255.f, b / 255.f};
	}
	
}

void TGAImage::LoadTex_with_margin(const char *filename)
{
	std::ifstream fin( filename, std::ios::binary );
    if( fin.fail() ) {
        std::cerr << "cannot open file " << filename << "\n";
        std::exit( - 1 );
    }

	int data_size;
	uint8_t div;
    fin.read( ( char* ) &data_size, sizeof( int ) );//テクスチャのデータサイズ
    fin.read( ( char* ) &m_margin_width , sizeof( unsigned int ) );//テクスチャのwidth
    fin.read( ( char* ) &m_margin_height, sizeof( unsigned int ) );//テクスチャのheight
    fin.read( ( char* ) &div, sizeof( uint8_t ) );//テクスチャの数
    fin.read( ( char* ) &m_margin_min, sizeof( float ) );//
    fin.read( ( char* ) &m_margin_max, sizeof( float ) );//


	for(int num = 0; num < div; ++num){
		std::unique_ptr<float []> tex_tmp = std::make_unique<float []>(data_size);
		for(int i = 0; i < data_size; i += 3){
			float r, g, b;
			fin.read( ( char* ) &r, sizeof( float ) );
			fin.read( ( char* ) &g, sizeof( float ) );
			fin.read( ( char* ) &b, sizeof( float ) );
			tex_tmp[i + 0] = r;
			tex_tmp[i + 1] = g;
			tex_tmp[i + 2] = b;
		}
		m_imageDataLOD_with_margin.emplace_back(std::move(tex_tmp));
	}
	m_sigmaArray_with_margin = std::make_unique<float []>(div);
	for(int i = 0; i < div; ++i){
		float sigma_tmp;
		fin.read( ( char* ) &sigma_tmp, sizeof( float ) );
		m_sigmaArray_with_margin[i] = sigma_tmp;
	}

	fin.close();
}