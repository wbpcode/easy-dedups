#pragma once
#include<string>
#include"data_struct.h"
#include"meta_struct.h"

class manager {
public:
    stream_manager stream;
    container_manager container;
    index_manager index;
    recipe_manager recipe;

    manager(std::wstring w_path, std::wstring b_path) :
        container(w_path), index(w_path), recipe(w_path, b_path) {
        ;
    }
    manager(int version,std::wstring w_path,std::wstring r_path):
        container(w_path), index(w_path), recipe(version, w_path, r_path) {
        ;
    }
    ~manager() = default;
};