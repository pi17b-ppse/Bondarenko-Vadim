//Автор(с): Кудуштеев Алексей
#if! defined(_GAME_KUDUSHTEEV_H_)
#define _GAME_KUDUSHTEEV_H_
#if defined(_MSC_VER) && _MSC_VER > 1000
#pragma once
#endif

#if defined(_MSC_VER) || defined(__BORLANDC__)
#define  ARGS2_FASTCALL  __fastcall
#else
#define  ARGS2_FASTCALL
#endif

#define  IDT_TIMER    1000
#define  TIME_BOOM    700UL
#define  DELAY_TIME   25
#define  FIELD_COLS   20
#define  FIELD_ROWS   25
#define  CELL_SIZE    24
#define  CELL_MID     12
#define  VELOCITY_DEF 1  //мин-1 макс-3

#define  VELOCITY_DOWN    9
#define  VELOCITY_BOOM    28
#define  CELL_OFFSET_BOOM 48

#define  BLOCK_NONE   0
#define  BLOCK_A      1
#define  BLOCK_B      25
#define  BLOCK_C      49
#define  BLOCK_BOOM   72
#define  MAX_BLOCKS   3

#define  OBJ_LENGTH   (OBJ_SIZE * CELL_SIZE)

#define  FIELD_WIDTH  (FIELD_COLS * CELL_SIZE)
#define  FIELD_HEIGHT (FIELD_ROWS * CELL_SIZE)

#ifndef min
#define min(a, b)  (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#define max(a, b)  (((a) > (b)) ? (a) : (b))
#endif

#ifdef __BORLANDC__
#pragma warn -8027
#endif

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <mmsystem.h>
#include "bitmap.h"
#include "util.h"
#include "sound.h"
#include "resource.h"

#ifdef _MSC_VER
#pragma comment(lib, "winmm.lib")
#endif

#define  RAND_BLOCKS  ((rand() % MAX_BLOCKS) * CELL_SIZE + 1)

#define  NEXTF_WIDTH  161
#define  NEXTF_HEIGHT 182
#define  PANE_WIDTH   100
#define  CTRL_HEIGHT  36


enum e_state_game {
	GAME_MENU, GAME_PLAY, GAME_OVER, GAME_PAUSE
};


//класс-приложение игры тетрис
class tetris {
private:
	bitmap*  bback;
	bitmap*  dcbmp;
	bitmap*  bfill;
	bitmap*  blocks;
	bitmap*  bother;
	bitmap*  bnextf;
	bitmap*  balpha;
	brush    bpane;
	bmp_mask mask_alpha;
	font     fnt;
	int      field_left;
	int      field_right;
	int      figure_x, figure_y;
	int      velocity;
	int      boom_x;
	int      sel_cmd;
	int      offset_m;
	int      prev_index;
	DWORD    delay;
	DWORD    prev_time;
	DWORD    time_boom;
	DWORD    score;
	SIZE     size;
	RECT     rect;
	int      field[FIELD_ROWS][FIELD_COLS];
	figure_t figure;
	figure_t next_figure;
	bool     start;
	sound    snd_rot;
	sound    snd_stop;
	sound    snd_boom;

	pod_block<boom_row, FIELD_ROWS> booms;
	pod_array<figure_t> figures;
	e_state_game        state;
public:
	tetris(void);
	tetris(const tetris&);
	~tetris();

	tetris& operator = (const tetris&);
public:
	void ARGS2_FASTCALL onCreate(HINSTANCE hinst, HWND hwnd);
	void ARGS2_FASTCALL onKeyDown(HWND hwnd, WORD key);
	void ARGS2_FASTCALL onDestroy(HWND hwnd);
	void onMouseMove(HWND hwnd, int x, int y);
	void onMouseDown(HWND hwnd, int x, int y);
	void ARGS2_FASTCALL onTimer(HWND hwnd, UINT id);
	void ARGS2_FASTCALL onPaint(HWND hwnd, HDC hDC);
private:
	void ARGS2_FASTCALL update_frame(HWND hwnd, DWORD tick);
	void ARGS2_FASTCALL start_game(HWND hwnd);
	void ARGS2_FASTCALL put_rand_figure(HWND hwnd);
	void ARGS2_FASTCALL draw_menu(HDC hDC);
	void ARGS2_FASTCALL draw_game(HDC hDC);
	void ARGS2_FASTCALL draw_over(HDC hDC);
	void ARGS2_FASTCALL draw_pause(HDC hDC);
	void ARGS2_FASTCALL update_figure(HWND hwnd);
	void score_add(int n);
	bool is_game_over(void);
	void ARGS2_FASTCALL set_rect(LPRECT prc, int top);
	int  control_menu(HWND hwnd, WORD key, int num);
	void update_menu(HWND hwnd);
	void set_state(HWND hwnd, e_state_game st);
	int  move_button(int x, int y, int num);
	void draw_buttons(HDC hDC, int num,...);
	void exec_menu(HWND hwnd, int cmd);
	void draw_background(HDC hDC);
};


#endif
