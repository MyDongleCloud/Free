#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <poll.h>
#include <arpa/inet.h>
#include "macro.h"
#include "communication.h"

//Functions
static void *comSocket_t(void *arg) {
	struct sockaddr_in server_addr, client_addr;
	socklen_t client_len;
	int listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock < 0) {
		PRINTF("comSocket: error socket\n");
		return 0;
	}
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_addr.sin_port = htons(COMSOCKET_PORT);
		int yes = 1;
	setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	if (bind(listen_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		PRINTF("comSocket: error socket bind\n");
		return 0;
	}
	if (listen(listen_sock, SOMAXCONN) < 0) {
		PRINTF("comSocket: error socket listen\n");
		return 0;
	}
	PRINTF("comSocket: OK\n");
	struct pollfd fds[SOMAXCONN + 1];
	int nfds = 1;
	fds[0].fd = listen_sock;
	fds[0].events = POLLIN;
	while (1) {
		poll(fds, nfds, -1);
		if (fds[0].revents & POLLIN) {
			int client_sock = accept(listen_sock, (struct sockaddr*)&client_addr, &client_len);
			if (client_sock < 0) {
				PRINTF("comSocket: error socket accept\n");
				continue;
			}
			//PRINTF("comSocket: new connection\n");
			if (nfds < SOMAXCONN + 1) {
				fds[nfds].fd = client_sock;
				fds[nfds].events = POLLIN;
				nfds++;
			} else {
				PRINTF("comSocket: error max reached\n");
				close(client_sock);
			}
		}
		for (int i = 1; i < nfds; i++) {
			if (fds[i].revents & (POLLIN | POLLHUP)) {
				char buf[1024];
				memset(buf, 0, sizeof(buf));
				int nbytes = read(fds[i].fd, buf, sizeof(buf));
				if (nbytes <= 0) {
					if (nbytes == 0) {
						//PRINTF("comSocket: connection ended\n");
					} else {
						PRINTF("comSocket: error socket read");
					}
					close(fds[i].fd);
					fds[i] = fds[nfds - 1];
					nfds--;
					i--;
				} else {
					communicationReceive(buf, nbytes, "socket");
					memset(buf, 0, nbytes);
					strcpy(buf, "{\"error\":0}");
					write(fds[i].fd, buf, strlen(buf));
				}
			}
		}
	}
	for (int i = 0; i < nfds; i++)
		close(fds[i].fd);
	return 0;
}

void communicationSocket() {
	pthread_t pth;
	pthread_create(&pth, NULL, comSocket_t, NULL);
}
