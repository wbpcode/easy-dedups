#include"bk_hash.h"

extern list<struct chunk*> chunk_list;
list<struct chunk*> hash_list;

void chunk_data_hash() {

	while (true) {
		if (chunk_list.empty()) {
			break;
		}
		struct chunk* ck = chunk_list.front();

		if (CHECK_CHUNK(ck, CHUNK_FILE_START) || CHECK_CHUNK(ck, CHUNK_FILE_END)) {
			hash_list.push_back(ck);
			chunk_list.pop_front();
			continue;
		}
		ck->chunk_fp = hash_sha1(ck->chunk_data);
		hash_list.push_back(ck);
		chunk_list.pop_front();
	}
}

void data_hash() {
	cout << "Hash start!!!" << endl;
	chunk_data_hash();
	cout << "Hash end!!!" << endl;
}

