#include <bvr/utils.h>

#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#ifdef _WIN32
	#include <Windows.h>
#elif
	#include <signal.h>
#endif

int bvr_sizeof(int type){
    switch (type)
    {
        case BVR_INT8: case BVR_UNSIGNED_INT8: return 1;
        case BVR_INT16: case BVR_UNSIGNED_INT16: return 2;
        case BVR_FLOAT: case BVR_INT32: case BVR_UNSIGNED_INT32: return 4;
        case BVR_MAT3: return 36;
        case BVR_MAT4: return 64;
        default: return 0L;
    }
}

static const char bvri_base64_table[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

unsigned char* bvr_base64_decode(const char* src, size_t len, size_t* new_len){
    unsigned char dtable[256], *out, *pos, block[4], tmp;
	size_t i, count, olen;
	int pad = 0;

	memset(dtable, 0x80, 256);
	for (i = 0; i < sizeof(bvri_base64_table) - 1; i++)
		dtable[bvri_base64_table[i]] = (unsigned char) i;
	dtable['='] = 0;

	count = 0;
	for (i = 0; i < len; i++) {
		if (dtable[src[i]] != 0x80)
			count++;
	}

	if (count == 0 || count % 4)
		return NULL;

	olen = count / 4 * 3;
	pos = out = malloc(olen);
	if (out == NULL)
		return NULL;

	count = 0;
	for (i = 0; i < len; i++) {
		tmp = dtable[src[i]];
		if (tmp == 0x80)
			continue;

		if (src[i] == '=')
			pad++;
		block[count] = tmp;
		count++;
		if (count == 4) {
			*pos++ = (block[0] << 2) | (block[1] >> 4);
			*pos++ = (block[1] << 4) | (block[2] >> 2);
			*pos++ = (block[2] << 6) | block[3];
			count = 0;
			if (pad) {
				if (pad == 1)
					pos--;
				else if (pad == 2)
					pos -= 2;
				else {
					/* Invalid padding */
					free(out);
					return NULL;
				}
				break;
			}
		}
	}

	*new_len = pos - out;
	return out;
}

#ifdef BVR_INCLUDE_IO

#endif

#ifdef BVR_INCLUDE_DEBUG

#define BVR_UTILS_BUFFER_SIZE 100

static char bvri_open_buffer[BVR_UTILS_BUFFER_SIZE];

char* bvri_string_format(const char* __string, ...){
	va_list arg_list;
	int f;
	
	va_start(arg_list, __string);
	f = vsnprintf(bvri_open_buffer, BVR_UTILS_BUFFER_SIZE, __string, arg_list);
	va_end(arg_list);
	
	return &bvri_open_buffer[0];
}

void bvri_wmessage(FILE* __stream, const int __line, const char* __file, const char* __message, ...){
    va_list arg_list;
    int f;

    if(__line > 0 || __file){
        fprintf(__stream, "%s:%i ", __file, __line);
    }

    va_start(arg_list, __message);
    f = vfprintf(__stream, __message, arg_list);
    va_end(arg_list);

    fprintf(__stream, "\n");
}

void bvri_wassert(const char* __message, const char* __file, unsigned long long __line){
    bvri_wmessage(stderr, -1, 0, "assertion failed: %s, %s, line %i\n", __message, __file, __line);

    exit(0);
}

void bvri_wassert_break(const char* __message, const char* __file, unsigned long long __line){
    bvri_wmessage(stderr, -1, 0, "assertion failed: %s, %s, line %i\n", __message, __file, __line);

	bvri_break(__file, __line);
}

int bvri_werror(const char* __message, int __code){
    assert(0 && "not implemented!");
}

void bvri_break(const char* __file, unsigned long long __line){	
	bvri_wmessage(stderr, __line, __file, "program break!");

#ifdef _WIN32
	DebugBreak();
#elif
	raise(SIGINT);
#endif
}

#endif