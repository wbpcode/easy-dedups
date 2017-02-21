#include"data_read.h"
#include<windows.h>


void read_file(wstring path) {
	struct chunk* cks = new chunk;
	cks->chunk_fp = TEMPORARY_FP;
	assert(cks->chunk_fp.size() == CHUNK_FP_SIZE);
	SET_CHUNK(cks, CHUNK_FILE_START);
	cks->file_path = path;
	cks->chunk_size = cks->file_path.size();
	cks->container_id = TEMPORARY_ID;
	read_list.push_back(cks);
	
	ifstream filestream(path, ifstream::binary);
	char* data_buffer = new char[READ_BLOCK_SIZE + 1];
	while (filestream.read(data_buffer, READ_BLOCK_SIZE)) {
		data_buffer[READ_BLOCK_SIZE] = 0;
		struct chunk* ck = new chunk;
		ck->chunk_fp = TEMPORARY_FP;
		SET_CHUNK(ck, CHUNK_UNIQUE);
		ck->chunk_size = READ_BLOCK_SIZE;
		ck->chunk_data = data_buffer;
		ck->container_id = TEMPORARY_ID;
		read_list.push_back(ck);
	}
	int last_data_size = filestream.gcount();
	if (last_data_size > 0) {
		data_buffer[last_data_size] = 0;
		struct chunk* ck = new chunk;
		ck->chunk_fp = TEMPORARY_FP;
		SET_CHUNK(ck, CHUNK_UNIQUE);
		ck->chunk_size = last_data_size;
		ck->chunk_data = data_buffer;
		ck->container_id = TEMPORARY_ID;
		read_list.push_back(ck);
	}
	delete data_buffer;
	filestream.close();

	struct chunk* cke = new chunk;
	cke->chunk_fp = TEMPORARY_FP;
	SET_CHUNK(cke, CHUNK_FILE_END);
	cke->file_path = path;
	cke->chunk_size = cke->file_path.size();
	cke->container_id = TEMPORARY_ID;
	read_list.push_back(cke);
}

int find_all_file(wstring path){
	if (path[path.size() - 1] != L'\\') {
		path = path + L"\\";
	}
	WIN32_FIND_DATA fileinfo;
	wstring search_path = path + L"*.*";
	HANDLE findend = FindFirstFile(search_path.c_str(), &fileinfo);
	if (findend == INVALID_HANDLE_VALUE) {
		cerr << "NO FILE!!!" << endl;
		return -1;
	}
	wstring current_file_name = fileinfo.cFileName;
	wstring current_directory = L".";
	wstring parent_directory = L"..";
	if (fileinfo.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY) {
		if (current_file_name!= current_directory && current_file_name != parent_directory) {
			find_all_file(path+current_file_name);
		}
	}
	else {
		read_file(path+current_file_name);
	}
	while (FindNextFile(findend, &fileinfo)) {
		current_file_name = fileinfo.cFileName;
		if (fileinfo.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY) {
			if (current_file_name != current_directory && current_file_name != parent_directory) {
				find_all_file(path+current_file_name);
			}
		}
		else {
			read_file(path+current_file_name);
		}
	}
	FindClose(findend);
	return 0;
}

void data_read() {
	cout << "Reading start!!!" << endl;
	find_all_file(backup_path);
	cout << "Reading end!!!";
}