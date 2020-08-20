#include<cstdio>
#include<unistd.h>
#include<iostream>
#include<string>
#include<vector>
#include<unordered_map>
#include<fstream>
#include<zlib.h>
#include"httplib.h"
#include<pthread.h>
#include<boost/algorithm/string.hpp>
#include<boost/filesystem.hpp>
#define NOHOT_TIME 5 //非热点文件最后一次访问时间在10s以外
#define INTERVAL_TIME 30 //非热点文件的检测每隔30s检测一次
#define BACKUP_DIR "./backupfile/" //备份文件路径
#define GZFILLE_DIR  "./gzfile/" //文件压缩路径
#define DATA_FILE "./list_backup" //数据管理模块的数备份文件名称


namespace Cloud_Sys
{
    class FileTool//文件具类
    {
        private:
        public:
            static bool Read(const std::string& filename,std::string& body)//从文件读取所有内容
            {
                std::ifstream ifs(filename,std::ios::binary);

                if(ifs.is_open() == false)
                {
                    std::cout << "open Readfile " << filename << " failed\n";
                    return false;
                }
                //boost::filesysem::file_size(); 获取文件大小
                int64_t fsize = boost::filesystem::file_size(filename);
                body.resize(fsize);
                ifs.read(&body[0],fsize);
                if(ifs.good() == false)
                {
                    std::cout << "file " << filename << " read data failed\n";
                    return false;
                }
                ifs.close();
                return true;
            }
            static bool Write(const std::string& filename,const std::string& body)//向文件写入内容
            {
                std::ofstream ofs(filename,std::ios::binary);
                //ofstream打开文件默认会清空原有内容
                //当前策略是覆盖写入,无需更改
                if(ofs.is_open() == false)
                {
                    std:: cout << "open Writefile " << filename << " failed\n";
                    return false;
                }

                ofs.write(&body[0],body.size());
                if(ofs.good() == false)
                {
                    std::cout << "write file" << filename << " failed\n";
                    return false;
                }

                ofs.close();
                return true;
            }
    };


    class CompressTool//压缩工具类
    {
        private:
        public:
            static bool Compress(const std::string& file_src,const std::string& file_det)//文件压缩
            {
                std::string body;
                bool ret = FileTool::Read(file_src,body);
                {
                    if(ret == false)
                    {
                        return false;
                    }
                } 
                gzFile gf = gzopen(file_det.c_str(),"wb");
                if(gf == NULL)
                {
                    std::cout << "open src file " << file_det << " failed\n";
                    return false;
                }

                int len = 0;
                while(len < body.size())//若一次不能将所有数据压缩完，则下次在未压缩的位置继续压缩
                {
                    int ret = gzwrite(gf,&body[len],body.size()-len);
                    if(ret == 0)
                    {
                        std::cout << "file " << file_det <<" write compress data failed\n";
                        return false;
                    }
                    len += ret;
                }
                std::cout << file_src <<" is compressed to " << file_det <<std::endl;
                gzclose(gf);
                return true;
            }

            static bool DeCompress(const std::string& file_src,const std::string& file_det)//文件解压缩>
            {
                std::ofstream ofs(file_det,std::ios::binary);
                if(ofs.is_open() == false)
                {
                    std::cout << "open gzip file " << file_det << " failed\n";
                    return false;
                }

                gzFile gf = gzopen(file_src.c_str(),"rb");
                if(gf == NULL)
                {
                    std::cout << "open ungzip file " << file_src << " failed\n";
                    ofs.close();
                    return false;
                }

                char buff[4096] = {0};
                int ret ;
                while((ret = gzread(gf,buff,4096)) > 0)
                {
                    ofs.write(buff,ret);
                }
                std::cout << file_src <<" is decompressed to " << file_det <<std::endl;
                ofs.close();
                gzclose(gf);
            }
    };



    class DataManage//数据管理类
    {
        private:
            std::string m_back_file;//文件名称
            std::unordered_map<std::string,std::string> m_file_list;//数据管理容器
            pthread_rwlock_t m_rwlock;//读写锁

        public:

            DataManage(const std::string& file_path):
                m_back_file(file_path)
        {
            pthread_rwlock_init(&m_rwlock,NULL);

        }

            ~DataManage()
            {
                pthread_rwlock_destroy(&m_rwlock);
            }

            bool Exit(const std::string& filename)//判断文件是否存在
            {
                pthread_rwlock_rdlock(&m_rwlock);//加读锁    
                for(auto it = m_file_list.begin();it != m_file_list.end();++it)
                {
                    if(it->first == filename)//m_file_list的first存储的是源文件名称
                    {
                        pthread_rwlock_unlock(&m_rwlock);
                        return true;
                    }
                }

                pthread_rwlock_unlock(&m_rwlock);
                return false;
            }


            bool IsCompress(const std::string& filename)//判断文件是否已经压缩
            {
                pthread_rwlock_rdlock(&m_rwlock);//先加读锁    
                for(auto it = m_file_list.begin();it != m_file_list.end();++it)
                {
                    //first--是源文件名称，seconf--是压缩包名称，文件未压缩，源文件就和压缩包名称相同
                    if((it->first == filename) && (it->second != filename))
                    {
                        pthread_rwlock_unlock(&m_rwlock);
                        return true;

                    }
                }
                pthread_rwlock_unlock(&m_rwlock);
                return false;
            }


            bool GetNoncompressList(std::vector<std::string>* list)//获取未压缩文件列表
            {
                pthread_rwlock_rdlock(&m_rwlock);
                for(auto it = m_file_list.begin();it != m_file_list.end();++it)
                {
                    if(it->first == it->second)
                    {
                        list->push_back(it->first);
                    }
                }

                pthread_rwlock_unlock(&m_rwlock);
                return true;
            }

            bool GetGzName(const std::string&src,std::string& det)//通过源文件获取压缩包名称
            {
                pthread_rwlock_rdlock(&m_rwlock);
                auto it = m_file_list.find(src);
                if(it == m_file_list.end())
                {
                    return false;
                }
		pthread_rwlock_unlock(&m_rwlock);
                det = it->second;
                return true;
            }

            bool Insert(const std::string& file_src,const std::string& file_det)//插入或更新数据
            {
                pthread_rwlock_wrlock(&m_rwlock);
                m_file_list[file_src] = file_det;
                pthread_rwlock_unlock(&m_rwlock);
                Storage();//插入数据后存储文件
                return true;

            }

            bool GetAllFileName(std::vector<std::string>* list)//获取所有文件名称
            {
                pthread_rwlock_rdlock(&m_rwlock);
                for(auto it = m_file_list.begin();it != m_file_list.end();++it)
                {
                    list->push_back(it->first);
                }
                pthread_rwlock_unlock(&m_rwlock);
                return true;

            }


            bool Storage()//数据改变后持久化存储，将m_file_list中的数据持久化存储
            {
                pthread_rwlock_rdlock(&m_rwlock);
                std::stringstream tmp;
                for(auto it = m_file_list.begin();it != m_file_list.end();++it)
                {
                    //存储源文件名和压缩包名，以"间隔",与下一个文件以"\r\n"间隔
                    tmp << it->first << " "<< it-> second <<"\r\n";
                }
                pthread_rwlock_unlock(&m_rwlock);
                FileTool::Write(m_back_file,tmp.str());
                return true;

            }

            bool InitLoad()//启动时从持久化存储文件中加载数据
            {

                // 1.将这个备份文件的数据读取出来
                std::string body;
                if(FileTool::Read(m_back_file,body) == false)
                {
                    return false;
                }


                // 2.进行字符串处理，按照\r\n进行分割
                //boost::split(vector,src,sep,flag)
                //将src按照sep分割存放在vector中
                //flag --token_compress_off(vector不保存sep) token_compress_on(vector中保存sep)
                std::vector<std::string> list;
                boost::split(list,body,boost::is_any_of("\r\n"),boost::token_compress_off);

                // 3.每一行按照空格进行分割,前面是key,后面是val
                for(auto i : list)
                {
                    size_t pos = i.find(" ");
                    if(pos == std::string::npos)
                    {
                        continue;
                    }
                    const std::string key = i.substr(0,pos);
                    const std::string val = i.substr(pos+1);

                    // 4.将key-val添加到m_file_list中
                    Insert(key,val); 
                }
                return true;
            }
    };

};
Cloud_Sys::DataManage data_manage(DATA_FILE);

class NonHotPotCompress//非热点压缩类
{
    private:
        std::string m_bu_dir;//压缩前的文件路径
        std::string m_gz_dir;//压缩后的文件路径
        bool IsHotPotFile(const std::string& filename)//判断文件是否为热点文件
        {
            //非热点文件：当前时间 - 最后一次访问时间 > 基准时间 
            time_t cur = time(NULL);//获取当前时间的秒数
            struct stat st;
            if(stat(filename.c_str(),&st) < 0)
            {
                std::cout << "get file " <<filename << " stat failed!\n";
                return false;
            }

            if((cur - st.st_atime) > NOHOT_TIME)
            {
                return false;//非热点文件返回false
            }
            return true;

        }

    public:

        NonHotPotCompress(const std::string& bu_dir,const std::string& gz_dir):
            m_bu_dir(bu_dir),
            m_gz_dir(gz_dir)
    {}
        bool Start()//总体向外提供的功能接口,开始压缩模块
        {
            //不断循环判断有没有非热点文件
            while(1)
            {
			    std::vector<std::string> ist;
			        data_manage.GetAllFileName(&ist);  
				    for(auto& e:ist)                      
					        {                                   
							            std::cout << e << std::endl;   
								        }
                //1.获取未压缩文件列表
                std::vector<std::string> list;        
                data_manage.GetNoncompressList(&list);

                //2.逐个判断这个文件是否是非热点文件
                for(int i = 0; i < list.size();i++ )
                {
                    bool ret = IsHotPotFile(m_bu_dir + list[i]);
                    if(ret == false)
                    {
                        std::string s_filename = list[i];//不带路径的源文件名称
                        std::string d_filename = list[i] + ".gz";//不带路径的压缩包名称
                        std::string src_name = m_bu_dir + s_filename;
                        std::string det_name = m_gz_dir + d_filename;

                        //3.是非热点文件就压缩，并删除源文件
                        if(Cloud_Sys::CompressTool::Compress(src_name,det_name) == true)
                        {
                            data_manage.Insert(s_filename,d_filename);//更新数据
                            unlink(src_name.c_str());//删除源文件
                            std::cout << s_filename << " is compressed\n";
                        }
                        else
                        {
                            std::cout << src_name << "file compress failed\n";
                        }
                    }
                }

                //4.休眠一会
                sleep(INTERVAL_TIME);
            }
            return true;
        }
};

class Server//服务器类
{
    private:
        std::string m_file_dir;//上传文件备份路径
        httplib::Server m_server;//实例化httplib库提供的Server对象,便于实现http协议
        static void FileUpLoad(const httplib::Request& req,httplib::Response& rsp)//文件上传处理回调函数
        {
            std::string filename = req.matches[1];//matches[1]为正则表达式后面捕捉道德字符串
            std::string pathname = BACKUP_DIR + filename;//文件备份在指定路径下
            Cloud_Sys::FileTool::Write(pathname,req.body);//向文件写入数据
            data_manage.Insert(filename,filename);//添加文件信息到数据管理模块
            std::cout << filename <<" Uploaded\n";
            rsp.status = 200;//上传成功状态码为200
            return ;
        }
        static void FileList(const httplib::Request& req,httplib::Response& rsp)//文件列表处理回调函数
        {
            std::vector<std::string> list;
            data_manage.GetAllFileName(&list);//获取文件管理模块中的所有文件信息
            std::stringstream tmp;
            tmp<<"<html><body><hr />";
            for(int i = 0;i < list.size();i++)
            {
                tmp<< "<a href='download/" << list[i] <<"'>"<<list[i] << "</a>";
                tmp << "<hr />";
            }
            tmp << "</body></html>";
            rsp.set_content(tmp.str().c_str(),tmp.str().size(),"text/html");
            rsp.status = 200;
            return;
        }
        static void FileDownLoad(const httplib::Request& req,httplib::Response& rsp)//文件下载处理回调函数
        {
            //1.判断文件是否存在
            std::string filename = req.matches[1];//这就是前面捕捉到的(.*)
            if(data_manage.Exit(filename) == false)
            {
                rsp.status = 404;//访问文件不存在
                return;
            }

            //2.判断文件是否压缩
            std::string pathname = BACKUP_DIR + filename;
            if(data_manage.IsCompress(filename))
            {
                //文件被压缩，解压它
                std::string gzfile; 
                data_manage.GetGzName(filename,gzfile);//获取压缩包名称
                std::string gzpathname = GZFILLE_DIR + gzfile;//压缩包路径
                Cloud_Sys::CompressTool::DeCompress(gzpathname,pathname);//将压缩包解压
                data_manage.Insert(filename,filename);//删除压缩包后更新数据管理模块
                unlink(gzpathname.c_str());//删除压缩包
            }
            //3.从文件读取数据,响应给客户端
            Cloud_Sys::FileTool::Read(pathname,rsp.body);
            std::vector<std::string> Content_Type = { "application/octet-stream", //二进制流数据（如常见的文件下载）
                                            "application/pdf",          //pdf格式
                                            "application/msword",       //word格式
                                            "image/jpeg",               //jpg个数
                                            "image/png",                //png格式
                                            "image/gif"};               //gif格式
            size_t pos = filename.find(".");
            std::string Type(filename.begin() + pos + 1,filename.end());//获取待下载文件格式
	    std::cout << Type <<std::endl;
	    if(Type == "cpp" || Type == "txt" || Type == "h" || Type == "hpp")
            {
                Type = Content_Type[0];
            }
            else if(Type == "pdf")
            {
                Type = Content_Type[1];
            }
            else if(Type == "docx" || Type == "doc")
            {
                Type = Content_Type[2];
            }
            else if(Type == "jpg")
            {
                Type = Content_Type[3];
            }
            else if(Type == "png")
            {
                Type = Content_Type[4];
            }
            else if(Type == "gif")
            {
                Type = Content_Type[5];
            }

            rsp.set_header("Content-Type",Type);//二进制下载流
            rsp.status = 200;
        }
    public:
        bool Start()
        {
            m_server.Put("/(.*)",FileUpLoad);
            m_server.Get("/list",FileList);
            m_server.Get("/download/(.*)",FileDownLoad);
            m_server.listen("0.0.0.0",9000);
            return true;

        }

};
