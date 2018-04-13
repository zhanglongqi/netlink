#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <poll.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>

// #define USING_POLL
#define USING_RECVMSG


#define NETLINK_USER 31
#define MY_GROUP 1
#define MAX_PAYLOAD 1024 /* maximum payload size*/

int open_netlink()
{
	int                socket_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_USER);
	struct sockaddr_nl addr;

	memset((void *)&addr, 0, sizeof(addr));

	if (socket_fd < 0)
		return socket_fd;
	addr.nl_family = AF_NETLINK;
	addr.nl_pid    = getpid();
	addr.nl_groups = MY_GROUP; // for multicast or broadcast
	// addr.nl_groups = 0; // for unicast
	if (bind(socket_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		return -1;
	return socket_fd;
}
int send_event(int socket_fd)
{
	struct iovec       iov;
	struct sockaddr_nl dest_addr;
	struct nlmsghdr *  nlh = NULL;
	struct msghdr      msg_hdr;

	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.nl_family = AF_NETLINK;
	dest_addr.nl_pid    = 0; /* For Linux Kernel */
	dest_addr.nl_groups = 0; /* unicast */

	nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));

	memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
	nlh->nlmsg_len   = NLMSG_SPACE(MAX_PAYLOAD);
	nlh->nlmsg_pid   = getpid();
	nlh->nlmsg_flags = 0;

	strcpy(NLMSG_DATA(nlh), "Message from User");
	memset(&msg_hdr, 0, sizeof(struct msghdr)); // very very very important

	iov.iov_base        = (void *)nlh;
	iov.iov_len         = nlh->nlmsg_len;
	msg_hdr.msg_name    = (void *)&dest_addr;
	msg_hdr.msg_namelen = sizeof(dest_addr);
	msg_hdr.msg_iov     = &iov;
	msg_hdr.msg_iovlen  = 1;

	printf("Sending message to kernel...");
	int ret = sendmsg(socket_fd, &msg_hdr, 0);
	printf("Done: %d %s\n", ret, strerror(errno));
	free(nlh);
}
int read_event(int socket_fd)
{
	struct sockaddr_nl nladdr;
	struct msghdr      msg_hdr;
	struct iovec       iov;
	struct nlmsghdr *  nlh;

	nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));

	iov.iov_base = (void *)nlh;
	iov.iov_len  = NLMSG_SPACE(MAX_PAYLOAD);

	memset(&nladdr, 0, sizeof(nladdr));

	msg_hdr.msg_name    = (void *)&(nladdr);
	msg_hdr.msg_namelen = sizeof(nladdr);
	msg_hdr.msg_iov     = &iov;
	msg_hdr.msg_iovlen  = 1;


	int ret = 0;

	#ifndef USING_POLL
	ret = recvmsg(socket_fd, &msg_hdr, 0);
	if (ret < 0) {
		return ret;
	}
	printf("Received message payload: %s\n", (char *)NLMSG_DATA(nlh));

	printf("\nret:%d (nlh)->nlmsg_len:%d\n", ret, nlh->nlmsg_len);

	printf("NLMSG_PAYLOAD: %d \n"
	       "NLMSG_SPACE(MAX_PAYLOAD): %d \n"
	       "NLMSG_PAYLOAD(nlh, 0): %d \n",
	       MAX_PAYLOAD,
	       NLMSG_SPACE(MAX_PAYLOAD),
	       NLMSG_PAYLOAD(nlh, 0));
	#else
	struct pollfd clientfd[1];
	clientfd[0].fd                              = socket_fd;
	clientfd[0].events                          = POLLIN; //POLLRDNORM;
	unsigned char buf[NLMSG_SPACE(MAX_PAYLOAD)] = { 0 };
	memset(buf, sizeof(buf), 0);

	while (1) {
		printf("before poll\n");
		if (0 >= poll(clientfd, 1, -1)) {
			printf("poll error\n");
			break;
		}
		printf("after poll\n");
		if (clientfd[0].revents & POLLIN) {
			#ifdef USING_RECVMSG
			ret = recvmsg(clientfd[0].fd, &msg_hdr, 0);
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

	free(nlh);
}

int main(int argc, char *argv[])
{
	int socket_fd = open_netlink();
	if (socket_fd < 0) {
		err(1, "netlink");
	}
	while (1) {
		send_event(socket_fd);
		sleep(1);
		read_event(socket_fd);
	}
	return 0;
}
