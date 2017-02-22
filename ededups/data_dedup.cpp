#include"data_dedup.h"



void chunk_data_dedup() {
	while (TRUE) {
		if (hash_list.empty()) {
			break;
		}
		struct chunk* ck=hash_list.front();
		dedup_list.push_back(ck);
		hash_list.pop_front();
	}
}

void data_dedup() {
	cout << "Dedup start!!!" << endl;
	chunk_data_dedup();
	cout << "Dedup end!!!" << endl;
}