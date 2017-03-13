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

//Data chunk flag
#define CHUNK_UNIQUE 0x0000
#define CHUNK_DEDUP 0x0001
#define CHUNK_FRAG 0x0002
#define CHUNK_REWRITE_DENY 0x0004

//Special chunk flag
#define CHUNK_FILE_START 0x0100
#define CHUNK_FILE_END 0x0200
#define CHUNK_SEG_START 0x0400
#define CHUNK_SEG_END 0x0800

//Flag set and check
#define SET_CHUNK(ck,flag) (ck->chunk_flag=ck->chunk_flag | flag)
#define CHECK_CHUNK(ck,flag) (ck->chunk_flag & flag)

//Preset value
#define CHUNK_FP_SIZE 40
#define TEMPORARY_ID -1L
#define TEMPORARY_FP "0000000000000000000000000000000000000000"

//Common identifier
using std::string; using std::wstring; using std::map; using std::vector; using std::list;
using std::ifstream; using std::ofstream; using std::ios;
using std::stringstream; using std::istringstream; using std::ostringstream; 
using std::wistringstream; using std::wostringstream;
using std::cout;using std::cin;using std::cerr;using std::endl;

//Working path
wstring workpath;
wstring backup_path;
wstring restore_path;

//Data list 
list<struct chunk*> read_list;
list<struct chunk*> chunk_list;
list<struct chunk*> hash_list;
list<struct chunk*> dedup_list;
list<struct chunk*> rewrite_list;

//*****CONFIG VARIABLES*****
int READ_BLOCK_SIZE = 4194304;
int READ_FIXED_CHUNK_SIZE = 8192;
int CONTAINER_MAX_CHUNK_NUM = 1024;
//*****CONFIG VARIABLES*****

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

//Container id
_int64 CONTAINER_COUNT;
//Container class
class container{

public:
    _int64 container_id;
    int container_size;
    int container_chunk_num;
    map<string,struct chunk_meta*> container_chunk_meta_map;
    string container_data;

	void container_init() {
		container_id = CONTAINER_COUNT;
		container_size = 0;
		container_chunk_num = 0;
		container_chunk_meta_map.clear();
		container_data = "";
	}

    //Add a chunk to container
    void add_chunk_to_container(struct chunk* ck){
        //Create a new chunk metadata and add it to container_chunk_meta_map
        struct chunk_meta* ckmeta=new chunk_meta;
        ckmeta->chunk_fp=ck->chunk_fp;
        ckmeta->chunk_size=ck->chunk_size;
        ckmeta->chunk_flag=ck->chunk_flag;
        ckmeta->chunk_offset=container_size;
        ckmeta->container_id=container_id; 
        container_chunk_meta_map.insert(make_pair(ckmeta->chunk_fp,ckmeta));
        //Update container attributes
        container_size+=ck->chunk_size;
        ++container_chunk_num;
        container_data=container_data+ck->chunk_data;
    }
    //Check chunk if it is in this container
    struct chunk_meta* check_chunk_in_container(struct chunk* ck){
        auto chunk_meta_pair=container_chunk_meta_map.find(ck->chunk_fp);
        if(chunk_meta_pair==container_chunk_meta_map.end()){
            return NULL;
        }else{
            return chunk_meta_pair->second;
        }
    }

    struct chunk* get_chunk_from_container(struct chunk* ck){
        assert(ck->container_id==container_id);
        auto chunk_meta_pair=container_chunk_meta_map.find(ck->chunk_fp);
        if(chunk_meta_pair==container_chunk_meta_map.end()){
			cerr << "Chunk " << ck->chunk_fp << " not in container " << container_id << " !!!" << endl;
			return NULL;
        }

		struct chunk_meta* ckmeta = chunk_meta_pair->second;
		ck->chunk_fp = ckmeta->chunk_fp;
		ck->chunk_size = ckmeta->chunk_size;
		ck->chunk_flag = ckmeta->chunk_flag;
		ck->chunk_data.assign(container_data, ckmeta->chunk_offset, ckmeta->chunk_size);
		ck->container_id = ckmeta->container_id;

		return ck;
    }

    void write_container(){

		wostringstream idstream;
		idstream << container_id;
		wstring idstring = idstream.str();
        wstring path=workpath+L"containers/"+L"container"+idstring;

        ofstream write_container_stream(path,ofstream::binary);

        write_container_stream.write((char*)(&container_id),sizeof(_int64));
        write_container_stream.write((char*)(&container_size),sizeof(int));
        write_container_stream.write((char*)(&container_chunk_num),sizeof(int));

        auto chunk_meta_pair=container_chunk_meta_map.begin();
        auto chunk_meta_pair_end_flag=container_chunk_meta_map.end();
        for(;chunk_meta_pair!=chunk_meta_pair_end_flag;++chunk_meta_pair){

            write_container_stream.write(chunk_meta_pair->second->chunk_fp.c_str(),CHUNK_FP_SIZE);
            write_container_stream.write((char*)(&chunk_meta_pair->second->chunk_size),sizeof(int));
            write_container_stream.write((char*)(&chunk_meta_pair->second->chunk_offset),sizeof(int)); 
        }

        write_container_stream.write(container_data.c_str(),container_size);

        write_container_stream.close();

		++CONTAINER_COUNT;
    }

    void read_container(_int64 id){

        container_id=id;
		wostringstream idstream;
		idstream << container_id;
		wstring idstring = idstream.str();

        wstring path=workpath+L"containers/"+L"container"+idstring;
        ifstream read_container_stream(path,ifstream::binary);

        char id_buffer[sizeof(_int64)];
        read_container_stream.read(id_buffer,sizeof(_int64));
        long t_id=*(long*)(id_buffer);
        assert(t_id==container_id);

        char size_buffer[sizeof(int)];
        read_container_stream.read(size_buffer,sizeof(int));
        container_size=*(int*)(size_buffer);

        char num_buffer[sizeof(int)];
        read_container_stream.read(num_buffer,sizeof(int));
        container_chunk_num=*(int*)(num_buffer);

        int num=0;
        char ckfp_buffer[CHUNK_FP_SIZE];
        char cksize_buffer[sizeof(int)];
        char ckoffset_buffer[sizeof(int)];
        for(;num<container_chunk_num;++num){

            struct chunk_meta* ckmeta=new chunk_meta;

            read_container_stream.read(ckfp_buffer,CHUNK_FP_SIZE);
            ckmeta->chunk_fp.assign(ckfp_buffer,CHUNK_FP_SIZE);

            read_container_stream.read(cksize_buffer,sizeof(int));
            ckmeta->chunk_size=*(int*)(cksize_buffer);

            read_container_stream.read(ckoffset_buffer,sizeof(int));
            ckmeta->chunk_offset=*(int*)(cksize_buffer);

            container_chunk_meta_map.insert(make_pair(ckmeta->chunk_fp,ckmeta));
        }

        char* data_buffer=new char[container_size];
        read_container_stream.read(data_buffer,container_size);
        container_data.assign(data_buffer,container_size);
        delete data_buffer;

		read_container_stream.close();
    }

    void delete_container(){
        auto chunk_meta_pair=container_chunk_meta_map.begin();
        auto chunk_meta_pair_end_flag=container_chunk_meta_map.end();
        for(;chunk_meta_pair!=chunk_meta_pair_end_flag;++chunk_meta_pair){
            delete chunk_meta_pair->second;
        }
        delete this;
    }

};



class backup_recipe {
public:

	int backup_version;

	wstring backup_path;

	_int64 backup_chunk_num;
	_int64 backup_data_size;

	_int64 backup_unique_num;
	_int64 backup_unique_size;

	int backup_status;

	stringstream recipe_buffer;
	stringstream file_meta_buffer;
	ofstream recipe_stream;
	ofstream file_meta_stream;

private:

	void backup_version_init() {

		if (workpath[workpath.size - 1] != L'\\') {
			workpath = workpath + L'\\';
		}
		ifstream backup_version_stream(workpath + L"backup_version_count", ifstream::binary);
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
		if (workpath[workpath.size - 1] != L'\\') {
			workpath = workpath + L'\\';
		}
		ofstream backup_version_stream(workpath + L"backup_version_count", ifstream::binary);
		backup_version_stream.write((char*)(&backup_version), sizeof(int));
		backup_version_stream.close();
	}

	void backup_recipe_stream_init() {

		if (workpath[workpath.size - 1] != L'\\') {
			workpath = workpath + L'\\';
		}

		wostringstream backup_version_stream;
		backup_version_stream << backup_version;
		wstring backup_version_str = backup_version_stream.str();
		backup_version_stream.clear();

		recipe_stream.open(workpath + L"version"+backup_version_str+L'\\'+L"recipe",ofstream::binary);
		file_meta_stream.open(workpath + L"version" + backup_version_str + L'\\' + L"filemeta", ofstream::binary);

		recipe_buffer.seekg(0, stringstream::beg);
		file_meta_buffer.seekg(0, stringstream::beg);
	}
	void buffer_to_stream() {
		recipe_buffer.seekg(0, stringstream::end);
		int buffer_size = recipe_buffer.tellg();
		recipe_stream.write(recipe_buffer.str().c_str(), buffer_size);
		recipe_buffer.clear();
		file_meta_buffer.seekg(0, stringstream::end);
		buffer_size = file_meta_buffer.tellg();
		file_meta_stream.write(file_meta_buffer.str().c_str(), buffer_size);
		file_meta_stream.clear();
	}

	void backup_recipe_stream_close() {

		buffer_to_stream();

		recipe_stream.close();
		file_meta_stream.close();

		recipe_buffer.clear();
		file_meta_buffer.clear();
	}

public:

	void backup_recipe_init() {
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
backup_recipe mine_backup_recipe;

class ededups_index {

public:

	map<string, _int64> finger_index;
	map<string, _int64> finger_index_buffer;

	void finger_index_init() {
		if (workpath[workpath.size() - 1] != L'\\') {
			workpath = workpath + L'\\';
		}
		ifstream index_stream(workpath + L"index", ifstream::binary);
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
		auto outcome = finger_index.find(ck->chunk_fp);
		if (outcome != finger_index.end()) {
			return outcome->second;
		}
		else {
			return -1;
		}
	}

	void finger_index_update() {
		auto begin = finger_index_buffer.begin();
		auto end = finger_index_buffer.end();
		for (; begin != end; ++begin) {
			finger_index.insert(*begin);
		}
	}
	void finger_index_close() {
		if (workpath[workpath.size() - 1] != L'\\') {
			workpath = workpath + L'\\';
		}
		ofstream index_stream(workpath + L"index", ofstream::binary);
		_int64 finger_index_num = finger_index.size();
		index_stream.write((char*)(&finger_index_num), sizeof(_int64));
		auto begin = finger_index.begin();
		auto end = finger_index.end();
		for (; begin != end; ++begin) {
			index_stream.write(begin->first.c_str(), CHUNK_FP_SIZE);
			index_stream.write((char*)(&begin->second), sizeof(_int64));
		}

		index_stream.close();
	}
};
ededups_index mine_ededups_index;