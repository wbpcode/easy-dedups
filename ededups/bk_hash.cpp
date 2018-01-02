#include"ededups.h"
#include"bk_hash.h"
#include"hash_sha.h"
#include"manager.h"
#include<iostream>

using namespace std;

extern manager* global_manager;

void chunk_data_hash() {
    while (true) {
        chunk* ck = global_manager->stream.get_chunk_from_chunk_list();
        if (!ck && global_manager->stream.chunk_atomic == true) {
            break;
        }
        if (!ck) { continue; }

        if (CHECK_CHUNK(ck, CHUNK_FILE_START) || CHECK_CHUNK(ck, CHUNK_FILE_END)) {
            global_manager->stream.put_chunk_to_hash_list(ck);
            continue;
        }
        ck->mark = hash_sha1(ck->data);
        global_manager->stream.put_chunk_to_hash_list(ck);
    }
}

void data_hash() {
    global_manager->stream.hash_atomic = false;
    cout << "Hash start...................." << endl;
    chunk_data_hash();
    cout << "Hash end......................" << endl;
    global_manager->stream.hash_atomic = true;
}

