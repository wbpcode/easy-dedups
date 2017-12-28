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

wstring work_path = L"C:/Users/ping/Documents/workpath/";

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

    cout << "Backup start!!!" << endl;
    data_read();
    data_chunk();
    data_hash();
    data_dedup();
    data_write();
    cout << "Backup end!!!" << endl;

    delete global_manager;

}

void data_restore(int version, wstring restore_path) {

    mine_restore_recipe.restore_recipe_init(version, work_path, restore_path);
    mine_container_set.container_set_init(work_path);

    cout << "Restore start!!!" << endl;
    restore_get_recipe();
    restore_get_chunk();
    restore_write_file();
    cout << "Restore end!!!" << endl;

    mine_restore_recipe.restore_recipe_close();
    mine_container_set.container_set_close();

}

int main(int argc, wchar_t** argv) {

    if (work_path[work_path.size() - 1] != '\\') {
        work_path += '\\';
    }
    CHECK_DIR(work_path);


    wstring backup_path = L"C:\\Users\\ping\\Downloads\\";
    wstring restore_path = L"C:\\Users\\ping\\Documents\\restore_path\\";

    data_backup(backup_path);
    data_restore(0,restore_path);

}