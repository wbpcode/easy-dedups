
#include"hash_sha.h"
#include<iostream>
#include<string>
#include<cassert>
#include<cmath>

#define SUB_DATA_SIZE (512/8) //Byte number
#define DATA_SIZE_LONG (64/8) 

#define md5_f(a,b,c) ((a&b)|((~a)&c))
#define md5_g(a,b,c) ((a&c)|(b&(~c)))
#define md5_h(a,b,c) (a^b^c)
#define md5_i(a,b,c) (b^(a|(~c)))

#define ring_ls(x,y) ((x>>(32-y))|(x<<y))
#define ring_rs(x,y) ((x<<(32-y))|(x>>y))

using std::string; 
using std::cin; using std::cout; using std::endl;
using std::hex;

string data_padding(string data) {
	//Get padding byte number
	unsigned _int64 data_size = data.size(); 
	unsigned _int64 padding_size = (data_size / SUB_DATA_SIZE + 1)*SUB_DATA_SIZE - DATA_SIZE_LONG - data_size;
	if (padding_size == 0) {
		padding_size = SUB_DATA_SIZE;
	}
	//Prepare byte for padding(The longest padding bits are 512bits(64byte))
	static char padding_first[] = { 0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };

	//Prepare last 8 byte for padding
	char* padding_last_8 = (char*)(&data_size);

	//Insert and append(can do it just by one function,but here just for show how to use append and insert)
	data.insert(data_size, &padding_first[0], padding_size);
	data.append(padding_last_8, DATA_SIZE_LONG);

	return data;
}
void md5_ff(unsigned _int32* sub_buffer_a, unsigned _int32* sub_buffer_b, unsigned _int32* sub_buffer_c, unsigned _int32* sub_buffer_d,
	unsigned _int32 sub_sub_data, unsigned _int32 constant_parameter, unsigned _int32 ring_lshift) {
	*sub_buffer_a = *sub_buffer_b + ring_ls((*sub_buffer_a + md5_f(*sub_buffer_b, *sub_buffer_c, *sub_buffer_d) + 
		sub_sub_data + constant_parameter), ring_lshift);
}
void md5_gg(unsigned _int32* sub_buffer_a, unsigned _int32* sub_buffer_b, unsigned _int32* sub_buffer_c, unsigned _int32* sub_buffer_d,
	unsigned _int32 sub_sub_data, unsigned _int32 constant_parameter, unsigned _int32 ring_lshift) {
	*sub_buffer_a = *sub_buffer_b + ring_ls((*sub_buffer_a + md5_g(*sub_buffer_b, *sub_buffer_c, *sub_buffer_d) + 
		sub_sub_data + constant_parameter), ring_lshift);
}
void md5_hh(unsigned _int32* sub_buffer_a, unsigned _int32* sub_buffer_b, unsigned _int32* sub_buffer_c, unsigned _int32* sub_buffer_d,
	unsigned _int32 sub_sub_data, unsigned _int32 constant_parameter, unsigned _int32 ring_lshift) {
	*sub_buffer_a = *sub_buffer_b + ring_ls((*sub_buffer_a + md5_h(*sub_buffer_b, *sub_buffer_c, *sub_buffer_d) + 
		sub_sub_data + constant_parameter), ring_lshift);
}
void md5_ii(unsigned _int32* sub_buffer_a, unsigned _int32* sub_buffer_b, unsigned _int32* sub_buffer_c, unsigned _int32* sub_buffer_d,
	unsigned _int32 sub_sub_data, unsigned _int32 constant_parameter, unsigned _int32 ring_lshift) {
	*sub_buffer_a = *sub_buffer_b + ring_ls((*sub_buffer_a + md5_i(*sub_buffer_b, *sub_buffer_c, *sub_buffer_d) + 
		sub_sub_data + constant_parameter), ring_lshift);
}
string hash_md5(string data) {
	string str_data = data_padding(data);
	//Prepare the end buffer
	unsigned _int32 sub_buffer[4] = { 0x67452301,0xEFCDAB89,0x98BADCFE,0x10325476 };
	unsigned _int32 sub_buffer_bk[4] = { 0x67452301,0xEFCDAB89,0x98BADCFE,0x10325476 };

	//The order of sub_data that be used 
	static unsigned _int32 sub_sub_data_pos_parameter[4][16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
								   1,6,11,0,5,10,15,4,9,14,3,8,13,2,7,12,
								   5,8,11,14,1,4,7,10,13,0,3,6,9,12,15,2,
								   0,7,14,5,12,3,10,1,8,15,6,13,4,11,2,9};

	//The constant
	static unsigned _int32 constant_parameter[4][16] = { 0 };
	if (constant_parameter[0][0] == 0) {
		int first_pos = 0;
		unsigned _int64 max_constant_parameter= pow(2, 32);
		for (; first_pos < 4; ++first_pos) {
			int second_pos = 0;
			for (; second_pos < 16; ++second_pos) {
				constant_parameter[first_pos][second_pos] = floor(max_constant_parameter*fabs(sin(first_pos * 16 + second_pos+1)));
			}
		}
	}
	static unsigned _int32 ring_lshift_parameter[4][16] = { 7,12,17,22,7,12,17,22,7,12,17,22,7,12,17,22,
									     5,9,14,20,5,9,14,20,5,9,14,20,5,9,14,20,
									     4,11,16,23,4,11,16,23,4,11,16,23,4,11,16,23,
									     6,10,15,21,6,10,15,21,6,10,15,21,6,10,15,21 };

	char* sub_data=new char[SUB_DATA_SIZE];
	int data_pos = 0;
	while (str_data.copy(sub_data,SUB_DATA_SIZE,data_pos)==SUB_DATA_SIZE){
		unsigned _int32 sub_sub_data[16];
		for (int sub_sub_pos = 0; sub_sub_pos < 16; ++sub_sub_pos) {
			sub_sub_data[sub_sub_pos] = *(unsigned _int32*)(unsigned char*)(&sub_data[sub_sub_pos*4]);
		}
		sub_buffer_bk[0] = sub_buffer[0]; sub_buffer_bk[1] = sub_buffer[1];
		sub_buffer_bk[2] = sub_buffer[2]; sub_buffer_bk[3] = sub_buffer[3];
		int cycle_pos = 0;
		for (; cycle_pos < 4; ++cycle_pos) {
			int sub_buffer_a = 0, sub_buffer_b = 1, sub_buffer_c = 2, sub_buffer_d = 3, temp, step_pos = 0;
			for (; step_pos < 16; ++step_pos) {
				if (cycle_pos == 0) {
					md5_ff(&sub_buffer[sub_buffer_a], &sub_buffer[sub_buffer_b], &sub_buffer[sub_buffer_c], &sub_buffer[sub_buffer_d],
						sub_sub_data[sub_sub_data_pos_parameter[cycle_pos][step_pos]], constant_parameter[cycle_pos][step_pos],
						ring_lshift_parameter[cycle_pos][step_pos]);
				}
				if (cycle_pos == 1) {
					md5_gg(&sub_buffer[sub_buffer_a], &sub_buffer[sub_buffer_b], &sub_buffer[sub_buffer_c], &sub_buffer[sub_buffer_d],
						sub_sub_data[sub_sub_data_pos_parameter[cycle_pos][step_pos]], constant_parameter[cycle_pos][step_pos],
						ring_lshift_parameter[cycle_pos][step_pos]);
				}
				if (cycle_pos == 2) {
					md5_hh(&sub_buffer[sub_buffer_a], &sub_buffer[sub_buffer_b], &sub_buffer[sub_buffer_c], &sub_buffer[sub_buffer_d],
						sub_sub_data[sub_sub_data_pos_parameter[cycle_pos][step_pos]], constant_parameter[cycle_pos][step_pos],
						ring_lshift_parameter[cycle_pos][step_pos]);
				}
				if (cycle_pos == 3) {
					md5_ii(&sub_buffer[sub_buffer_a], &sub_buffer[sub_buffer_b], &sub_buffer[sub_buffer_c], &sub_buffer[sub_buffer_d],
						sub_sub_data[sub_sub_data_pos_parameter[cycle_pos][step_pos]], constant_parameter[cycle_pos][step_pos],
						ring_lshift_parameter[cycle_pos][step_pos]);
				}
				temp = sub_buffer_d; sub_buffer_d = sub_buffer_c; sub_buffer_c = sub_buffer_b; sub_buffer_b = sub_buffer_a; sub_buffer_a = temp;
			}
		}
		sub_buffer[0] += sub_buffer_bk[0]; sub_buffer[1] += sub_buffer_bk[1];
		sub_buffer[2] += sub_buffer_bk[2]; sub_buffer[3] += sub_buffer_bk[3];
		data_pos += SUB_DATA_SIZE;
	}
	string md5_str;
	for (auto buffer_outcome : sub_buffer) {
		md5_str.append((char*)(&buffer_outcome), 4);
	}

	return md5_str;
}

string hash_sha1(string data) {
	string sha1_str;
	return sha1_str;
}
