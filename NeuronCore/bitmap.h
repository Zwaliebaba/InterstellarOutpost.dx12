 #ifndef INCLUDED_BITMAP_H
#define INCLUDED_BITMAP_H


class RGBAColour;
class jpeg_decoder;
class BitmapFileHeader;
class BitmapInfoHeader;
class BinaryReader;


class BitmapRGBA
{
private:
	void ReadBMPFileHeader(BinaryReader *f, BitmapFileHeader *fileheader);
	void ReadWinBMPInfoHeader(BinaryReader *f, BitmapInfoHeader *infoheader);
	void ReadOS2BMPInfoHeader(BinaryReader *f, BitmapInfoHeader *infoheader);

	void ReadBMPPalette(int ncols, RGBAColour pal[256], BinaryReader *f, int win_flag);
	void Read4BitLine(int length, BinaryReader *f, RGBAColour *pal, int line);
	void Read8BitLine(int length, BinaryReader *f, RGBAColour *pal, int line);
	void Read24BitLine(int length, BinaryReader *f, int line);
    void Read32BitLine(int length, BinaryReader *f, int line);
	void LoadBmp(BinaryReader *_in);

  public:
	int m_width;
	int m_height;
	RGBAColour *m_pixels;
	RGBAColour **m_lines;

	BitmapRGBA();
	BitmapRGBA(BitmapRGBA const &_other);
	BitmapRGBA(int _width, int _height);
	BitmapRGBA(char const *_filename);
	BitmapRGBA(BinaryReader *_reader, char const *_type);
	~BitmapRGBA();

	void Initialise(int _width, int _height);
	void Initialise(char const *_filename);
	void Initialise(BinaryReader *_reader, char const *_type);

  void Clear( RGBAColour const &colour );

	void PutPixel(int x, int y, RGBAColour const &colour);
	void PutPixelOr(int x, int y, RGBAColour const &colour);
	RGBAColour const &GetPixel(int x, int y) const;

	void PutPixelClipped(int x, int y, RGBAColour const &colour);
	RGBAColour const &GetPixelClipped(int x, int y) const;

  RGBAColour GetInterpolatedPixel(float x, float y) const;

	void Blit(int srcX,  int srcY,  int srcW,  int srcH, const BitmapRGBA *_srcBmp, 
			  int destX, int destY, int destW, int destH, bool _bilinear);

  void ApplyBlurFilter(float _scale);

  void ConvertPinkToTransparent();
	void ConvertColourToAlpha();	// Luminance of rgb data is copied into the alpha channel and the rgb data is set to 255,255,255

  void ConvertRedChannel( RGBAColour newColour );
  int ConvertToTexture(bool _mipmapping = true, bool _compressed = false) const;
	int ConvertToTextureAsync(bool _mipmapping = true, bool _compressed = false) const;

  static int GetMaxTextureSize(void);
	
	void CheckGLError() const;
};


#endif
