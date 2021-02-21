//Автор(с): Кудуштеев Алексей
#if! defined(_UTIL_KUDUSHTEEV_H_)
#define _UTIL_KUDUSHTEEV_H_
#if defined(_MSC_VER) && _MSC_VER > 1000
#pragma once
#endif
#define  OBJ_SIZE   4


typedef struct {
	int figure[OBJ_SIZE][OBJ_SIZE];
	int size;

	int* operator [] (int i){
		return figure[i];
	}

	const int* operator [] (int i) const {
		return figure[i];
	}

} figure_t;


struct boom_row {
	short int pos;
	short int dir;
	boom_row(void){}
	boom_row(int p):pos(p), dir(0){} 
};

//динамический массив-блок для POD-типов
template<typename T>
class pod_array {
private:
	T*     arr;
	size_t cnt;
public:
	pod_array(void):arr(NULL), cnt(0){}
	pod_array(const pod_array&);
	~pod_array() { clear(); }

	pod_array& operator = (const pod_array&);
public:

	bool setSize(size_t n){
		clear();
		arr = reinterpret_cast<T*>( HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (DWORD)n * sizeof(T)) );
		if(arr != NULL)
			cnt = n;
		return (arr != NULL);
	}

	void clear(void){
		if(arr != NULL)
			HeapFree(GetProcessHeap(), 0, arr);
		arr = NULL;
		cnt = 0;
	}

	size_t getSize(void) const {
		return cnt;
	}

	T& operator [] (size_t i){
		return arr[i];
	}

	const T& operator [] (size_t i) const {
		return arr[i];
	}
};


//статический массив-блок для POD-типов
template<typename T, int _max_>
class pod_block {
private:
	T   arr[_max_];
	int cnt;
public:
	pod_block(void):cnt(0){}
	~pod_block() { reset(); }

	pod_block(const pod_block&);
	pod_block& operator = (const pod_block&);
public:

	void add(const T& val){
		if(cnt < _max_)
			arr[cnt++] = val;
	}

	void removeAt(size_t index){
		if(index < cnt){
			memcpy(&arr[index], &arr[index + 1], (cnt - (index + 1)) * sizeof(T));
			--cnt;
		}
	}

	void reset(void){
		cnt = 0;
	}

	int getSize(void) const {
		return cnt;
	}

	int getMaxSize(void) const {
		return _max_;
	}

	T& operator [] (int i){
		return arr[i];
	}

	const T& operator [] (int i) const {
		return arr[i];
	}
};


template<typename T>
void swap_value(T& a, T& b){
	T t = a;
	a = b;
	b = t;
}

extern void figure_size(const figure_t& figure, LPRECT prc, int size);
extern bool field_remove(int field[][FIELD_COLS], bool line_erase = false, pod_block<boom_row, FIELD_ROWS>* booms = NULL);
extern int  field_find_line(int field[][FIELD_COLS], int& last);
extern void figure_put_field(int x, int y, const figure_t& figure, int field[][FIELD_COLS]);
extern bool is_move_horz(int x, int y, const figure_t& fg, int field[][FIELD_COLS], int velocity, bool left);
extern BOOL is_move_down(int x, int y, const figure_t& figure, int step, int field[][FIELD_COLS]);
extern bool figure_rotate(int x, int y, figure_t& figure, int field[][FIELD_COLS]);
extern void figure_set(figure_t& src, int val);
extern void figure_reverse_horz(figure_t& src);
extern void figure_reverse_vert(figure_t& src);
extern void figure_transponse(figure_t& dst, const figure_t& src);
extern void figure_transponse(figure_t& src);
extern bool figures_load(pod_array<figure_t>& fgs, HINSTANCE hinst, LPCWSTR filename, LPCWSTR type = NULL);
extern void* operator new (size_t size);
extern void  operator delete (void* p);
#endif
