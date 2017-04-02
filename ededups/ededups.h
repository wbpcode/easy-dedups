#pragma once

#include<iostream>
#include<fstream>
#include<sstream>
#include<string>
#include<vector>
#include<list>
#include<map>
#include<cassert>
#include<windows.h>
#include<direct.h>

//Data chunk flag
#define CHUNK_INIT 0x00000000
#define CHUNK_UNIQUE 0x00000001
#define CHUNK_DEDUP 0x00000002

/*#define CHUNK_FRAG 0x0002
#define CHUNK_REWRITE_DENY 0x0004*/

//Special chunk flag
#define CHUNK_FILE_START 0x00000100
#define CHUNK_FILE_END 0x00000200
#define CHUNK_SEG_START 0x00000400
#define CHUNK_SEG_END 0x00000800

//Flag set and check
#define SET_CHUNK(ck,flag) (ck->chunk_flag = (ck->chunk_flag & CHUNK_INIT) | flag)
#define CHECK_CHUNK(ck,flag) (ck->chunk_flag & flag)

//Preset value
#define CHUNK_FP_SIZE 40
#define TEMPORARY_ID -1L
#define TEMPORARY_FP "0000000000000000000000000000000000000000"
#define READ_BLOCK_SIZE 4194304
#define READ_FIXED_CHUNK_SIZE 8192
#define CONTAINER_MAX_CHUNK_NUM 1024
#define CONTAINER_SET_NUM 5
#define CONTAINER_SET_CACHE 20

//Special operations
#define CHECK_DIR(path) if(_waccess(path.c_str(),00)==-1){_wmkdir(path.c_str());}





//Common identifier
using std::string; using std::wstring; using std::map; using std::vector; using std::list;
using std::ifstream; using std::ofstream; using std::ios;
using std::stringstream; using std::istringstream; using std::ostringstream; 
using std::wistringstream; using std::wostringstream;
using std::cout; using std::cin; using std::wcout; using std::wcin; using std::cerr; using std::endl;

wstring string2wstring(string path);

string wstring2string(wstring path);


/*BASE DATA STUCTURE: CHUNK CHUNK_META CONTAINER*/
struct chunk{
    string chunk_fp;
    int chunk_size;
    int chunk_flag;
    string chunk_data;
    _int64 container_id;
};

//Chunk metadata in container
struct chunk_meta{
    string chunk_fp;
    int chunk_size;
    int chunk_flag;
    int chunk_offset;
    _int64 container_id;
};

//Container
struct container{
	_int64 container_id;
	int container_size;
	int container_chunk_num;
	map<string, struct chunk_meta*> container_chunk_meta_map;
	string container_data;
	
};


//container_set class
class container_set {
public:
	_int64 global_container_count;
	list<struct container*> container_list;
	wstring work_path;

	int container_size() {
		return container_list.size();
	}

	void container_set_init(wstring path) {

		work_path = path;
		if (work_path[work_path.size() - 1] != '\\') {
			work_path += '\\';
		}
		CHECK_DIR(work_path);

		ifstream container_count_stream(work_path + L"container_count", ifstream::binary);
		container_count_stream.seekg(0, ifstream::end);
		if (container_count_stream.tellg() > 0) {

			container_count_stream.seekg(0, ifstream::beg);

			char container_count_buffer[sizeof(_int64)];
			container_count_stream.read(container_count_buffer, sizeof(_int64));
			global_container_count = *(_int64*)(container_count_buffer);
		}
		else {
			global_container_count = 0;
		}
		container_count_stream.close();
	}

	void write_container_set() {

		while (TRUE) {
			if (container_list.empty()) {
				break;
			}
			auto cnr = container_list.front();

			wostringstream idstream;
			idstream << cnr->container_id;
			wstring idstring = idstream.str();
			idstream.str(L"");

			wstring containers_path = work_path + L"containers\\";
			CHECK_DIR(containers_path);

			ofstream write_container_stream(containers_path + L"container" + idstring, ofstream::binary);

			write_container_stream.write((char*)(&(cnr->container_id)), sizeof(_int64));
			write_container_stream.write((char*)(&(cnr->container_size)), sizeof(int));
			write_container_stream.write((char*)(&(cnr->container_chunk_num)), sizeof(int));


			auto chunk_meta_pair = cnr->container_chunk_meta_map.begin();
			auto chunk_meta_pair_end_flag = cnr->container_chunk_meta_map.end();
			for (; chunk_meta_pair != chunk_meta_pair_end_flag; ++chunk_meta_pair) {

				write_container_stream.write(chunk_meta_pair->second->chunk_fp.c_str(), CHUNK_FP_SIZE);
				write_container_stream.write((char*)(&(chunk_meta_pair->second->chunk_size)), sizeof(int));
				write_container_stream.write((char*)(&(chunk_meta_pair->second->chunk_offset)), sizeof(int));
			}

			write_container_stream.write(cnr->container_data.c_str(), cnr->container_size);

			write_container_stream.close();

			delete_container(cnr);

			container_list.pop_front();
		}
	}
	struct container* read_container(_int64 container_id) {
		assert(container_id != TEMPORARY_ID);

		wostringstream idstream;
		idstream << container_id;
		wstring idstring = idstream.str();
		idstream.str(L"");

		wstring containers_path = work_path + L"containers\\";
		CHECK_DIR(containers_path);

		struct container* cnr = new container;

		ifstream read_container_stream(containers_path + L"container" + idstring, ifstream::binary);

		char id_buffer[sizeof(_int64)];
		read_container_stream.read(id_buffer, sizeof(_int64));
		cnr->container_id = *(_int64*)id_buffer;
		assert(cnr->container_id == container_id);

		char size_buffer[sizeof(int)];
		read_container_stream.read(size_buffer, sizeof(int));
		cnr->container_size = *(int*)size_buffer;

		char num_buffer[sizeof(int)];
		read_container_stream.read(num_buffer, sizeof(int));
		cnr->container_chunk_num = *(int*)num_buffer;

		int num = cnr->container_chunk_num;
		char fp_buffer[CHUNK_FP_SIZE];
		char cksize_buffer[sizeof(int)];
		char ckoffset_buffer[sizeof(int)];
		for (; num > 0; --num) {
			struct chunk_meta* ckmeta = new chunk_meta;
			read_container_stream.read(fp_buffer, CHUNK_FP_SIZE);
			read_container_stream.read(cksize_buffer, sizeof(int));
			read_container_stream.read(ckoffset_buffer, sizeof(int));

			ckmeta->chunk_fp.assign(fp_buffer, CHUNK_FP_SIZE);
			ckmeta->chunk_size = *(int*)cksize_buffer;
			ckmeta->chunk_offset = *(int*)ckoffset_buffer;
			ckmeta->container_id = container_id;
			ckmeta->chunk_flag = 0;

			cnr->container_chunk_meta_map.insert(make_pair(ckmeta->chunk_fp, ckmeta));
		}

		char* data_buffer = new char[cnr->container_size];
		read_container_stream.read(data_buffer, cnr->container_size);
		cnr->container_data.assign(data_buffer, cnr->container_size);
		delete data_buffer;


		if (container_list.size() >= CONTAINER_SET_CACHE){

			delete_container(container_list.front());
			container_list.pop_front();
		}

		container_list.push_back(cnr);
		return cnr;

	}
	void add_chunk_to_container_set(struct chunk* ck) {

		if (!container_list.empty()){
			if (container_list.back()->container_chunk_num >= CONTAINER_MAX_CHUNK_NUM) {

				auto newcn = new container;
				newcn->container_id = global_container_count;
				newcn->container_size = 0;
				newcn->container_chunk_num = 0;
				newcn->container_chunk_meta_map.clear();
				newcn->container_data = "";
				++global_container_count;

				if (container_list.size() >= CONTAINER_SET_NUM) {
					write_container_set();
					assert(container_list.empty());
				}

				container_list.push_back(newcn);
			}
		}
		else {
			auto newcn = new container;
			newcn->container_id = global_container_count;
			newcn->container_size = 0;
			newcn->container_chunk_num = 0;
			newcn->container_chunk_meta_map.clear();
			newcn->container_data = "";
			++global_container_count;

			container_list.push_back(newcn);
		}

		auto cnr = container_list.back();
		//Create a new chunk metadata and add it to container_chunk_meta_map
		struct chunk_meta* ckmeta = new chunk_meta;
		ckmeta->chunk_fp = ck->chunk_fp;
		ckmeta->chunk_size = ck->chunk_size;
		ckmeta->chunk_flag = ck->chunk_flag;
		ckmeta->chunk_offset = cnr->container_size;
		ckmeta->container_id = cnr->container_id;
		cnr->container_chunk_meta_map.insert(make_pair(ckmeta->chunk_fp, ckmeta));
		//Update container attributes
		cnr->container_size += ck->chunk_size;
		cnr->container_chunk_num+=1;
		cnr->container_data = cnr->container_data + ck->chunk_data;

	}

	struct container* check_container_in_set(_int64 container_id) {
		auto begin=container_list.begin();
		auto end = container_list.end();
		for (; begin != end; ++begin) {
			if ((*begin)->container_id == container_id) {
				return *begin;
			}
		}
		return NULL;
	}

	struct chunk_meta* check_chunk_in_container_set(struct chunk* ck) {
		auto begin=container_list.begin();
		auto end = container_list.end();
		for (; begin != end; ++begin) {
			auto outcome=(*begin)->container_chunk_meta_map.find(ck->chunk_fp);
			if (outcome != (*begin)->container_chunk_meta_map.end()) {
				return outcome->second;
			}
		}
		return NULL;
	}

	struct chunk* get_chunk_from_container_set(struct chunk* ck) {
		assert(ck->container_id!=TEMPORARY_ID);
		struct container* cnr = check_container_in_set(ck->container_id);
		if (cnr == NULL) {
			cnr = read_container(ck->container_id);
		}
		struct chunk_meta* ckmeta = cnr->container_chunk_meta_map.find(ck->chunk_fp)->second;
		ck->chunk_size = ckmeta->chunk_size;
		ck->chunk_data.assign(cnr->container_data, ckmeta->chunk_offset, ckmeta->chunk_size);
		return ck;
	}
	void delete_container(struct container* cnr) {

		auto chunk_meta_pair = cnr->container_chunk_meta_map.begin();
		auto chunk_meta_pair_end_flag = cnr->container_chunk_meta_map.end();
		for (; chunk_meta_pair != chunk_meta_pair_end_flag; ++chunk_meta_pair) {
			delete chunk_meta_pair->second;
		}
		delete cnr;

	}
	void container_set_close() {

		while (!container_list.empty()) {
			auto cnr = container_list.front();
			delete_container(cnr);
			container_list.pop_front();
		}

		ofstream container_count_stream(work_path + L"container_count", ofstream::binary);
		container_count_stream.write((char*)(&global_container_count), sizeof(_int64));
		container_count_stream.close();
	}
};


class backup_recipe {
public:

	int backup_version;

	wstring work_path;
	wstring backup_path;

	_int64 backup_chunk_num;
	_int64 backup_data_size;
	_int64 backup_unique_num;
	_int64 backup_unique_size;
	_int64 backup_file_num;

	int backup_status;

	stringstream recipe_buffer;
	stringstream file_meta_buffer;
	ofstream recipe_stream;
	ofstream file_meta_stream;

private:

	void backup_version_init() {

		ifstream backup_version_stream(work_path + L"backup_version_count", ifstream::binary);
		backup_version_stream.seekg(0, ifstream::end);
		if (backup_version_stream.tellg() > 0) {
			backup_version_stream.seekg(0, ifstream::beg);
			char count_buffer[sizeof(int)];
			backup_version_stream.read(count_buffer, sizeof(int));
			backup_version = *(int*)(unsigned char*)(count_buffer)+1;

		}
		else {
			backup_version = 0;
		}
		backup_version_stream.close();
	}

	void backup_version_close() {

		ofstream backup_version_stream(work_path + L"backup_version_count", ifstream::binary);
		backup_version_stream.write((char*)(&backup_version), sizeof(int));

		backup_version_stream.close();

	}

	void backup_recipe_stream_init() {

		wostringstream backup_version_stream;
		backup_version_stream << backup_version;
		wstring backup_version_str = backup_version_stream.str();
		backup_version_stream.str(L"");

		wstring recipe_path = work_path + L"version" + backup_version_str + L'\\';
		CHECK_DIR(recipe_path);

		recipe_stream.open(recipe_path + L"recipe", ofstream::binary);
		file_meta_stream.open(recipe_path + L"filemeta", ofstream::binary);

		recipe_stream.write((char*)(&backup_chunk_num), sizeof(_int64));
		file_meta_stream.write((char*)(&backup_file_num), sizeof(_int64));


		ofstream backup_info(recipe_path + L"backup_info", ofstream::binary);
		
		string muti_backup_path = wstring2string(backup_path);
		int path_size = muti_backup_path.size();
		backup_info.write((char*)(&backup_version), sizeof(int));
		backup_info.write((char*)(&path_size), sizeof(int));
		backup_info.write(muti_backup_path.c_str(), path_size);

		backup_info.close();

	}

	void backup_recipe_stream_close() {

		recipe_buffer.seekg(0, stringstream::end);
		int buffer_size = recipe_buffer.tellg();
		recipe_stream.write(recipe_buffer.str().c_str(), buffer_size);
		recipe_buffer.str("");

		file_meta_buffer.seekg(0, stringstream::end);
		buffer_size = file_meta_buffer.tellg();
		file_meta_stream.write(file_meta_buffer.str().c_str(), buffer_size);
		file_meta_buffer.str("");

		recipe_stream.seekp(0, ofstream::beg);
		recipe_stream.write((char*)(&backup_chunk_num), sizeof(_int64));
		file_meta_stream.seekp(0, ofstream::beg);
		file_meta_stream.write((char*)(&backup_file_num), sizeof(_int64));

		recipe_stream.close();
		file_meta_stream.close();
	}

public:

	void backup_recipe_init(wstring w_path,wstring b_path) {

		work_path = w_path;
		backup_path = b_path;

		if (work_path[work_path.size() - 1] != '\\') {
			work_path += '\\';
		}
		CHECK_DIR(work_path);
		if (backup_path[backup_path.size() - 1] != '\\') {
			backup_path += '\\';
		}
		CHECK_DIR(backup_path);

		backup_chunk_num = 0;
		backup_data_size = 0;
		backup_unique_num = 0;
		backup_unique_size = 0;
		backup_file_num = 0;

		backup_version_init();
		backup_recipe_stream_init();


	}

	void backup_recipe_close() {

		backup_version_close();
		backup_recipe_stream_close();

	}

	void backup_recipe_add(struct chunk* ck) {

		static int file_chunk_num;
		if (CHECK_CHUNK(ck, CHUNK_FILE_START)) {
			file_meta_buffer.write((char*)(&ck->chunk_size), sizeof(int));
			file_meta_buffer.write(ck->chunk_data.c_str(), ck->chunk_size);

			++backup_file_num;

			file_chunk_num = 0;
		}else if (CHECK_CHUNK(ck, CHUNK_FILE_END)) {
			file_meta_buffer.write((char*)(&file_chunk_num), sizeof(int));
		}
		else {
			recipe_buffer.write(ck->chunk_fp.c_str(), CHUNK_FP_SIZE);
			recipe_buffer.write((char*)(&ck->container_id), sizeof(_int64));
			++file_chunk_num;
		}
	}
};

class restore_recipe {

public:
	int backup_version;

	wstring work_path;
	wstring backup_path;
	wstring restore_path;

	_int64 restore_chunk_num;
	_int64 restore_data_size;
	_int64 restore_file_num;

	int restore_status;

	ifstream recipe_stream;
	ifstream file_meta_stream;

	void restore_recipe_init(int version, wstring w_path, wstring r_path) {

		backup_version = version;
		work_path = w_path;
		if (work_path[work_path.size() - 1] != L'\\') {
			work_path += L'\\';
		}
		CHECK_DIR(work_path);
		restore_path = r_path;
		if (restore_path[restore_path.size() - 1] != L'\\') {
			restore_path += L'\\';
		}
		CHECK_DIR(restore_path);

		wostringstream backup_version_stream;
		backup_version_stream << backup_version;
		wstring backup_version_str = backup_version_stream.str();
		backup_version_stream.str(L"");

		wstring recipe_path = work_path + L"version" + backup_version_str + L'\\';
		CHECK_DIR(recipe_path);

		ifstream restore_info(recipe_path + L"backup_info", ifstream::binary);

		char buffer[sizeof(int)];
		restore_info.read(buffer, sizeof(int));
		int read_version = *(int*)buffer;
		assert(read_version == backup_version);
		restore_info.read(buffer, sizeof(int));
		int path_size = *(int*)buffer;

		char *path_buffer = new char[path_size];
		restore_info.read(path_buffer, path_size);
		restore_info.close();
		string muti_backup_path;
		muti_backup_path.assign(path_buffer, path_size);
		delete path_buffer;

		backup_path = string2wstring(muti_backup_path);
		if (backup_path[backup_path.size() - 1] != L'\\') {
			backup_path += L'\\';
		}
		CHECK_DIR(backup_path);

		recipe_stream.open(recipe_path + L"recipe", ifstream::binary);
		file_meta_stream.open(recipe_path + L"filemeta", ifstream::binary);

		char num_buffer[sizeof(_int64)];
		file_meta_stream.read(num_buffer, sizeof(_int64));
		restore_file_num = *(_int64*)num_buffer;

		recipe_stream.read(num_buffer, sizeof(_int64));
		restore_chunk_num = *(_int64*)num_buffer;

		restore_data_size = 0;
	}

	void restore_recipe_close() {

		recipe_stream.close();
		file_meta_stream.close();

	}
};

class ededups_index {

public:

	map<string, _int64> finger_index;
	map<string, _int64> finger_index_buffer;
	wstring work_path;

	void finger_index_init(wstring path) {

		work_path = path;
		if (work_path[work_path.size() - 1] != '\\') {
			work_path += '\\';
		}
		CHECK_DIR(work_path);

		finger_index.clear();
		finger_index_buffer.clear();

		ifstream index_stream(work_path + L"index", ifstream::binary);

		index_stream.seekg(0, ifstream::end);
		if (index_stream.tellg() > 0) {

			index_stream.seekg(0, ifstream::beg);

			char index_num[sizeof(_int64)];
			index_stream.read(index_num, sizeof(_int64));
			_int64 finger_index_num = *(_int64*)(index_num);
			_int64 num = 0;
			char fp_buffer[CHUNK_FP_SIZE];
			char id_buffer[sizeof(_int64)];
			string fp;
			_int64 id;
			for (; num < finger_index_num; ++num) {
				index_stream.read(fp_buffer, CHUNK_FP_SIZE);
				fp.assign(fp_buffer, CHUNK_FP_SIZE);

				index_stream.read(id_buffer, sizeof(_int64));
				id = *(_int64*)(id_buffer);
				finger_index.insert(make_pair(fp, id));
			}
			assert(finger_index.size() == finger_index_num);
		}
		index_stream.close();
	}

	_int64 finger_index_check(struct chunk* ck) {
		auto outcome = finger_index.find(ck->chunk_fp);
		if (outcome != finger_index.end()) {
			return outcome->second;
		}
		else {
			return -1;
		}
	}

	_int64 finger_index_buffer_check(struct chunk* ck) {
		auto outcome = finger_index_buffer.find(ck->chunk_fp);
		if (outcome != finger_index_buffer.end()) {
			return outcome->second;
		}
		else {
			return -1;
		}
	}

	_int64 finger_dedup_check(struct chunk* ck) {

		auto outcome = finger_index_buffer.find(ck->chunk_fp);
		if (outcome != finger_index_buffer.end()) {
			SET_CHUNK(ck, CHUNK_DEDUP);
			ck->container_id = outcome->second;
			return outcome->second;
		}
		else {
			outcome = finger_index.find(ck->chunk_fp);
			if (outcome != finger_index.end()) {
				SET_CHUNK(ck, CHUNK_DEDUP);
				ck->container_id = outcome->second;
				finger_index_buffer.insert(make_pair(ck->chunk_fp, ck->container_id));
				return outcome->second;
			}
			else {
				finger_index_buffer.insert(make_pair(ck->chunk_fp, ck->container_id));
				return -1;
			}
		}
	}

	void finger_index_update() {
		auto begin = finger_index_buffer.begin();
		auto end = finger_index_buffer.end();
		for (; begin != end; ++begin) {
			finger_index.insert(*begin);
		}

		finger_index_buffer.clear();
	}
	void finger_index_close() {

		ofstream index_stream(work_path + L"index", ofstream::binary);
		_int64 finger_index_num = finger_index.size();
		index_stream.write((char*)(&finger_index_num), sizeof(_int64));
		auto begin = finger_index.begin();
		auto end = finger_index.end();
		for (; begin != end; ++begin) {
			index_stream.write(begin->first.c_str(), CHUNK_FP_SIZE);
			index_stream.write((char*)(&begin->second), sizeof(_int64));
		}
		finger_index.clear();

		index_stream.close();
	}
};