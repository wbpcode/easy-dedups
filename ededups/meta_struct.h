#pragma once

#include<fstream>
#include<sstream>
#include<string>
#include<mutex>
#include"ededups.h"
#include"data_struct.h"

class recipe_manager {
private:
    std::stringstream recipe_buffer;
    std::stringstream fileinfo_buffer;
    std::ofstream recipe_out;
    std::ofstream fileinfo_out;

    std::ifstream recipe_in;
    std::ifstream fileinfo_in;

    void init_backup_version();
    void init_backup_recipe_stream();
    void close_backup_version();
    void close_backup_recipe_stream();
    void write_buffer_to_stream();

public:
    int backup_version;
    int status;

    std::wstring work_path;
    std::wstring backup_path;
    std::wstring restore_path;

    _int64 chunk_num;
    _int64 data_size;
    _int64 unique_num;
    _int64 unique_size;
    _int64 file_num;

    recipe_manager() = delete;
    recipe_manager(std::wstring w_path, std::wstring b_path);
    recipe_manager(int version, std::wstring w_path, std::wstring r_path);

    ~recipe_manager();
    // This method write information to file 
    // but never destruct objects and reclaim memory
    void add_recipe_to_stream(chunk* ck);

    // This method will read information 
    // but never apply for memory create object such as chunk
    chunk* get_recipe_from_stream(chunk* ck);
};

class index_manager {
private:
    std::unordered_map<std::string, _int64> mark_index;
    std::unordered_map<std::string, _int64> mark_buffer;

    std::mutex index_mutex;
    std::mutex buffer_mutex;

    void insert_mark_to_buffer(chunk* ck);

public:
    std::wstring work_path;
    index_manager() = delete;
    index_manager(std::wstring w_path);
    ~index_manager() = default;

    void mark_dedup_check(chunk* ck);

    void update_mark_index();
    void update_id_in_buffer(chunk* ck);
    void update_id_of_chunk(chunk* ck);
};