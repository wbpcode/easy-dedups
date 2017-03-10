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
using std::ifstream;using std::ofstream;
using std::istringstream; using std::ostringstream; using std::wistringstream; using std::wostringstream;
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
list<struct chunk*> filter_list;

//*****CONFIG VARIABLES*****
int READ_BLOCK_SIZE = 4194304;
int READ_FIXED_CHUNK_SIZE = 8192;
//*****CONFIG VARIABLES*****

//Chunk struct
struct chunk{
    string chunk_fp;
    int chunk_size;
    int chunk_flag;
    string chunk_data;
    long container_id;
};

//Chunk metadata in container
struct chunk_meta{
    string chunk_fp;
    int chunk_size;
    int chunk_flag;
    int chunk_offset;
    long container_id;
};

//Container class
class container{

    long container_id;
    int container_size;
    int container_chunk_num;
    map<string,struct chunk_meta*> container_chunk_meta_map;
    string container_data;

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
        }

		struct chunk_meta* ckmeta = chunk_meta_pair->second;
		ck->chunk_fp = ckmeta->chunk_fp;
		ck->chunk_size = ckmeta->chunk_size;
		ck->chunk_flag = ckmeta->chunk_flag;
		ck->container_id = ckmeta->container_id;

		char* data_buffer = new char[ckmeta->chunk_size + 1];
		container_data.copy(data_buffer, ckmeta->chunk_size, ckmeta->chunk_offset);
		data_buffer[ckmeta->chunk_size] = 0;
		ck->chunk_data = data_buffer;
		delete data_buffer;

		return ck;
    }
    void write_container(){
		wostringstream idstream;
		idstream << container_id;
		wstring idstring = idstream.str();
        wstring path=workpath+L"containers/"+L"container"+idstring;
        ofstream write_container_stream(path,ofstream::binary);
        write_container_stream.write((char*)(&container_id),sizeof(long));
        write_container_stream.write((char*)(&container_size),sizeof(int));
        write_container_stream.write((char*)(&container_chunk_num),sizeof(int));
        auto chunk_meta_pair=container_chunk_meta_map.begin();
        auto chunk_meta_pair_end_flag=container_chunk_meta_map.end();
        for(;chunk_meta_pair!=chunk_meta_pair_end_flag;++chunk_meta_pair){
            write_container_stream.write(chunk_meta_pair->second->chunk_fp.data(),CHUNK_FP_SIZE);
            write_container_stream.write((char*)(&chunk_meta_pair->second->chunk_size),sizeof(int));
            write_container_stream.write((char*)(&chunk_meta_pair->second->chunk_offset),sizeof(int)); 
        }
        write_container_stream.write(container_data.data(),container_size);
        write_container_stream.close();
    }
    void read_container(long id){
        container_id=id;

		wostringstream idstream;
		idstream << container_id;
		wstring idstring = idstream.str();

        wstring path=workpath+L"containers/"+L"container"+idstring;
        ifstream read_container_stream(path,ifstream::binary);

        char* id_buffer=new char[sizeof(long)];
        read_container_stream.read(id_buffer,sizeof(long));
        long t_id=*(long*)(id_buffer);
        assert(t_id==container_id);
        delete id_buffer;

        char* size_buffer=new char[sizeof(int)];
        read_container_stream.read(size_buffer,sizeof(int));
        container_size=*(int*)(size_buffer);
        delete size_buffer;

        char* num_buffer=new char[sizeof(int)];
        read_container_stream.read(num_buffer,sizeof(int));
        container_chunk_num=*(int*)(num_buffer);
        delete num_buffer;

        int num=0;
        char* ckfp_buffer=new char[CHUNK_FP_SIZE+1];
        char* cksize_buffer=new char[sizeof(int)];
        char* ckoffset_buffer=new char[sizeof(int)];
        for(;num<container_chunk_num;++num){

            struct chunk_meta* ckmeta=new chunk_meta;

            read_container_stream.read(ckfp_buffer,CHUNK_FP_SIZE);
            ckfp_buffer[CHUNK_FP_SIZE]=0;
            ckmeta->chunk_fp=ckfp_buffer;

            read_container_stream.read(cksize_buffer,sizeof(int));
            ckmeta->chunk_size=*(int*)(cksize_buffer);

            read_container_stream.read(ckoffset_buffer,sizeof(int));
            ckmeta->chunk_offset=*(int*)(cksize_buffer);

            container_chunk_meta_map.insert(make_pair(ckmeta->chunk_fp,ckmeta));
        }
        delete ckfp_buffer;
        delete cksize_buffer;
        delete ckoffset_buffer;

        char* data_buffer=new char[container_size+1];
        read_container_stream.read(data_buffer,container_size);
        data_buffer[container_size]=0;
        container_data=data_buffer;
        delete data_buffer;
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