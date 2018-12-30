/* \u804a\u5929\u5ba4\u670d\u52a1\u5668\u7aef\u7a0b\u5e8f */

#include <stdlib.h>
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
#include <signal.h>
#include "common.h"

/* \u804a\u5929\u5ba4\u6210\u5458\u4fe1\u606f */
typedef struct _member 
{
    /* \u6210\u5458\u59d3\u540d */
    char *name;

    /* \u6210\u5458 socket \u63cf\u8ff0\u7b26 */
    int sock;

    /* \u6210\u5458\u6240\u5c5e\u804a\u5929\u5ba4 */
    int grid;

    /* \u4e0b\u4e00\u4e2a\u6210\u5458 */
    struct _member *next;

    /* \u524d\u4e00\u4e2a\u6210\u5458 */
    struct _member *prev;

} Member;

/* \u804a\u5929\u5ba4\u4fe1\u606f */
typedef struct _group 
{
    /* \u804a\u5929\u5ba4\u540d\u5b57 */
    char *name;

    /* \u804a\u5929\u5ba4\u6700\u5927\u5bb9\u91cf\uff08\u4eba\u6570\uff09 */
    int capa;

    /* \u5f53\u524d\u5360\u6709\u7387\uff08\u4eba\u6570\uff09 */
    int occu;

    /* \u8bb0\u5f55\u804a\u5929\u5ba4\u5185\u6240\u6709\u6210\u5458\u4fe1\u606f\u7684\u94fe\u8868 */
    struct _member *mems;

} Group;

/* \u6240\u6709\u804a\u5929\u5ba4\u7684\u4fe1\u606f\u8868 */
Group *group;
int ngroups;

/* \u901a\u8fc7\u804a\u5929\u5ba4\u540d\u5b57\u627e\u5230\u804a\u5929\u5ba4 ID */
int findgroup(char *name)
{
    int grid; /* \u804a\u5929\u5ba4ID */

	for (grid = 0; grid < ngroups; grid++)
	{
		if(strcmp(group[grid].name, name) == 0)
			return(grid);
	}
    return(-1);
}

/* \u901a\u8fc7\u5ba4\u6210\u5458\u540d\u5b57\u627e\u5230\u5ba4\u6210\u5458\u7684\u4fe1\u606f */
Member *findmemberbyname(char *name)
{
    int grid; /* \u804a\u5929\u5ba4 ID */

    /* \u904d\u5386\u6bcf\u4e2a\u7ec4 */
    for (grid=0; grid < ngroups; grid++) 
	{
        Member *memb;

        /* \u904d\u5386\u6539\u7ec4\u7684\u6240\u6709\u6210\u5458 */
        for (memb = group[grid].mems; memb ; memb = memb->next)
		{
            if (strcmp(memb->name, name) == 0)
	        return(memb);
        }
    }
    return(NULL);
}

/* \u901a\u8fc7 socket \u63cf\u8ff0\u7b26\u627e\u5230\u5ba4\u6210\u5458\u7684\u4fe1\u606f */
Member *findmemberbysock(int sock)
{
    int grid; /* \u804a\u5929\u5ba4ID */

    /* \u904d\u5386\u6240\u6709\u7684\u804a\u5929\u5ba4 */
    for (grid=0; grid < ngroups; grid++) 
	{
		Member *memb;

		/* \u904d\u5386\u6240\u6709\u7684\u5f53\u524d\u804a\u5929\u5ba4\u6210\u5458 */
		for (memb = group[grid].mems; memb; memb = memb->next)
		{
			if (memb->sock == sock)
			return(memb);
		}
    }
    return(NULL);
}

/* \u9000\u51fa\u524d\u7684\u6e05\u7406\u5de5\u4f5c */
void cleanup()
{
  char linkname[MAXNAMELEN];

  /* \u53d6\u6d88\u6587\u4ef6\u94fe\u63a5 */
  sprintf(linkname, "%s/%s", getenv("HOME"), PORTLINK);
  unlink(linkname);
  exit(0);
}

/* \u4e3b\u51fd\u6570\u7a0b\u5e8f */
main(int argc, char *argv[])
{
	int    servsock;   /* \u804a\u5929\u5ba4\u670d\u52a1\u5668\u7aef\u76d1\u542c socket \u63cf\u8ff0\u7b26 */
	int    maxsd;	     /* \u8fde\u63a5\u7684\u5ba2\u6237\u7aef socket \u63cf\u8ff0\u7b26\u7684\u6700\u5927\u503c */
	fd_set livesdset, tempset; /* \u5ba2\u6237\u7aef sockets \u63cf\u8ff0\u7b26\u96c6 */


	/* \u7528\u6237\u8f93\u5165\u5408\u6cd5\u6027\u68c0\u6d4b */
	if (argc != 2) 
		{
			fprintf(stderr, "usage : %s <groups-file>\n", argv[0]);
			exit(1);
		}

	/* \u8c03\u7528 initgroups \u51fd\u6570\uff0c\u521d\u59cb\u5316\u804a\u5929\u5ba4\u4fe1\u606f */
	if (!initgroups(argv[1]))
		exit(1);

	/* \u8bbe\u7f6e\u4fe1\u53f7\u5904\u7406\u51fd\u6570 */
	signal(SIGTERM, cleanup);
	signal(SIGINT, cleanup);

	/* \u51c6\u5907\u63a5\u53d7\u8bf7\u6c42 */
	servsock = startserver(); /* \u5b9a\u4e49\u5728 "chatlinker.c" \u6587\u4ef6\u4e2d\uff0c
							\u4e3b\u8981\u5b8c\u6210\u521b\u5efa\u670d\u52a1\u5668\u5957\u63a5\u5b57\uff0c\u7ed1\u5b9a\u7aef\u53e3\u53f7\uff0c
							\u5e76\u8bbe\u7f6e\u628a\u5957\u63a5\u5b57\u4e3a\u76d1\u542c\u72b6\u6001 */
	if (servsock == -1)
		exit(1);

	/* \u521d\u59cb\u5316 maxsd */
	maxsd = servsock;

	/* \u521d\u59cb\u5316\u63cf\u8ff0\u7b26\u96c6 */
	FD_ZERO(&livesdset); /* \u6e05\u7406 livesdset \u7684\u6240\u6709\u7684\u6bd4\u7279\u4f4d*/
	FD_ZERO(&tempset);  /* \u6e05\u7406 tempset \u7684\u6240\u6709\u7684\u6bd4\u7279\u4f4d */
	FD_SET(servsock, &livesdset); /* \u6253\u5f00\u670d\u52a1\u5668\u76d1\u542c\u5957\u63a5\u5b57\u7684\u5957\u63a5\u5b57
								  \u63cf\u8ff0\u7b26 servsock \u5bf9\u5e94\u7684fd_set \u6bd4\u7279\u4f4d */

	/* \u63a5\u53d7\u5e76\u5904\u7406\u6765\u81ea\u5ba2\u6237\u7aef\u7684\u8bf7\u6c42 */
	while (1) 
		{
			int sock;    /* \u5faa\u73af\u53d8\u91cf */

			/* \u7279\u522b\u6ce8\u610f tempset \u4f5c\u4e3a select \u53c2\u6570\u65f6\u662f\u4e00\u4e2a "\u503c-\u7ed3\u679c" \u53c2\u6570\uff0c
			select \u51fd\u6570\u8fd4\u56de\u65f6\uff0ctempset \u4e2d\u6253\u5f00\u7684\u6bd4\u7279\u4f4d\u53ea\u662f\u8bfb\u5c31\u7eea\u7684 socket
			\u63cf\u8ff0\u7b26\uff0c\u6240\u4ee5\u6211\u4eec\u6bcf\u6b21\u5faa\u73af\u90fd\u8981\u5c06\u5176\u66f4\u65b0\u4e3a\u6211\u4eec\u9700\u8981\u5185\u6838\u6d4b\u8bd5\u8bfb\u5c31\u7eea\u6761\u4ef6
			\u7684 socket \u63cf\u8ff0\u7b26\u96c6\u5408 livesdset */
			tempset = livesdset; 

		
			/* \u8c03\u7528 select \u51fd\u6570\u7b49\u5f85\u5df2\u8fde\u63a5\u5957\u63a5\u5b57\u4e0a\u7684\u5305\u548c\u6765\u81ea
			\u65b0\u7684\u5957\u63a5\u5b57\u7684\u94fe\u63a5\u8bf7\u6c42 */
			select(maxsd + 1, &tempset, NULL, NULL, NULL);

			/* \u5faa\u73af\u67e5\u627e\u6765\u81ea\u5ba2\u6237\u673a\u7684\u8bf7\u6c42 */
			for (sock=3; sock <= maxsd; sock++)
				{
					/* \u5982\u679c\u662f\u670d\u52a1\u5668\u76d1\u542c socket\uff0c\u5219\u8df3\u51fa\u63a5\u6536\u6570\u636e\u5305\u73af\u8282\uff0c\u6267\u884c\u63a5\u53d7\u8fde\u63a5 */
					if (sock == servsock)
						continue;

					/* \u6709\u6765\u81ea\u5ba2\u6237 socket \u7684\u6d88\u606f */
					if(FD_ISSET(sock, &tempset))
					{
						Packet *pkt;

						/* \u8bfb\u6d88\u606f */
						pkt = recvpkt(sock); /* \u51fd\u6570 recvpkt \u5b9a\u4e49\u5728"chatlinker.c" */

						if (!pkt)
							{
								/* \u5ba2\u6237\u673a\u65ad\u5f00\u4e86\u8fde\u63a5 */
								char *clientname;  /* host name of the client */

								/* \u4f7f\u7528 gethostbyaddr\uff0cgetpeername \u51fd\u6570\u5f97\u5230 client \u7684\u4e3b\u673a\u540d */
								socklen_t len;
								struct sockaddr_in addr;
								len = sizeof(addr);
								if (getpeername(sock, (struct sockaddr*) &addr, &len) == 0) 
									{
										struct sockaddr_in *s = (struct sockaddr_in *) &addr;
										struct hostent *he;
										he = gethostbyaddr(&s->sin_addr, sizeof(struct in_addr), AF_INET);
										clientname = he->h_name;
									}
								else
									printf("Cannot get peer name");

								printf("admin: disconnect from '%s' at '%d'\n",
									clientname, sock);

								/* \u4ece\u804a\u5929\u5ba4\u5220\u9664\u8be5\u6210\u5458 */
								leavegroup(sock);

								/* \u5173\u95ed\u5957\u63a5\u5b57 */
								close(sock);

								/* \u6e05\u9664\u5957\u63a5\u5b57\u63cf\u8ff0\u7b26\u5728 livesdset \u4e2d\u7684\u6bd4\u7279\u4f4d */
								FD_CLR(sock, &livesdset);

							} 
						else 
							{
								char *gname, *mname;

								/* \u57fa\u4e8e\u6d88\u606f\u7c7b\u578b\u91c7\u53d6\u884c\u52a8 */
								switch (pkt->type) 
								{
									case LIST_GROUPS :
										listgroups(sock);
										break;
									case JOIN_GROUP :
										gname = pkt->text;
										mname = gname + strlen(gname) + 1;
										joingroup(sock, gname, mname);
										break;
									case LEAVE_GROUP :
										leavegroup(sock);
										break;
									case USER_TEXT :
										relaymsg(sock, pkt->text);
										break;
								}

								/* \u91ca\u653e\u5305\u7ed3\u6784 */
								freepkt(pkt);
							}
					}
				}

			struct sockaddr_in remoteaddr; /* \u5ba2\u6237\u673a\u5730\u5740\u7ed3\u6784 */
			socklen_t addrlen;

			/* \u6709\u6765\u81ea\u65b0\u7684\u5ba2\u6237\u673a\u7684\u8fde\u63a5\u8bf7\u6c42\u8bf7\u6c42 */
			if(FD_ISSET(servsock, &tempset))
			{
				int  csd; /* \u5df2\u8fde\u63a5\u7684 socket \u63cf\u8ff0\u7b26 */

				/* \u63a5\u53d7\u4e00\u4e2a\u65b0\u7684\u8fde\u63a5\u8bf7\u6c42 */
				addrlen = sizeof remoteaddr;
				csd = accept(servsock, (struct sockaddr *) &remoteaddr, &addrlen);

				/* \u5982\u679c\u8fde\u63a5\u6210\u529f */
				if (csd != -1) 
					{
						char *clientname;

						/* \u4f7f\u7528 gethostbyaddr \u51fd\u6570\u5f97\u5230 client \u7684\u4e3b\u673a\u540d */
						struct hostent *h;
						h = gethostbyaddr((char *)&remoteaddr.sin_addr.s_addr,
							sizeof(struct in_addr), AF_INET);

						if (h != (struct hostent *) 0) 
							clientname = h->h_name;
						else
							printf("gethostbyaddr failed\n");

						/* \u663e\u793a\u5ba2\u6237\u673a\u7684\u4e3b\u673a\u540d\u548c\u5bf9\u5e94\u7684 socket \u63cf\u8ff0\u7b26 */
						printf("admin: connect from '%s' at '%d'\n",
							clientname, csd);

						/* \u5c06\u8be5\u8fde\u63a5\u7684\u5957\u63a5\u5b57\u63cf\u8ff0\u7b26 csd \u52a0\u5165livesdset */
						FD_SET(csd, &livesdset);

						/* \u4fdd\u6301 maxsd \u8bb0\u5f55\u7684\u662f\u6700\u5927\u7684\u5957\u63a5\u5b57\u63cf\u8ff0\u7b26 */
						if (csd > maxsd)
							maxsd = csd;
					}
				else 
					{
						perror("accept");
						exit(0);
					}
			}
		}
}

/* \u521d\u59cb\u5316\u804a\u5929\u5ba4\u94fe\u8868 */
int initgroups(char *groupsfile)
{
	FILE *fp;
	char name[MAXNAMELEN];
	int capa;
	int grid;

	/* \u6253\u5f00\u5b58\u50a8\u804a\u5929\u5ba4\u4fe1\u606f\u7684\u914d\u7f6e\u6587\u4ef6 */
	fp = fopen(groupsfile, "r");
	if (!fp) 
	{
		fprintf(stderr, "error : unable to open file '%s'\n", groupsfile);
		return(0);
    }

	/* \u4ece\u914d\u7f6e\u6587\u4ef6\u4e2d\u8bfb\u53d6\u804a\u5929\u5ba4\u7684\u6570\u91cf */
	fscanf(fp, "%d", &ngroups);

	/* \u4e3a\u6240\u6709\u7684\u804a\u5929\u5ba4\u5206\u914d\u5185\u5b58\u7a7a\u95f4 */
	group = (Group *) calloc(ngroups, sizeof(Group));
    if (!group) 
	{
		fprintf(stderr, "error : unable to calloc\n");
		return(0);
    }

	/* \u4ece\u914d\u7f6e\u6587\u4ef6\u8bfb\u53d6\u804a\u5929\u5ba4\u4fe1\u606f */
	for (grid =0; grid < ngroups; grid++) 
	{
		/* \u8bfb\u53d6\u804a\u5929\u5ba4\u540d\u548c\u5bb9\u91cf */
		if (fscanf(fp, "%s %d", name, &capa) != 2)
		{
			fprintf(stderr, "error : no info on group %d\n", grid + 1);
			return(0);
		}

    /* \u5c06\u4fe1\u606f\u5b58\u8fdb group \u7ed3\u6784 */
		group[grid].name = strdup(name);
		group[grid].capa = capa;
		group[grid].occu = 0;
		group[grid].mems = NULL;
    }
	return(1);
}

/* \u628a\u6240\u6709\u804a\u5929\u5ba4\u7684\u4fe1\u606f\u53d1\u7ed9\u5ba2\u6237\u7aef */
int listgroups(int sock)
{
	int      grid;
	char     pktbufr[MAXPKTLEN];
	char *   bufrptr;
	long     bufrlen;

	/* \u6bcf\u4e00\u5757\u4fe1\u606f\u5728\u5b57\u7b26\u4e32\u4e2d\u7528 NULL \u5206\u5272 */
	bufrptr = pktbufr;
	for (grid=0; grid < ngroups; grid++) 
	{
		/* \u83b7\u53d6\u804a\u5929\u5ba4\u540d\u5b57 */
		sprintf(bufrptr, "%s", group[grid].name);
		bufrptr += strlen(bufrptr) + 1;

		/* \u83b7\u53d6\u804a\u5929\u5ba4\u5bb9\u91cf */
		sprintf(bufrptr, "%d", group[grid].capa);
		bufrptr += strlen(bufrptr) + 1;

		/* \u83b7\u53d6\u804a\u5929\u5ba4\u5360\u6709\u7387 */
		sprintf(bufrptr, "%d", group[grid].occu);
		bufrptr += strlen(bufrptr) + 1;
    }
	bufrlen = bufrptr - pktbufr;

	/* \u53d1\u9001\u6d88\u606f\u7ed9\u56de\u590d\u5ba2\u6237\u673a\u7684\u8bf7\u6c42 */
	sendpkt(sock, LIST_GROUPS, bufrlen, pktbufr);
	return(1);
}

/* \u52a0\u5165\u804a\u5929\u5ba4 */
int joingroup(int sock, char *gname, char *mname)
{
	int       grid;
	Member *  memb;

	/* \u6839\u636e\u804a\u5929\u5ba4\u540d\u83b7\u5f97\u804a\u5929\u5ba4 ID */
	grid = findgroup(gname);
	if (grid == -1) 
	{
		char *errmsg = "no such group";
		sendpkt(sock, JOIN_REJECTED, strlen(errmsg), errmsg); /* \u53d1\u9001\u62d2\u7edd\u52a0\u5165\u6d88\u606f */
		return(0);
    }

	/* \u68c0\u67e5\u662f\u5426\u804a\u5929\u5ba4\u6210\u5458\u540d\u5b57\u5df2\u88ab\u5360\u7528 */
	memb = findmemberbyname(mname);

	/* \u5982\u679c\u804a\u5929\u5ba4\u6210\u5458\u540d\u5df2\u5b58\u5728\uff0c\u5219\u8fd4\u56de\u9519\u8bef\u6d88\u606f */
	if (memb) 
	{
		char *errmsg = "member name already exists";
		sendpkt(sock, JOIN_REJECTED, strlen(errmsg), errmsg); /* \u53d1\u9001\u62d2\u7edd\u52a0\u5165\u6d88\u606f */
		return(0);
    }

	/* \u68c0\u67e5\u804a\u5929\u5ba4\u662f\u5426\u5df2\u6ee1 */
	if (group[grid].capa == group[grid].occu) 
	{
		char *errmsg = "room is full";
		sendpkt(sock, JOIN_REJECTED, strlen(errmsg), errmsg); /* \u53d1\u9001\u62d2\u7edd\u52a0\u5165\u6d88\u606f */
		return(0);
	}

	/* \u4e3a\u804a\u5929\u5ba4\u65b0\u6210\u5458\u7533\u8bf7\u5185\u5b58\u7a7a\u95f4\u6765\u5b58\u50a8\u6210\u5458\u4fe1\u606f */
	memb = (Member *) calloc(1, sizeof(Member));
	if (!memb) 
	{
		fprintf(stderr, "error : unable to calloc\n");
		cleanup();
    }
	memb->name = strdup(mname);
	memb->sock = sock;
	memb->grid = grid;
	memb->prev = NULL;
	memb->next = group[grid].mems;
	if (group[grid].mems) 
	{
		group[grid].mems->prev = memb;
	}
	group[grid].mems = memb;
	printf("admin: '%s' joined '%s'\n", mname, gname);

	/* \u66f4\u65b0\u804a\u5929\u5ba4\u7684\u5728\u7ebf\u4eba\u6570 */
	group[grid].occu++;

	sendpkt(sock, JOIN_ACCEPTED, 0, NULL); /* \u53d1\u9001\u63a5\u53d7\u6210\u5458\u6d88\u606f */
	return(1);
}

/* \u79bb\u5f00\u804a\u5929\u5ba4 */
int leavegroup(int sock)
{
	Member *memb;

	/* \u5f97\u5230\u804a\u5929\u5ba4\u6210\u5458\u4fe1\u606f */
	memb = findmemberbysock(sock);
	if (!memb) 
		return(0);

	/* \u4ece\u804a\u5929\u5ba4\u4fe1\u606f\u7ed3\u6784\u4e2d\u5220\u9664 memb \u6210\u5458 */
	if (memb->next) 
		memb->next->prev = memb->prev; /* \u5728\u804a\u5929\u5ba4\u6210\u5458\u94fe\u8868\u7684\u5c3e\u90e8 */

	/* remove from ... */
	if (group[memb->grid].mems == memb) /* \u5728\u804a\u5929\u5ba4\u6210\u5458\u94fe\u8868\u7684\u5934\u90e8 */
		group[memb->grid].mems = memb->next;

	else 
		memb->prev->next = memb->next; /* \u5728\u804a\u5929\u5ba4\u6210\u5458\u94fe\u8868\u7684\u4e2d\u90e8 */
	
	printf("admin: '%s' left '%s'\n",
		memb->name, group[memb->grid].name);

	/* \u66f4\u65b0\u804a\u5929\u5ba4\u7684\u5360\u6709\u7387 */
	group[memb->grid].occu--;

	/* \u91ca\u653e\u5185\u5b58 */
	free(memb->name);
	free(memb);
	return(1);
}

/* \u628a\u6210\u5458\u7684\u6d88\u606f\u53d1\u9001\u7ed9\u5176\u4ed6\u804a\u5929\u5ba4\u6210\u5458 */
int relaymsg(int sock, char *text)
{
	Member *memb;
	Member *sender;
	char pktbufr[MAXPKTLEN];
	char *bufrptr;
	long bufrlen;

	/* \u6839\u636e socket \u63cf\u8ff0\u7b26\u83b7\u5f97\u8be5\u804a\u5929\u5ba4\u6210\u5458\u7684\u4fe1\u606f */
	sender = findmemberbysock(sock);
	if (!sender)
	{
		fprintf(stderr, "strange: no member at %d\n", sock);
		return(0);
	}

	/* \u628a\u53d1\u9001\u8005\u7684\u59d3\u540d\u6dfb\u52a0\u5230\u6d88\u606f\u6587\u672c\u524d\u8fb9 */
	bufrptr = pktbufr;
	strcpy(bufrptr,sender->name);
	bufrptr += strlen(bufrptr) + 1;
	strcpy(bufrptr, text);
	bufrptr += strlen(bufrptr) + 1;
	bufrlen = bufrptr - pktbufr;

	/* \u5e7f\u64ad\u8be5\u6d88\u606f\u7ed9\u8be5\u6210\u5458\u6240\u5728\u804a\u5929\u5ba4\u7684\u5176\u4ed6\u6210\u5458 */
	for (memb = group[sender->grid].mems; memb; memb = memb->next)
	{
		/* \u8df3\u8fc7\u53d1\u9001\u8005 */
		if (memb->sock == sock) 
			continue;
		sendpkt(memb->sock, USER_TEXT, bufrlen, pktbufr); /* \u7ed9\u804a\u5929\u5ba4\u5176\u4ed6\u6210\u5458
														  \u53d1\u9001\u6d88\u606f\uff08TCP\u662f\u5168\u53cc\u5de5\u7684\uff09 */
	}
	printf("%s: %s", sender->name, text);
	return(1);
}
