
#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS
#include<iostream>
#include<ctime>
#include<string>
#include<fstream>
#include<boost/multiprecision/cpp_int.hpp>
#include<boost/multiprecision/miller_rabin.hpp>//大素数检测
#define NUMBER 2048   //一次加密NUMBER个字节大小的数据
#define OFFSET 128  //钥匙的位数
#define KEYS 5 //产生钥匙的个数
#define EKEYFILE "./ekey.txt" //产生钥匙的默认文件路径
#define DKEYFILE "./dkey.txt" //会在第一次产生100把钥匙供以后使用，以后加解密数据不在浪费时间产生素数和钥匙
#define PKEYFILE "./pkey.txt"
namespace mp = boost::multiprecision;
namespace rp = boost::random;
typedef mp::uint1024_t DataType;//DataType是一个任意位的大数
#define LENGTH sizeof(DataType) //DataType的大小

struct Key
{ 
	DataType m_eKey;//加密秘钥，e
	DataType m_dKey;//解密秘钥, d
	DataType m_pKey;//n
};

class RSA
{
private:

	Key m_key;
	

public:

	void ProdureKeyFile(const char* ekey_file = EKEYFILE, const char* dkey_file = DKEYFILE, const char* pkey_file = PKEYFILE);//产生钥匙文件
	DataType ExGcd(DataType a, DataType b, DataType& x, DataType& y);//求模范元素(公钥e)
	DataType Encrypt(DataType data, DataType ekey, DataType pkey);//加密函数
	DataType Decrypt(DataType data, DataType dkey, DataType pkey);//解密函数
	DataType GetPrime();//获取素数
	bool IsPrime(DataType data);//判断是否为素数
	DataType GetPKey(DataType prime1, DataType prime2);//获取公钥
	DataType GetOrla(DataType prime1, DataType prime2);//欧拉函数
	DataType GetEKey(DataType orla);//获取公钥e
	DataType GetDKey(DataType ekey, DataType orla);//获取私钥d
	DataType GetGcd(DataType data1, DataType data2);//获取两个数的最大公约数
	Key GetAllKey();//对外封装访问私有成员的接口

	void GetKeys();//获取全部的钥匙
	void Encrypt(const char* filename, const char* fileout);//文件加密
	void Decrypt(const char* filename, const char* fileout);//文件解密

};
