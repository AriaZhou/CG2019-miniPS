#include "Application.h"
#include "qt_opengl_framework.h"
#include <vector>
#include <map>
#include <string>
#include <time.h>
#include <math.h> 
#define PI 3.14159265

Application::Application()
{

}
Application::~Application()
{

}
//****************************************************************************
//
// * ªì©lµe­±¡A¨ÃÅã¥ÜNtust.png¹ÏÀÉ
// 
//============================================================================
void Application::createScene( void )
{
	
	ui_instance = Qt_Opengl_Framework::getInstance();
	
}

//****************************************************************************
//
// * ¥´¶}«ü©w¹ÏÀÉ
// 
//============================================================================
void Application::openImage( QString filePath )
{
	mImageSrc.load(filePath);
	mImageDst.load(filePath);

	renew();

	img_data = mImageSrc.bits();
	img_width = mImageSrc.width();
	img_height = mImageSrc.height();

	ui_instance->ui.label->setFixedHeight(img_height);
	ui_instance->ui.label->setFixedWidth(img_width);
}
//****************************************************************************
//
// * ¨ê·sµe­±
// 
//============================================================================
void Application::renew()
{
	ui_instance = Qt_Opengl_Framework::getInstance();

	ui_instance->ui.label->clear();
	ui_instance->ui.label->setPixmap(QPixmap::fromImage(mImageDst));

	std::cout << "Renew" << std::endl;
}

//****************************************************************************
//
// * µe­±ªì©l¤Æ
// 
//============================================================================
void Application::reload()
{
	ui_instance = Qt_Opengl_Framework::getInstance();

	ui_instance->ui.label->clear();
	ui_instance->ui.label->setPixmap(QPixmap::fromImage(mImageSrc));
}

//****************************************************************************
//
// * Àx¦s¹ÏÀÉ
// 
//============================================================================
void Application::saveImage(QString filePath )
{
	mImageDst.save(filePath);
}

//****************************************************************************
//
// * ±N¹ÏÀÉ¸ê®ÆÂà´«¬°RGB¦â±m¸ê®Æ
// 
//============================================================================
unsigned char* Application::To_RGB( void )
{
	unsigned char *rgb = new unsigned char[img_width * img_height * 3];
	int i, j;

	if (! img_data )
		return NULL;

	// Divide out the alpha
	for (i = 0; i < img_height; i++)
	{
		int in_offset = i * img_width * 4;
		int out_offset = i * img_width * 3;

		for (j = 0 ; j < img_width ; j++)
		{
			RGBA_To_RGB(img_data + (in_offset + j*4), rgb + (out_offset + j*3));
		}
	}

	return rgb;
}

void Application::RGBA_To_RGB( unsigned char *rgba, unsigned char *rgb )
{
	const unsigned char	BACKGROUND[3] = { 0, 0, 0 };

	unsigned char  alpha = rgba[3];

	if (alpha == 0)
	{
		rgb[0] = BACKGROUND[0];
		rgb[1] = BACKGROUND[1];
		rgb[2] = BACKGROUND[2];
	}
	else
	{
		float	alpha_scale = (float)255 / (float)alpha;
		int	val;
		int	i;

		for (i = 0 ; i < 3 ; i++)
		{
			val = (int)floor(rgba[i] * alpha_scale);
			if (val < 0)
				rgb[i] = 0;
			else if (val > 255)
				rgb[i] = 255;
			else
				rgb[i] = val;
		}
	}
}
//------------------------Color------------------------

///////////////////////////////////////////////////////////////////////////////
//
//  Convert image to grayscale.  Red, green, and blue channels should all 
//  contain grayscale value.  Alpha channel shoould be left unchanged.  Return
//  success of operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Gray()
{
	unsigned char *rgb = To_RGB();

	for (int i=0; i<img_height; i++)
	{
		for (int j=0; j<img_width; j++)
		{
			int offset_rgb = i*img_width*3+j*3;
			int offset_rgba = i*img_width*4+j*4;
			unsigned char gray = 0.3 * rgb[offset_rgb + rr] + 0.59 * rgb[offset_rgb + gg] + 0.11 * rgb[offset_rgb + bb];

			for (int k=0; k<3; k++)
				img_data[offset_rgba+k] = gray;
			img_data[offset_rgba + aa] = WHITE;
		}
	}
	
	delete[] rgb;
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32 );
	renew();
}
///////////////////////////////////////////////////////////////////////////////
//
//  Convert the image to an 8 bit image using uniform quantization.  Return 
//  success of operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Quant_Uniform()
{
	unsigned char *rgb = this->To_RGB();

	for (int i = 0; i < img_height; i++)
	{
		for (int j = 0; j < img_width; j++)
		{
			int offset_rgb = i * img_width * 3 + j * 3;
			int offset_rgba = i * img_width * 4 + j * 4;
			unsigned char blueq = (unsigned char)(rgb[offset_rgb + bb] / (255. / 3.) * (255. / 3.) + 0.5);;
			unsigned char redq = (unsigned char)(rgb[offset_rgb + rr] / (255. / 7.) + 0.5) * 255. / 7. + 0.5;
			unsigned char greenq = (unsigned char)(rgb[offset_rgb + gg] / (255. / 7.) + 0.5) * 255. / 7. + 0.5;

			img_data[offset_rgba + bb] = blueq;
			img_data[offset_rgba + rr] = redq;
			img_data[offset_rgba + gg] = greenq;
			img_data[offset_rgba + aa] = WHITE;
		}
	}

	delete[] rgb;
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32 );
	renew();
}
///////////////////////////////////////////////////////////////////////////////
//
//  Convert the image to an 8 bit image using populosity quantization.  
//  Return success of operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Quant_Populosity()
{
	unsigned char *rgb = this->To_RGB();
	std::map<unsigned int, unsigned int> colorCount;
	unsigned int fqt256[256];

	//convert to 32 shades
	for (unsigned int i = 0; i < img_height; i++)
	{
		for (unsigned int j = 0; j < img_width; j++)
		{
			int offset_rgb = i * img_width * 3 + j * 3;
			unsigned char blue32 = (unsigned char)(rgb[offset_rgb + bb] / 8) * 8;
			unsigned char red32 = (unsigned char)(rgb[offset_rgb + rr] / 8) * 8;
			unsigned char green32 = (unsigned char)(rgb[offset_rgb + gg] / 8) * 8;

			//calculate each color repeat times
			colorCount[blue32 | green32 << 8 | red32 << 16]++;
		}
	}

	//sort the color repeat times map
	typedef std::pair<unsigned int, unsigned int> PAIR;
	struct CmpByValue {
		bool operator()(const PAIR& lhs, const PAIR& rhs) {
			return lhs.second > rhs.second;
		}
	};

	std::vector<PAIR> color_vec(colorCount.begin(), colorCount.end());
	std::sort(color_vec.begin(), color_vec.end(), CmpByValue());
	
	//pick up the 256 most frequent color
	for (unsigned int i = 0; i < 256; i++) {
		fqt256[i] = color_vec[i].first;
	}

	//save the most similar color
	unsigned int color, min_distance, distance;
	int r, g, b;

	for (unsigned int i = 0; i < img_height; i++)
	{
		for (unsigned int j = 0; j < img_width; j++)
		{
			int offset_rgb = i * img_width * 3 + j * 3;
			int offset_rgba = i * img_width * 4 + j * 4;

			//find the closest color by euclidean distance
			b = fqt256[0] & 255;
			g = (fqt256[0] >> 8) & 255;
			r = (fqt256[0] >> 16) & 255;

			min_distance = sqrt((int)pow((int)rgb[offset_rgb + bb] - b, 2) + (int)pow((int)rgb[offset_rgb + rr] - r, 2) + (int)pow((int)rgb[offset_rgb + gg] - g, 2));
			color = fqt256[0];

			for (int k = 1; k < 256; k++) {
				b = fqt256[k] & 255;
				g = (fqt256[k] >> 8) & 255;
				r = (fqt256[k] >> 16) & 255;
				distance = sqrt((int)pow((int)rgb[offset_rgb + bb] - b, 2) + (int)pow((int)rgb[offset_rgb + rr] - r, 2) + (int)pow((int)rgb[offset_rgb + gg] - g, 2));
				if (distance < min_distance) {
					min_distance = distance;
					color = fqt256[k];
				}
			}

			img_data[offset_rgba + bb] = color & 255;
			img_data[offset_rgba + gg] = (color >> 8) & 255;
			img_data[offset_rgba + rr] = (color >> 16) & 255;
			img_data[offset_rgba + aa] = WHITE;
		}
	}

	delete[] rgb;
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32 );
	renew();
}

//------------------------Dithering------------------------

///////////////////////////////////////////////////////////////////////////////
//
//  Dither the image using a threshold of 1/2.  Return success of operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Dither_Threshold()
{
	unsigned char *rgb = this->To_RGB();

	for (int i = 0; i < img_height; i++)
	{
		for (int j = 0; j < img_width; j++)
		{
			int offset_rgb = i * img_width * 3 + j * 3;
			int offset_rgba = i * img_width * 4 + j * 4;
			double gray = (0.299 * (float)rgb[offset_rgb + rr] + 0.587 * (float)rgb[offset_rgb + gg] + 0.114 * (float)rgb[offset_rgb + bb])/(float)255;
			
			for (int k = 0; k < 3; k++) {
				if (gray >= 0.5) 
					img_data[offset_rgba + k] = WHITE;
				else
					img_data[offset_rgba + k] = BLACK;
			}
			img_data[offset_rgba + aa] = WHITE;
		}
	}

	delete[] rgb;
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32 );
	renew();
}
///////////////////////////////////////////////////////////////////////////////
//
//  Dither image using random dithering.  Return success of operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Dither_Random()
{
	unsigned char *rgb = this->To_RGB();
	srand(time(NULL));

	for (int i = 0; i < img_height; i++)
	{
		for (int j = 0; j < img_width; j++)
		{
			int offset_rgb = i * img_width * 3 + j * 3;
			int offset_rgba = i * img_width * 4 + j * 4;
			float gray = (0.299 * (float)rgb[offset_rgb + rr] + 0.587 * (float)rgb[offset_rgb + gg] + 0.114 * (float)rgb[offset_rgb + bb]) / (float)255;

			float x = 0.4 * rand() / (RAND_MAX + 1.0) + (-0.2);
			gray += x;

			for (int k = 0; k < 3; k++) {
				if (gray >= 0.5)
					img_data[offset_rgba + k] = WHITE;
				else
					img_data[offset_rgba + k] = BLACK;
			}
			img_data[offset_rgba + aa] = WHITE;
		}
	}
		

	delete[] rgb;
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32 );
	renew();
}
///////////////////////////////////////////////////////////////////////////////
//
//  Perform Floyd-Steinberg dithering on the image.  Return success of 
//  operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Dither_FS()
{
	unsigned char *rgb = this->To_RGB();
	float *img_gray = new float[img_height*img_width];

	for (int i = 0; i < img_height; i++)
	{
		for (int j = 0; j < img_width; j++)
		{
			int offset_rgb = i * img_width * 3 + j * 3;
			float gray = (0.299 * (float)rgb[offset_rgb + rr] + 0.587 * (float)rgb[offset_rgb + gg] + 0.114 * (float)rgb[offset_rgb + bb]) / 255.;

			img_gray[offset_rgb/3] = gray;
		}
	}
	
	for (int i = 0; i < img_height; i++)
	{
		for (int j = 0; j < img_width; j++)
		{
			int offset_rgba = i * img_width * 4 + j * 4;
			int offset_rgb = i * img_width * 3 + j * 3;
			float e[3];

			for (int k = 0; k < 3; k++) {
				if (rgb[offset_rgb + rr] >= 127) {
					img_data[offset_rgba + k] = WHITE;
				}
				else {
					img_data[offset_rgba + k] = BLACK;
				}
			}
			e[rr] = rgb[offset_rgb + rr] - img_data[offset_rgba + rr];
			e[gg] = rgb[offset_rgb + gg] - img_data[offset_rgba + gg];
			e[bb] = rgb[offset_rgb + bb] - img_data[offset_rgba + bb];

			img_data[offset_rgba + aa] = WHITE;

			for (int k = 0; k < 3; k++) {
				if (j != img_width - 1) {
					if (rgb[(i*img_width + j + 1) * 3 + k] + 7. / 16. * e[k] > 255)
						rgb[(i*img_width + j + 1) * 3 + k] = 255;
					else if (rgb[(i*img_width + j + 1) * 3 + k] + 7. / 16. * e[k] < 0)
						rgb[(i*img_width + j + 1) * 3 + k] = 0;
					else
						rgb[(i*img_width + j + 1) * 3 + k] += 7. / 16. * e[k];
				}
				if(i != img_height - 1){
					if (rgb[((i + 1)*img_width + j) * 3 + k] + 5. / 16. * e[k] > 255)
						rgb[((i + 1)*img_width + j) * 3 + k] = 255;
					else if (rgb[((i + 1)*img_width + j) * 3 + k] + 5. / 16. * e[k] < 0)
						rgb[((i + 1)*img_width + j) * 3 + k] = 0;
					else
						rgb[((i + 1)*img_width + j) * 3 + k] += 5. / 16. * e[k];

					if (j != 0) {
						if (rgb[((i + 1)*img_width + j - 1) * 3 + k] + 3. / 16. * e[k] > 255)
							rgb[((i + 1)*img_width + j - 1) * 3 + k] = 255;
						else if (rgb[((i + 1)*img_width + j - 1) * 3 + k] + 3. / 16. * e[k] < 0)
							rgb[((i + 1)*img_width + j - 1) * 3 + k] = 0;
						else
							rgb[((i + 1)*img_width + j - 1) * 3 + k] += 3. / 16. * e[k];
					}

					if (j != img_width - 1) {
						if (rgb[((i + 1)*img_width + j + 1) * 3 + k] + 1. / 16. * e[k] > 255)
							rgb[((i + 1)*img_width + j + 1) * 3 + k] = 255;
						else if (rgb[((i + 1)*img_width + j + 1) * 3 + k] + 1. / 16. * e[k] < 0)
							rgb[((i + 1)*img_width + j + 1) * 3 + k] = 0;
						else
							rgb[((i + 1)*img_width + j + 1) * 3 + k] += 1. / 16. * e[k];
					}
					
				}

			}

		}
	}

	delete[] rgb;
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32 );
	renew();
}
///////////////////////////////////////////////////////////////////////////////
//
//  Dither the image while conserving the average brightness.  Return 
//  success of operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Dither_Bright()
{
	unsigned char *rgb = this->To_RGB();
	std::map<float, unsigned int> brightCount;
	float *img_gray = new float[img_height*img_width];

	for (int i = 0; i < img_height; i++)
	{
		for (int j = 0; j < img_width; j++)
		{
			int offset_rgb = i * img_width * 3 + j * 3;
			int offset_rgba = i * img_width * 4 + j * 4;
			float gray = (0.299 * (float)rgb[offset_rgb + rr] + 0.587 * (float)rgb[offset_rgb + gg] + 0.114 * (float)rgb[offset_rgb + bb]) / (float)255;

			brightCount[gray]++;
			img_gray[i*img_width+j] = gray;
		}
	}

	float avr_bright, tot_bright=0;
	for (int i = 0; i < img_height*img_width; i++) {
		tot_bright += img_gray[i];
	}
	avr_bright = tot_bright / (float)(img_height*img_width);

	std::vector<std::pair<float, unsigned int>> brightVector(brightCount.begin(), brightCount.end());
	sort(brightVector.begin(), brightVector.end());

	unsigned int bCount=0;
	float treshold;

	for (int i = 0; i < brightVector.size(); i++) {
		bCount += brightVector[i].second;
		if (bCount >= (1-avr_bright) * img_height * img_width) {
			treshold = brightVector[i].first;
			break;
		}
	}

	for (int i = 0; i < img_height; i++)
	{
		for (int j = 0; j < img_width; j++)
		{
			int offset_rgb = i * img_width * 3 + j * 3;
			int offset_rgba = i * img_width * 4 + j * 4;

			for (int k = 0; k < 3; k++) {
				if(img_gray[i*img_width + j] >= treshold)
					img_data[offset_rgba + k] = WHITE;
				else
					img_data[offset_rgba + k] = BLACK;
			}
			img_data[offset_rgba + aa] = WHITE;
				
		}
	}

	delete[] rgb;
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32 );
	renew();
}
///////////////////////////////////////////////////////////////////////////////
//
//  Perform clustered differing of the image.  Return success of operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Dither_Cluster()
{
	unsigned char *rgb = this->To_RGB();

	float mask[16] = { 0.7059, 0.3529, 0.5882, 0.2353,
					   0.0588, 0.9412, 0.8235, 0.4118,
					   0.4706, 0.7647, 0.8824, 0.1176,
					   0.1765, 0.5294, 0.2941, 0.6471 };

	for (int i = 0; i < img_height; i++)
	{
		for (int j = 0; j < img_width; j++)
		{
			int offset_rgb = i * img_width * 3 + j * 3;
			int offset_rgba = i * img_width * 4 + j * 4;
			float gray = (0.299 * (float)rgb[offset_rgb + rr] + 0.587 * (float)rgb[offset_rgb + gg] + 0.114 * (float)rgb[offset_rgb + bb]) / (float)256;

			float x = mask[(i % 4) * 4 + (j % 4)];

			for (int k = 0; k < 3; k++) {
				if (gray >= x)
					img_data[offset_rgba + k] = WHITE;
				else
					img_data[offset_rgba + k] = BLACK;
			}
			img_data[offset_rgba + aa] = WHITE;
		}
	}

	delete[] rgb;
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32 );
	renew();
}
///////////////////////////////////////////////////////////////////////////////
//
//  Convert the image to an 8 bit image using Floyd-Steinberg dithering over
//  a uniform quantization - the same quantization as in Quant_Uniform.
//  Return success of operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Dither_Color()
{
	unsigned char *rgb = this->To_RGB();
	unsigned char *rgb256 = new unsigned char[img_height*img_width * 3];

	for (int i = 0; i < img_height; i++)
	{
		for (int j = 0; j < img_width; j++)
		{
			int offset_rgba = i * img_width * 4 + j * 4;
			int offset_rgb = i * img_width * 3 + j * 3;
			float e[3];

			img_data[offset_rgba + rr] = (unsigned char)(rgb[offset_rgb + rr] / (255. / 7.) + 0.5) * 255. / 7. + 0.5;
			e[rr] = rgb[offset_rgb + rr] - img_data[offset_rgba + rr];
			img_data[offset_rgba + gg] = (unsigned char)(rgb[offset_rgb + gg] / (255. / 7.) + 0.5) * 255. / 7. + 0.5;
			e[gg] = rgb[offset_rgb + gg] - img_data[offset_rgba + gg];
			img_data[offset_rgba + bb] = (unsigned char)(rgb[offset_rgb + bb] / (255. / 3.) * (255. / 3.) + 0.5);
			e[bb] = rgb[offset_rgb + bb] - img_data[offset_rgba + bb];
			
			img_data[offset_rgba + aa] = WHITE;

			for (int k = 0; k < 3; k++) {
				if (j != img_width - 1) {
					if (rgb[(i*img_width + j + 1) * 3 + k] + 7. / 16. * e[k] > 255)
						rgb[(i*img_width + j + 1) * 3 + k] = 255;
					else if (rgb[(i*img_width + j + 1) * 3 + k] + 7. / 16. * e[k] < 0)
						rgb[(i*img_width + j + 1) * 3 + k] = 0;
					else
						rgb[(i*img_width + j + 1) * 3 + k] += 7. / 16. * e[k];
				}
				if (i != img_height - 1) {
					if (rgb[((i + 1)*img_width + j) * 3 + k] + 5. / 16. * e[k] > 255)
						rgb[((i + 1)*img_width + j) * 3 + k] = 255;
					else if (rgb[((i + 1)*img_width + j) * 3 + k] + 5. / 16. * e[k] < 0)
						rgb[((i + 1)*img_width + j) * 3 + k] = 0;
					else
						rgb[((i + 1)*img_width + j) * 3 + k] += 5. / 16. * e[k];

					if (j != 0) {
						if (rgb[((i + 1)*img_width + j - 1) * 3 + k] + 3. / 16. * e[k] > 255)
							rgb[((i + 1)*img_width + j - 1) * 3 + k] = 255;
						else if (rgb[((i + 1)*img_width + j - 1) * 3 + k] + 3. / 16. * e[k] < 0)
							rgb[((i + 1)*img_width + j - 1) * 3 + k] = 0;
						else
							rgb[((i + 1)*img_width + j - 1) * 3 + k] += 3. / 16. * e[k];
					}

					if (j != img_width - 1) {
						if (rgb[((i + 1)*img_width + j + 1) * 3 + k] + 1. / 16. * e[k] > 255)
							rgb[((i + 1)*img_width + j + 1) * 3 + k] = 255;
						else if (rgb[((i + 1)*img_width + j + 1) * 3 + k] + 1. / 16. * e[k] < 0)
							rgb[((i + 1)*img_width + j + 1) * 3 + k] = 0;
						else
							rgb[((i + 1)*img_width + j + 1) * 3 + k] += 1. / 16. * e[k];
					}

				}

			}

		}
	}


	delete[] rgb;
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32 );
	renew();
}

//------------------------Filter------------------------

///////////////////////////////////////////////////////////////////////////////
//
//     Filtering the img_data array by the filter from the parameters
//
///////////////////////////////////////////////////////////////////////////////
void Application::filtering( double filter[][5] )
{
	unsigned char *rgb = this->To_RGB();
	int n = 0;
	for (int k = 0; k < 5; k++) {
		for (int s = 0; s < 5; s++) {
			n+=filter[k][s];
		}
	}

	for (int i = 0; i < img_height; i++) {
		for (int j = 0; j < img_width; j++) {
			int offset_rgba = i * img_width * 4 + j * 4;

			for (int a = 0; a < 3; a++) {
				float tot = 0;
				for (int k = -2; k < 3; k++) {
					for (int s = -2; s < 3; s++) {
						if (i + k > 0 && i + k < img_height && j + s>0 && j + s < img_width) {
							tot += rgb[((i + k) * img_width * 3 + (j + s) * 3) + a] * filter[(k + 2)][(s + 2)];
						}
					}
				}

				tot /= n;
				if (tot < 0)
					img_data[offset_rgba + a] = 0;
				else if (tot > 255)
					img_data[offset_rgba + a] = 255;
				else
					img_data[offset_rgba + a] = tot;
			}
			img_data[offset_rgba + aa] = WHITE;
		}
	}

	delete[] rgb;
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32);
	renew();
}

void Application::filtering( double *filter, int n )
{
	unsigned char *rgb = this->To_RGB();

	for (int i = 0; i < img_height; i++) {
		for (int j = 0; j < img_width; j++) {
			int offset_rgba = i * img_width * 4 + j * 4;

			for (int a = 0; a < 3; a++) {
				double tot = 0;
				for (int k = -(n / 2); k <= n / 2; k++) {
					for (int s = -(n / 2); s <= n / 2; s++) {
						if (i + k > 0 && i + k < img_height && j + s>0 && j + s < img_width) {
							tot += rgb[((i + k) * img_width * 3 + (j + s) * 3) + a] * filter[(k + (n / 2))*n+(s + (n / 2))];
						}
					}
				}
				if (tot < 0)
					img_data[offset_rgba + a] = 0;
				else if (tot > 255)
					img_data[offset_rgba + a] = 255;
				else
					img_data[offset_rgba + a] = tot;
			}
			img_data[offset_rgba + aa] = WHITE;
		}
	}

	delete[] rgb;
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32 );
	renew();
}
///////////////////////////////////////////////////////////////////////////////
//
//  Perform 5x5 box filter on this image.  Return success of operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Filter_Box()
{
	double filter[5][5] = { 1,1,1,1,1,
						  1,1,1,1,1,
						  1,1,1,1,1,
						  1,1,1,1,1,
						  1,1,1,1,1 };

	filtering(filter);
}
///////////////////////////////////////////////////////////////////////////////
//
//  Perform 5x5 Bartlett filter on this image.  Return success of 
//  operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Filter_Bartlett()
{
	double filter[5][5] = { 1,2,3,2,1,
						    2,4,6,4,2,
						    3,6,9,6,3,
						    2,4,6,4,2,
						    1,2,3,2,1 };

	filtering(filter);
}
///////////////////////////////////////////////////////////////////////////////
//
//  Perform 5x5 Gaussian filter on this image.  Return success of 
//  operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Filter_Gaussian()
{
	double filter[5][5] = { 1, 4, 6, 4,1,
						    4,16,24,16,4,
						    6,24,36,24,6,
						    4,16,24,16,4,
						    1, 4, 6, 4,1 };

	filtering(filter);
}
///////////////////////////////////////////////////////////////////////////////
//
//  Perform NxN Gaussian filter on this image.  Return success of 
//  operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Filter_Gaussian_N( unsigned int N )
{
	double *filter = new double[N*N];
	double tot = 0;
	N = N / 2 * 2 + 1;

	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			filter[i*N + j] = exp(-(pow(i - (int)N / 2, 2.) + pow(j - (int)N / 2, 2.)));
			tot += filter[i*N + j];
		}
	}
	
	//格式化
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			filter[i*N + j] /= tot;
		}
	}

	filtering(filter, N);
}
///////////////////////////////////////////////////////////////////////////////
//
//  Perform 5x5 edge detect (high pass) filter on this image.  Return 
//  success of operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Filter_Edge()
{ 
	unsigned char *rgb = this->To_RGB();
	double filter[5][5] = { 1 / 81.,2 / 81.,3 / 81.,2 / 81.,1 / 81.,
							2 / 81.,4 / 81.,6 / 81.,4 / 81.,2 / 81.,
							3 / 81.,6 / 81.,9 / 81.,6 / 81.,3 / 81.,
							2 / 81.,4 / 81.,6 / 81.,4 / 81.,2 / 81.,
							1 / 81.,2 / 81.,3 / 81.,2 / 81.,1 / 81. };

	for (int i = 0; i < img_height; i++) {
		for (int j = 0; j < img_width; j++) {
			int offset_rgba = i * img_width * 4 + j * 4;
			int offset_rgb = i * img_width * 3 + j * 3;

			for (int a = 0; a < 3; a++) {
				float tot = 0;
				for (int k = -2; k < 3; k++) {
					for (int s = -2; s < 3; s++) {
						if (i + k > 0 && i + k < img_height && j + s>0 && j + s < img_width) {
							tot += rgb[((i + k) * img_width * 3 + (j + s) * 3) + a] * filter[(k + 2)][(s + 2)];
						}
					}
				}
				tot = ((int)rgb[offset_rgb + a] - tot);
				if (tot < 0)
					img_data[offset_rgba + a] = 0;
				else if (tot > 255)
					img_data[offset_rgba + a] = 255;
				else
					img_data[offset_rgba + a] = tot;
			}
			img_data[offset_rgba + aa] = WHITE;
		}
	}



	delete[] rgb;
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32);
	renew();
}
///////////////////////////////////////////////////////////////////////////////
//
//  Perform a 5x5 enhancement filter to this image.  Return success of 
//  operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Filter_Enhance()
{
	unsigned char *rgb = this->To_RGB();
	double filter[5][5] = { 1 / 81.,2 / 81.,3 / 81.,2 / 81.,1 / 81.,
							2 / 81.,4 / 81.,6 / 81.,4 / 81.,2 / 81.,
							3 / 81.,6 / 81.,9 / 81.,6 / 81.,3 / 81.,
							2 / 81.,4 / 81.,6 / 81.,4 / 81.,2 / 81.,
							1 / 81.,2 / 81.,3 / 81.,2 / 81.,1 / 81. };

	for (int i = 0; i < img_height; i++) {
		for (int j = 0; j < img_width; j++) {
			int offset_rgba = i * img_width * 4 + j * 4;
			int offset_rgb = i * img_width * 3 + j * 3;

			for (int a = 0; a < 3; a++) {
				float tot = 0;
				for (int k = -2; k < 3; k++) {
					for (int s = -2; s < 3; s++) {
						if (i + k > 0 && i + k < img_height && j + s>0 && j + s < img_width) {
							tot += rgb[((i + k) * img_width * 3 + (j + s) * 3) + a] * filter[(k + 2)][(s + 2)];
						}
					}
				}
				tot = ((int)rgb[offset_rgb + a] - tot) + (int)rgb[offset_rgb+a];
				if (tot < 0)
					img_data[offset_rgba + a] = 0;
				else if (tot > 255)
					img_data[offset_rgba + a] = 255;
				else
					img_data[offset_rgba + a] = tot;
			}
		}
	}



	delete[] rgb;
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32 );
	renew();
}

//------------------------Size------------------------

///////////////////////////////////////////////////////////////////////////////
//
//  Halve the dimensions of this image.  Return success of operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Half_Size()
{
	unsigned char* img_half = new unsigned char[img_height/2*img_width/2*4];
	double filter[3][3] = { 1 / 16, 1 / 8, 1 / 16,
							1 /  8, 1 / 4, 1 /  8,
							1 / 16, 1 / 8, 1 / 16 };


	for (int i = 0; i < img_height/2; i++) {
		for (int j = 0; j < img_width/2; j++) {
			int offset_rgba = i * (img_width/2) * 4 + j * 4;
			int offset_rgbao = i * 2 * img_width * 4 + j * 2 * 4;
			for (int a = 0; a < 4; a++) {
				double tot = 0;
				for (int k = -1; k <= 1; k++) {
					for (int s = -1; s <= 1; s++) {
						if (i * 2 + k > 0 && i * 2 + k < img_height && j * 2 + s>0 && j * 2 + s < img_width) {
							tot += img_data[((i * 2 + k) * img_width * 4 + (j * 2 + s) * 4) + a] * filter[k + 1][s + 1];
						}
					}
				}
				if (tot == 0)
					img_half[offset_rgba + a] = img_data[offset_rgbao + a];
				else
					img_half[offset_rgba + a] = tot;
			}
		}
	}

	img_data = img_half;
	img_width /= 2;
	img_height /= 2;
	ui_instance->ui.label->setFixedHeight(img_height);
	ui_instance->ui.label->setFixedWidth(img_width);
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32);
	renew();
}
///////////////////////////////////////////////////////////////////////////////
//
//  Double the dimensions of this image.  Return success of operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Double_Size()
{
	unsigned char* img_double = new unsigned char[img_height * 2 * img_width * 2 * 4];
	double filterEven[3][3] =    { 1 / 16., 1 / 8., 1 / 16.,
								   1 /  8., 1 / 4., 1 /  8.,
								   1 / 16., 1 / 8., 1 / 16. };

	double filterOdd[4][4] =	 { 1 / 64., 3 / 64., 3 / 64., 1 / 64.,
								   3 / 64., 9 / 64., 9 / 64., 3 / 64.,
								   3 / 64., 9 / 64., 9 / 64., 3 / 64.,
								   1 / 64., 3 / 64., 3 / 64., 1 / 64. };

	double filterEvenOdd[3][4] = { 1 / 32., 2 / 32., 1 / 32., 3 / 32., 
								   6 / 32., 3 / 32., 3 / 32., 6 / 32., 
								   3 / 32., 1 / 32., 2 / 32., 1 / 32. };

	double filterOddEven[4][3] = { 1 / 32., 2 / 32., 1 / 32.,
								   3 / 32., 6 / 32., 3 / 32., 
								   3 / 32., 6 / 32., 3 / 32., 
								   1 / 32., 2 / 32., 1 / 32. };

	for (int i = 0; i < img_height * 2; i++) {
		for (int j = 0; j < img_width * 2; j++) {
			int offset_rgba = i * (img_width * 2) * 4 + j * 4;
			int offset_rgbao = i / 2 * img_width * 4 + j / 2 * 4;
			for (int a = 0; a < 4; a++) {
				double tot = 0;

				if (i % 2 == 0 && j % 2 == 0) {
					for (int k = -1; k <= 1; k++) {
						for (int s = -1; s <= 1; s++) {
							if (i / 2 + k > 0 && i / 2 + k < img_height && j / 2 + s>0 && j / 2 + s < img_width) {
								tot += img_data[((i / 2 + k) * img_width * 4 + (j / 2 + s) * 4) + a] * filterEven[k + 1][s + 1];
							}
						}
					}
					if (tot == 0)
						img_double[offset_rgba + a] = img_data[offset_rgbao + a];
					else
						img_double[offset_rgba + a] = tot;
				}
				else if (i % 2 != 0 && j % 2 != 0) {
					for (int k = -1; k <= 2; k++) {
						for (int s = -1; s <= 2; s++) {
							if (i / 2 + k > 0 && i / 2 + k < img_height && j / 2 + s>0 && j / 2 + s < img_width) {
								tot += img_data[((i / 2 + k) * img_width * 4 + (j / 2 + s) * 4) + a] * filterOdd[k + 1][s + 1];
							}
						}
					}
					if (tot == 0)
						img_double[offset_rgba + a] = 0;
					else
						img_double[offset_rgba + a] = tot;
				}
				else if (i % 2 == 0 && j % 2 != 0) {
					for (int k = -1; k <= 1; k++) {
						for (int s = -1; s <= 2; s++) {
							if (i / 2 + k > 0 && i / 2 + k < img_height && j / 2 + s>0 && j / 2 + s < img_width) {
								tot += img_data[((i / 2 + k) * img_width * 4 + (j / 2 + s) * 4) + a] * filterEvenOdd[k + 1][s + 1];
							}
						}
					}
					if (tot == 0)
						img_double[offset_rgba + a] = 0;
					else
						img_double[offset_rgba + a] = tot;
				}
				else {
					for (int k = -1; k <= 2; k++) {
						for (int s = -1; s <= 1; s++) {
							if (i / 2 + k > 0 && i / 2 + k < img_height && j / 2 + s>0 && j / 2 + s < img_width) {
								tot += img_data[((i / 2 + k) * img_width * 4 + (j / 2 + s) * 4) + a] * filterOddEven[k + 1][s + 1];
							}
						}
					}
					if (tot == 0)
						img_double[offset_rgba + a] = 0;
					else
						img_double[offset_rgba + a] = tot;
				}
			}
		}
	}

	img_data = img_double;
	img_width *= 2;
	img_height *= 2;
	ui_instance->ui.label->setFixedHeight(img_height);
	ui_instance->ui.label->setFixedWidth(img_width);
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32);
	renew();
}
///////////////////////////////////////////////////////////////////////////////
//
//  resample_src for resize and rotate
//
///////////////////////////////////////////////////////////////////////////////
void Application::resample_src(int u, int v, float ww, unsigned char* rgba)
{

}

///////////////////////////////////////////////////////////////////////////////
//
//  Scale the image dimensions by the given factor.  The given factor is 
//	assumed to be greater than one.  Return success of operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Resize( float scale )
{
	unsigned char* img_scale = new unsigned char[(int)(img_height * scale) * (int)(img_width * scale) * 4];
	double filter[4][4] = { 1 / 64., 3 / 64., 3 / 64., 1 / 64.,
							3 / 64., 9 / 64., 9 / 64., 3 / 64.,
							3 / 64., 9 / 64., 9 / 64., 3 / 64.,
							1 / 64., 3 / 64., 3 / 64., 1 / 64. };


	for (int i = 0; i < (int)(img_height * scale); i++) {
		for (int j = 0; j < (int)(img_width * scale); j++) {
			int offset_rgba = i * (int)(img_width * scale) * 4 + j * 4;
			int offset_rgbao = (int)(i / scale) * img_width * 4 + (int)(j / scale) * 4;
			for (int a = 0; a < 4; a++) {
				double tot = 0;
				for (int k = -1; k <= 2; k++) {
					for (int s = -1; s <= 2; s++) {
						if ((int)(i / scale) + k > 0 && (int)(i / scale) + k < img_height && (int)(j / scale) + s>0 && (int)(j / scale) + s < img_width) {
							tot += img_data[(((int)(i / scale) + k) * img_width * 4 + ((int)(j / scale) + s) * 4) + a] * filter[k + 1][s + 1];
						}
					}
				}
				
				img_scale[offset_rgba + a] = tot;
			}
		}
	}

	img_data = img_scale;
	img_width *= scale;
	img_height *= scale;
	ui_instance->ui.label->setFixedHeight(img_height);
	ui_instance->ui.label->setFixedWidth(img_width);
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32);
	renew();
}

//////////////////////////////////////////////////////////////////////////////
//
//  Rotate the image clockwise by the given angle.  Do not resize the 
//  image.  Return success of operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Rotate( float angleDegrees )
{
	int new_height = sin(angleDegrees*PI / 180)*img_width + cos(angleDegrees*PI / 180)*img_height;
	int new_width = sin(angleDegrees*PI / 180)*img_height + cos(angleDegrees*PI / 180)*img_width;
	unsigned char* img_scale = new unsigned char[(new_height) * (new_width) * 4];
	double filter[4][4] = { 1 / 64., 3 / 64., 3 / 64., 1 / 64.,
							3 / 64., 9 / 64., 9 / 64., 3 / 64.,
							3 / 64., 9 / 64., 9 / 64., 3 / 64.,
							1 / 64., 3 / 64., 3 / 64., 1 / 64. };

	for (int i = 0; i < new_height; i++) {
		for (int j = 0; j < new_width; j++) {
			int offset_rgbao = i * new_width * 4 + j * 4;
			for (int a = 0; a < 3; a++) {
				img_scale[offset_rgbao+a] = 0;
			}

			img_scale[offset_rgbao + aa] = WHITE;
		}
	}

	for (int i = 0; i < img_height; i++) {
		for (int j = 0; j < img_width; j++) {
			int u = ((double)j*sin(angleDegrees*PI / 180) + (double)i * cos(angleDegrees*PI / 180));
			int v = ((double)j*cos(angleDegrees*PI / 180) - (double)i * sin(angleDegrees*PI / 180)) + img_height * sin(angleDegrees*PI / 180);
			int offset_rgba = u * new_width * 4 + v * 4;
			
			for (int a = 0; a < 3; a++) {
				double tot = 0;
				for (int k = -1; k <= 2; k++) {
					for (int s = -1; s <= 2; s++) {
						if (i + k > 0 && i + k < img_height && j + s>0 && j + s < img_width) {
							tot += img_data[((i + k) * img_width * 4 + (j + s) * 4) + a] * filter[k + 1][s + 1];
						}
					}
				}
				
				img_scale[offset_rgba + a] = tot;
			}
		}
	}

	img_data = img_scale;
	img_width = new_height;
	img_height = new_width;
	ui_instance->ui.label->setFixedHeight(img_height);
	ui_instance->ui.label->setFixedWidth(img_width);
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32);
	renew();
}

//------------------------Composing------------------------


void Application::loadSecondaryImge( QString filePath )
{
	mImageSrcSecond.load(filePath);

	renew();

	img_data2 = mImageSrcSecond.bits();
	img_width2 = mImageSrcSecond.width();
	img_height2 = mImageSrcSecond.height();
}

//////////////////////////////////////////////////////////////////////////
//
//	Composite the image A and image B by Over, In, Out, Xor and Atom. 
//
//////////////////////////////////////////////////////////////////////////
void Application::Comp_image( int tMethod )
{
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32 );
	renew();
}

///////////////////////////////////////////////////////////////////////////////
//
//      Composite the current image over the given image.  Return success of 
//  operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Comp_Over()
{
	if (img_height == img_height2 && img_width == img_width2)
	{

	}
	else
	{
		std::cout << "Images not the same size" << std::endl;
	}
}

///////////////////////////////////////////////////////////////////////////////
//
//      Composite this image "in" the given image.  See lecture notes for 
//  details.  Return success of operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Comp_In()
{
	if (img_height == img_height2 && img_width == img_width2)
	{

	}
	else
	{
		std::cout << "Images not the same size" << std::endl;
	}
}

///////////////////////////////////////////////////////////////////////////////
//
//      Composite this image "out" the given image.  See lecture notes for 
//  details.  Return success of operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Comp_Out()
{
	if (img_height == img_height2 && img_width == img_width2)
	{

	}
	else
	{
		std::cout << "Images not the same size" << std::endl;
	}
}

///////////////////////////////////////////////////////////////////////////////
//
//      Composite current image "atop" given image.  Return success of 
//  operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Comp_Atop()
{
	if (img_height == img_height2 && img_width == img_width2)
	{

	}
	else
	{
		std::cout << "Images not the same size" << std::endl;
	}
}

///////////////////////////////////////////////////////////////////////////////
//
//      Composite this image with given image using exclusive or (XOR).  Return
//  success of operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Comp_Xor()
{
	if (img_height == img_height2 && img_width == img_width2)
	{

	}
	else
	{
		std::cout << "Images not the same size" << std::endl;
	}
}

//------------------------NPR------------------------

///////////////////////////////////////////////////////////////////////////////
//
//      Run simplified version of Hertzmann's painterly image filter.
//      You probably will want to use the Draw_Stroke funciton and the
//      Stroke class to help.
// Return success of operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::NPR_Paint()
{
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32 );
	renew();
}

void Application::NPR_Paint_Layer( unsigned char *tCanvas, unsigned char *tReferenceImage, int tBrushSize )
{

}

///////////////////////////////////////////////////////////////////////////////
//
//      Helper function for the painterly filter; paint a stroke at
// the given location
//
///////////////////////////////////////////////////////////////////////////////
void Application::Paint_Stroke( const Stroke& s )
{
	int radius_squared = (int)s.radius * (int)s.radius;
	for (int x_off = -((int)s.radius); x_off <= (int)s.radius; x_off++) 
	{
		for (int y_off = -((int)s.radius); y_off <= (int)s.radius; y_off++) 
		{
			int x_loc = (int)s.x + x_off;
			int y_loc = (int)s.y + y_off;

			// are we inside the circle, and inside the image?
			if ((x_loc >= 0 && x_loc < img_width && y_loc >= 0 && y_loc < img_height)) 
			{
				int dist_squared = x_off * x_off + y_off * y_off;
				int offset_rgba = (y_loc * img_width + x_loc) * 4;

				if (dist_squared <= radius_squared) 
				{
					img_data[offset_rgba + rr] = s.r;
					img_data[offset_rgba + gg] = s.g;
					img_data[offset_rgba + bb] = s.b;
					img_data[offset_rgba + aa] = s.a;
				} 
				else if (dist_squared == radius_squared + 1) 
				{
					img_data[offset_rgba + rr] = (img_data[offset_rgba + rr] + s.r) / 2;
					img_data[offset_rgba + gg] = (img_data[offset_rgba + gg] + s.g) / 2;
					img_data[offset_rgba + bb] = (img_data[offset_rgba + bb] + s.b) / 2;
					img_data[offset_rgba + aa] = (img_data[offset_rgba + aa] + s.a) / 2;
				}
			}
		}
	}
}





///////////////////////////////////////////////////////////////////////////////
//
//      Build a Stroke
//
///////////////////////////////////////////////////////////////////////////////
Stroke::Stroke() {}

///////////////////////////////////////////////////////////////////////////////
//
//      Build a Stroke
//
///////////////////////////////////////////////////////////////////////////////
Stroke::Stroke(unsigned int iradius, unsigned int ix, unsigned int iy,
	unsigned char ir, unsigned char ig, unsigned char ib, unsigned char ia) :
radius(iradius),x(ix),y(iy),r(ir),g(ig),b(ib),a(ia)
{
}



