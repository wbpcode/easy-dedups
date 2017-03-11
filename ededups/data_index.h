#pragma once

#include"ededups.h"

struct{ 
	map<string, long> index_map;
	_int64 index_num;
}finger_index;


void finger_index_init();

_int64 finger_index_check(struct chunk* ck);

void finger_index_update(struct chunk* ck);

void finger_index_close();