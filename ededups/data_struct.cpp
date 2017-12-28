#include<fstream>
#include<cassert>
#include"data_struct.h"

using namespace std;

container_manager::container_manager(wstring path): work_path(path) {

    if (work_path[work_path.size() - 1] != L'/') {
        work_path += L'/';
    }
    CHECK_DIR(work_path);

    ifstream in(work_path + L"container_count", ifstream::binary);
    in.seekg(0, ifstream::end);
    if (in.tellg() > 0) {

        in.seekg(0, ifstream::beg);

        char container_count_buffer[sizeof(_int64)];
        in.read(container_count_buffer, sizeof(_int64));
        container_count = *(_int64*)(container_count_buffer);
    }
    else {
        container_count = 0;
    }
    in.close();
}

container_manager::~container_manager() {
    for (auto e : container_list) {
        delete e;
    }
    ofstream out(work_path + L"container_count", ofstream::binary);
    out.write((char*)(&container_count), sizeof(_int64));
    out.close();
}

int container_manager::get_container_list_size() {
    return container_list.size();
}

void container_manager::write_container_to_file() {
    wstring path = work_path + L"containers_set/";
    CHECK_DIR(path);
    while (!container_list.empty()) {
        auto cnr = container_list.front();
        ofstream out(path + L"container" + to_wstring(cnr->id), ofstream::binary);

        // Write container information
        out.write((char*)(&(cnr->id)), sizeof(_int64));
        out.write((char*)(&(cnr->size)), sizeof(int));
        out.write((char*)(&(cnr->chunk_num)), sizeof(int));

        // Write container metadata
        for (auto &e : cnr->chunk_map) {
            out.write(e.second->mark.c_str(), FP_SIZE);
            out.write((char*)(&(e.second->size)), sizeof(int));
            out.write((char*)(&(e.second->offset)), sizeof(int));
        }

        // Write container data: c_str method return a const char pointer
        out.write(cnr->data.c_str(), cnr->size);

        out.close();     
        delete cnr;
        container_list.pop_front();
    }
}

container* container_manager::read_container_to_mem(_int64 id) {
    assert(id != TEMP_ID);

    wstring path = work_path + L"containers_set/";
    CHECK_DIR(path);

    container* cnr = new container();

    ifstream in(path + L"container" + to_wstring(id), ifstream::binary);
    // Read container information
    char id_buffer[sizeof(_int64)];
    in.read(id_buffer, sizeof(_int64));
    cnr->id = *(_int64*)id_buffer;
    assert(cnr->id == id);

    char size_buffer[sizeof(int)];
    in.read(size_buffer, sizeof(int));
    cnr->size = *(int*)size_buffer;

    char num_buffer[sizeof(int)];
    in.read(num_buffer, sizeof(int));
    cnr->chunk_num = *(int*)num_buffer;

    char ckmark_buffer[FP_SIZE];
    char cksize_buffer[sizeof(int)];
    char ckoffset_buffer[sizeof(int)];
    // Read container metadata
    for (int num = cnr->chunk_num; num > 0; --num) {
        chunk_meta* ckmeta = new chunk_meta;
        in.read(ckmark_buffer, FP_SIZE);
        in.read(cksize_buffer, sizeof(int));
        in.read(ckoffset_buffer, sizeof(int));

        ckmeta->mark.assign(ckmark_buffer, FP_SIZE);
        ckmeta->size = *(int*)cksize_buffer;
        ckmeta->offset = *(int*)ckoffset_buffer;
        ckmeta->id = id;
        ckmeta->flag = 0;
        cnr->chunk_map.insert({ ckmeta->mark, ckmeta });
    }
    // Read container data
    char* data_buffer = new char[cnr->size];
    in.read(data_buffer, cnr->size);
    cnr->data.assign(data_buffer, cnr->size);
    delete data_buffer;
    // If num of container in memory is greater than cache num, kick one out
    if (container_list.size() >= CONTAINER_MAX_CACHE_SIZE) {
        delete container_list.front();
        container_list.pop_front();
    }
    container_list.push_back(cnr);
    return cnr;
}

void container_manager::add_chunk_to_container(struct chunk* ck) {
    container* cnr;
    if (container_list.empty()) {
        cnr = new container();
        cnr->id = container_count;
        ++container_count;
        container_list.push_back(cnr);
    }
    else {
        cnr = container_list.back();
        if (cnr->chunk_num >= CONTAINER_MAX_CHUNK_NUM) {
            cnr = new container();
            cnr->id = container_count;
            ++container_count;
            if (container_list.size() >= CONTAINER_MAX_LIST_SIZE) {
                write_container_to_file();
                assert(container_list.empty());
            }
            container_list.push_back(cnr);
        }
    }

    // Create a new chunk metadata and add it to chunk_map
    chunk_meta* ckmeta = new chunk_meta();
    ckmeta->mark = ck->mark;
    ckmeta->size = ck->size;
    ckmeta->flag = ck->flag;
    ckmeta->offset = cnr->size;
    ckmeta->id = cnr->id;
    cnr->chunk_map.insert({ ckmeta->mark, ckmeta });
    // Update container information
    cnr->size += ck->size;
    cnr->chunk_num += 1;
    cnr->data = cnr->data + ck->data;
    // Update chunk id
    ck->id = cnr->id;
}

container* container_manager::check_container(_int64 id) {
    for (auto e : container_list) {
        if (e->id == id) {
            return e;
        }
    }
    return nullptr;
}

chunk_meta* container_manager::check_chunk(struct chunk* ck) {
    for (auto e : container_list) {
        auto outcome = e->chunk_map.find(ck->mark);
        if (outcome != e->chunk_map.end()) {
            return outcome->second;
        }
    }
    return nullptr;
}

struct chunk* container_manager::get_chunk_from_container(struct chunk* ck) {
    assert(ck->id != TEMP_ID);
    struct container* cnr = check_container(ck->id);
    if (cnr == nullptr) {
        cnr = read_container_to_mem(ck->id);
    }
    auto outcome = cnr->chunk_map.find(ck->mark);
    assert(outcome != cnr->chunk_map.end());
    struct chunk_meta* ckmeta = outcome->second;
    ck->size = ckmeta->size;
    ck->data.assign(cnr->data, ckmeta->offset, ckmeta->size);
    return ck;
}

chunk* stream_manager::get_chunk_from_read_list() {
    chunk* temp = nullptr;
    read_mutex.lock();
    if (!read_list.empty()) { 
        temp = read_list.front(); 
        read_list.pop_front(); 
    }
    read_mutex.unlock();
    return temp;
}

void stream_manager::put_chunk_to_read_list(chunk* ck) {
    read_mutex.lock();
    read_list.push_back(ck);
    read_mutex.unlock();
}

chunk* stream_manager::get_chunk_from_chunk_list() {
    chunk* temp = nullptr;
    chunk_mutex.lock();
    if (!chunk_list.empty()) {
        temp = chunk_list.front();
        chunk_list.pop_front();
    }
    chunk_mutex.unlock();
    return temp;
}

void stream_manager::put_chunk_to_chunk_list(chunk* ck) {
    chunk_mutex.lock();
    chunk_list.push_back(ck);
    chunk_mutex.unlock();
}

chunk* stream_manager::get_chunk_from_hash_list() {
    chunk* temp = nullptr;
    hash_mutex.lock();
    if (!hash_list.empty()) {
        temp = hash_list.front();
        hash_list.pop_front();
    }
    hash_mutex.unlock();
    return temp;
}

void stream_manager::put_chunk_to_hash_list(chunk* ck) {
    hash_mutex.lock();
    hash_list.push_back(ck);
    hash_mutex.unlock();
}

chunk* stream_manager::get_chunk_from_dedup_list() {
    chunk* temp = nullptr;
    dedup_mutex.lock();
    if (!dedup_list.empty()) {
        temp = dedup_list.front();
        dedup_list.pop_front();
    }
    dedup_mutex.unlock();
    return temp;
}

void stream_manager::put_chunk_to_dedup_list(chunk* ck) {
    dedup_mutex.lock();
    dedup_list.push_back(ck);
    dedup_mutex.unlock();
}