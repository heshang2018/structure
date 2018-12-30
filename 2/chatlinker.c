/* \u8fde\u63a5\u670d\u52a1\u5668\u548c\u5ba2\u6237\u673a\u7684\u51fd\u6570 */

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <errno.h>
#include <stdlib.h>
#include "common.h"

/*
  \u4e3a\u670d\u52a1\u5668\u63a5\u6536\u5ba2\u6237\u7aef\u8bf7\u6c42\u505a\u51c6\u5907\uff0c
  \u6b63\u786e\u8fd4\u56de socket \u6587\u4ef6\u63cf\u8ff0\u7b26
  \u9519\u8bef\u8fd4\u56de -1
*/
int startserver()
{
  int     sd;      /* socket \u63cf\u8ff0\u7b26 */
  int     myport;  /* \u670d\u52a1\u5668\u7aef\u53e3 */
  const char *  myname;  /* \u672c\u5730\u4e3b\u673a\u7684\u5168\u79f0 */

  char 	  linktrgt[MAXNAMELEN];
  char 	  linkname[MAXNAMELEN];

  /*
	\u8c03\u7528 socket \u51fd\u6570\u521b\u5efa TCP socket \u63cf\u8ff0\u7b26
  */
  sd = socket(PF_INET, SOCK_STREAM, 0);

  /*
    \u8c03\u7528bind\u51fd\u6570\u5c06\u4e00\u4e2a\u672c\u5730\u5730\u5740\u6307\u6d3e\u7ed9 socket
  */

  struct sockaddr_in server_address;
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = htonl(INADDR_ANY); /* \u901a\u914d\u5730\u5740 INADDR_ANY \u8868\u793aIP\u5730\u5740\u4e3a 0.0.0.0\uff0c
													  \u5185\u6838\u5728\u5957\u63a5\u5b57\u88ab\u8fde\u63a5\u540e\u9009\u62e9\u4e00\u4e2a\u672c\u5730\u5730\u5740
													  htonl\u51fd\u6570 \u7528\u4e8e\u5c06 INADDR_ANY \u8f6c\u6362\u4e3a\u7f51\u7edc\u5b57\u8282\u5e8f */
  server_address.sin_port = htons(0);  /* \u6307\u6d3e\u4e3a\u901a\u914d\u7aef\u53e3 0\uff0c\u8c03\u7528 bind \u51fd\u6570\u540e\u5185\u6838\u5c06\u4efb\u610f\u9009\u62e9\u4e00\u4e2a\u4e34\u65f6\u7aef\u53e3 */

  bind(sd, (struct sockaddr *) &server_address, sizeof(server_address));

  /* \u8c03\u7528listen \u5c06\u670d\u52a1\u5668\u7aef socket \u63cf\u8ff0\u7b26 sd \u8bbe\u7f6e\u4e3a\u88ab\u52a8\u5730\u76d1\u542c\u72b6\u6001\uff0c\u5e76\u8bbe\u7f6e\u63a5\u53d7\u961f\u5217\u7684\u957f\u5ea6\u4e3a20 */
  listen(sd, 20);

  /*
    \u8c03\u7528 getsockname\u3001gethostname \u548c gethostbyname \u786e\u5b9a\u672c\u5730\u4e3b\u673a\u540d\u548c\u670d\u52a1\u5668\u7aef\u53e3\u53f7
  */

  char hostname[MAXNAMELEN];

  if (gethostname(hostname, sizeof hostname) != 0)
  	perror("gethostname");

  struct hostent* h;
	h = gethostbyname(hostname);

  int len = sizeof(struct sockaddr);

  getsockname(sd, (struct sockaddr *) &server_address, &len);

  myname = h->h_name;
  myport = ntohs(server_address.sin_port);

  /* \u5728\u5bb6\u76ee\u5f55\u4e0b\u521b\u5efa\u7b26\u53f7\u94fe\u63a5'.chatport'\u6307\u5411linktrgt */
  sprintf(linktrgt, "%s:%d", myname, myport);
  sprintf(linkname, "%s/%s", getenv("HOME"), PORTLINK); /* \u5728\u5934\u6587\u4ef6 common.h \u4e2d\uff1a
														#define PORTLINK ".chatport" */
  if (symlink(linktrgt, linkname) != 0) {
    fprintf(stderr, "error : server already exists\n");
    return(-1);
  }

  /* \u51c6\u5907\u63a5\u53d7\u5ba2\u6237\u7aef\u8bf7\u6c42 */
  printf("admin: started server on '%s' at '%d'\n",
	 myname, myport);
  return(sd);
}

/*
  \u548c\u670d\u52a1\u5668\u5efa\u7acb\u8fde\u63a5\uff0c\u6b63\u786e\u8fd4\u56de socket \u63cf\u8ff0\u7b26\uff0c
  \u5931\u8d25\u8fd4\u56de  -1
*/
int hooktoserver()
{
	int sd;                 

	char linkname[MAXNAMELEN];
	char linktrgt[MAXNAMELEN];
	char *servhost;
	char *servport;
	int bytecnt;

  /* \u83b7\u53d6\u670d\u52a1\u5668\u5730\u5740 */
  sprintf(linkname, "%s/%s", getenv("HOME"), PORTLINK);
  bytecnt = readlink(linkname, linktrgt, MAXNAMELEN);
  if (bytecnt == -1) 
	{
		fprintf(stderr, "error : no active chat server\n");
		return(-1);
	}

	linktrgt[bytecnt] = '\0';

	/* \u83b7\u5f97\u670d\u52a1\u5668 IP \u5730\u5740\u548c\u7aef\u53e3\u53f7 */
	servport = index(linktrgt, ':');
	*servport = '\0';
	servport++;
	servhost = linktrgt;

	/* \u83b7\u5f97\u670d\u52a1\u5668 IP \u5730\u5740\u7684 unsigned short \u5f62\u5f0f */
	unsigned short number = (unsigned short) strtoul(servport, NULL, 0);

	/*
	\u8c03\u7528\u51fd\u6570 socket \u521b\u5efa TCP \u5957\u63a5\u5b57
	*/

	sd = socket(AF_INET, SOCK_STREAM, 0);
	
	/*
	\u8c03\u7528 gethostbyname() \u548c connect()\u8fde\u63a5 'servhost' \u7684 'servport' \u7aef\u53e3
	*/
	struct hostent *hostinfo;
	struct sockaddr_in address;

	hostinfo = gethostbyname(servhost); /* \u5f97\u5230\u670d\u52a1\u5668\u4e3b\u673a\u540d */
	address.sin_addr = *(struct in_addr *) *hostinfo->h_addr_list;
	address.sin_family = AF_INET;
	address.sin_port = htons(number);

  

	if (connect(sd, (struct sockaddr *) &address, sizeof(address)) < 0)
	{
		perror("connecting");
		exit(1);
	}

	/* \u8fde\u63a5\u6210\u529f */
	printf("admin: connected to server on '%s' at '%s'\n",
		servhost, servport);
	return(sd);
}

/* \u4ece\u5185\u6838\u8bfb\u53d6\u4e00\u4e2a\u5957\u63a5\u5b57\u7684\u4fe1\u606f */
int readn(int sd, char *buf, int n)
{
  int     toberead;
  char *  ptr;

  toberead = n;
  ptr = buf;
  while (toberead > 0) {
    int byteread;

    byteread = read(sd, ptr, toberead);
    if (byteread <= 0) {
      if (byteread == -1)
	perror("read");
      return(0);
    }

    toberead -= byteread;
    ptr += byteread;
  }
  return(1);
}

/* \u63a5\u6536\u6570\u636e\u5305 */
Packet *recvpkt(int sd)
{
  Packet *pkt;

  /* \u52a8\u6001\u5206\u914d\u5185\u5b58 */
  pkt = (Packet *) calloc(1, sizeof(Packet));
  if (!pkt) {
    fprintf(stderr, "error : unable to calloc\n");
    return(NULL);
  }

  /* \u8bfb\u53d6\u6d88\u606f\u7c7b\u578b */
  if (!readn(sd, (char *) &pkt->type, sizeof(pkt->type))) {
    free(pkt);
    return(NULL);
  }

  /* \u8bfb\u53d6\u6d88\u606f\u957f\u5ea6 */
  if (!readn(sd, (char *) &pkt->lent, sizeof(pkt->lent))) {
    free(pkt);
    return(NULL);
  }
  pkt->lent = ntohl(pkt->lent);

  /* \u4e3a\u6d88\u606f\u5185\u5bb9\u5206\u914d\u7a7a\u95f4 */
  if (pkt->lent > 0) {
    pkt->text = (char *) malloc(pkt->lent);
    if (!pkt) {
      fprintf(stderr, "error : unable to malloc\n");
      return(NULL);
    }

    /* \u8bfb\u53d6\u6d88\u606f\u6587\u672c */
    if (!readn(sd, pkt->text, pkt->lent)) {
      freepkt(pkt);
      return(NULL);
    }
  }
  return(pkt);
}

/* \u53d1\u9001\u6570\u636e\u5305 */
int sendpkt(int sd, char typ, long len, char *buf)
{
  char tmp[8];
  long siz;

  /* \u628a\u5305\u7684\u7c7b\u578b\u548c\u957f\u5ea6\u5199\u5165\u5957\u63a5\u5b57 */
  bcopy(&typ, tmp, sizeof(typ));
  siz = htonl(len);
  bcopy((char *) &siz, tmp+sizeof(typ), sizeof(len));
  write(sd, tmp, sizeof(typ) + sizeof(len));

  /* \u628a\u6d88\u606f\u6587\u672c\u5199\u5165\u5957\u63a5\u5b57 */
  if (len > 0)
    write(sd, buf, len);
  return(1);
}

/* \u91ca\u653e\u6570\u636e\u5305\u5360\u7528\u7684\u5185\u5b58\u7a7a\u95f4 */
void freepkt(Packet *pkt)
{
  free(pkt->text);
  free(pkt);
}
