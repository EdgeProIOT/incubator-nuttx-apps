#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
#include <debug.h>

#include <nng/nng.h>
#include <nng/protocol/pubsub0/pub.h>
#include <nng/protocol/pubsub0/sub.h>


void
fatal(const char *func, int rv)
{
	fprintf(stderr, "%s: %s\n", func, nng_strerror(rv));
}

char *
date(void)
{
	time_t now = time(&now);
	struct tm *info = localtime(&now);
	char *text = asctime(info);
	text[strlen(text)-1] = '\0';
	return (text);
}


void *client_thread(pthread_addr_t pvarg)
{
	nng_socket sock;
	int rv;

	sleep(2);

	if ((rv = nng_sub0_open(&sock)) != 0) {
		fatal("nng_sub0_open", rv);
	}

	/* subscribe to everything (empty means all topics) */
	if ((rv = nng_setopt(sock, NNG_OPT_SUB_SUBSCRIBE, "", 0)) != 0) {
		fatal("nng_setopt", rv);
	}
	if ((rv = nng_dial(sock, "ipc:///pubsub.ipc", NULL, 0)) != 0) {
		fatal("nng_dial", rv);
	}
	for (;;) {
		char *buf = NULL;
		size_t sz;
		if ((rv = nng_recv(sock, &buf, &sz, NNG_FLAG_ALLOC)) != 0) {
			fatal("nng_recv", rv);
		}
		printf("CLIENT: RECEIVED %s\n", buf); 
		nng_free(buf, sz);
	}

	return NULL;  /* Keeps some compilers from complaining */
}

int
main(const int argc, const char **argv)
{
	pthread_t tid;
	nng_socket sock;
	int rv;

	if ((rv = nng_pub0_open(&sock)) != 0) {
		fatal("nng_pub0_open", rv);
	}

	rv = pthread_create(&tid, NULL, client_thread, NULL);
  	if (rv != 0)
    {
		printf("main: Failed to create client thread: %d\n", rv);
    }

	if ((rv = nng_listen(sock, "ipc:///pubsub.ipc", NULL, 0)) < 0) {
		fatal("nng_listen", rv);
	}

	for (;;) {
		char *d = date();
		//printf("SERVER: PUBLISHING DATE %s\n", d);
		if ((rv = nng_send(sock, d, strlen(d) + 1, 0)) != 0) {
			fatal("nng_send", rv);
		}
		sleep(1);
	}
	
	return 1;
}
