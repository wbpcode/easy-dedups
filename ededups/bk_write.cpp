#include"ededups.h"
#include"manager.h"
#include"bk_write.h"
#include<iostream>

using namespace std;

extern manager* global_manager;

void chunk_data_write() {
    while (true) {
        struct chunk* ck = global_manager->stream.get_chunk_from_dedup_list();
        if (!ck && global_manager->stream.dedup_atomic == true) {
            break;
        }
        if (!ck) { continue; }

        if (CHECK_CHUNK(ck, CHUNK_UNIQUE)) {
            global_manager->container.add_chunk_to_container(ck);
            global_manager->index.update_id_in_buffer(ck);
        }

        global_manager->index.update_id_of_chunk(ck);
        global_manager->recipe.add_recipe_to_stream(ck);

        delete ck;
    }
    global_manager->container.write_container_to_file();
    global_manager->index.update_mark_index();
};

void data_write() {
    cout << "Write start..................." << endl;
    chunk_data_write();
    cout << "Write end....................." << endl;
}