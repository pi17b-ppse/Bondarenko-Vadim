#include "game.h"



brush::brush(void):hbr(NULL){}

brush::brush(LPCWSTR filename):hbr(NULL){
	load(filename);
}

brush::brush(HINSTANCE hinst, UINT id):hbr(NULL){
	load(hinst, id);
}

brush::~brush(){ destroy(); }


bool brush::load(LPCWSTR filename){
	destroy();
	HBITMAP hbm = reinterpret_cast<HBITMAP>(LoadImageW(NULL, filename, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE));
	if(hbm == NULL)
		return false;

	hbr = CreatePatternBrush(hbm);
	DeleteObject(hbm);
	return (hbr != NULL);
}


bool brush::load(HINSTANCE hinst, UINT id){
	destroy();
	HBITMAP hbm = LoadBitmapW(hinst, MAKEINTRESOURCEW(id));
	if(hbm == NULL)
		return false;

	hbr = CreatePatternBrush(hbm);
	DeleteObject(hbm);
	return (hbr != NULL);
}


BOOL brush::fill(HDC hDC, int left, int top, int right, int bottom){
	RECT rc = { left, top, right, bottom };
	return FillRect(hDC, &rc, hbr);
}


void brush::destroy(void){
	if(hbr != NULL)
		DeleteObject(hbr);
	hbr = NULL;
}


//----------------------------------------------------------------------------------------------------------


bitmap::bitmap(void):hbm(NULL), mdc(NULL), width(0), height(0) {}

bitmap::bitmap(LPCWSTR filename):hbm(NULL), mdc(NULL), width(0), height(0){
	load(filename);
}

bitmap::bitmap(HINSTANCE hinst, UINT id):hbm(NULL), mdc(NULL), width(0), height(0) {
	load(hinst, id);
}

bitmap::bitmap(int cx, int cy):hbm(NULL), mdc(NULL), width(0), height(0) {
	create(cx, cy);
}

bitmap::~bitmap(){ destroy(); }


bool bitmap::create(int cx, int cy){
	if(hbm != NULL)
		DeleteObject(hbm);
		
	HDC hdc = GetDC(NULL);
	hbm = CreateCompatibleBitmap(hdc, cx, cy);
	ReleaseDC(NULL, hdc);
	if(hbm != NULL){
		create_dc();
		SelectObject(mdc, hbm);
		PatBlt(mdc, 0, 0, cx, cy, BLACKNESS);
		put_size();	
	}
	return (hbm != NULL);
}


bool bitmap::load(LPCWSTR filename){
	if(hbm != NULL)
		DeleteObject(hbm);

	hbm = reinterpret_cast<HBITMAP>(LoadImageW(NULL, filename, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE));
	if(hbm != NULL){
		create_dc();
		SelectObject(mdc, hbm);
		put_size();
	}
	return (hbm != NULL);
}


bool bitmap::load(HINSTANCE hinst, UINT id){
	if(hbm != NULL)
		DeleteObject(hbm);

	hbm = LoadBitmapW(hinst, MAKEINTRESOURCEW(id));
	if(hbm != NULL){
		create_dc();
		SelectObject(mdc, hbm);
		put_size();
	}
	return (hbm != NULL);
}


BOOL bitmap::draw(const bitmap* const bmp, DWORD rop){
	return BitBlt(mdc, 0, 0, width, height, bmp->getDC(), 0, 0, rop);
}

BOOL bitmap::draw(HDC hDC, int x, int y, DWORD rop){
	return BitBlt(hDC, x, y, width, height, mdc, 0, 0, rop);
}

BOOL bitmap::draw(HDC hDC, int x, int y, int cx, int cy, DWORD rop){
	return BitBlt(hDC, x, y, cx, cy, mdc, 0, 0, rop);
}

BOOL bitmap::draw(HDC hDC, int x, int y, int cx, int cy, int sx, int sy, DWORD rop){
	return BitBlt(hDC, x, y, cx, cy, mdc, sx, sy, rop);
}

BOOL bitmap::draw(HDC hDC, int x, int y, int cx, int cy, int ox, int oy, int sx, int sy, DWORD rop){
	return StretchBlt(hDC, x, y, cx, cy, mdc, ox, oy, sx, sy, rop);
}


void bitmap::destroy(void){
	if(hbm != NULL)
		DeleteObject(hbm);
	hbm = NULL;

	if(mdc != NULL)
		DeleteDC(mdc);
	mdc = NULL;

	width = height = 0;
}


void bitmap::create_dc(void){
	if(mdc == NULL){
		HDC hdc = GetDC(NULL);
		mdc = CreateCompatibleDC(hdc);
		ReleaseDC(NULL, hdc);
	}
}


void bitmap::put_size(void){
	BITMAP inf;
	if(GetObject(hbm, sizeof(inf), &inf)){
		width  = inf.bmWidth;
		height = (inf.bmHeight < 0) ? -inf.bmHeight : inf.bmHeight;
	}
}


//-----------------------------------------------------------------------------------------------------


bmp_mask::bmp_mask(void):hbm(NULL){}

bmp_mask::~bmp_mask() { destroy(); }


bool bmp_mask::create(const bitmap* const bmp, COLORREF color){
	destroy();
	hbm = CreateBitmap(bmp->getWidth(), bmp->getHeight(), 1, 1, NULL);
	if(hbm == NULL)
		return false;

	HDC hdc = GetDC(NULL);
	HDC mdc = CreateCompatibleDC(hdc);
	ReleaseDC(NULL, hdc);

	SelectObject(mdc, hbm);
	PatBlt(mdc, 0, 0, bmp->getWidth(), bmp->getHeight(), BLACKNESS);

	for(int y = 0; y < bmp->getHeight(); ++y){
		for(int x = 0; x < bmp->getWidth(); ++x){
			if(GetPixel(bmp->getDC(), x, y) != color)
				SetPixel(mdc, x, y, RGB(0xFF, 0xFF, 0xFF));
		}
	}
	DeleteDC(mdc);
	return true;
}


bool bmp_mask::create(const bitmap* const bmp){
	return create(bmp, GetPixel(bmp->getDC(), 0, 0));
}


bool bmp_mask::load(HINSTANCE hinst, UINT id){
	destroy();
	hbm = reinterpret_cast<HBITMAP>( LoadImageW(hinst, MAKEINTRESOURCEW(id), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_MONOCHROME) );
	return (hbm != NULL);
}


bool bmp_mask::load(LPCWSTR filename){
	destroy();
	hbm = reinterpret_cast<HBITMAP>( LoadImageW(NULL, filename, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION | LR_MONOCHROME) );
	return (hbm != NULL);
}


void bmp_mask::destroy(void){
	if(hbm != NULL)
		DeleteObject(hbm);
	hbm = NULL;
}


//-----------------------------------------------------------------------------------------------------


void font::create(bitmap* pbmp, bmp_mask* pmask, int columns){
	bmp     = pbmp;
	mask    = pmask;
	fwidth  = pbmp->getWidth() / columns;
	fheight = pbmp->getHeight(); 
}
	

void font::draw(HDC hDC, const BYTE* arr, UINT n, int x, int y){
	int px;
	const BYTE* e = arr + n;
	for(const BYTE* p = arr; p != e; ++p, x += fwidth){
		if(*p != FONT_SPACE_CODE){
			px = static_cast<int>(*p) * fwidth;
			MaskBlt(hDC, x, y, fwidth, fheight, bmp->getDC(), px, 0, mask->getHandle(), px, 0, MAKEROP4(SRCCOPY, SRCPAINT));
		}
	}
}
	

void font::draw(HDC hDC, const BYTE* arr, UINT n, int x, int y, DWORD rop){
	const BYTE* e = arr + n;
	for(const BYTE* p = arr; p != e; ++p, x += fwidth){
		if(*p != FONT_SPACE_CODE)
			bmp->draw(hDC, x, y, fwidth, fheight, static_cast<int>(*p) * fwidth, 0, rop);
	}
}


void font::draw(HDC hDC, const BYTE* arr, UINT n, int x, int y, int cx, int cy, DWORD rop){
	int px;
	const BYTE* e = arr + n;
	for(const BYTE* p = arr; p != e; ++p, x += cx){
		if(*p != FONT_SPACE_CODE){
			px = static_cast<int>(*p) * fwidth;
			StretchBlt(hDC, x, y, cx, cy, bmp->getDC(), px, 0, fwidth, fheight, rop);
		}
	}
}


//-----------------------------------------------------------------------------------------------------


sprite::sprite(void):ltime(0), 
                     delay(0), 
                     state(SP_STOP), 
                     width(0),
                     height(0),
                     px(0),
                     py(0), 
                     sizex(0), 
                     sizey(0){}


void sprite::create(const bitmap* const bmp, int num_cols, int num_rows, DWORD time_delay){
	width  = bmp->getWidth();
	height = bmp->getHeight(); 
	sizex  = bmp->getWidth()  / num_cols;
	sizey  = bmp->getHeight() / num_rows;
	delay  = time_delay;
	px     = 0;
	py     = 0;
}


BOOL sprite::draw(HDC hDC, const bitmap* const bmp, const bmp_mask* const mask, int x, int y, DWORD rop){
	return MaskBlt(hDC, x, y, sizex, sizey, bmp->getDC(), px, py, mask->getHandle(), px, py, rop);
}

BOOL sprite::draw(HDC hDC, const bitmap* const bmp, const bmp_mask* const mask, int x, int y, int cx, int cy, DWORD rop){
	return MaskBlt(hDC, x, y, cx, cy, bmp->getDC(), px, py, mask->getHandle(), px, py, rop);
}


void sprite::play(eSpState play){
	state = play;
	switch(state){
	case SP_PLAY:
	case SP_LOOP:
		px = py = 0;
		break;
	case SP_PLAYBACK: 
	case SP_LOOPBACK:
		px = width  - sizex;
		py = height - sizey;
		break;
	}
}


void sprite::stop(void){
	state = SP_STOP;
}


BOOL sprite::updateFrame(DWORD tick){
	switch(state){
	case SP_PLAY:
	case SP_LOOP:
		if((tick - ltime) < delay)
			break;

		ltime = tick;
		px   += sizex;
		if(px >= (width-5)){
			px  = 0;
			py += sizey;
			if(py >= (height-5)){
				py = 0;
				if(state == SP_PLAY)
					stop();
				return TRUE;
			}
		}
		break;
	case SP_PLAYBACK: 
	case SP_LOOPBACK:
		if((tick - ltime) < delay)
			break;

		ltime = tick;
		px   -= sizex;
		if(px < 5){
			px  = width - sizex;
			py -= sizey;
			if(py < 5){
				py = height - sizey;
				if(state == SP_PLAYBACK)
					stop();
				return TRUE;
			}
		}
		break;
	}
	return FALSE;
}
