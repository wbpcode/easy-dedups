#pragma once

#include<string>
#include<list>
#include<mutex>
#include<atomic>
#include<unordered_map>
#include"ededups.h"


// Chunk: minimal data structure that will be processed
struct chunk {
    std::string mark;
    int size;
    int flag;
    _int64 id;
    std::string data;
    chunk() :size(0), flag(0), id(TEMP_ID), mark(TEMP_FP) { ; }
    ~chunk() = default;
};

// Chunk metadata: used in container to help manage chunk
struct chunk_meta {
    std::string mark;
    int size;
    int flag;
    int offset;
    _int64 id;
    chunk_meta() :size(0), flag(0), offset(0), id(TEMP_ID) { ; }
    ~chunk_meta() = default;
    
};

// Container: largest data structure that will be processed
struct container {
    _int64 id;
    int size;
    int chunk_num;
    std::unordered_map<std::string, chunk_meta*> chunk_map;
    std::string data;
    container() :id(TEMP_ID), size(0), chunk_num(0) { ; }
    ~container() {
        for (auto &e : chunk_map) { delete e.second; }
    }
};

// Container manager: class that is used to manage container
class container_manager {
private:
    _int64 container_count;
    std::list<container*> container_list;
    std::wstring work_path;

public:
    container_manager() = delete;
    container_manager(std::wstring path);
    ~container_manager();

    int get_container_list_size();
    void write_container_to_file();
    container* read_container_to_mem(_int64 id);
    void add_chunk_to_container(chunk* ck);

    container* check_container(_int64 id);
    chunk_meta* check_chunk(chunk* ck);
    chunk* get_chunk_from_container(chunk* ck);
};

class stream_manager {
private:
    std::list<chunk*> read_list;
    std::list<chunk*> chunk_list;
    std::list<chunk*> hash_list;
    std::list<chunk*> dedup_list;

    std::mutex read_mutex;
    std::mutex chunk_mutex;
    std::mutex hash_mutex;
    std::mutex dedup_mutex;
public:
    std::atomic_bool read_atomic;
    std::atomic_bool chunk_atomic;
    std::atomic_bool hash_atomic;
    std::atomic_bool dedup_atomic;

    stream_manager() = default;
    ~stream_manager() = default;

    chunk* get_chunk_from_read_list();
    void put_chunk_to_read_list(chunk* ck);
    chunk* get_chunk_from_chunk_list();
    void put_chunk_to_chunk_list(chunk* ck);
    chunk* get_chunk_from_hash_list();
    void put_chunk_to_hash_list(chunk* ck);
    chunk* get_chunk_from_dedup_list();
    void put_chunk_to_dedup_list(chunk* ck);
};