#include"restore.h"


list<struct chunk*> recipe_list;
list<struct chunk*> restore_list;
extern restore_recipe mine_restore_recipe;
extern container_set mine_container_set;

void restore_get_recipe() {

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

void restore_get_chunk() {
	while (TRUE) { 
		if (recipe_list.empty()) {
			break;
		}
		struct chunk* ck = recipe_list.front();

		if (CHECK_CHUNK(ck, CHUNK_FILE_START) || CHECK_CHUNK(ck, CHUNK_FILE_END)) {
			restore_list.push_back(ck);
			recipe_list.pop_front();
			continue;
		}

		mine_container_set.get_chunk_from_container_set(ck);

		restore_list.push_back(ck);

		recipe_list.pop_front();
		
	}
}

void restore_write_file() {
	const wstring restore_path = mine_restore_recipe.restore_path;
	const wstring backup_path = mine_restore_recipe.backup_path;

	int backup_path_size = backup_path.size();

	ofstream write_file_stream;

	while (TRUE) {
		if (restore_list.empty()) {
			break;
		}
		struct chunk* ck = restore_list.front();

		if (CHECK_CHUNK(ck, CHUNK_FILE_START)) {
			wstring backup_file_path = string2wstring(ck->chunk_data);
			wstring tem_file_path;
			tem_file_path.assign(backup_file_path, backup_path.size(), backup_file_path.size() - backup_path_size);
			wstring restore_file_path = restore_path + tem_file_path;
			wstring dir_path = restore_file_path;
			while (TRUE) {
				if (dir_path.back() != L'\\') {
					dir_path.pop_back();
				}
				else {
					break;
				}
			}
			CHECK_DIR(dir_path);

			write_file_stream.open(restore_file_path, ofstream::binary);
			
			restore_list.pop_front();

			delete ck;

			continue;
		}
		if (CHECK_CHUNK(ck, CHUNK_FILE_END)) {

			write_file_stream.close();
			
			restore_list.pop_front();

			delete ck;

			continue;
		}

		write_file_stream.write(ck->chunk_data.c_str(), ck->chunk_size);
		mine_restore_recipe.restore_data_size += ck->chunk_size;

		restore_list.pop_front();

		delete ck;
	}
}


