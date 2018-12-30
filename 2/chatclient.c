
/* \u804a\u5929\u5ba4\u5ba2\u6237\u7aef\u7a0b\u5e8f */
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <errno.h>
#include <stdlib.h>

#include "common.h"

#define QUIT_STRING "/end"

/* \u6253\u5370\u804a\u5929\u5ba4\u540d\u5355 */
void showgroups(long lent, char *text) 
{
	char *tptr;

	tptr = text;
	printf("%15s %15s %15s\n", "group", "capacity", "occupancy");
	while (tptr < text + lent) 
	{
		char *name, *capa, *occu;

		name = tptr;
		tptr = name + strlen(name) + 1;
		capa = tptr;
		tptr = capa + strlen(capa) + 1;
		occu = tptr;
		tptr = occu + strlen(occu) + 1;

		printf("%15s %15s %15s\n", name, capa, occu);
	}
}

/* \u52a0\u5165\u804a\u5929\u5ba4 */
int joinagroup(int sock) {
	
	Packet * pkt;
	char bufr[MAXPKTLEN];
	char * bufrptr;
	int bufrlen;
	char * gname;
	char * mname;

	/* \u8bf7\u6c42\u804a\u5929\u5ba4\u4fe1\u606f */
	sendpkt(sock, LIST_GROUPS, 0, NULL);

	/* \u63a5\u6536\u804a\u5929\u5ba4\u4fe1\u606f\u56de\u590d */
	pkt = recvpkt(sock);
	if (!pkt) 
	{
		fprintf(stderr, "error: server died\n");
		exit(1);
	}

	if (pkt->type != LIST_GROUPS) 
	{
		fprintf(stderr, "error: unexpected reply from server\n");
		exit(1);
	}

	/* \u663e\u793a\u804a\u5929\u5ba4 */
	showgroups(pkt->lent, pkt->text);

	/* \u4ece\u6807\u51c6\u8f93\u5165\u8bfb\u5165\u804a\u5929\u5ba4\u540d */
	printf("which group? ");
	fgets(bufr, MAXPKTLEN, stdin);
	bufr[strlen(bufr) - 1] = '\0';

	/* \u6b64\u65f6\u53ef\u80fd\u7528\u6237\u60f3\u9000\u51fa */
	if (strcmp(bufr, "") == 0
			|| strncmp(bufr, QUIT_STRING, strlen(QUIT_STRING)) == 0)
	{
		close(sock);
		exit(0);
	}
	gname = strdup(bufr);

	/* \u8bfb\u5165\u6210\u5458\u540d\u5b57 */
	printf("what nickname? ");
	fgets(bufr, MAXPKTLEN, stdin);
	bufr[strlen(bufr) - 1] = '\0';

	/* \u6b64\u65f6\u53ef\u80fd\u7528\u6237\u60f3\u9000\u51fa */
	if (strcmp(bufr, "") == 0
			|| strncmp(bufr, QUIT_STRING, strlen(QUIT_STRING)) == 0) 
	{
		close(sock);
		exit(0);
	}
	mname = strdup(bufr);

	/* \u53d1\u9001\u52a0\u5165\u804a\u5929\u5ba4\u7684\u4fe1\u606f */
	bufrptr = bufr;
	strcpy(bufrptr, gname);
	bufrptr += strlen(bufrptr) + 1;
	strcpy(bufrptr, mname);
	bufrptr += strlen(bufrptr) + 1;
	bufrlen = bufrptr - bufr;
	sendpkt(sock, JOIN_GROUP, bufrlen, bufr);

	/* \u8bfb\u53d6\u6765\u81ea\u670d\u52a1\u5668\u7684\u56de\u590d */
	pkt = recvpkt(sock);
	if (!pkt) 
	{
		fprintf(stderr, "error: server died\n");
		exit(1);
	}
	if (pkt->type != JOIN_ACCEPTED && pkt->type != JOIN_REJECTED) 
	{
		fprintf(stderr, "error: unexpected reply from server\n");
		exit(1);
	}

	/* \u5982\u679c\u62d2\u7edd\u663e\u793a\u5176\u539f\u56e0 */
	if (pkt->type == JOIN_REJECTED)
	{
		printf("admin: %s\n", pkt->text);
		free(gname);
		free(mname);
		return (0);
	}
	else /* \u6210\u529f\u52a0\u5165 */
	{
		printf("admin: joined '%s' as '%s'\n", gname, mname);
		free(gname);
		free(mname);
		return (1);
	}
}

/* \u4e3b\u51fd\u6570\u5165\u53e3 */
main(int argc, char *argv[]) 
{
	int sock;

	/* \u7528\u6237\u8f93\u5165\u5408\u6cd5\u6027\u68c0\u6d4b */
	if (argc != 1) 
	{
		fprintf(stderr, "usage : %s\n", argv[0]);
		exit(1);
	}

	/* \u4e0e\u670d\u52a1\u5668\u8fde\u63a5 */
	sock = hooktoserver();
	if (sock == -1)
		exit(1);

	fflush(stdout); /* \u6e05\u9664\u6807\u51c6\u8f93\u51fa\u7f13\u51b2\u533a */

	
	/* \u521d\u59cb\u5316\u63cf\u8ff0\u7b26\u96c6 */
	fd_set clientfds, tempfds;
	FD_ZERO(&clientfds);
	FD_ZERO(&tempfds);
	FD_SET(sock, &clientfds); /* \u8bbe\u7f6e\u670d\u52a1\u5668\u5957\u63a5\u5b57\u5728 clientfds \u4e2d\u7684\u6bd4\u7279\u4f4d */
	FD_SET(0, &clientfds); /* \u8bbe\u7f6e\u6807\u51c6\u8f93\u5165\u5728 clientfds \u4e2d\u7684\u6bd4\u7279\u4f4d */

	/* \u5faa\u73af */
	while (1) 
	{
		/* \u52a0\u5165\u804a\u5929\u5ba4 */
		if (!joinagroup(sock))
			continue;

		/* \u4fdd\u6301\u804a\u5929\u72b6\u6001 */
		while (1) 
		{
			/* \u8c03\u7528 select \u51fd\u6570\u540c\u65f6\u76d1\u6d4b\u952e\u76d8\u548c\u670d\u52a1\u5668\u4fe1\u606f */
			tempfds = clientfds;

			if (select(FD_SETSIZE, &tempfds, NULL, NULL, NULL) == -1) 
			{
				perror("select");
				exit(4);
			}

			/* \u5bf9\u4e8e\u6240\u6709\u5728 tempfds \u4e2d\u88ab\u7f6e\u4f4d\u7684\u6587\u4ef6\u63cf\u8ff0\u7b26\uff0c\u68c0\u6d4b\u5b83\u662f\u5426\u662f\u5957\u63a5\u5b57\u63cf\u8ff0\u7b26\uff0c
			\u5982\u679c\u662f\uff0c\u610f\u5473\u670d\u52a1\u5668\u4f20\u6765\u6d88\u606f\u3002\u5982\u679c\u5b83\u6587\u4ef6\u63cf\u8ff0\u7b26\u662f 0\uff0c\u5219\u610f\u5473\u6709\u6765\u81ea\u7528\u6237
			\u952e\u76d8\u7684\u8f93\u5165\u8981\u53d1\u9001\u7ed9\u670d\u52a1\u5668 */

			/* \u5904\u7406\u670d\u52a1\u5668\u4f20\u6765\u4fe1\u606f */
			if (FD_ISSET(sock,&tempfds)) 
			{
				Packet *pkt;
				pkt = recvpkt(sock);
				if (!pkt) 
				{
					/* \u670d\u52a1\u5668\u5b95\u673a */
					fprintf(stderr, "error: server died\n");
					exit(1);
				}

				/* \u663e\u793a\u6d88\u606f\u6587\u672c */
				if (pkt->type != USER_TEXT) 
				{
					fprintf(stderr, "error: unexpected reply from server\n");
					exit(1);
				}

				printf("%s: %s", pkt->text, pkt->text + strlen(pkt->text) + 1);
				freepkt(pkt);
			}

			/* \u5904\u7406\u952e\u76d8\u8f93\u5165 */
			if (FD_ISSET(0,&tempfds)) 
			{
				char bufr[MAXPKTLEN];

				fgets(bufr, MAXPKTLEN, stdin);
				if (strncmp(bufr, QUIT_STRING, strlen(QUIT_STRING)) == 0) 
				{
					/* \u9000\u51fa\u804a\u5929\u5ba4 */
					sendpkt(sock, LEAVE_GROUP, 0, NULL);
					break;
				}

				/* \u53d1\u9001\u6d88\u606f\u6587\u672c\u5230\u670d\u52a1\u5668 */
				sendpkt(sock, USER_TEXT, strlen(bufr) + 1, bufr);
			}

		}

	}
}
