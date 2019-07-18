#include "Log.hpp"
#include "pthreadPoll.hpp"

///////////////////////////////////////////////////////////////
void usage(const char* );
int startup(int );
int getLine(int, char*, int);
void clearHeaer(int );
void show_400(int );
void show_404(int );
void show_500(int );
void echoErrMsg(int , int );
int exec_cgi(int , char*, char*, char* );
int echo_www(int , char*, int );
int handlerRequest(int );
///////////////////////////////////////////////////////////////

void usage(const char *proc)
{
	printf("Usage %s [port]\n", proc);
}

int startup(int port)
{
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0){
		perror("socket");
		exit(2);
	}

	int opt = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	struct sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_port = htons(port);
	local.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(sock, (struct sockaddr*)&local, sizeof(local)) < 0){
		perror("bind");
		exit(3);
	}

	if(listen(sock, 5) < 0){
		perror("listen");
		exit(4);
	}

	return sock;
}

int getLine(int sock, char line[], int len)
{
	char c = '\0';
	int i = 0;
	while(c != '\n' && i < len-1){
		recv(sock, &c, 1, 0);
		if(c == '\r'){
			recv(sock, &c, 1, MSG_PEEK); //窥探功能
			if(c == '\n'){
				recv(sock, &c, 1, 0); //覆盖 \r 为 \n
			}else{
				c = '\n';
			}
		}
		//\r\n \r->\n
		line[i++] = c;
	}
	line[i] = '\0';

	return i;
}

void clearHeaer(int sock) //清理头部
{
	char line[MAX];
	do{
		getLine(sock, line, sizeof(line));
	}while(strcmp("\n", line));
}

void show_400(int sock) 
{
	char line[MAX];
	struct stat st;
	sprintf(line, "HTTP/1.0 400 Bad Request\r\n");
	send(sock, line, strlen(line), 0);
	sprintf(line, "Content-Type: text/html;charset=utf-8\r\n");
	send(sock, line, strlen(line), 0);
	sprintf(line, "\r\n");
	send(sock, line, strlen(line), 0);

	int fd = open(PAGE_400, O_RDONLY);

	stat(PAGE_400, &st);
	sendfile(sock, fd, NULL, st.st_size);
	close(fd);
}

void show_404(int sock)
{
	char line[MAX];
	struct stat st;
	sprintf(line, "HTTP/1.0 404 Not Found\r\n");
	send(sock, line, strlen(line), 0);
	sprintf(line, "Content-Type: text/html;charset=uft-8\r\n");
	send(sock, line, strlen(line), 0);
	sprintf(line, "\r\n");
	send(sock, line, strlen(line), 0);

	int fd = open(PAGE_404, O_RDONLY);

	stat(PAGE_404, &st);
	sendfile(sock, fd, NULL, st.st_size);
	close(fd);
}

void show_500(int sock) 
{
	char line[MAX];
	struct stat st;
	sprintf(line, "HTTP/1.0 500 Internal Server Error\r\n");
	send(sock, line, strlen(line), 0);
	sprintf(line, "Content-Type: text/html;charset=utf-8\r\n");
	send(sock, line, strlen(line), 0);
	sprintf(line, "\r\n");
	send(sock, line, strlen(line), 0);

	int fd = open(PAGE_500, O_RDONLY);

	stat(PAGE_500, &st);
	sendfile(sock, fd, NULL, st.st_size);
	close(fd);
}

void echoErrMsg(int sock, int status_code)
{
	switch(status_code){
		case 400:
			show_400(sock);
			break;
		case 404:
			show_404(sock);
			break;
		case 500:
			show_500(sock);
			break;
		default:
			break;
	}
}

int exec_cgi(int sock, char *method, char *path, char *query_string)
{
	char line[MAX];
	int content_length = -1;
	char method_env[MAX/32];
	char query_string_env[MAX];
	char content_length_env[MAX/8];

	if(strcasecmp(method, "GET") == 0){
		clearHeaer(sock); //清理头部
	}
	else{ //POST
		do{
			getLine(sock, line, sizeof(line));
			//Content-Length: xxx
			if(strncasecmp(line, "Content-Length: ", 16) == 0){
				content_length = atoi(line + 16);
			}
		}while(strcmp("\n", line));
		if(content_length == -1){
			return 400;
		}
	}

	int input[2];
	int output[2];

	//管道站在子进程角度
	pipe(input);
	pipe(output);

	pid_t id = fork();
	if(id < 0){
		LOG(ERROR, "fork error");
		return 500;
	}
	else if(id == 0){
		close(input[1]);
		close(output[0]);

		dup2(input[0], 0);
		dup2(output[1], 1);
		dup2(output[1], 2);

		sprintf(method_env, "METHOD=%s", method);
		putenv(method_env);
		if(strcasecmp(method, "GET") == 0){
			sprintf(query_string_env, "QUERY_STRING=%s", query_string);
			putenv(query_string_env);
		}
		else{
			sprintf(content_length_env, "CONTENT_LENGTH=%d", content_length);
			putenv(content_length_env);
		}

		execl(path, path, NULL);
		exit(1);
	}
	else{
		close(input[0]);
		close(output[1]);

		sprintf(line, "HTTP/1.0 200 OK\r\n");
		send(sock, line, strlen(line), 0);
		sprintf(line, "Content-type: text/html;charset=utf-8\r\n");
		send(sock, line, strlen(line), 0);
		sprintf(line, "\r\n");
		send(sock, line, strlen(line), 0);

		int i = 0;
		char c;
		if(strcasecmp(method, "POST") == 0){
			for(; i < content_length; i++){
				recv(sock, &c, 1, 0);
				write(input[1], &c, 1);
			}
		}

		while(read(output[0], &c, 1) > 0){
			send(sock, &c, 1, 0);
		}

		waitpid(id, NULL, 0);
		close(input[1]);
		close(output[0]);
	}

	return 200;
}

int echo_www(int sock, char *path, int size)
{
	char line[MAX];
	clearHeaer(sock);

	int fd = open(path, O_RDONLY);
	if(fd < 0){
		return 404;
	}

	sprintf(line, "HTTP/1.0 200 OK\r\n");
	send(sock, line, strlen(line), 0);
	
	if(strcasecmp("html", path+strlen(path)-4) == 0){
	    sprintf(line, "Content-type: text/html;charset=utf-8\r\n");
    } else if (strcasecmp("js", path+strlen(path)-2) == 0){
	    sprintf(line, "Content-type: application/x-javascript;charset=utf-8\r\n");
	} else if (strcasecmp("css", path+strlen(path)-3) == 0){
	    sprintf(line, "Content-type: text/css;charset=utf-8\r\n");
	}
	send(sock, line, strlen(line), 0);
	
	sprintf(line, "\r\n");
	send(sock, line, strlen(line), 0);

	//发送正文,文件内容  sendfile, 拷贝两个文件描述符的内容
	sendfile(sock, fd, NULL, size);

	close(fd);

	return 200;
}

int handlerRequest(int sock) //请求行
{
	int status_code = 200;
	char line[MAX];
	char method[MAX/16] = {0};
	char url[MAX] = {0};
	char path[MAX];
	int cgi = 0;
	char *query_string = NULL;

	getLine(sock, line, sizeof(line));
	//printf("%s", line);

	LOG(INFO, "Start parsing the first line");

	size_t i = 0;
	size_t j = 0;
	while(i < sizeof(method)-1 && j < sizeof(line) && \
          !isspace(line[j])){
		method[i] = line[j];
        i++, j++;
	}
	method[i] = '\0';

	while(j < sizeof(line) && isspace(line[j])){
		j++;
	}

	i = 0;
	while(i < sizeof(url)-1 && j < sizeof(line) && !isspace(line[j])){
		url[i] = line[j];
        i++, j++;
	}
	url[i] = '\0'; 
	//printf("method: %s, url: %s\n", method, url);
	LOG(INFO, "frst line parse done");

	//忽略大小写比较
	//有传参,模式改为cgi
	//get 有传参,是通过url传参
	//post 传参是将参数填充至报文的正文部分, 传敏感数据时使用
	if(strcasecmp(method, "GET") == 0){
	}
	else if(strcasecmp(method, "POST") == 0){
		cgi = 1;
	}
	else{ //method error
		status_code = 400;
		clearHeaer(sock); //清理头部
		LOG(WARNING, "method error");
		goto end;
	}
	LOG(INFO, "method parse done");

	if(strcasecmp(method, "GET") == 0){
		query_string = url;
		while(*query_string){
			if(*query_string == '?'){
				cgi = 1; //传参设置
				*query_string = '\0';
				query_string++;
				break;
			}
			query_string++;
		}
	}

	//   [wwwroot]/a/b/c\0x=100&y=200
	sprintf(path, "wwwroot%s", url); //格式化输出到path
	if(path[strlen(path)-1] == '/'){
		strcat(path, HOME_PAGE); //给出默认首页
	}

	struct stat st;
	//判断访问资源是否存在, 函数stat
	if(stat(path, &st) < 0){
		status_code = 404;
		clearHeaer(sock);
		LOG(WARNING, "path not found");
		goto end;
	}
	else{
		//   [wwwroot]/a/b/c \0 x=100&y=200
		if(S_ISDIR(st.st_mode)){
			strcat(path, "/");
			strcat(path, HOME_PAGE);
		}else if((st.st_mode&S_IXUSR) || (st.st_mode&S_IXGRP) || (st.st_mode&S_IXOTH)){
			cgi = 1;
		}

		//method, path, cgi, get->query_string
		LOG(INFO, "starting responce");
		if(cgi == 1){
			status_code = exec_cgi(sock, method, path, query_string);
		}else{
			status_code = echo_www(sock, path, st.st_size); //响应资源, 此处是get方法
		}
	}
	

end:
	//status_code 204:请求资源存在,但是为空(成功)
	//			  206:局部请求
	//			  301:永久性重定向
	//			  302:临时性重定向
	//			  400:请求报文中存在语法错误
	//			  403:请求资源被服务器拒绝
	//			  404:请求资源不存在(error)
	//			  500:服务器执行发生错误
	//			  503:服务器处于超负载或停机维护状态
	if(status_code != 200){
		echoErrMsg(sock, status_code);
	}
	LOG(INFO, "close sock");
	close(sock);
}

int main(int argc, char *argv[])
{
	if(argc != 2){
		usage(argv[0]);
		return 1;
	}

	signal(SIGPIPE, SIG_IGN);

	LOG(INFO, "Start server");
	int listen_sock = startup(atoi(argv[1]));


	for( ; ; ){
		struct sockaddr_in client;
		socklen_t len = sizeof(client);
		int sock = accept(listen_sock, (struct sockaddr*)&client, &len);
		if(sock < 0){
			perror("accept");
			continue;
		}

		LOG(INFO, "get a new link, handlerRequest...");

		Task t_;
		threadPoll *tp_ = new threadPoll();
		tp_->initPthread();
		t_.setTask(sock, handlerRequest);
		tp_->pushTask(t_);
	}
}
