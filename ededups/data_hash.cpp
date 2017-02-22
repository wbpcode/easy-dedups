#include"data_hash.h"


string chunk_hash_compute(string chunk_data) {
	string data_fp = TEMPORARY_FP;
	return data_fp;
}
void hash_compute() {
	while (TRUE) {
		struct chunk* ck = chunk_list.front();
		ck->chunk_fp = hash_compute(ck->chunk_data);
		hash_list.push_back(ck);
		chunk_list.pop_front();
		if (chunk_list.empty()) {
			break;
		}
	}
}

void data_hash() {
	cout << "Hash start!!!" << endl;
	hash_compute();
	cout << "Hash end!!!" << endl;
}

