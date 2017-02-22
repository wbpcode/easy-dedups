#include"data_chunk.h"

void data_chunk_fixed() {
	while (TRUE) {
		struct chunk* ck = read_list.front();
		if (CHECK_CHUNK(ck, CHUNK_FILE_START) || CHECK_CHUNK(ck, CHUNK_FILE_END)) {
			chunk_list.push_back(ck);
			continue;
		}
		int data_pos = 0, data_size;
		char* chunk_data_buffer = new char[READ_FIXED_CHUNK_SIZE + 1];
		while (data_size = ck->chunk_data.copy(chunk_data_buffer, READ_FIXED_CHUNK_SIZE, data_pos)) {
			struct chunk* new_ck = new chunk;
			chunk_data_buffer[data_size] = 0;
			new_ck->chunk_data = chunk_data_buffer;
			SET_CHUNK(new_ck, CHUNK_UNIQUE);
			new_ck->chunk_fp = TEMPORARY_FP;
			new_ck->container_id = TEMPORARY_ID;
			new_ck->chunk_size = data_size;
			chunk_list.push_back(new_ck);
			data_pos += data_size;
		}
		delete chunk_data_buffer;
		delete ck;
		read_list.pop_front();
		if (read_list.empty()) {
			break;
		}
	}
}

void data_chunk() {
	cout << "Chunking start!!!" << endl;
	data_chunk_fixed();
	cout << "Chunking end!!!" << endl;
}