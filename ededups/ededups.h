#pragma once

#include<string>

// Data chunk flag
#define CHUNK_INIT 0x00000000
#define CHUNK_UNIQUE 0x00000001
#define CHUNK_DEDUP 0x00000002

// Special chunk flag
#define CHUNK_FILE_START 0x00000100
#define CHUNK_FILE_END 0x00000200

// Flag set and check
#define SET_CHUNK(c,f) (c->flag=(c->flag & CHUNK_INIT)|f)
#define CHECK_CHUNK(c,f) (c->flag & f)

// Special operations
#define CHECK_DIR(p) if(_waccess(p.c_str(),00)==-1){_wmkdir(p.c_str());}

// Preset value
#define FP_SIZE 40
#define TEMP_ID -1L
#define TEMP_FP "0000000000000000000000000000000000000000"
#define READ_BLOCK_SIZE 4194304
#define READ_FIXED_SIZE 8192
#define CONTAINER_MAX_CHUNK_NUM 1024
#define CONTAINER_MAX_LIST_SIZE 5
#define CONTAINER_MAX_CACHE_SIZE 20

// Covert wstring path to string data
std::wstring string2wstring(std::string path);
std::string wstring2string(std::wstring path);




