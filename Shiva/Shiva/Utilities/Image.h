#ifndef __IMAGE_H_INCLUDED
#define __IMAGE_H_INCLUDED

class Image
{
public:
    struct Pixel
    {
        unsigned char r, g, b;
        Pixel(unsigned char ir, unsigned char ig, unsigned char ib) {set(ir, ig, ib);}
        Pixel() : r(0), g(0), b(0) {}
        void set(unsigned char ir, unsigned char ig, unsigned char ib) {r = ir; g = ig; b = ib;}
    };

    Image();
    ~Image();

    void resize(int width, int height);
    void setPixel(int x, int y, const Pixel& p);

    void glDrawToScreenBuffer(); // assumes screen buffer is the same size as the image
    void glDrawScanline(int y);
    void clear(const Pixel& c);
    void writePPM(char* pcFile); // write data to a ppm image file
    void writePPM(char *pcName, unsigned char *data, int width, int height);
	void readPPM(char* pcFile);

    unsigned char* getCharPixels()  {return (unsigned char*)m_pixels;}
    int width() const               {return m_width;}
    int height() const              {return m_height;}

private:
    Pixel* m_pixels;
    int m_width;
    int m_height;
};

#endif // __IMAGE_H_INCLUDED
