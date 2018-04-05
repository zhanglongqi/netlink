//Taken from https://stackoverflow.com/questions/15215865/netlink-sockets-in-c-using-the-3-x-linux-kernel?lq=1

#include <sys/socket.h>
#include <linux/netlink.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <poll.h>

#define USING_POLL

#define NETLINK_USER 31

#define MAX_PAYLOAD 1024 /* maximum payload size*/
struct sockaddr_nl src_addr, dest_addr;
struct nlmsghdr *  nlh = NULL;
struct iovec       iov;
int                sock_fd;
struct msghdr      msg;

int main()
{
	sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER);
	if (sock_fd < 0)
		return -1;

	memset(&src_addr, 0, sizeof(src_addr));
	src_addr.nl_family = AF_NETLINK;
	src_addr.nl_pid    = getpid(); /* self pid */

	bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr));

	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.nl_family = AF_NETLINK;
	dest_addr.nl_pid    = 0; /* For Linux Kernel */
	dest_addr.nl_groups = 0; /* unicast */

	nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
	memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
	nlh->nlmsg_len   = NLMSG_SPACE(MAX_PAYLOAD);
	nlh->nlmsg_pid   = getpid();
	nlh->nlmsg_flags = 0;

	strcpy(NLMSG_DATA(nlh), "Hello from USER");

	iov.iov_base    = (void *)nlh;
	iov.iov_len     = nlh->nlmsg_len;
	msg.msg_name    = (void *)&dest_addr;
	msg.msg_namelen = sizeof(dest_addr);
	msg.msg_iov     = &iov;
	msg.msg_iovlen  = 1;

	printf("Sending message to kernel\n");
	sendmsg(sock_fd, &msg, 0);
	printf("Waiting for message from kernel\n");

	/* Read message from kernel */
	#ifndef USING_POLL
	recvmsg(sock_fd, &msg, 0);
	printf("Received message payload: %s\n", (char *)NLMSG_DATA(nlh));
	close(sock_fd);
	#else
	int           ret = 0;
	struct pollfd clientfd[1];
	clientfd[0].fd         = sock_fd;
	clientfd[0].events     = POLLIN; //POLLRDNORM;
	unsigned char buf[100] = { 0 };
	memset(buf, sizeof(buf), 0);

	while (1) {
		printf("before poll\n");
		if (0 >= poll(clientfd, 1, 1000)) {
			printf("poll error\n");
			break;
		}
		printf("after poll\n");
		if (clientfd[0].revents & POLLIN) {
			#ifdef USING_RECVMSG
			ret = recvmsg(clientfd[0].fd, &msg, 0);
			printf("Received message payload: %s\n", (char *)NLMSG_DATA(nlh));
			#else
			ret = recv(clientfd[0].fd, buf, 100, 0);
			if (ret > 0) {
				for (int i = 0; i < ret; i++)
					printf("payloud %d: %d:%c\n", i, buf[i], buf[i]);
			}
			#endif
			ret = 0;
		}
	}
	#endif
}