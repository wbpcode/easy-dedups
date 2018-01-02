#include"restore.h"
#include"ededups.h"
#include"manager.h"
#include<iostream>

using namespace std;

extern manager* global_manager;

void get_restore_recipe() {
    global_manager->stream.read_atomic = false;
    while (true) {
        chunk* ck = new chunk();
        chunk* ckt = global_manager->recipe.get_recipe_from_stream(ck);
        if (ckt) {
            global_manager->stream.put_chunk_to_read_list(ck);
        }
        else {
            delete ck;
            break;
        }
    }
    global_manager->stream.read_atomic = true;
}

void get_restore_chunk() {
    global_manager->stream.chunk_atomic = false;
    while (true) { 
        chunk* ck = global_manager->stream.get_chunk_from_read_list();
        if (!ck && global_manager->stream.read_atomic == true) {
            break;
        }

        if (CHECK_CHUNK(ck, CHUNK_FILE_START) || CHECK_CHUNK(ck, CHUNK_FILE_END)) {
            global_manager->stream.put_chunk_to_chunk_list(ck);
            continue;
        }

        global_manager->container.get_chunk_from_container(ck);
        global_manager->stream.put_chunk_to_chunk_list(ck);      
    }
    global_manager->stream.chunk_atomic = true;
}

void write_restore_file() {
    ofstream write_file_stream;
    while (true) {
        chunk* ck = global_manager->stream.get_chunk_from_chunk_list();
        if (!ck && global_manager->stream.chunk_atomic == true) {
            break;
        }
        if (!ck) { continue; }
        if (CHECK_CHUNK(ck, CHUNK_FILE_START)) {
            wstring file_path = string2wstring(ck->data);
            wstring dir_path = file_path;
            while (true) {
                if (dir_path.back() != L'/') {
                    dir_path.pop_back();
                }
                else {
                    break;
                }
            }
            CHECK_DIR(dir_path);
            write_file_stream.open(file_path, ofstream::binary);

            delete ck;
            continue;
        }
        if (CHECK_CHUNK(ck, CHUNK_FILE_END)) {
            write_file_stream.close();

            delete ck;
            continue;
        }

        write_file_stream.write(ck->data.c_str(), ck->size);
        global_manager->recipe.data_size += ck->size;

        delete ck;
    }
}


