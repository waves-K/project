#include "comm.h"

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

static string urlDecode(string& str)
{
    string strtmp = "";
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
    
    //arg
    //name=wangwu&sex=man&hobby=coding
    //const string name = "张三";
    //const string sex = "12131231@qq.com";
    //const string hobby = "1232143423";

    strtok(arg, "=&");
    string name = strtok(NULL, "=&");
    name = urlDecode(name);
    strtok(NULL, "=&");
    string sex = strtok(NULL, "=&");
    sex = urlDecode(sex);
    strtok(NULL, "=&");
    string hobby = strtok(NULL, "=&");
    hobby = urlDecode(hobby);


    MYSQL *myfd = connectMysql();
    insertmysql(myfd, name, sex, hobby);
    closeMysql(myfd);
}
