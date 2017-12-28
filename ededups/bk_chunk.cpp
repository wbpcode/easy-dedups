#include"bk_chunk.h"
#include"ededups.h"
#include"manager.h"
#include<cassert>
#include<string>
#include<iostream>

using namespace std;

extern manager* global_manager;

void data_chunk_fixed() {
    while (true) {
        chunk* ck = global_manager->stream.get_chunk_from_read_list();

        if (!ck && global_manager->stream.read_atomic) {
            break;
        }
        if (!ck) { continue; }

        if (CHECK_CHUNK(ck, CHUNK_FILE_START) || CHECK_CHUNK(ck, CHUNK_FILE_END)) {
            global_manager->stream.put_chunk_to_chunk_list(ck);
            continue;
        }

        int data_pos = 0, data_size=ck->data.size(),sub_data_size=0;
        while (data_pos < data_size) {
            chunk* new_ck = new chunk();
            if (data_pos + READ_FIXED_SIZE <= data_size) {
                sub_data_size = READ_FIXED_SIZE;
            }
            else {
                sub_data_size = data_size - data_pos;
            }

            new_ck->data.assign(ck->data,data_pos,sub_data_size);
            new_ck->size = sub_data_size;
            global_manager->stream.put_chunk_to_chunk_list(new_ck);
            data_pos += sub_data_size;
        }
        delete ck;
    }
}

void data_chunk() {
    cout << "Chunking start................" << endl;
    global_manager->stream.chunk_atomic = false;
    data_chunk_fixed();
    global_manager->stream.chunk_atomic = true;
    cout << "Chunking end.................." << endl;
}