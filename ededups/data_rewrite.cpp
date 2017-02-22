#include"data_rewrite.h"


void chunk_data_rewrite() {
	while (TRUE) {
		if (dedup_list.empty()) {
			break;
		}
		struct chunk* ck=dedup_list.front();
		rewrite_list.push_back(ck);
		dedup_list.pop_front();
	}
}








void data_rewrite() {
	cout << "Rewrite start!!!" << endl;
	chunk_data_rewrite();
	cout << "Rewrite end!!!" << endl;
}