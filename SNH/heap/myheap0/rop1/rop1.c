#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <string.h>

#ifndef PORT
#define PORT 10000
#endif

extern char *gets(char *);

#define BUFSZ 1024
#define MSGSZ 32

void catfile(char *filename)
{
	FILE *f;
	char buf[BUFSZ];

	printf("%s:\n", filename);
	f = fopen(filename, "r");
	if (f == NULL) {
		perror(filename);
		exit(1);
	}
	while (fgets(buf, BUFSZ, f)) {
		printf("%s", buf);
	}
}

void child()
{
	char buf[32];
	puts("Welcome to rop1\n");
	puts("Please tell me you name\n");
	gets(buf);
	printf("Dear %s, you won't be able to read flag.txt", buf);
	puts("\nread this instead:\n");
	catfile("boringfile.txt");
}

int main()
{
	int lstn;
	int enable;
	struct sockaddr_in lstn_addr;

	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

	lstn = socket(AF_INET, SOCK_STREAM, 0);
	if (lstn < 0) {
		perror("socket");
		return 1;
	}
	enable = 1;
	if (setsockopt(lstn, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0) {
		perror("setsockopt");
		return 1;
	}
	bzero(&lstn_addr, sizeof(lstn_addr));

	lstn_addr.sin_family = AF_INET;
	lstn_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	lstn_addr.sin_port = htons(PORT);

	if (bind(lstn, (struct sockaddr *)&lstn_addr, sizeof(lstn_addr)) < 0) {
		perror("bind");
		return 1;
	}

	if (listen(lstn, 10) < 0) {
		perror("listen");
		return 1;
	}
	printf("Listening on port %d\n", PORT);

	signal(SIGCHLD, SIG_IGN);

	for (;;) {
		int con = accept(lstn, NULL, NULL);
		if (con < 0) {
			perror("accept");
			return 1;
		}

		switch (fork()) {
		case -1:
			perror("fork");
			return 1;
		case 0:
			printf("New connection, child %d\n", getpid());

			close(0);
			dup(con);
			close(1);
			dup(con);
			close(2);
			dup(con);
			close(con);
			child();
			exit(0);
			break;
		default:
			close(con);
			break;
		}
	}
	return 0;
}
