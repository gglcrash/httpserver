#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#define BUF_SIZE 100

void parseFileName(char *line, char **filepath, size_t *len) {
	char *begin = NULL;
	while ((*line) != '/') line++;
	begin = line + 1;
	while ((*line) != ' ') line++;
	(*len) = line - begin;
	*filepath = (char*)malloc(*len + 1);
	*filepath = strncpy(*filepath, begin, *len);
	(*filepath)[*len] = '\0';
	printf("%s \n", *filepath);
}

void headers(int client, int size, int httpcode) {
	char buf[1024];
	char strsize[20];
	sprintf(strsize, "%d", size);
	if (httpcode == 200) {
		strcpy(buf, "HTTP/1.0 200 OK\r\n");
	}
	else if (httpcode == 404) {
		strcpy(buf, "HTTP/1.0 404 Not Found\r\n");
	}
	else {
		strcpy(buf, "HTTP/1.0 500 Internal Server Error\r\n");
	}
	send(client, buf, strlen(buf), 0);
	strcpy(buf, "Connection: keep-alive\r\n");
	send(client, buf, strlen(buf), 0);
        sprintf(buf, "Content-length: %s\r\n", strsize);
	send(client, buf, strlen(buf), 0);
	strcpy(buf, strsize);
	strcpy(buf, "Server: simple-server\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "Content-Type: text/html; charset=UTF-8\r\n");
	send(client, buf, strlen(buf), 0);
	strcpy(buf, "\r\n");
	send(client, buf, strlen(buf), 0);
}

int main() {
	int pid;
	int count = 0;
	int mysock = 0;
	int result = 0;
	int cd = 0;
	int filesize = 0;
	const int backlog = 10;
	struct sockaddr_in saddres;
	struct sockaddr_in caddres;
	char *line = NULL;
        char *buf = NULL;
	size_t len = 0;
	char *filepath = NULL;
	size_t filepath_len = 0;
	int empty_str_count = 0;
	socklen_t size_caddres;
	FILE *fd;
	FILE *file;

	mysock = socket(AF_INET, SOCK_STREAM, 0);
	if (mysock == -1) {
		printf("listener create error \n");
	}
	
	saddres.sin_family = AF_INET;
	saddres.sin_port = htons(8080);
	saddres.sin_addr.s_addr = INADDR_ANY;
	
	result = bind(mysock, (struct sockaddr *)&saddres, sizeof(saddres));
	if (result == -1) {
		printf("bind error \n");
	}
	
	result = listen(mysock, backlog);
	if (result == -1) {
		printf("listener error \n");
	}
	while (1) {
		cd = accept(mysock, (struct sockaddr *)&caddres, &size_caddres);
		if (cd == -1) {
			printf("accept error \n");
		}
		pid = fork();
		if(pid == -1)
		{
			printf("fork error \n"); 
		}
		if(pid==0)
		{
		fd = fdopen(cd, "r");
		if (fd == NULL) {
			printf("error open file\n");
		}
		while ((result = getline(&line, &len, fd)) != -1) {
			if (strstr(line, "GET")) {
				parseFileName(line, &filepath, &filepath_len);
			}
			if (strcmp(line, "\r\n") == 0) {
				empty_str_count++;
			}
			else {
				empty_str_count = 0;
			}
			if (empty_str_count == 1) {
				break;
			}
			printf("%s", line);
		}
		printf("open %s \n", filepath);
		file = fopen(filepath, "rb");
		if (file == NULL) {
			printf("File Not Found \n");
			headers(cd, 0,404);
		}
		else {
			fseek(file, 0L, SEEK_END);
			filesize = ftell(file);
			fseek(file, 0L, SEEK_SET);
			headers(cd, filesize,200);
                        while (!feof(file)) {
                          len = fread(line, 1, 1024, file);
                          result = send(cd, line, len, 0);
                          if (result == -1) {
                            printf("send error\n");
                          }
			}
		      }
		close(cd);
		}
		else
		{
		  close(cd);
		}
	}
	return 0;
}
