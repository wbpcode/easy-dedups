#include"restore.h"


list<chunk*> recipe_list;
list<chunk*> restore_list;
extern restore_recipe mine_restore_recipe;
extern container_set mine_container_set;

void restore_recipe_get() {

	_int64 restore_file_num = mine_restore_recipe.restore_file_num;
	_int64 restore_chunk_num = 0;
	for(;restore_file_num>0;--restore_file_num){

		chunk* cks = new chunk;
		char num_buffer[sizeof(int)];
		mine_restore_recipe.file_meta_stream.read(num_buffer, sizeof(int));
		int path_size = *(int*)num_buffer;
		char* path_buffer = new char[path_size];
		mine_restore_recipe.file_meta_stream.read(path_buffer, path_size);

		cks->chunk_fp = TEMPORARY_FP;
		SET_CHUNK(cks, CHUNK_FILE_START);
		cks->chunk_data.assign(path_buffer, path_size);
		delete path_buffer;
		cks->chunk_size = cks->chunk_data.size();
		assert(cks->chunk_size == path_size);
		cks->container_id = TEMPORARY_ID;
		recipe_list.push_back(cks);

		mine_restore_recipe.file_meta_stream.read(num_buffer, sizeof(int));
		int file_chunk_num = *(int*)num_buffer;

		char fp_buffer[CHUNK_FP_SIZE];
		char id_buffer[sizeof(_int64)];
		for (; file_chunk_num > 0; --file_chunk_num) {
			chunk* ck = new chunk;

			mine_restore_recipe.recipe_stream.read(fp_buffer, CHUNK_FP_SIZE);
			mine_restore_recipe.recipe_stream.read(id_buffer, sizeof(_int64));

			ck->chunk_fp.assign(fp_buffer, CHUNK_FP_SIZE);
			SET_CHUNK(ck, CHUNK_INIT);
			ck->chunk_data = "";
			ck->chunk_size = 0;
			ck->container_id = *(_int64*)id_buffer;
			recipe_list.push_back(ck);

			++restore_chunk_num;
		}

		chunk* cke = new chunk;
		cke->chunk_fp = TEMPORARY_FP;
		SET_CHUNK(cke, CHUNK_FILE_END);
		cke->chunk_data = cks->chunk_data;
		cke->chunk_size = cks->chunk_size;
		cke->container_id = TEMPORARY_ID;
		recipe_list.push_back(cke);
	}
}

void restore_chunk_get() {
	while (TRUE) { 
		if (recipe_list.empty()) {
			break;
		}
		chunk* ck = recipe_list.front();

		if (CHECK_CHUNK(ck, CHUNK_FILE_START) || CHECK_CHUNK(ck, CHUNK_FILE_END)) {
			restore_list.push_back(ck);
			recipe_list.pop_front();
			continue;
		}

		mine_container_set.get_chunk_from_container(ck);
		
	}
}


