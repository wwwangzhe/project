#include<thread>
#include"Backup.hpp"
void compress_test(char* argv[])
{
    //argv[1]---源文件名称
    //argv[2]---压缩包名称
    Cloud_Sys::CompressTool::Compress(argv[1],argv[2]);
    std::string file = argv[2];
    file += ".txt";
    Cloud_Sys::CompressTool::DeCompress(argv[2],file.c_str());
}

void DataManage_test()
{
    data_manage.Insert("a.txt","a.txt");
    data_manage.Insert("b.txt","b.txt.gz");
    data_manage.Insert("c.txt","c.txt");
    data_manage.Insert("d.txt","d.txt.gz");
    data_manage.Insert("e.txt","e.txt.gz");
    data_manage.Storage();
}

void DataManage_test2()
{
    data_manage.InitLoad();
    std::vector<std::string> list;
    data_manage.GetAllFileName(&list);
    //获取所有信息
    printf("----所有文件----\n");
    for(auto i : list)
    {
        printf("%s\n",i.c_str());
    }

    //获取未压缩文件
    list.clear();
    data_manage.GetNoncompressList(&list);
    printf("----未压缩文件----\n");
    for(auto i: list)
    {
        printf("%s\n",i.c_str());
    }

}

void Thr_NonHotPotFile_Monitor_Compress()//非热点文件检测压缩模块线程
{
    NonHotPotCompress ncom(BACKUP_DIR,GZFILLE_DIR);
    ncom.Start();
    return ;
}

void Thr_Http_Server()//网络服务端模块线程
{
    Server server;
    server.Start();
    return;
}

int main(int argc, char* argv[])
{

    //compress_test(argv);//文件压缩模块测试
    //DataManage_test(); //数据管理模块插入数据测试
    // DataManage_test2();//数据管理模块初始化，获取文件信息测试
    
    //源文件目录和压缩文件目录的创建
    if(boost::filesystem::exists(GZFILLE_DIR) == false)
    {
        boost::filesystem::create_directory(GZFILLE_DIR);
    }

    if(boost::filesystem::exists(BACKUP_DIR) == false)
    {
        boost::filesystem::create_directory(BACKUP_DIR);
    }

    data_manage.InitLoad();//初始化数据管理模块
    std::vector<std::string> list;
    data_manage.GetAllFileName(&list);
    for(auto& e:list)
    {
	    std::cout << e << std::endl;
    }
    //data_manage.Insert("Test","Test");
    std::thread thr_HotPot(Thr_NonHotPotFile_Monitor_Compress);//非热点文件监测线程
    std::thread thr_server(Thr_Http_Server);//网络服务器线程
    thr_HotPot.join();
    thr_server.join();
    return 0;
}#include<thread>
#include"Backup.hpp"
void compress_test(char* argv[])
{
    //argv[1]---源文件名称
    //argv[2]---压缩包名称
    Cloud_Sys::CompressTool::Compress(argv[1],argv[2]);
    std::string file = argv[2];
    file += ".txt";
    Cloud_Sys::CompressTool::DeCompress(argv[2],file.c_str());
}

void DataManage_test()
{
    data_manage.Insert("a.txt","a.txt");
    data_manage.Insert("b.txt","b.txt.gz");
    data_manage.Insert("c.txt","c.txt");
    data_manage.Insert("d.txt","d.txt.gz");
    data_manage.Insert("e.txt","e.txt.gz");
    data_manage.Storage();
}

void DataManage_test2()
{
    data_manage.InitLoad();
    std::vector<std::string> list;
    data_manage.GetAllFileName(&list);
    //获取所有信息
    printf("----所有文件----\n");
    for(auto i : list)
    {
        printf("%s\n",i.c_str());
    }

    //获取未压缩文件
    list.clear();
    data_manage.GetNoncompressList(&list);
    printf("----未压缩文件----\n");
    for(auto i: list)
    {
        printf("%s\n",i.c_str());
    }

}

void Thr_NonHotPotFile_Monitor_Compress()//非热点文件检测压缩模块线程
{
    NonHotPotCompress ncom(BACKUP_DIR,GZFILLE_DIR);
    ncom.Start();
    return ;
}

void Thr_Http_Server()//网络服务端模块线程
{
    Server server;
    server.Start();
    return;
}

int main(int argc, char* argv[])
{

    //compress_test(argv);//文件压缩模块测试
    //DataManage_test(); //数据管理模块插入数据测试
    // DataManage_test2();//数据管理模块初始化，获取文件信息测试
    
    //源文件目录和压缩文件目录的创建
    if(boost::filesystem::exists(GZFILLE_DIR) == false)
    {
        boost::filesystem::create_directory(GZFILLE_DIR);
    }

    if(boost::filesystem::exists(BACKUP_DIR) == false)
    {
        boost::filesystem::create_directory(BACKUP_DIR);
    }

    data_manage.InitLoad();//初始化数据管理模块
    std::vector<std::string> list;
    data_manage.GetAllFileName(&list);
    for(auto& e:list)
    {
	    std::cout << e << std::endl;
    }
    //data_manage.Insert("Test","Test");
    std::thread thr_HotPot(Thr_NonHotPotFile_Monitor_Compress);//非热点文件监测线程
    std::thread thr_server(Thr_Http_Server);//网络服务器线程
    thr_HotPot.join();
    thr_server.join();
    return 0;
}
