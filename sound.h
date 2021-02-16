//Автор(с): Кудуштеев Алексей
#if! defined(_SOUND_KUDUSHTEEV_H_)
#define _SOUND_KUDUSHTEEV_H_
#if defined(_MSC_VER) && _MSC_VER > 1000
#pragma once
#endif


//проигрывание звуков
class sound {
	enum sound_type { snd_none, snd_file, snd_res };
private:
	HGLOBAL    data;
	LPBYTE     buf;
	sound_type type;
public:
	sound(void):data(NULL), buf(NULL), type(snd_none){}
	sound(const sound&);
	~sound(){ destroy(); }

	sound& operator = (const sound&);
public:

	bool load(HINSTANCE hinst, LPCWSTR res_name, LPCWSTR res_type){
		destroy();
		HRSRC res = FindResourceW((HMODULE)hinst, res_name, res_type);
		if(res == NULL)
			return false;

		data = LoadResource((HMODULE)hinst, res);
		if(data == NULL)
			return false;

		type = snd_res;
		buf  = reinterpret_cast<LPBYTE>(LockResource(data));
		if(buf == NULL){
			destroy();
			return false;
		}
		return true;
	}

	bool load(LPCWSTR filename){
		destroy();
		HANDLE fp = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 
		                        FILE_ATTRIBUTE_NORMAL, NULL);
		if(fp == INVALID_HANDLE_VALUE)
			return false;

		DWORD size = GetFileSize(fp, NULL);
		if(size == INVALID_FILE_SIZE){
			CloseHandle(fp);
			return false;
		}

		data = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, size);
		if(data == NULL){
			CloseHandle(fp);
			return false;
		}

		type = snd_file;
		buf  = reinterpret_cast<LPBYTE>(GlobalLock(data));
		if(buf == NULL){
			CloseHandle(fp);
			destroy();
			return false;
		}

		DWORD n = 0;
		BOOL  r = ReadFile(fp, buf, size, &n, NULL);
		CloseHandle(fp);
		bool ret = (r && (n == size));

		if(!ret)
			destroy();
		return ret;
	}

	BOOL play(PBOOL _play = NULL, bool _async = true, bool _loop = false){
		if((_play != NULL) && !*_play)
			return FALSE;

		UINT flags = SND_MEMORY;
		if(_async)
			flags |= SND_ASYNC;
		else
			flags |= SND_SYNC;

		if(_loop)
			flags |= SND_LOOP;
		return sndPlaySoundW((LPCWSTR)buf, flags);
	}

	BOOL stop(void){
		return sndPlaySoundW(NULL, 0);
	}

	void destroy(void){
		switch(type){
		case snd_file:
			if(buf != NULL)
				GlobalUnlock(data);
			if(data != NULL)
				GlobalFree(data);
			break;
		case snd_res:
			if(buf != NULL)
				UnlockResource(data);
			if(data != NULL)
				FreeResource(data);
			break;
		}
		type = snd_none;
		buf  = NULL;
		data = NULL;
	}
};


#endif
