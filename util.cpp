#include "game.h"
static LPCSTR _next_scan(LPCSTR s, int& v);


//загрузка фигур из файла или ресурсов
bool figures_load(pod_array<figure_t>& fgs, HINSTANCE hinst, LPCWSTR filename, LPCWSTR type){
	LPSTR   buf;
	HGLOBAL data = NULL;

	if(hinst == NULL){
		HANDLE fp = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, NULL, 
		                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if(fp == INVALID_HANDLE_VALUE)
			return false;

		DWORD len = GetFileSize(fp, NULL);
		if(len == INVALID_FILE_SIZE){
			CloseHandle(fp);
			return false;
		}

		buf = reinterpret_cast<LPSTR>( HeapAlloc(GetProcessHeap(), 0, (len + 1) * sizeof(CHAR)) );
		if(buf == NULL){
			CloseHandle(fp);
			return false;
		}

		DWORD   n = 0;
		BOOL  ret = ReadFile(fp, buf, len, &n, NULL);
		CloseHandle(fp);

		if(!ret || (n != len)){
			delete[] buf;
			return false;
		}
		buf[n] = '\0';
	} else {

		if(type == NULL)
			return false;

		HRSRC src = FindResourceW((HMODULE)hinst, filename, type);
		if(src == NULL)
			return false;

		data = LoadResource((HMODULE)hinst, src);
		if(data == NULL)
			return false;

		buf = reinterpret_cast<LPSTR>( LockResource(data) );
		if(buf == NULL){
			FreeResource(data);
			return false;
		}
	}

	int      m, a, b;
	size_t   i;
	figure_t fg;
	LPCSTR   p = _next_scan(buf, m);
	if(p == NULL)
		goto _err;

	fgs.setSize(static_cast<size_t>(m));
	for(i = 0; i < fgs.getSize(); ++i){
		if((p = _next_scan(p, m)) == NULL)
			goto _err;

		fg.size = m;
		for(a = 0; a < m; ++a){
			for(b = 0; b < m; ++b){
				if((p = _next_scan(p, fg.figure[a][b])) == NULL)
					goto _err;
			}
		}
		memcpy(&fgs[i], &fg, sizeof(fg));
	}
_err:

	if(data != NULL){
		UnlockResource(data);
		FreeResource(data);
	} else
		HeapFree(GetProcessHeap(), 0, buf);
	return (fgs.getSize() > 0);
}


//реверсирование строк
void figure_reverse_horz(figure_t& src){
	int i = 0, j = src.size - 1;
	for(; i < j; ++i, --j){
		for(int r = 0; r < src.size; ++r)
			swap_value(src[r][i], src[r][j]);
	}	
}


//реверсирование столбцов
void figure_reverse_vert(figure_t& src){
	int i = 0, j = src.size - 1;
	for(; i < j; ++i, --j){
		for(int c = 0; c < src.size; ++c)
			swap_value(src[i][c], src[j][c]);
	}
}


//транспонирование
void figure_transponse(figure_t& dst, const figure_t& src){
	dst.size = src.size;
	for(int i = 0; i < src.size; ++i){
		for(int j = 0; j < src.size; ++j)
			dst[j][i] = src[i][j];
	}	
}


//транспонирование
void figure_transponse(figure_t& src){
	for(int i = 0; i < src.size; ++i){
		for(int j = 0; j < src.size; ++j)
			swap_value(src[j][i], src[i][j]);
	}
}


//изменение
void figure_set(figure_t& src, int val){
	for(int i = 0; i < src.size; ++i){
		for(int j = 0; j < src.size; ++j){
			if(src[i][j] != BLOCK_NONE)
				src[i][j] = val;
		}
	}
}


//вращение
bool figure_rotate(int x, int y, figure_t& figure, int field[][FIELD_COLS]){
	int i, j, col, row;
	figure_t temp;

	//вращение
	memset(&temp, 0, sizeof(temp));
	figure_transponse(temp, figure);
	figure_reverse_vert(temp);

	for(i = 0; i < temp.size; ++i){
		for(j = 0; j < temp.size; ++j){
			if(temp[i][j] == BLOCK_NONE)
				continue;

			col = x + j;
			row = (y + i * CELL_SIZE) / CELL_SIZE;
			if(row < 0)
				continue;
			else if((col < 0) || (col >= FIELD_COLS) || (row >= FIELD_ROWS))
				return false;
			else if(field[row][col] > BLOCK_NONE)
				return false;

			row = ((y + CELL_SIZE) + i * CELL_SIZE) / CELL_SIZE;
			if((row >= FIELD_ROWS) || (field[row][col] > BLOCK_NONE))
				return false;
		}
	}
	memcpy(&figure, &temp, sizeof(temp));
	return true;
}


//проверка на движение влево или вправо
bool is_move_horz(int x, int y, const figure_t& figure, int field[][FIELD_COLS], int velocity, bool left){
	int col, row;
	x = (left) ? x - 1 : x + 1;
	for(int i = 0; i < figure.size; ++i){
		for(int j = 0; j < figure.size; ++j){
			if(figure.figure[i][j] > BLOCK_NONE){

				col = x + j;
				row = (y + velocity + i * CELL_SIZE) / CELL_SIZE;

				if((col < 0) || (col >= FIELD_COLS) || (row >= FIELD_ROWS))
					return false;
				else if(row < 0)
					continue;
				else if(field[row][col] > BLOCK_NONE)
					return false;

				row = ((y + CELL_SIZE - velocity) + i * CELL_SIZE) / CELL_SIZE;
				if((row >= FIELD_ROWS) || (field[row][col] > BLOCK_NONE))
					return false;
			}
		}
	}
	return true;
}


//проверка на движение вниз
BOOL is_move_down(int x, int y, const figure_t& figure, int step, int field[][FIELD_COLS]){
	int col, row;
	y += CELL_SIZE + step;
	for(int i = 0; i < figure.size; ++i){
		for(int j = 0; j < figure.size; ++j){
			if(figure[i][j] == BLOCK_NONE)
				continue;

			col = x + j;
			row = (y + i * CELL_SIZE) / CELL_SIZE;
			if(row < 0)
				continue;
			else if((row >= FIELD_ROWS) || (field[row][col] > BLOCK_NONE))
				return FALSE;
		}
	}
	return TRUE;
}


//присвoить фигуру полю
void figure_put_field(int x, int y, const figure_t& figure, int field[][FIELD_COLS]){
	int row, col;
	y += CELL_MID;
	for(int i = 0; i < figure.size; ++i){
		for(int j = 0; j < figure.size; ++j){
			col = x + j;
			row = (y + i * CELL_SIZE) / CELL_SIZE;
			if((col >= 0 && col < FIELD_COLS) && (row >= 0 && row < FIELD_ROWS)){
				if(field[row][col] == BLOCK_NONE)
					field[row][col] = figure.figure[i][j];
			}
		}
	}
}


//поиск линии
static int field_find_line(int field[][FIELD_COLS], int& last, int row = 0){
	int  j, first = -1;
	last = -1;
	for(int i = row; i < FIELD_ROWS; ++i){
		j = 0;
		while((j < FIELD_COLS) && (field[i][j] != BLOCK_NONE))
			++j;
		
		if(j == FIELD_COLS){
			first = i;
			break;
		}
	}

	if(first == -1)
		return -1;

	last = first + 1;
	for(int r = first + 1; r < FIELD_ROWS; ++r){
		j = 0;
		while((j < FIELD_COLS) && (field[r][j] != BLOCK_NONE))
			++j;
		
		if(j == FIELD_COLS)
			last = r + 1;
		else
			break;
	}
	return first;
}


//пометка или удаление линии для взрыва
bool field_remove(int field[][FIELD_COLS], bool line_erase, pod_block<boom_row, FIELD_ROWS>* booms){
	bool   ok = false;
	int first = 0, last, i, j, n;

	if(!line_erase && (booms != NULL))
		booms->reset();

	while((first = field_find_line(field, last, first)) != -1){

		if(line_erase){
			n = last - first;
			for(i = last - 1; i > n; --i){
				for(j = 0; j < FIELD_COLS; ++j)
					field[i][j] = field[i - n][j];
			}

			for(i = 0; i < n; ++i)
				memset(field[i], 0, FIELD_COLS * sizeof(int));
		} else {

			for(i = first; i < last; ++i){

				if(booms != NULL)
					booms->add(boom_row(i * CELL_SIZE + (CELL_SIZE - 16)/2));

				for(j = 0; j < FIELD_COLS; ++j)
					field[i][j] += BLOCK_BOOM;
			}
			first = last;
		}
		ok = true;
	}
	return ok;
}


//размер фигуры без пустых клеток
void figure_size(const figure_t& figure, LPRECT prc, int size){
	prc->left = prc->top = prc->right = prc->bottom = -1;
	for(int i = 0; i < figure.size; ++i){
		for(int j = 0; j < figure.size; ++j){
			if(figure.figure[i][j] == BLOCK_NONE)
				continue;

			if((prc->left == -1) || (prc->left > j))
				prc->left = j;
			if((prc->right == -1) || (prc->right < j))
				prc->right = j;

			if((prc->top == -1) || (prc->top > i))
				prc->top = i;
			if((prc->bottom == -1) || (prc->bottom < i))
				prc->bottom = i;
		}
	}
	prc->left  *= size;
	prc->top   *= size;
	prc->right  = (prc->right  + 1) * size;
	prc->bottom = (prc->bottom + 1) * size;
}


//----------------------------------------------------------------------------------------------------------


static LPCSTR _next_scan(LPCSTR s, int& v){
	int n;
	if(sscanf(s, "%d%n", &v, &n) == 1)
		return s + n;
	return NULL;
}


void* operator new (size_t size){
	return HeapAlloc(GetProcessHeap(), 0, static_cast<DWORD>(size));
}


void operator delete (void* p){
	if(p != NULL)
		HeapFree(GetProcessHeap(), 0, p);
}
