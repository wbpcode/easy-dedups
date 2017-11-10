#include"bk_dedup.h"

extern list<struct chunk*> hash_list;
list<struct chunk*> dedup_list;
extern ededups_index mine_finger_index;

void chunk_data_dedup() {
    while (true) {

        if (hash_list.empty()) {
            break;
        }
        struct chunk* ck=hash_list.front();
        if (CHECK_CHUNK(ck, CHUNK_FILE_START) || CHECK_CHUNK(ck, CHUNK_FILE_END)) {
            dedup_list.push_back(ck);
            hash_list.pop_front();
            continue;
        }

        mine_finger_index.finger_dedup_check(ck);

        dedup_list.push_back(ck);
        hash_list.pop_front();
    }
}



void data_dedup() {
    cout << "Dedup start!!!" << endl;
    chunk_data_dedup();
    cout << "Dedup end!!!" << endl;
}