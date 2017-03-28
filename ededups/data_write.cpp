#include"data_write.h"

extern list<struct chunk*> dedup_list;

extern backup_recipe mine_backup_recipe;
extern finger_index mine_finger_index;
extern container_set mine_container_set;



void chunk_data_write() {

	//Get the init container id
	

	while (TRUE) {
		if (dedup_list.empty()) {
			break;
		}
		struct chunk* ck = dedup_list.front();

		if (CHECK_CHUNK(ck, CHUNK_UNIQUE)) {
			mine_container_set.add_chunk_to_container_set(ck);

			mine_finger_index.finger_index_buffer[ck->chunk_fp]= mine_container_set.global_container_count-1;

			mine_backup_recipe.backup_unique_num++;
			mine_backup_recipe.backup_unique_size += ck->chunk_size;
		}

		if (!CHECK_CHUNK(ck, CHUNK_FILE_START) && !CHECK_CHUNK(ck, CHUNK_FILE_END)) {
			mine_backup_recipe.backup_chunk_num++;
			mine_backup_recipe.backup_data_size += ck->chunk_size;
		}

		ck->container_id = mine_finger_index.finger_index_buffer_check(ck);

		mine_backup_recipe.backup_recipe_add(ck);

		dedup_list.pop_front();
		delete ck;
	}

	mine_finger_index.finger_index_update();


};

void data_write() {
	cout << "Write start!!!" << endl;
	chunk_data_write();
	cout << "Write end!!!" << endl;
}