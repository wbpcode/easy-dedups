#include"data_index.h"

void finger_index_init(){ 
	if (workpath[workpath.size() - 1] != L'\\') {
		workpath = workpath + L'\\';
	}
	ifstream index_stream(workpath + L"index", ifstream::binary);
	char index_num[sizeof(_int64)];
	index_stream.read(index_num, sizeof(_int64));
	finger_index.index_num = *(_int64*)(index_num);
	_int64 num = 0;
	char fp_buffer[CHUNK_FP_SIZE];
	char id_buffer[sizeof(_int64)];
	string fp;
	_int64 id;
	for (; num < finger_index.index_num; ++num) {
		index_stream.read(fp_buffer, CHUNK_FP_SIZE);
		fp.assign(fp_buffer, CHUNK_FP_SIZE);

		index_stream.read(id_buffer, sizeof(_int64));
		id = *(_int64*)(id_buffer);

		finger_index.index_map.insert(make_pair(fp, id));
	}
}