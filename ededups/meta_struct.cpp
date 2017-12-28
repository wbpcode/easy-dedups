#include<cassert>
#include"meta_struct.h"

using namespace std;

void recipe_manager::init_backup_version() {
    ifstream in(work_path + L"version_count", ifstream::binary);
    in.seekg(0, ifstream::end);
    if (in.tellg() > 0) {
        in.seekg(0, ifstream::beg);
        char buffer[sizeof(int)];
        in.read(buffer, sizeof(int));
        backup_version = *(int*)(buffer) + 1;
    }
    else {
        backup_version = 0;
    }
    in.close();
}

void recipe_manager::close_backup_version() {

    ofstream out(work_path + L"version_count", ifstream::binary);
    out.write((char*)(&backup_version), sizeof(int));

    out.close();
}

void recipe_manager::init_backup_recipe_stream() {

    wstring recipe_path = work_path + L"version_recipe" + L'/';
    CHECK_DIR(recipe_path);

    recipe_out.open(recipe_path + L"recipe" + 
        to_wstring(backup_version), ofstream::binary);
    fileinfo_out.open(recipe_path + L"fileinfo" +
        to_wstring(backup_version), ofstream::binary);

    recipe_out.write((char*)(&chunk_num), sizeof(_int64));
    fileinfo_out.write((char*)(&file_num), sizeof(_int64));


    ofstream out(recipe_path + L"pathinfo" +
        to_wstring(backup_version), ofstream::binary);

    string muti_backup_path = wstring2string(backup_path);
    int path_size = muti_backup_path.size();
    out.write((char*)(&backup_version), sizeof(int));
    out.write((char*)(&path_size), sizeof(int));
    out.write(muti_backup_path.c_str(), path_size);
    out.close();
}

void recipe_manager::close_backup_recipe_stream() {
    write_buffer_to_stream();

    recipe_out.seekp(0, ofstream::beg);
    recipe_out.write((char*)(&chunk_num), sizeof(_int64));
    fileinfo_out.seekp(0, ofstream::beg);
    fileinfo_out.write((char*)(&file_num), sizeof(_int64));

    recipe_out.close();
    fileinfo_out.close();
}

recipe_manager::recipe_manager(wstring w_path, wstring b_path) :
    work_path(w_path), backup_path(b_path) {
    if (work_path[work_path.size() - 1] != L'/') {
        work_path += L'/';
    }
    CHECK_DIR(work_path);
    if (backup_path[backup_path.size() - 1] != '/') {
        backup_path += '/';
    }
    CHECK_DIR(backup_path);

    chunk_num = 0; data_size = 0;
    unique_num = 0; unique_size = 0;
    file_num = 0;

    status = 0;

    init_backup_version();
    init_backup_recipe_stream();
}

recipe_manager::recipe_manager(int version, wstring w_path, wstring r_path) :
    backup_version(version), work_path(w_path), restore_path(r_path) {
    if (work_path[work_path.size() - 1] != L'/') {
        work_path += L'/';
    }
    CHECK_DIR(work_path);
    if (restore_path[restore_path.size() - 1] != L'/') {
        restore_path += L'/';
    }
    CHECK_DIR(restore_path);

    wstring recipe_path = work_path + L"version_recipe" + L'/';
    CHECK_DIR(recipe_path);

    ifstream pahtinfo(recipe_path + L"pathinfo" +
        to_wstring(backup_version), ifstream::binary);

    char buffer[sizeof(int)];
    pahtinfo.read(buffer, sizeof(int));
    int read_version = *(int*)buffer;
    assert(read_version == backup_version);
    pahtinfo.read(buffer, sizeof(int));
    int path_size = *(int*)buffer;

    char *path_buffer = new char[path_size];
    pahtinfo.read(path_buffer, path_size);
    pahtinfo.close();
    string muti_backup_path;
    muti_backup_path.assign(path_buffer, path_size);
    delete path_buffer;

    backup_path = string2wstring(muti_backup_path);
    if (backup_path[backup_path.size() - 1] != L'/') {
        backup_path += L'/';
    }
    CHECK_DIR(backup_path);

    recipe_in.open(recipe_path + L"recipe" +
        to_wstring(backup_version), ofstream::binary);
    fileinfo_in.open(recipe_path + L"fileinfo" +
        to_wstring(backup_version), ofstream::binary);

    char num_buffer[sizeof(_int64)];
    recipe_in.read(num_buffer, sizeof(_int64));
    chunk_num = *(_int64*)num_buffer;
    fileinfo_in.read(num_buffer, sizeof(_int64));
    file_num = *(_int64*)num_buffer;

    data_size = 0;

    status = 1;
}


recipe_manager::~recipe_manager() {
    if (status == 0) {
        close_backup_recipe_stream();
        close_backup_version();
    }
    else {
        recipe_in.close();
        fileinfo_in.close();
    }
}

void recipe_manager::add_recipe_to_stream(chunk* ck) {

    static int file_chunk_num;

    if (CHECK_CHUNK(ck, CHUNK_FILE_START)) {
        // Length of file name and file name
        fileinfo_buffer.write((char*)(&ck->size), sizeof(int));
        fileinfo_buffer.write(ck->data.c_str(), ck->size);
        ++file_num;

        file_chunk_num = 0;
    }
    else if (CHECK_CHUNK(ck, CHUNK_FILE_END)) {
        // Chunk num of file
        fileinfo_buffer.write((char*)(&file_chunk_num), sizeof(int));
        write_buffer_to_stream();   
    }
    else {
        // Chunk mark and container id
        recipe_buffer.write(ck->mark.c_str(), FP_SIZE);
        recipe_buffer.write((char*)(&ck->id), sizeof(_int64));
        ++chunk_num; data_size += ck->size;
        if (CHECK_CHUNK(ck, CHUNK_UNIQUE)) {
            ++unique_num; unique_size += ck->size;
        }

        ++file_chunk_num;
    }
}

void recipe_manager::write_buffer_to_stream() {
    // Write content of buffer to file stream
    recipe_buffer.seekg(0, stringstream::end);
    int buffer_size = recipe_buffer.tellg();
    recipe_out.write(recipe_buffer.str().c_str(), buffer_size);
    recipe_buffer.str("");

    fileinfo_buffer.seekg(0, stringstream::end);
    buffer_size = fileinfo_buffer.tellg();
    fileinfo_out.write(fileinfo_buffer.str().c_str(), buffer_size);
    fileinfo_buffer.str("");
}

chunk* recipe_manager::get_recipe_from_stream(chunk* ck) {
    static _int64 cur_file_num = 0;
    static int file_chunk_num = 0;
    static int cur_file_chunk_num = 0;
    
    if (cur_file_num == file_num && cur_file_chunk_num == 0) {
        return nullptr;
    }
    
    if (cur_file_chunk_num == 0) {
        // Read file path size
        char path_size_buffer[sizeof(int)];
        fileinfo_in.read(path_size_buffer, sizeof(int));
        int path_size = *(int*)path_size_buffer;

        // Read file path
        char* path_buffer = new char[path_size];
        fileinfo_in.read(path_buffer, path_size);
        string temp(path_buffer, path_size);
        wstring wtemp = string2wstring(temp);
        wtemp = wtemp.substr(backup_path.size());
        ck->data = wstring2string(restore_path + wtemp);
        delete path_buffer;

        // Set chunk
        ck->size = path_size;
        SET_CHUNK(ck, CHUNK_FILE_START);

        // Read chunk num of file
        char chunk_num_buffer[sizeof(int)];
        fileinfo_in.read(chunk_num_buffer, sizeof(int));
        file_chunk_num = *(int*)chunk_num_buffer;

        ++cur_file_chunk_num;
        ++cur_file_num;
        return ck;
    }
    if (cur_file_chunk_num == file_chunk_num + 1) {
        // Set chunk
        SET_CHUNK(ck, CHUNK_FILE_END);

        cur_file_chunk_num = 0;
        file_chunk_num = 0;
        return ck;
    }

    char mark_buffer[FP_SIZE];
    recipe_in.read(mark_buffer, FP_SIZE);
    ck->mark.assign(mark_buffer, FP_SIZE);
    char id_buffer[sizeof(_int64)];
    recipe_in.read(id_buffer, sizeof(_int64));
    ck->id = *(_int64*)id_buffer;

    ++cur_file_chunk_num;

    return ck;
}

index_manager::index_manager(wstring w_path) : work_path(w_path) {
    if (work_path[work_path.size() - 1] != '/') {
        work_path += '/';
    }
    CHECK_DIR(work_path);

    ifstream in(work_path + L"dedup_index", ifstream::binary);

    in.seekg(0, ifstream::end);
    if (in.tellg() > 0) {
        in.seekg(0, ifstream::beg);
        char index_num_buffer[sizeof(_int64)];
        in.read(index_num_buffer, sizeof(_int64));
        _int64 mark_index_num = *(_int64*)(index_num_buffer);

        char mark_buffer[FP_SIZE];
        char id_buffer[sizeof(_int64)];

        string mark; _int64 id;
        for (_int64 num = 0; num < mark_index_num; ++num) {
            in.read(mark_buffer, FP_SIZE);
            mark.assign(mark_buffer, FP_SIZE);

            in.read(id_buffer, sizeof(_int64));
            id = *(_int64*)(id_buffer);
            mark_index.insert({ mark, id });
        }
        assert(mark_index.size() == mark_index_num);
    }
    in.close();
}

void index_manager::insert_mark_to_buffer(chunk* ck) {
    buffer_mutex.lock();
    mark_buffer.insert({ ck->mark,ck->id });
    buffer_mutex.unlock();
}

void index_manager::mark_dedup_check(struct chunk* ck) {
    if (CHECK_CHUNK(ck, CHUNK_FILE_START) || 
        CHECK_CHUNK(ck, CHUNK_FILE_END)) {
        return;
    }
    bool buffer_dup(false), index_dup(false);

    buffer_mutex.lock();
    auto buffer_res = mark_buffer.find(ck->mark);
    if (buffer_res != mark_buffer.end()) {
        SET_CHUNK(ck, CHUNK_DEDUP);
        ck->id = buffer_res->second;
        buffer_dup = true;
    }
    buffer_mutex.unlock();
    if (buffer_dup) { return; }

    index_mutex.lock();
    auto index_res = mark_index.find(ck->mark);
    if (buffer_res!=mark_index.end()) {
        SET_CHUNK(ck, CHUNK_DEDUP);
        ck->id = index_res->second;
        insert_mark_to_buffer(ck);
        index_dup = true;
    }
    index_mutex.unlock();
    if (index_dup) { return; }

    SET_CHUNK(ck, CHUNK_UNIQUE);
    insert_mark_to_buffer(ck);
}

void index_manager::update_mark_index() {
    index_mutex.lock();
    buffer_mutex.lock();
    for (auto e : mark_buffer) {
        mark_index.insert(e);
    }
    mark_buffer.clear();
    buffer_mutex.unlock();
    index_mutex.unlock();

    ofstream out(work_path + L"dedup_index", ofstream::binary);
    index_mutex.lock();
    _int64 mark_index_num = mark_index.size();
    index_mutex.unlock();
    out.write((char*)(&mark_index_num), sizeof(_int64));

    index_mutex.lock();
    for (auto &e : mark_index) {
        out.write(e.first.c_str(), FP_SIZE);
        out.write((char*)(&e.second), sizeof(_int64));
    }
    mark_index.clear();
    index_mutex.unlock();

    out.close();
}

void index_manager::update_id_in_buffer(chunk* ck) {
    if (CHECK_CHUNK(ck, CHUNK_FILE_START) ||
        CHECK_CHUNK(ck, CHUNK_FILE_END)) {
        return;
    }
    if (!CHECK_CHUNK(ck, CHUNK_UNIQUE)) {
        return;
    }
    assert(ck->id != TEMP_ID);
    buffer_mutex.lock();
    mark_buffer[ck->mark] = ck->id;
    buffer_mutex.unlock();
}

void index_manager::update_id_of_chunk(chunk* ck) {
    if (CHECK_CHUNK(ck, CHUNK_FILE_START) ||
        CHECK_CHUNK(ck, CHUNK_FILE_END)) {
        return;
    }
    buffer_mutex.lock();
    auto res = mark_buffer.find(ck->mark);
    if (res != mark_buffer.end()) {
        ck->id = res->second;
    }
    buffer_mutex.unlock();
    assert(ck->id != TEMP_ID);
}