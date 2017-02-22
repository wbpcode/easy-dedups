
#include"hash_sha.h"
#include<iostream>
#include<string>
#include<cassert>
#include<cmath>

#define SUB_DATA_SIZE (512/8) //Byte number
#define DATA_SIZE_LONG (64/8) 

using std::string; 
using std::cin; using std::cout; using std::endl;

string data_padding(string data) {
	//Get padding byte number
	unsigned _int64 data_size = data.size(); 
	int padding_size = (data_size / SUB_DATA_SIZE + 1)*SUB_DATA_SIZE - DATA_SIZE_LONG - data_size;
	if (padding_size == 0) {
		padding_size = SUB_DATA_SIZE;
	}

	//Prepare first byte for padding
	unsigned _int16 padding_num = 32768;//binary:1000000000000000 or 0x8000
	char* padding_first = (char*)(&padding_num);

	//Prepare last 8 byte for padding
	char* padding_last_8 = (char*)(&data_size);

	//Allocate memory for data
	char* c_data = new char[data_size + padding_size + DATA_SIZE_LONG];
	data.copy(c_data, data_size);

	int padding_pos = data_size;
	int padding_end = data_size + padding_size;
	for (; padding_pos < padding_end; ++padding_pos) {
		if (padding_pos == data_size) {
			c_data[padding_pos] = padding_first[0];
		}
		c_data[padding_pos] = 0;
	}

	padding_pos = padding_end;
	padding_end = padding_end + DATA_SIZE_LONG;
	for (; padding_pos < padding_end; ++padding_pos) {
		c_data[padding_pos] = padding_last_8[DATA_SIZE_LONG - padding_end - padding_pos];
	}

	string str_data(padding_end, 0);
	int pos = 0;
	for (; pos < padding_end; ++pos) {
		str_data[pos] = c_data[pos];
	}
	delete c_data;
	return str_data;
}

string hash_md5(string data) {
	string str_data = data_padding(data);
	unsigned _int32 buffer_sub_one=0x01234567,
					buffer_sub_two=0x89abcdef,
					buffer_sub_three=0xfedcba98,
					buffer_sub_four=0x76543210;




	char* sub_data = new char[SUB_DATA_SIZE];
	int data_pos = 0;
	if (str_data.copy(sub_data, SUB_DATA_SIZE, data_pos)){
		;
	}

}

string hash_sha1(string data) {
	;
}
