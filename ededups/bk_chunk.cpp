#include"bk_chunk.h"

extern list<struct chunk*> read_list;
list<struct chunk*> chunk_list;

void data_chunk_fixed() {

	while (TRUE) {

		if (read_list.empty()) {
			break;
		}
		struct chunk* ck = read_list.front();


		if (CHECK_CHUNK(ck, CHUNK_FILE_START) || CHECK_CHUNK(ck, CHUNK_FILE_END)) {
			chunk_list.push_back(ck);
			read_list.pop_front();
			//cout << ck->chunk_flag << endl;
			continue;
		}

		int data_pos = 0, data_size=ck->chunk_data.size(),sub_data_size=0;
		while (TRUE) {
			if (data_pos >= data_size) {
				assert(data_pos == data_size);
				break;
			}
			struct chunk* new_ck = new chunk;
			if (data_pos + READ_FIXED_CHUNK_SIZE <= data_size) {
				sub_data_size = READ_FIXED_CHUNK_SIZE;
			}
			else {
				sub_data_size = data_size - data_pos;
			}

			new_ck->chunk_data.assign(ck->chunk_data,data_pos,sub_data_size);
			SET_CHUNK(new_ck, CHUNK_UNIQUE);
			new_ck->chunk_fp = TEMPORARY_FP;
			new_ck->container_id = TEMPORARY_ID;
			new_ck->chunk_size = sub_data_size;
			chunk_list.push_back(new_ck);
			data_pos += sub_data_size;
		}
		delete ck;
		read_list.pop_front();
	}
}

void data_chunk() {
	cout << "Chunking start!!!" << endl;
	data_chunk_fixed();
	cout << "Chunking end!!!" << endl;
}