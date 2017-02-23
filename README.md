#Introduction
A easy deduplication system.

#Getting Started
Mainly using C++ standard library.
在开发过程中，发现对于大量二进制数据流的处理，unsigned char*是最好的数据结构，虽然string同样支持二进制类型，
但是由于windows.h提供的大量关于文件操作的API都必须使用char*类型，所以之间不可避免涉及到数据的转换。
虽然string类型可以完美转换成字符数据char*，但是字符数据类型转换成string却是一个问题，虽然string可以通过c字符串
直接赋值，但是，此种方式以来空字符\0作为分界，可是在二进制流中，可能遍布\0，赋值将无法正将进行。
一种解决方法是通过对数组的便利来逐字符给string对象赋值，但是成本太高。
---20170223---wbpcode

前面问题未必不可以解决，实际上string提供大量函数方法，其中插入功能以及复制其他string对象赋值功能可以帮助解决上面问题。
---20170223---wbpcode

#Build and Test


