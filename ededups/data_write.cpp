#include"data_write.h"

extern list<struct chunk*> dedup_list;
extern wstring workpath;

container cnr;


void chunk_data_write() {

	//Get the init container id
	

	cnr.container_init();
	while (TRUE) {
		if (dedup_list.empty()) {
			break;
		}
		struct chunk* ck = dedup_list.front();

		if (CHECK_CHUNK(ck, CHUNK_UNIQUE)) {

			if (cnr.container_chunk_num < CONTAINER_MAX_CHUNK_NUM) {
				ck->container_id = cnr.container_id;
				cnr.add_chunk_to_container(ck);
			}
			else {
				assert(cnr.container_chunk_num == CONTAINER_MAX_CHUNK_NUM);
				//Write a container and container count plus 1
				cnr.write_container();
				//re-init container for following chunk
				cnr.container_init();

				ck->container_id = cnr.container_id;
				cnr.add_chunk_to_container(ck);

			}

			mine_ededups_index.finger_index_buffer[ck->chunk_fp]= ck->container_id;

			if (CHECK_CHUNK(ck, CHUNK_UNIQUE)) {
				mine_backup_recipe.backup_unique_num++;
				mine_backup_recipe.backup_unique_size += ck->chunk_size;
			}
		}

		if (!CHECK_CHUNK(ck, CHUNK_FILE_START) && !CHECK_CHUNK(ck, CHUNK_FILE_END)) {
			mine_backup_recipe.backup_chunk_num++;
			mine_backup_recipe.backup_data_size += ck->chunk_size;
		}

		ck->container_id = mine_ededups_index.finger_index_buffer_check(ck);

		mine_backup_recipe.backup_recipe_add(ck);

		dedup_list.pop_front();
		delete ck;
	}
	cnr.write_container();
	cnr.container_init();

	ofstream w_container_count_stream(workpath + L"container_count", ofstream::binary);
	w_container_count_stream.write((char*)(&CONTAINER_COUNT), sizeof(_int64));

	mine_ededups_index.finger_index_update();


};

void data_write() {
	cout << "Write start!!!" << endl;
	chunk_data_write();
	cout << "Write end!!!" << endl;
}