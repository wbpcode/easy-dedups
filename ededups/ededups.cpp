#include"ededups.h"
#include"bk_read.h"
#include"bk_chunk.h"
#include"bk_hash.h"
#include"bk_dedup.h"
#include"bk_write.h"
#include"manager.h"
#include"restore.h"
#include<windows.h>
#include<thread>
#include<iostream>

using namespace std;

wstring work_path = L"C:/Users/ping/Documents/work_path/";

manager* global_manager;

wstring string2wstring(string path) {
    int path_size = MultiByteToWideChar(CP_ACP, 0, path.c_str(), -1, 
        nullptr, 0);
    wchar_t* path_buffer = new wchar_t[path_size];
    MultiByteToWideChar(CP_ACP, 0, path.c_str(), -1, path_buffer, 
        path_size);
    wstring newpath = path_buffer;
    delete path_buffer;

    return newpath;
}

string wstring2string(wstring path) {

    int path_size = WideCharToMultiByte(CP_ACP, 0, path.c_str(), -1, 
        nullptr, 0, nullptr, 0);
    char *path_buffer = new char[path_size];
    WideCharToMultiByte(CP_ACP, 0, path.c_str(), -1, path_buffer, 
        path_size, nullptr, 0);
    string newpath = path_buffer;
    delete path_buffer;
    return newpath;
}

void data_backup(wstring backup_path) {

    global_manager = new manager(work_path, backup_path);

    cout << "Backup start.................." << endl;

    thread read_thread(data_read);
    thread chunk_thread(data_chunk);
    thread hash_thread(data_hash);
    thread dedup_thread(data_dedup);
    thread write_thread(data_write);

    read_thread.join();
    chunk_thread.join();
    hash_thread.join();
    dedup_thread.join();
    write_thread.join();

    cout << "Backup end...................." << endl;

    delete global_manager;

}

void data_restore(int version, wstring restore_path) {

    global_manager = new manager(version, work_path, restore_path);

    cout << "Restore start................." << endl;
    thread recipe_thread(get_restore_recipe);
    thread chunk_thread(get_restore_chunk);
    thread write_thread(write_restore_file);
    
    recipe_thread.join();
    chunk_thread.join();
    write_thread.join();

    cout << "Restore end..................." << endl;

    delete global_manager;

}

int main(int argc, wchar_t** argv) {

    if (work_path[work_path.size() - 1] != L'/') {
        work_path += L'/';
    }
    CHECK_DIR(work_path);


    wstring backup_path = L"C:/Users/ping/Downloads/";
    wstring restore_path = L"C:/Users/ping/Documents/restore_path/";

    //data_backup(backup_path);
    data_restore(1,restore_path);
    system("pause");

}