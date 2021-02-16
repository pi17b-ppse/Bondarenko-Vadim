#include "game.h"
#define isKeyState(vk)   (GetAsyncKeyState((vk)) & 0x8000)
#define isKeyLeftRight() (isKeyState(VK_LEFT) || isKeyState('A') || isKeyState(VK_RIGHT) || isKeyState('D'))
static LRESULT CALLBACK options_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL g_audio    = TRUE;
static BOOL g_repeat   = TRUE;
static int  g_velocity = VELOCITY_DEF;



tetris::tetris(void):bback(NULL), 
                     dcbmp(NULL), 
                     bfill(NULL), 
                     blocks(NULL), 
                     bother(NULL), 
                     bnextf(NULL), 
                     balpha(NULL), 
                     field_left(0), 
                     field_right(0), 
                     figure_x(0), 
                     figure_y(0), 
                     velocity(0), 
                     boom_x(0), 
                     sel_cmd(0), 
                     offset_m(0), 
                     prev_index(-1), 
                     delay(DELAY_TIME), 
                     prev_time(0), 
                     time_boom(0), 
                     score(0), 
                     start(false), 
                     state(GAME_MENU) {}

tetris::~tetris(){}


//создание
void ARGS2_FASTCALL tetris::onCreate(HINSTANCE hinst, HWND hwnd){
	srand(static_cast<unsigned int>(time(NULL)));

	RECT rc;
	GetClientRect(hwnd, &rc);
	size.cx = rc.right;
	size.cy = rc.bottom;

	field_left  = 40;
	field_right = field_left + FIELD_COLS * CELL_SIZE;

	int fcx = FIELD_COLS * CELL_SIZE;
	int fcy = FIELD_ROWS * CELL_SIZE;
	dcbmp   = new bitmap(fcx, fcy);
	bfill   = new bitmap(fcx, fcy);

	brush br(hinst, IDB_BITMAP_GROUND);
	bback = new bitmap(size.cx - field_right, fcy);
	br.fill(bback->getDC(), 0, 0, bback->getWidth(), bback->getHeight());
	br.destroy();

	blocks = new bitmap(hinst, IDB_BITMAP_BLOCKS);

	br.load(hinst, IDB_BITMAP_FILL);
	POINT pt;
	SetBrushOrgEx(bfill->getDC(), 8, 0, &pt);
	br.fill(bfill->getDC(), 0, 0, fcx, fcy);
	SetBrushOrgEx(bfill->getDC(), pt.x, pt.y, NULL);
	br.destroy();

	rect.left   = field_left;
	rect.top    = 0;
	rect.right  = rect.left + fcx;
	rect.bottom = fcy;

	figure_x  = figure_y = 0;
	delay     = DELAY_TIME;
	prev_time = 0;
	sel_cmd   = 0;
	state     = GAME_MENU;
	offset_m  = (size.cx - dcbmp->getWidth()) / 2;

	bother = new bitmap(hinst, IDB_BITMAP_NEXTF);
	bnextf = new bitmap(NEXTF_WIDTH, NEXTF_HEIGHT);
	balpha = new bitmap(hinst, IDB_BITMAP_ALPHA);

	mask_alpha.create(balpha);
	fnt.create(balpha, &mask_alpha, 32);

	figures_load(figures, hinst, MAKEINTRESOURCEW(IDR_TEXT_FIGURES), L"TEXT");

	snd_rot.load(hinst,  MAKEINTRESOURCEW(IDR_WAVE_ROTATE), L"WAVE");
	snd_stop.load(hinst, MAKEINTRESOURCEW(IDR_WAVE_STOP),   L"WAVE");
	snd_boom.load(hinst, MAKEINTRESOURCEW(IDR_WAVE_BOOM),   L"WAVE");

	bpane.load(hinst, IDB_BITMAP_PANE);

	set_state(hwnd, GAME_MENU);

	SetTimer(hwnd, IDT_TIMER, DELAY_TIME, NULL);
}


//событие клавиатуры(нажатие)
void ARGS2_FASTCALL tetris::onKeyDown(HWND hwnd, WORD key){
	int  ret;
	bool ok;
	switch(state){
	case GAME_PLAY:

		switch(key){
		case VK_LEFT: //движение влево
		case 'A':
			if(is_move_horz(figure_x, figure_y, figure, field, velocity, true)){
				--figure_x;
				update_figure(hwnd);
			}
			break;
		case VK_RIGHT: //движение вправо
		case 'D':
			if(is_move_horz(figure_x, figure_y, figure, field, velocity, false)){
				++figure_x;
				update_figure(hwnd);
			}
			break;
		case VK_DOWN: //движение вниз
		case 'S':
			if(is_move_down(figure_x, figure_y, figure, VELOCITY_DOWN, field)){
				figure_y += VELOCITY_DOWN;
				update_figure(hwnd);
			} else { //здесь сделать блоки частью стены

				if(is_game_over()){ //вы проиграли
					set_state(hwnd, GAME_OVER);
					break;
				}

				figure_put_field(figure_x, figure_y, figure, field);

				ok = field_remove(field, false, &booms);
				score_add(booms.getSize());
				put_rand_figure(hwnd);

				if(ok){
					time_boom = timeGetTime() + TIME_BOOM;
					boom_x    = 0;
					snd_boom.play(&g_audio);
				} else
					snd_stop.play(&g_audio);
			}
			break;
		case VK_UP: //вращение
		case 'W':
			if(figure_rotate(figure_x, figure_y, figure, field)){
				snd_rot.play(&g_audio);
				update_figure(hwnd);
			}
			break;
		case VK_ESCAPE:
			set_state(hwnd, GAME_PAUSE);
			Sleep(100);
			break;
		}
		break;
	case GAME_MENU:
		exec_menu(hwnd, control_menu(hwnd, key, 3));
		break;
	case GAME_OVER:
		ret = control_menu(hwnd, key, 2);
		if(ret == 0)
			start_game(hwnd);
		else if(ret == 1)
			set_state(hwnd, GAME_MENU);
		break;
	case GAME_PAUSE:
		ret = control_menu(hwnd, key, 2);
		if(ret == 0)
			set_state(hwnd, GAME_PLAY);
		else if(ret == 1)
			set_state(hwnd, GAME_MENU);
		break;
	}
}


//движение мыши
void tetris::onMouseMove(HWND hwnd, int x, int y){
	int ret;
	switch(state){
	case GAME_OVER:
	case GAME_PAUSE:
		ret = move_button(x, y, 2);
		if((ret != -1) && (ret != sel_cmd)){
			sel_cmd = ret;
			update_menu(hwnd);
		}
		break;
	case GAME_MENU:
		ret = move_button(x, y, 3);
		if((ret != -1) && (ret != sel_cmd)){
			sel_cmd = ret;
			update_menu(hwnd);
		}
		break;
	}
}
	

//нажатие кнопки мыши
void tetris::onMouseDown(HWND hwnd, int x, int y){
	int ret;
	switch(state){
	case GAME_PAUSE:
		ret = move_button(x, y, 2);
		if(ret == 0)
			set_state(hwnd, GAME_PLAY);
		else if(ret == 1)
			set_state(hwnd, GAME_MENU);
		break;
	case GAME_OVER:
		ret = move_button(x, y, 2);
		if(ret == 0)
			start_game(hwnd);
		else if(ret == 1)
			set_state(hwnd, GAME_MENU);
		break;
	case GAME_MENU:
		exec_menu(hwnd, move_button(x, y, 3));
		break;
	}
}


//таймер
void ARGS2_FASTCALL tetris::onTimer(HWND hwnd, UINT id){
	DWORD tick;
	switch(state){
	case GAME_PLAY:
		tick = timeGetTime();
		if((tick - prev_time) > delay){
			prev_time = tick;
			update_frame(hwnd, tick);
			InvalidateRect(hwnd, &rect, TRUE);
		}
		break;
	}
}


//рисование
void ARGS2_FASTCALL tetris::onPaint(HWND hwnd, HDC hDC){
	switch(state){
	case GAME_PLAY:
		draw_game(hDC);
		break;
	case GAME_MENU:
		draw_menu(hDC);
		break;
	case GAME_PAUSE:
		draw_pause(hDC);
		break;
	case GAME_OVER:
		draw_over(hDC);
		break;
	}
}


//уничтожение окна
void ARGS2_FASTCALL tetris::onDestroy(HWND hwnd){
	KillTimer(hwnd, IDT_TIMER);
	figures.clear();
	delete bback;
	delete dcbmp;
	delete bfill;
	delete blocks;
	delete bother;
	delete bnextf;
	delete balpha;
	bpane.destroy();
	mask_alpha.destroy();
	snd_rot.destroy();
	snd_stop.destroy();
	snd_boom.destroy();
}


//-----------------------------------------------------------------------------------------------------


//старт игры
void ARGS2_FASTCALL tetris::start_game(HWND hwnd){
	booms.reset();
	memset(field, 0, sizeof(field));

	velocity   = g_velocity;
	start      = false;
	time_boom  = score = 0;
	boom_x     = 0;
	prev_index = -1;
	put_rand_figure(hwnd);
	set_state(hwnd, GAME_PLAY);
}


//задание случайной фигуры
void ARGS2_FASTCALL tetris::put_rand_figure(HWND hwnd){
	int index, icur;
	if(!start){
		prev_index = index = rand() % figures.getSize();
		memcpy(&figure, &figures[index], sizeof(figure_t));

		if(!g_repeat){
			do {
				icur = rand() % figures.getSize();
			} while(icur == index);
		} else
			icur = rand() % figures.getSize();

		memcpy(&next_figure, &figures[icur], sizeof(figure_t));
		figure_set(figure, RAND_BLOCKS);
		figure_set(next_figure, RAND_BLOCKS);
		start = true;
	} else {
		memcpy(&figure, &next_figure, sizeof(figure_t));

		if(!g_repeat){
			do {
				icur = rand() % figures.getSize();
			} while(icur == prev_index);
		} else
			icur = rand() % figures.getSize();

		memcpy(&next_figure, &figures[icur], sizeof(figure_t));
		figure_set(next_figure, RAND_BLOCKS);
		prev_index = icur;
	}

	if((rand() % 2) != 0)
		figure_transponse(next_figure);

	figure_y = -figure.size * CELL_SIZE;
	figure_x = rand() % (FIELD_COLS - figure.size);

	bother->draw(bnextf->getDC(), 0, 0, NEXTF_WIDTH, NEXTF_HEIGHT, 0, 0);

	const int edge  = 31;
	const int csize = 18;

	RECT rc;
	figure_size(next_figure, &rc, csize);

	const int left = (NEXTF_WIDTH - (rc.right - rc.left)) / 2 - rc.left;
	const int top  = (edge + (91 - (rc.bottom - rc.top)) / 2) - rc.top;

	for(int i = 0; i < next_figure.size; ++i){
		for(int j = 0; j < next_figure.size; ++j){
			if(next_figure.figure[i][j] != BLOCK_NONE)
				bother->draw(bnextf->getDC(), left + j*csize, top + i*csize, csize, csize, 0, 182, SRCCOPY);
		}
	}

	WCHAR    s[16];
	int      n = swprintf(s, L"%lu", score);
	COLORREF c = SetTextColor(bnextf->getDC(), RGB(0xFF, 0xFF, 0xFF));
	int   mode = SetBkMode(bnextf->getDC(), TRANSPARENT);

	POINT pos = {0, bnextf->getHeight() };
	SIZE  fsz = {0};
	if(GetTextExtentPoint32W(bnextf->getDC(), s, n, &fsz)){
		pos.x  = (bnextf->getWidth() - fsz.cx) / 2;
		pos.y -= fsz.cy + fsz.cy/2 - 1;
	}
	TextOutW(bnextf->getDC(), pos.x, pos.y, s, n);

	SetBkMode(bnextf->getDC(), mode);
	SetTextColor(bnextf->getDC(), c);

	rc.left   = field_right;
	rc.top    = 0;
	rc.right  = rc.left + NEXTF_WIDTH;
	rc.bottom = rc.top  + NEXTF_HEIGHT;
	InvalidateRect(hwnd, &rc, TRUE);
}


//обновление кадра
void ARGS2_FASTCALL tetris::update_frame(HWND hwnd, DWORD tick){
	if((time_boom != 0) && (tick > time_boom)){
		field_remove(field, true);
		time_boom = 0;
		booms.reset();
	}

	if(is_move_down(figure_x, figure_y, figure, velocity, field))
		figure_y += velocity;
	else if(! isKeyLeftRight()){ //здесь сделать блоки частью стены

		if(is_game_over()){ //вы проиграли
			set_state(hwnd, GAME_OVER);
			return;
		}
		figure_put_field(figure_x, figure_y, figure, field);

		bool ok = field_remove(field, false, &booms);
		score_add(booms.getSize());
		put_rand_figure(hwnd);
		
		if(ok){
			time_boom = tick + TIME_BOOM;
			boom_x    = 0;
			snd_boom.play(&g_audio);
		} else
			snd_stop.play(&g_audio);
	}
}


//вывод игры
void ARGS2_FASTCALL tetris::draw_game(HDC hDC){
	int i, j, box;
	bback->draw(hDC, 0, 0, field_left, dcbmp->getHeight());
	bback->draw(hDC, field_right, bnextf->getHeight(), bback->getWidth(), size.cy - bnextf->getHeight() - CTRL_HEIGHT);
	dcbmp->draw(bfill);

	//здесь выводим
	HDC hdc = dcbmp->getDC();

	//вывод фигуры
	int px = figure_x * CELL_SIZE;
	int py = figure_y;
	for(i = 0; i < figure.size; ++i){
		for(j = 0; j < figure.size; ++j){
			if(figure.figure[i][j] != BLOCK_NONE)
				blocks->draw(hdc, px + j * CELL_SIZE, py + i * CELL_SIZE, CELL_SIZE, CELL_SIZE, figure.figure[i][j] - 1, 0);
		}
	}

	//вывод блоков
	for(i = 0; i < FIELD_ROWS; ++i){
		for(j = 0; j < FIELD_COLS; ++j){
			if((box = field[i][j]) == BLOCK_NONE)
				continue;

			if(box <= BLOCK_C) //неподвижные блоки
				blocks->draw(hdc, j * CELL_SIZE, i * CELL_SIZE, CELL_SIZE, CELL_SIZE, box - 1, CELL_SIZE);
			else//блоки которые для взрыва
				blocks->draw(hdc, j * CELL_SIZE, i * CELL_SIZE, CELL_SIZE, CELL_SIZE, box - BLOCK_BOOM - 1, CELL_OFFSET_BOOM);
		}
	}

	//вывод "взрыва"
	boom_row* p;
	for(i = 0; i < booms.getSize(); ++i){
		p = &booms[i];
		if(p->dir == 0){
			boom_x += VELOCITY_BOOM;
			if(boom_x >= (dcbmp->getWidth() - 20)){
				boom_x = dcbmp->getWidth() - 20;
				p->dir = 1;
			}
		} else {
			boom_x -= VELOCITY_BOOM;
			if(boom_x < 20){
				boom_x = 20;
				p->dir = 0;
			}
		}
		bother->draw(hdc, boom_x, p->pos, 20, 20, 0, 200, SRCPAINT);
	}

	BitBlt(hDC, field_left, 0, dcbmp->getWidth(), dcbmp->getHeight(), hdc, 0, 0, SRCCOPY);

	bnextf->draw(hDC, field_right, 0, NEXTF_WIDTH, NEXTF_HEIGHT, 0, 0);
	bother->draw(hDC, field_right, size.cy - CTRL_HEIGHT, NEXTF_WIDTH, CTRL_HEIGHT, 0, 220);
}


//вывод проигрыша
void ARGS2_FASTCALL tetris::draw_over(HDC hDC){
	draw_background(hDC);
	const int width = dcbmp->getWidth();

	const BYTE game[] = { 2, 27, 77, 15, 16, 14, 8, 3, 16, 0, 11, 8, 77, 8, 3, 16, 19};
	const int     fsz = 24;
	fnt.draw(dcbmp->getDC(), game, sizeof(game), (width - fsz*int(sizeof(game)))/2, 100, fsz, 28, SRCPAINT);

	const BYTE ply[]   = { 13, 0, 23, 0, 18, 28, 77, 7, 0, 13, 14, 2, 14};
	const BYTE title[] = { 2, 27, 9, 18, 8, 77, 2, 77, 13, 0, 23, 0, 11, 14};
	draw_buttons(dcbmp->getDC(), 4, ply, sizeof(ply), title, sizeof(title));

	const int left = (size.cx - dcbmp->getWidth())/2;
	BitBlt(hDC, left, 0, dcbmp->getWidth(), dcbmp->getHeight(), dcbmp->getDC(), 0, 0, SRCCOPY);
}


//вывод паузы
void ARGS2_FASTCALL tetris::draw_pause(HDC hDC){
	draw_background(hDC);
	const int width = dcbmp->getWidth();

	const BYTE ps[] = { 15, 0, 19, 7, 0};
	const int   fsz = 32;
	fnt.draw(dcbmp->getDC(), ps, sizeof(ps), (width - fsz*int(sizeof(ps)))/2, 100, fsz, 34, SRCPAINT);

	const BYTE cns[] = { 15, 16, 14, 4, 14, 11, 6, 8, 18, 28, 77, 8, 3, 16, 0, 18, 28};
	const BYTE exs[] = { 2, 27, 21, 14, 4, 77, 2, 77, 12, 5, 13, 30};
	draw_buttons(dcbmp->getDC(), 4, cns, sizeof(cns), exs, sizeof(exs));

	const int left = (size.cx - dcbmp->getWidth())/2;
	BitBlt(hDC, left, 0, dcbmp->getWidth(), dcbmp->getHeight(), dcbmp->getDC(), 0, 0, SRCCOPY);
}


//вывод меню игры
void ARGS2_FASTCALL tetris::draw_menu(HDC hDC){
	draw_background(hDC);

	//вывести надпись тетрис
	const CHAR tetris[] = {
		"### ### ### ### #   # ###\n"\
		" #  #    #  # # #  ## #   \n"\
		" #  ###  #  ### # # # #   \n"\
		" #  #    #  #   ##  # #   \n"\
		" #  ###  #  #   #   # ###"
	};

	const int csize = 18;
	int    x = 0, y = 0, px = csize - 4, py = csize * 3;
	LPCSTR p = &tetris[0];
	while(*p){
		if(*p == '#'){
			bother->draw(dcbmp->getDC(), px + x*csize, py + y*csize, csize, csize, 0, 182, SRCCOPY);
			++x;
		} else if(*p == '\n'){
			++y;
			x = 0;
		} else
			++x;
		++p;
	}

	const BYTE start[] = { 13, 0, 23, 0, 18, 28, 77, 8, 3, 16, 0, 18, 28};
	const BYTE opts[]  = { 13, 0, 17, 18, 16, 14, 9, 10, 0};
	const BYTE quit[]  = { 2, 27, 9, 18, 8, 77, 8, 7, 77, 8, 3, 16, 27};
	draw_buttons(dcbmp->getDC(), 6, start, sizeof(start), opts, sizeof(opts), quit, sizeof(quit));

	const BYTE ctrl[] = { 19, 15, 16, 0, 2, 11, 5, 13, 8, 5, 77, 8, 3, 16, 14, 9};
	const int cx = 14;
	fnt.draw(dcbmp->getDC(), ctrl, sizeof(ctrl), (dcbmp->getWidth() - cx*int(sizeof(ctrl)))/2, dcbmp->getHeight() - 47, cx, cx + 1, SRCCOPY);

	const BYTE help[] = { 10, 11, 0, 2, 8, 24, 0, 77, 2, 2, 5, 16, 21, 77, 2, 13, 8, 7, 77, 2, 11, 5, 2, 14, 77, 2, 15, 16, 0, 2, 14};
	fnt.draw(dcbmp->getDC(), help, sizeof(help), (dcbmp->getWidth() - cx*int(sizeof(help)))/2, dcbmp->getHeight() - 27, cx, cx + 1, SRCCOPY);

	const int left = (size.cx - dcbmp->getWidth())/2;
	BitBlt(hDC, left, 0, dcbmp->getWidth(), dcbmp->getHeight(), dcbmp->getDC(), 0, 0, SRCCOPY);
}


//----------------------------------------------------------------------------------------------------------------------


void tetris::draw_background(HDC hDC){
	PatBlt(dcbmp->getDC(), 0, 0, dcbmp->getWidth(), dcbmp->getHeight(), BLACKNESS);
	bpane.fill(hDC, 0, 0, PANE_WIDTH, dcbmp->getHeight());
	bpane.fill(hDC, PANE_WIDTH + dcbmp->getWidth(), 0, dcbmp->getWidth() + (PANE_WIDTH << 1) + 1, bback->getHeight());
}


void tetris::exec_menu(HWND hwnd, int cmd){
	if(cmd == 0)
		start_game(hwnd);
	else if(cmd == 1) //диалог опции(options)
		DialogBoxW((HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCEW(IDD_OPTIONS), hwnd, reinterpret_cast<DLGPROC>(&options_proc));
	else if(cmd == 2) //выход
		SendMessage(hwnd, WM_CLOSE, 0, 0);
}


//перерисовка фигуры
void ARGS2_FASTCALL tetris::update_figure(HWND hwnd){
	const int m = CELL_SIZE << 1;
	RECT rc;
	rc.left   = figure_x * CELL_SIZE - CELL_SIZE;
	rc.top    = figure_y - CELL_SIZE;
	rc.right  = min(rc.left + OBJ_LENGTH + (m << 1), field_right); 
	rc.bottom = rc.top  + OBJ_LENGTH + m;
	InvalidateRect(hwnd, &rc, TRUE);
}


//добавление очков
void tetris::score_add(int n){
	if(n > 0)
		score += (n == 1) ? 10UL : static_cast<DWORD>(n)*15UL;
}


//проверка на проигрыш
bool tetris::is_game_over(void){
	int j = 0;
	while((j < FIELD_COLS) && (field[0][j] == BLOCK_NONE))
		++j;
	return (j < FIELD_COLS);
}


//вывод кнопок
void tetris::draw_buttons(HDC hDC, int num,...){
	const BYTE* pb;
	UINT    nb;
	int     x, p = 0;
	RECT    rc;
	va_list lst;
	va_start(lst, num);
	for(int i = 0; i < num; i += 2, ++p){
		set_rect(&rc, p*80);
		pb = va_arg(lst, const BYTE*);
		nb = va_arg(lst, UINT);
		x  = (p == sel_cmd) ? 89 : 21;

		bother->draw(hDC, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, x, 184, 69, 33, SRCCOPY);

		fnt.draw(hDC, pb, nb, rc.left + ((rc.right - rc.left) - static_cast<int>(nb)*fnt.getWidth())/2, 
		         rc.top + ((rc.bottom - rc.top) - fnt.getHeight())/2);
	}
	va_end(lst);
}


void ARGS2_FASTCALL tetris::set_rect(LPRECT prc, int top){
	SetRect(prc, 20, 250 + top, dcbmp->getWidth() - 20, 300 + top);
}


int tetris::control_menu(HWND hwnd, WORD key, int num){
	switch(key){
	case VK_UP:
	case 'W':
		if(sel_cmd > 0){
			--sel_cmd;
			update_menu(hwnd);
		}
		break;
	case VK_DOWN:
	case 'S':
		if(sel_cmd < (num - 1)){
			++sel_cmd;
			update_menu(hwnd);
		}
		break;
	case VK_RETURN:
		return sel_cmd;
	}
	return -1;
}


void tetris::update_menu(HWND hwnd){
	RECT rc = { offset_m, 240, offset_m + dcbmp->getWidth(), size.cy - 2 };
	InvalidateRect(hwnd, &rc, TRUE);
}


void tetris::set_state(HWND hwnd, e_state_game st){
	sel_cmd = 0;
	state   = st;
	InvalidateRect(hwnd, NULL, TRUE);
}


int tetris::move_button(int x, int y, int num){
	RECT rc;
	for(int i = 0; i < num; ++i){
		set_rect(&rc, i*80);
		OffsetRect(&rc, offset_m, 0);
		if((x > rc.left && x < rc.right) && (y > rc.top && y < rc.bottom))
			return i;
	}
	return -1;
}


//обработчик диалога
static LRESULT CALLBACK options_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
	const WCHAR vel1[] = { 0x41F,0x435,0x440,0x432,0x430,0x44F,0x20,0x441,0x43A,0x43E,0x440,0x43E,0x441,0x442,0x44C,0x0};
	const WCHAR vel2[] = { 0x412,0x442,0x43E,0x440,0x430,0x44F,0x20,0x441,0x43A,0x43E,0x440,0x43E,0x441,0x442,0x44C,0x0};
	const WCHAR vel3[] = { 0x422,0x440,0x435,0x442,0x44C,0x44F,0x20,0x441,0x43A,0x43E,0x440,0x43E,0x441,0x442,0x44C,0x0};

	switch(msg){
	case WM_INITDIALOG:
		SendDlgItemMessageW(hwnd, IDC_COMBO_VELOCITY, CB_ADDSTRING, 0, (LPARAM)vel1);
		SendDlgItemMessageW(hwnd, IDC_COMBO_VELOCITY, CB_ADDSTRING, 0, (LPARAM)vel2);
		SendDlgItemMessageW(hwnd, IDC_COMBO_VELOCITY, CB_ADDSTRING, 0, (LPARAM)vel3);
		SendDlgItemMessage(hwnd, IDC_COMBO_VELOCITY, CB_SETCURSEL, (WPARAM)(g_velocity - 1), 0);
		SendDlgItemMessage(hwnd, IDC_CHECK_AUDIO, BM_SETCHECK, (g_audio) ? BST_CHECKED : BST_UNCHECKED, 0);
		SendDlgItemMessage(hwnd, IDC_CHECK_REPEAT, BM_SETCHECK, (!g_repeat) ? BST_CHECKED : BST_UNCHECKED, 0);
		return 1;
	case WM_COMMAND:
		switch(LOWORD(wParam)){
		case IDC_BUTTON_OK:
			g_velocity = (int)SendDlgItemMessage(hwnd, IDC_COMBO_VELOCITY, CB_GETCURSEL, 0, 0) + 1;
			g_audio    = (SendDlgItemMessage(hwnd, IDC_CHECK_AUDIO, BM_GETCHECK, 0, 0)  == BST_CHECKED) ? TRUE  : FALSE;
			g_repeat   = (SendDlgItemMessage(hwnd, IDC_CHECK_REPEAT, BM_GETCHECK, 0, 0) == BST_CHECKED) ? FALSE : TRUE;
			EndDialog(hwnd, 1);
			break;
		case IDC_BUTTON_CANCEL:
			EndDialog(hwnd, 0);
			break;
		}
		break;
	case WM_CLOSE:
		EndDialog(hwnd, 0);
		break;
	}
	return 0;
}
