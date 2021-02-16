//Автор(с): Кудуштеев Алексей
#if! defined(_BITMAP_KUDUSHTEEV_H_)
#define _BITMAP_KUDUSHTEEV_H_
#if defined(_MSC_VER) && _MSC_VER > 1000
#pragma once
#endif
#define FONT_SPACE_CODE  77


//кисть
class brush {
private:
	HBRUSH hbr;
public:
	brush(void);
	brush(LPCWSTR filename);
	brush(HINSTANCE hinst, UINT id);
	brush(const brush&);
	~brush();

	brush& operator = (const brush&);
public:
	bool load(LPCWSTR filename);
	bool load(HINSTANCE hinst, UINT id);
	BOOL fill(HDC hDC, int left, int top, int right, int bottom);
	void destroy(void);
};


//-------------------------------------------------------------------------------------------------------


//битмап
class bitmap {
private:
	HBITMAP hbm;
	HDC     mdc;
	int     width;
	int     height;
public:
	bitmap(void);
	bitmap(LPCWSTR filename);
	bitmap(HINSTANCE hinst, UINT id);
	bitmap(int cx, int cy);
	bitmap(const bitmap&);
	~bitmap();

	bitmap& operator = (const bitmap&);
public:
	bool create(int cx, int cy);
	bool load(LPCWSTR filename);
	bool load(HINSTANCE hinst, UINT id);
	BOOL draw(const bitmap* const bmp, DWORD rop = SRCCOPY);
	BOOL draw(HDC hDC, int x, int y, DWORD rop = SRCCOPY);
	BOOL draw(HDC hDC, int x, int y, int cx, int cy, DWORD rop = SRCCOPY);
	BOOL draw(HDC hDC, int x, int y, int cx, int cy, int sx, int sy, DWORD rop = SRCCOPY);
	BOOL draw(HDC hDC, int x, int y, int cx, int cy, int ox, int oy, int sx, int sy, DWORD rop = SRCCOPY);
	void destroy(void);

	HDC getDC(void) const { 
		return mdc; 
	}

	HBITMAP getHandle(void) const { 
		return hbm; 
	}

	int getWidth(void) const { 
		return width;
	}

	int getHeight(void) const {
		return height;
	}

private:
	void create_dc(void);
	void put_size(void);
};


//-------------------------------------------------------------------------------------------------------


//маска
class bmp_mask {
private:
	HBITMAP hbm;
public:
	bmp_mask(void);
	bmp_mask(const bmp_mask&);
	~bmp_mask();

	bmp_mask& operator = (const bmp_mask&);
public:
	bool create(const bitmap* const bmp, COLORREF color);
	bool create(const bitmap* const bmp);
	bool load(HINSTANCE hinst, UINT id);
	bool load(LPCWSTR filename);
	void destroy(void);

	HBITMAP getHandle(void) const {
		return hbm;
	}
};


//-------------------------------------------------------------------------------------------------------


class font {
private:
	bitmap*   bmp;
	bmp_mask* mask;
	int       fwidth;
	int       fheight;
public:
	void create(bitmap* pbmp, bmp_mask* pmask, int columns);
	void draw(HDC hDC, const BYTE* arr, UINT n, int x, int y);
	void draw(HDC hDC, const BYTE* arr, UINT n, int x, int y, DWORD rop);
	void draw(HDC hDC, const BYTE* arr, UINT n, int x, int y, int cx, int cy, DWORD rop);
	void destroy(void);

	int getWidth(void) const {
		return fwidth;
	}

	int getHeight(void) const {
		return fheight;
	}
};


//-------------------------------------------------------------------------------------------------------


enum eSpState {
	SP_STOP, SP_PLAY, SP_LOOP, SP_PLAYBACK, SP_LOOPBACK
};

//анимация
class sprite {
private:
	DWORD     ltime;
	DWORD     delay;
	eSpState  state;
	short int width;
	short int height;
	short int px;
	short int py;
	short int sizex;
	short int sizey;
public:
	sprite(void);
public:
	void create(const bitmap* const bmp, int num_cols, int num_rows, DWORD time_delay);
	BOOL draw(HDC hDC, const bitmap* const bmp, const bmp_mask* const mask, int x, int y, DWORD rop = MAKEROP4(SRCCOPY, SRCPAINT));
	BOOL draw(HDC hDC, const bitmap* const bmp, const bmp_mask* const mask, int x, int y, int cx, int cy, DWORD rop = MAKEROP4(SRCCOPY, SRCPAINT));

	void play(eSpState play = SP_PLAY);
	void stop(void);

	BOOL updateFrame(DWORD tick);

	short int getWidth(void) const {
		return sizex;
	}

	short int getHeight(void) const {
		return sizey;
	}

	bool isPlay(void) const {
		return (state != SP_STOP);
	}

	bool isStop(void) const {
		return (state == SP_STOP);
	}
};


#endif
