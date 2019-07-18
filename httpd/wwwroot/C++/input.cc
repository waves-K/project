#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>

static unsigned char FromHex(unsigned char x)
{
    unsigned char y;
    if(x >= 'A' && x <= 'Z'){
        y = x - 'A' + 10;
    }
    else if(x >= 'a' && x <= 'z'){
        y = x - 'a' + 10;
    }
    else if(x >= '0' && x <= '9'){
        y = x - '0';
    }
    else
        assert(0);

    return y;
}

static std::string urlDecode(std::string& str)
{
	std::string strtmp = "";
    size_t length = str.length();
    size_t i = 0;
    for(; i < length; i++){
        if(str[i] == '+')
            strtmp += ' ';
        else if(str[i] == '%'){
            assert(i+2 < length);
            unsigned char high = FromHex((unsigned char)str[++i]);
            unsigned char low = FromHex((unsigned char)str[++i]);
            strtmp += high*16 + low;
        }
        else{
            strtmp += str[i];
        }
    }
    return strtmp;
}

int main()
{
    char arg[1024] = {0};
    if(getenv("METHOD")){
        if(strcasecmp(getenv("METHOD"), "GET") == 0){
            strcpy(arg, getenv("QUERY_STRING"));
        }else{
            char c;
            int i = 0;
            int len = atoi(getenv("CONTENT_LENGTH"));
            for(; i < len; i++){
                read(0, &c, 1);
                arg[i] = c;
            }
            arg[i] = 0;
        }
    }
    
	std::string code_ = arg;
	code_ = urlDecode(code_);
	code_ = code_.substr(5);

//	std::cout << code_.c_str() << std::endl;

	const char *path_ = "code_online_cc.cc";

	umask(0);
	int fd = open(path_, O_RDWR | O_CREAT, 0664);
	write(fd, code_.c_str(), strlen(code_.c_str()));
	close(fd);

	system("g++ code_online_cc.cc -o code_cc");
	system("rm code_online_cc.cc");
	const char *exe = "code_cc";
	execl(exe, exe, NULL);
}
