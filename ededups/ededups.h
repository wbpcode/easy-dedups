#pragma once

#include<iostream>
#include<fstream>
#include<sstream>
#include<string>
#include<vector>
#include<map>
#include<cassert>

//读取文件数据时初始划分大小
#define READ_BLOCK_SIZE 4194304 //4MB

//数据chunk的各种标志
#define CHUNK_UNIQUE 0x0000
#define CHUNK_DEDUP 0x0001
#define CHUNK_FRAG 0x0002
#define CHUNK_REWRITE_DENY 0x0004
//特殊chunk的各种标志
#define CHUNK_FILE_START 0x0100
#define CHUNK_FILE_END 0x0200
#define CHUNK_SEG_START 0x0400
#define CHUNK_SEG_END 0x0800
//标志的设置和检查
#define SET_CHUNK(ck,flag) (ck->chunk_flag=ck->chunk_flag | flag)
#define CHECK_CHUNK(ck,flag) (ck->chunk_flag & flag)

//一些预设值
#define CHUNK_FP_SIZE 20
#define TEMPORARY_ID -1L
#define TEMPORARY_FP "00000000000000000000"


using std::string; using std::wstring; using std::map; using std::vector;
using std::ifstream;using std::ofstream;
using std::istringstream; using std::ostringstream; using std::wistringstream; using std::wostringstream;
using std::cout;using std::cin;using std::cerr;using std::endl;

//工作路径，所有路径都使用宽字符串对象wstring
wstring workpath;
//数据处理序列
vector<struct chunk*> read_seq;


struct chunk{
    string chunk_fp;
    int chunk_size;
    int chunk_flag;
    string chunk_data;
	wstring file_path;//本来文件名应该存储在chunk_data中，但是路径的宽字符与string转换比较复杂
    long container_id;
};

struct chunk_meta{
    string chunk_fp;
    int chunk_size;
    int chunk_flag;
    int chunk_offset;
    long container_id;
};

class container{

    long container_id;
    int container_size;
    int container_chunk_num;
    map<string,struct chunk_meta*> container_chunk_meta_map;
    string container_data;

    //添加一个chunk到container
    void add_chunk_to_container(struct chunk* ck){
        //创建新的chunk记录并添加到container_chunk_meta_map
        struct chunk_meta* ckmeta=new chunk_meta;
        ckmeta->chunk_fp=ck->chunk_fp;
        ckmeta->chunk_size=ck->chunk_size;
        ckmeta->chunk_flag=ck->chunk_flag;
        ckmeta->chunk_offset=container_size;
        ckmeta->container_id=container_id; 
        container_chunk_meta_map.insert(make_pair(ckmeta->chunk_fp,ckmeta));
        //更新container属性数据
        container_size+=ck->chunk_size;
        ++container_chunk_num;
        container_data=container_data+ck->chunk_data;
    }
    //检查chunk是否存在于container中
    struct chunk_meta* check_chunk_in_container(struct chunk* ck){
        auto chunk_meta_pair=container_chunk_meta_map.find(ck->chunk_fp);
        if(chunk_meta_pair==container_chunk_meta_map.end()){
            return NULL;
        }else{
            return chunk_meta_pair->second;
        }
    }
    //container类本身不会新建chunk，而是将没有数据的chunk补全，所以传入参数是chunk指针而非指纹
    //container类本身不会做检查，所以调用函数时，必须保证对应的chunk确实存在于container中
    struct chunk* get_chunk_from_container(struct chunk* ck){
        assert(ck->container_id==container_id);
        auto chunk_meta_pair=container_chunk_meta_map.find(ck->chunk_fp);
        if(chunk_meta_pair==container_chunk_meta_map.end()){
			cerr << "Chunk " << ck->chunk_fp << " not in container " << container_id << " !!!" << endl;
        }
		//更新除chunk数据外的其他属性
		struct chunk_meta* ckmeta = chunk_meta_pair->second;
		ck->chunk_fp = ckmeta->chunk_fp;
		ck->chunk_size = ckmeta->chunk_size;
		ck->chunk_flag = ckmeta->chunk_flag;
		ck->container_id = ckmeta->container_id;
		//获取数据
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
		//将整型数转换为字符串
		wostringstream idstream;
		idstream << container_id;
		wstring idstring = idstream.str();
		//获得路径并打开
        wstring path=workpath+L"containers/"+L"container"+idstring;
        ifstream read_container_stream(path,ifstream::binary);
        //读取container_id
        char* id_buffer=new char[sizeof(long)];
        read_container_stream.read(id_buffer,sizeof(long));
        long t_id=*(long*)(id_buffer);
        assert(t_id==container_id);
        delete id_buffer;
        //读取container_size
        char* size_buffer=new char[sizeof(int)];
        read_container_stream.read(size_buffer,sizeof(int));
        container_size=*(int*)(size_buffer);
        delete size_buffer;
        //读取container_chunk_num
        char* num_buffer=new char[sizeof(int)];
        read_container_stream.read(num_buffer,sizeof(int));
        container_chunk_num=*(int*)(num_buffer);
        delete num_buffer;
        //读取container_chunk_meta_map
        int num=0;
        char* ckfp_buffer=new char[CHUNK_FP_SIZE+1];
        char* cksize_buffer=new char[sizeof(int)];
        char* ckoffset_buffer=new char[sizeof(int)];
        for(;num<container_chunk_num;++num){
            //新建chunk_meta记录
            struct chunk_meta* ckmeta=new chunk_meta;
            //读取指纹
            read_container_stream.read(ckfp_buffer,CHUNK_FP_SIZE);
            ckfp_buffer[CHUNK_FP_SIZE]=0;
            ckmeta->chunk_fp=ckfp_buffer;
            //读取size
            read_container_stream.read(cksize_buffer,sizeof(int));
            ckmeta->chunk_size=*(int*)(cksize_buffer);
            //读取offset
            read_container_stream.read(ckoffset_buffer,sizeof(int));
            ckmeta->chunk_offset=*(int*)(cksize_buffer);
            //插入到map中
            container_chunk_meta_map.insert(make_pair(ckmeta->chunk_fp,ckmeta));
        }
        delete ckfp_buffer;
        delete cksize_buffer;
        delete ckoffset_buffer;
        //读取container数据
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