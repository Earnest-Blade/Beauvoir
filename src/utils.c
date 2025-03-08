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

#ifdef BVR_INCLUDE_DEBUG

#define BVR_UTILS_BUFFER_SIZE 100

static char bvri_debug_buffer[BVR_UTILS_BUFFER_SIZE];

char* bvri_string_format(const char* __string, ...){
	va_list arg_list;
	int f;
	
	va_start(arg_list, __string);
	f = vsnprintf(bvri_debug_buffer, BVR_UTILS_BUFFER_SIZE, __string, arg_list);
	va_end(arg_list);
	
	return &bvri_debug_buffer[0];
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