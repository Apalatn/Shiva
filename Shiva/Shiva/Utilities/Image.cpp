#include "Image.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#ifdef WIN32
// disable useless warnings
#pragma warning(disable:4996)
#endif

Image::Image()
{
    m_pixels = 0;
    m_width = 1;
    m_height = 1;
}

Image::~Image()
{
    if (m_pixels) delete [] m_pixels;
}

void Image::resize(int width, int height)
{
    if (m_pixels) delete [] m_pixels;
    m_pixels = 0;
    m_pixels = new Pixel[width*height];
    memset(m_pixels, 0, width*height*sizeof(Pixel));
    m_width = width;
    m_height = height;
}

void Image::clear(const Pixel& c)
{
    // should be bg color
    for (int y=0; y<m_height; y++)
        for (int x=0; x<m_width; x++)
            setPixel(x, y, c);
}

// map floating point values between 0.0f and 1.0f (if > 1.0f then wrap around) to byte values for pixels
unsigned char Map(float r)
{
    float rMap = 255*r;
    return ( rMap > 255 ? 255 : (unsigned char)rMap );
}

void Image::setPixel(int x, int y, const Pixel& p)
{
    if (x >= 0 && x < m_width && y < m_height && y >= 0) {
        m_pixels[y*m_width+x]= p;
    }
}

void Image::glDrawScanline(int y)
{
//    glRasterPos2f(-1, -1 + 2*y / (float)m_height);
//    glDrawPixels(m_width, 1, GL_RGB, GL_UNSIGNED_BYTE, &m_pixels[y*m_width]);
}

void Image::glDrawToScreenBuffer()
{
    for (int i = 0; i < m_height; i++) {
        glDrawScanline(i);
	}
}

void Image::writePPM(char* pcFile)
{
    writePPM(pcFile, (unsigned char*)m_pixels, m_width, m_height);
}

void Image::writePPM(char *pcFile, unsigned char *data, int width, int height)
{
    FILE *fp = fopen(pcFile, "wb");
    if (!fp) {
        fprintf(stderr, "Couldn't open PPM file %s for writing\n", pcFile);
	}
    else
    {
        fprintf(fp, "P6\n");
        fprintf(fp, "%d %d\n", width, height );
        fprintf(fp, "255\n" );

        // invert image
        int stride = width*3;
        for (int i = height-1; i >= 0; i--)
            fwrite(&data[stride*i], stride, 1, fp);
        fclose(fp);
    }
}

// code originally from http://rosettacode.org/wiki/Bitmap/Read_a_PPM_file#C
// modified for class Image
#define PPMREADBUFLEN 256
void Image::readPPM(char * pcFile)
{
	FILE *fp = fopen(pcFile, "r");
    if (!fp) {
        fprintf(stderr, "Couldn't open PPM file %s for reading\n", pcFile);
	}
	else {
		char buf[PPMREADBUFLEN], *t;
		unsigned int w, h, d;
		int r;
 
		t = fgets(buf, PPMREADBUFLEN, fp);
		// the code fails if the white space following "P6" is not '\n'
		if ( (t == NULL) || ( strncmp(buf, "P6\n", 3) != 0 ) ) return;
		do
		{ // Px formats can have # comments after first line 
			t = fgets(buf, PPMREADBUFLEN, fp);
			if ( t == NULL ) return;
		} while ( strncmp(buf, "#", 1) == 0 );
		r = sscanf(buf, "%u %u", &w, &h);
		if ( r < 2 ) return;
 
		r = fscanf(fp, "%u", &d);
		if ( (r < 1) || ( d != 255 ) ) return;
		fseek(fp, 1, SEEK_CUR); // skip one byte, should be whitespace
 
		// allocate (or re-allocate) image
		resize(w, h);
		unsigned char* data = (unsigned char*)m_pixels;

		size_t rd = fread(data, sizeof(Pixel), w*h, fp);
	}
}