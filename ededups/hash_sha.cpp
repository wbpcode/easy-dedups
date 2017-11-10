#include<iostream>
#include<string>
#include<sstream>
#include<cassert>
#include<cmath>

#define SUB_DATA_SIZE (512/8) //Byte number
#define DATA_SIZE_LONG (64/8) 

//md5
#define md5_f(a,b,c) ((a&b)|((~a)&c))
#define md5_g(a,b,c) ((a&c)|(b&(~c)))
#define md5_h(a,b,c) (a^b^c)
#define md5_i(a,b,c) (b^(a|(~c)))

//sha1
#define sha1_f1(a,b,c) ((a&b)|((~a)&c))
#define sha1_f2(a,b,c) (a^b^c)
#define sha1_f3(a,b,c) ((a&b)|(a&c)|(b&c))
#define sha1_f4(a,b,c) (a^b^c)

#define ring_ls(x,y) ((x>>(32-y))|(x<<y))
#define ring_rs(x,y) ((x<<(32-y))|(x>>y))

#define data_ext(a,b,c,d) ring_ls((a^b^c^d),1)



using std::string; using std::ostringstream;
using std::cin; using std::cout; using std::endl;
using std::hex;

string md5_data_padding(string data) {
    //Get padding byte number
    unsigned _int64 data_size = data.size(); 
    unsigned _int64 padding_size = (data_size / SUB_DATA_SIZE + 2)*SUB_DATA_SIZE - DATA_SIZE_LONG - data_size;
    if (padding_size > SUB_DATA_SIZE) {
        padding_size -= SUB_DATA_SIZE;
    }
    //Prepare byte for padding(The longest padding bits are 512bits(64byte))
    static char padding_first[] = { 0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };

    //Prepare last 8 byte for padding
    unsigned _int64 data_size_bit = data_size * 8;
    char* padding_last_8 = (char*)(&data_size_bit);


    data.append(padding_first, padding_size);
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
    string str_data = md5_data_padding(data);
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


    string md5_str_end;
    

    for (auto buffer_outcome : sub_buffer) {
        unsigned _int32 buffer_outcome_sort = { (buffer_outcome >> 24) & 0x000000ff |
            (buffer_outcome >> 8) & 0x0000ff00 |
            (buffer_outcome << 8) & 0x00ff0000 |
            (buffer_outcome << 24 & 0xff000000) };

            static ostringstream md5_str;
            md5_str.str("");
            md5_str.width(8);
            md5_str.fill('0');

            md5_str << hex << buffer_outcome_sort;
            md5_str_end += md5_str.str();

            md5_str.str("");
    }

    delete sub_data;

    return md5_str_end;
}

//sha1 function

string sha1_data_padding(string data) {
    //Get padding byte number
    unsigned _int64 data_size = data.size();
    unsigned _int64 padding_size = (data_size / SUB_DATA_SIZE + 2)*SUB_DATA_SIZE - DATA_SIZE_LONG - data_size;
    if (padding_size > SUB_DATA_SIZE) {
        padding_size -= SUB_DATA_SIZE;
    }
    //Prepare byte for padding(The longest padding bits are 512bits(64byte))
    static char padding_first[] = { 0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };


    data.append(padding_first, padding_size);

    for (int last_pos = 7; last_pos >= 0; --last_pos) {
        data.push_back((unsigned char)((data_size*8)>> (last_pos*8)));
    }

    return data;
    

}

void sha1_ff1(unsigned _int32* sub_buffer_a, unsigned _int32* sub_buffer_b, unsigned _int32* sub_buffer_c,unsigned _int32* sub_buffer_d, 
    unsigned _int32* sub_buffer_e, unsigned _int32 sub_sub_data, unsigned _int32 constant_parameter ) {

    unsigned _int32 temp = *sub_buffer_e + sha1_f1(*sub_buffer_b, *sub_buffer_c, *sub_buffer_d) + ring_ls(*sub_buffer_a, 5) + sub_sub_data + constant_parameter;

    *sub_buffer_e = *sub_buffer_d;
    *sub_buffer_d = *sub_buffer_c;
    *sub_buffer_c = ring_ls((*sub_buffer_b), 30);
    *sub_buffer_b = *sub_buffer_a;

    *sub_buffer_a = temp;
}

void sha1_ff2(unsigned _int32* sub_buffer_a, unsigned _int32* sub_buffer_b, unsigned _int32* sub_buffer_c, unsigned _int32* sub_buffer_d,
    unsigned _int32* sub_buffer_e, unsigned _int32 sub_sub_data, unsigned _int32 constant_parameter) {

    unsigned _int32 temp = *sub_buffer_e + sha1_f2(*sub_buffer_b, *sub_buffer_c, *sub_buffer_d) + ring_ls(*sub_buffer_a, 5) + sub_sub_data + constant_parameter;

    *sub_buffer_e = *sub_buffer_d; 
    *sub_buffer_d = *sub_buffer_c; 
    *sub_buffer_c = ring_ls((*sub_buffer_b), 30); 
    *sub_buffer_b = *sub_buffer_a;

    *sub_buffer_a = temp;
}

void sha1_ff3(unsigned _int32* sub_buffer_a, unsigned _int32* sub_buffer_b, unsigned _int32* sub_buffer_c, unsigned _int32* sub_buffer_d,
    unsigned _int32* sub_buffer_e, unsigned _int32 sub_sub_data, unsigned _int32 constant_parameter) {

    unsigned _int32 temp = *sub_buffer_e + sha1_f3(*sub_buffer_b, *sub_buffer_c, *sub_buffer_d) + ring_ls(*sub_buffer_a, 5) + sub_sub_data + constant_parameter;

    *sub_buffer_e = *sub_buffer_d;
    *sub_buffer_d = *sub_buffer_c; 
    *sub_buffer_c = ring_ls((*sub_buffer_b), 30); 
    *sub_buffer_b = *sub_buffer_a;
    *sub_buffer_a = temp;
}

void sha1_ff4(unsigned _int32* sub_buffer_a, unsigned _int32* sub_buffer_b, unsigned _int32* sub_buffer_c, unsigned _int32* sub_buffer_d,
    unsigned _int32* sub_buffer_e, unsigned _int32 sub_sub_data, unsigned _int32 constant_parameter) {

    unsigned _int32 temp = ring_ls(*sub_buffer_a, 5)  + sha1_f4(*sub_buffer_b, *sub_buffer_c, *sub_buffer_d) + *sub_buffer_e + sub_sub_data + constant_parameter;
    *sub_buffer_e = *sub_buffer_d;
    *sub_buffer_d = *sub_buffer_c; 
    *sub_buffer_c = ring_ls(*sub_buffer_b, 30); 
    *sub_buffer_b = *sub_buffer_a;
    *sub_buffer_a = temp;
}

string hash_sha1(string data) {
    string str_data=sha1_data_padding(data);

    unsigned _int32 sub_buffer[5] = { 0x67452301,0xefcdab89,0x98badcfe,0x10325476,0xc3d2e1f0 };
    unsigned _int32 sub_buffer_bk[5] = { 0x67452301,0xefcdab89,0x98badcfe,0x10325476,0xc3d2e1f0 };

    static unsigned _int32 constant_parameter[4] = { 0x5a827999,0x6ed9eba1,0x8f1bbcdc,0xca62c1d6 };

    int data_pos = 0;
    char* sub_data = new char[SUB_DATA_SIZE];
    while (str_data.copy(sub_data, SUB_DATA_SIZE, data_pos)==SUB_DATA_SIZE) {
        unsigned _int32 sub_sub_data[80];
        for (int sub_sub_pos = 0; sub_sub_pos < 80; ++sub_sub_pos) {
            if (sub_sub_pos < 16) {
                sub_sub_data[sub_sub_pos] = ((unsigned _int32)(unsigned char)sub_data[sub_sub_pos * 4] & 0xffffff)<<24 |
                    ((unsigned _int32)(unsigned char)sub_data[sub_sub_pos * 4 + 1] & 0xffffff)<<16 |
                    ((unsigned _int32)(unsigned char)sub_data[sub_sub_pos * 4 + 2] & 0xffffff)<<8 |
                    ((unsigned _int32)(unsigned char)sub_data[sub_sub_pos * 4 + 3] & 0xffffff);
            }
            else {
                sub_sub_data[sub_sub_pos] = data_ext(sub_sub_data[sub_sub_pos - 3],sub_sub_data[sub_sub_pos - 8],
                    sub_sub_data[sub_sub_pos - 14],sub_sub_data[sub_sub_pos-16]);
            }
        }

        sub_buffer_bk[0] = sub_buffer[0]; 
        sub_buffer_bk[1] = sub_buffer[1];
        sub_buffer_bk[2] = sub_buffer[2]; 
        sub_buffer_bk[3] = sub_buffer[3]; 
        sub_buffer_bk[4] = sub_buffer[4];

        for (int step_pos = 0; step_pos < 80; ++step_pos) {
            if (step_pos < 20) {
                sha1_ff1(&sub_buffer[0], &sub_buffer[1], &sub_buffer[2], &sub_buffer[3], &sub_buffer[4], sub_sub_data[step_pos], constant_parameter[0]);

            }
            else if(step_pos<40) {
                sha1_ff2(&sub_buffer[0], &sub_buffer[1], &sub_buffer[2], &sub_buffer[3], &sub_buffer[4], sub_sub_data[step_pos], constant_parameter[1]);

            }
            else if (step_pos < 60) {
                sha1_ff3(&sub_buffer[0], &sub_buffer[1], &sub_buffer[2], &sub_buffer[3], &sub_buffer[4], sub_sub_data[step_pos], constant_parameter[2]);

            }
            else if (step_pos< 80) {
                sha1_ff4(&sub_buffer[0], &sub_buffer[1], &sub_buffer[2], &sub_buffer[3], &sub_buffer[4], sub_sub_data[step_pos], constant_parameter[3]);

            }

        }

        sub_buffer[0] += sub_buffer_bk[0]; 
        sub_buffer[1] += sub_buffer_bk[1]; 
        sub_buffer[2] += sub_buffer_bk[2];
        sub_buffer[3] += sub_buffer_bk[3]; 
        sub_buffer[4] += sub_buffer_bk[4];

        data_pos += SUB_DATA_SIZE;

    }


    string sha1_str_end;

    for (auto buffer_outcome : sub_buffer) {
        static ostringstream sha1_str;
        sha1_str.str("");
        sha1_str.width(8);
        sha1_str.fill('0');

        sha1_str << hex << buffer_outcome;
        sha1_str_end += sha1_str.str();

        sha1_str.str("");	
    }

    delete sub_data;

    //cout << sha1_str_end << endl;

    return sha1_str_end;
}
