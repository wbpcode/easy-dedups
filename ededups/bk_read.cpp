#include"ededups.h"
#include"bk_read.h"
#include"manager.h"
#include<fstream>
#include<string>
#include<cassert>
#include<iostream>
#include<windows.h>

using namespace std;

extern manager* global_manager;

void read_one_file(wstring path) {

    chunk* cks = new chunk();
    SET_CHUNK(cks, CHUNK_FILE_START);
    cks->data = wstring2string(path);
    cks->size = cks->data.size();
    global_manager->stream.put_chunk_to_read_list(cks);
    
    ifstream filestream(path, ifstream::binary);
    char* data_buffer = new char[READ_BLOCK_SIZE];
    while (filestream.read(data_buffer, READ_BLOCK_SIZE)) {
        chunk* ck = new chunk();
        ck->size = READ_BLOCK_SIZE;
        ck->data.assign(data_buffer, READ_BLOCK_SIZE);
        global_manager->stream.put_chunk_to_read_list(ck);
    }
    int last_data_size = filestream.gcount();
    if (last_data_size > 0) {
        chunk* ck = new chunk();
        ck->size = last_data_size;
        ck->data.assign(data_buffer, last_data_size);
        global_manager->stream.put_chunk_to_read_list(ck);
    }

    delete data_buffer;
    filestream.close();

    chunk* cke = new chunk();
    SET_CHUNK(cke, CHUNK_FILE_END);
    global_manager->stream.put_chunk_to_read_list(cke);
}

void find_all_file(wstring path) {
    if (path[path.size() - 1] != L'/') { path += L"/"; }

    WIN32_FIND_DATA fileinfo;
    wstring search_path = path + L"*.*";
    HANDLE findend = FindFirstFile(search_path.c_str(), &fileinfo);

    if (findend == INVALID_HANDLE_VALUE) {
        return;
    }
    do {
        if (fileinfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (wstring(fileinfo.cFileName) != L"." &&
                wstring(fileinfo.cFileName) != L"..") {
                find_all_file(path + wstring(fileinfo.cFileName));
            }
        }
        else {
            read_one_file(path + wstring(fileinfo.cFileName));
        }
    } while (FindNextFile(findend, &fileinfo));

    FindClose(findend);
}

void data_read() {
    cout << "Reading start................." << endl;
    global_manager->stream.read_atomic = false;
    find_all_file(global_manager->recipe.backup_path);
    global_manager->stream.read_atomic = true;
    cout << "Reading end..................." << endl;
}