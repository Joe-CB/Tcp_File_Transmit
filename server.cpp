// Dll.cpp : 定义 DLL 的导出函数。
//
#include "pch.h"
#include "framework.h"
#include "Dll.h"
#include <iostream>
#include <string>
#include <fstream>
#include <strstream>
#include <sstream>
#include <streambuf>
#include <stdio.h>
#include <winsock2.h>
#include <stdio.h>
#include <WinSock2.h>
#include <windows.h>
#include<io.h>
#include <vector>
#include <sys/stat.h>
#include <algorithm>
#include <direct.h>
#include <stdlib.h>
#include <filesystem>


#pragma comment (lib, "ws2_32.lib")  //加载 ws2_32.dll
#define BUF_SIZE 2048


namespace {


	std::string read_file(std::string filename) {
		std::ifstream t(filename);
		return std::string((std::istreambuf_iterator<char>(t)),
			std::istreambuf_iterator<char>());
	}

	int load_file(char **buffer, std::string filename) {
		std::ifstream t;
		int length = 0;
		t.open(filename);      // open input file
		t.seekg(0, std::ios::end);    // go to the end
		length = t.tellg();           // report location (this is the length)
		t.seekg(0, std::ios::beg);    // go back to the beginning
		*buffer = new char[length];    // allocate memory for a buffer of appropriate dimension
		t.read(*buffer, length);       // read the whole file into the buffer
		t.close();                    // close file handle
		return length;
	}


	bool send_file(SOCKET client_socket, std::string filename, char buffer[]) {
		//先检查文件是否存在
		FILE *fp = fopen(filename.c_str(), "rb");  //以二进制方式打开文件
		if (fp == NULL) {
			printf("Cannot open file, press any key to exit!\n");
			return false;
		}
		else {
			printf("Open file success.\n");
		}

		//循环发送数据，直到文件结尾
		int nCount;
		while ((nCount = fread(buffer, 1, BUF_SIZE, fp)) > 0) {
			send(client_socket, buffer, nCount, 0);
		}
		shutdown(client_socket, SD_SEND);  //文件读取完毕，断开输出流，向客户端发送FIN包
		fclose(fp);
		return true;
	}

	namespace {
		using namespace std;
		vector<string> getFiles(string cate_dir)
		{
			vector<string> files;//存放文件名
#ifdef WIN32
			_finddata_t file;
			long lf;
			//输入文件夹路径
			if ((lf = _findfirst(cate_dir.c_str(), &file)) == -1) {
				cout << cate_dir << " not found!!!" << endl;
			}
			else {
				while (_findnext(lf, &file) == 0) {
					//输出文件名
					//cout<<file.name<<endl;
					if (strcmp(file.name, ".") == 0 || strcmp(file.name, "..") == 0)
						continue;
					files.push_back(file.name);
				}
			}
			_findclose(lf);
#endif
#ifdef linux
			DIR *dir;
			struct dirent *ptr;
			char base[1000];
			if ((dir = opendir(cate_dir.c_str())) == NULL)
			{
				perror("Open dir error...");
				exit(1);
			}
			while ((ptr = readdir(dir)) != NULL)
			{
				if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) ///current dir OR parrent dir
					continue;
				else if (ptr->d_type == 8) ///file
				//printf("d_name:%s/%s\n",basePath,ptr->d_name);
					files.push_back(ptr->d_name);
				else if (ptr->d_type == 10) ///link file
				//printf("d_name:%s/%s\n",basePath,ptr->d_name);
					continue;
				else if (ptr->d_type == 4) ///dir
				{
					files.push_back(ptr->d_name);
					/*
					memset(base,'\0',sizeof(base));
					strcpy(base,basePath);
					strcat(base,"/");
					strcat(base,ptr->d_nSame);
					readFileList(base);
					*/
				}
			}
			closedir(dir);
#endif
			//排序，按从小到大排序
			std::sort(files.begin(), files.end());
			return files;
		}

		namespace fs = std::filesystem;
		vector<string> get_all_file(string cate_dir) {
			std::vector<std::string> files;
			for (const auto &entry : fs::directory_iterator(cate_dir)) {
				files.push_back(entry.path().string());
			}
			return files;
		}

		void get_all_files(string dir, std::vector<string> &files) {
			struct stat s;
			if (stat(dir.c_str(), &s) == 0) {
				if (s.st_mode & S_IFDIR) {
					// 路径返回所有文件
					for (const auto &entry : fs::directory_iterator(dir)) {
						get_all_files(entry.path().string(), files);
					}
				}
				else if (s.st_mode & S_IFREG) {
					files.push_back(dir);

				}
				else {
				}
			}
			else {
			}

			
		}

		void getFiles(string path, vector<string>& files)
		{
			//文件句柄 
			long   hFile = 0;
			//文件信息 
			struct _finddata_t fileinfo;
			string p;
			if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
			{
				do
				{
					//如果是目录,迭代之 
					//如果不是,加入列表 
					if ((fileinfo.attrib &  _A_SUBDIR))
					{
						if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
							getFiles(p.assign(path).append("\\").append(fileinfo.name), files);
					}
					else
					{
						files.push_back(p.assign(path).append("\\").append(fileinfo.name));
					}
				} while (_findnext(hFile, &fileinfo) == 0);
				_findclose(hFile);
			}
		}
	}


	// 遍历所有的文件夹，并保存到文件
	void directory_visitor(std::string dir, std::string output) {
		std::vector<std::string> files;// = get_all_file(dir);
		get_all_files(dir, files);
		std::cout << "Find file size: " << files.size() << std::endl;

		std::stringstream ss;
		int counter = 0;
		for (auto &file : files) {
			//ss << counter++ << "  ";
			ss << file;
			ss << '\n';
		}
		std::string temp = ss.str();
		std::ofstream file(output);
		file << ss.str();
		file.close();
		return;
	}



	int begin_serve() {
		WSADATA wsaData;
		WSAStartup(MAKEWORD(2, 2), &wsaData);
		//创建套接字
		SOCKET servSock = socket(AF_INET, SOCK_STREAM, 0);
		//绑定套接字
		sockaddr_in sockAddr;
		memset(&sockAddr, 0, sizeof(sockAddr));  //每个字节都用0填充
		sockAddr.sin_family = PF_INET;  //使用IPv4地址
		sockAddr.sin_addr.s_addr = inet_addr("192.168.31.89");  //具体的IP地址
		sockAddr.sin_port = htons(1080);  //端口
		bind(servSock, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR));
		//进入监听状态
		listen(servSock, 20);
		//接收客户端请求
		SOCKADDR clntAddr;
		int nSize = sizeof(SOCKADDR);
		char buffer[BUF_SIZE] = { 0 };  //缓冲区
		std::cout << "开始监听：" << std::endl;
		while (1) {



			while (true) {
				SOCKET clntSock = accept(servSock, (SOCKADDR*)&clntAddr, &nSize);

				// 等待客户端发来请求
				int strLen = recv(clntSock, buffer, BUF_SIZE, 0);  //接收客户端发来的数据
				std::string received = buffer;
				std::cout << "Received :" << received << std::endl;
				struct stat s;
				if (stat(received.c_str(), &s) == 0) {
					if (s.st_mode & S_IFDIR) {
						// 路径返回所有文件
						std::cout << "Client requrest a directory" << buffer << std::endl;
						directory_visitor(received, "cache_filelist.txt");
						send_file(clntSock, "cache_filelist.txt", buffer);
						recv(clntSock, buffer, BUF_SIZE, 0);  //阻塞，等待客户端接收完毕

					}
					else if (s.st_mode & S_IFREG) {
						std::cout << "Client requrest a file" << std::endl;
						send_file(clntSock, received, buffer);
						recv(clntSock, buffer, BUF_SIZE, 0);  //阻塞，等待客户端接收完毕

					}
					else {
						std::cout << "not file not directory" << std::endl;
					}
				}
				else {
					std::cout << "error, doesn't exist" << std::endl;
				}
				memset(buffer, 0, BUF_SIZE);  //重置缓冲区
				closesocket(clntSock);  //关闭套接字
			}
		}
		//关闭套接字
		closesocket(servSock);
		//终止 DLL 的使用
		WSACleanup();
		return 0;
	}

}



// 这是导出变量的一个示例
DLL_API int nDll=0;

// 这是导出函数的一个示例。
DLL_API int test_fun(void)
{
	begin_serve();
    return 0;
}

// 这是已导出类的构造函数。
CDll::CDll()
{
    return;
}
