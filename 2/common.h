/*--------------------------------------------------------------------*/
/* \u670d\u52a1\u5668\u7aef\u53e3\u4fe1\u606f */
#define PORTLINK ".chatport"

/* \u7f13\u5b58\u9650\u5236 */
#define MAXNAMELEN 256
#define MAXPKTLEN  2048

/* \u4fe1\u606f\u7c7b\u578b\u7684\u5b9a\u4e49 */
#define LIST_GROUPS    0
#define JOIN_GROUP     1
#define LEAVE_GROUP    2
#define USER_TEXT      3
#define JOIN_REJECTED  4
#define JOIN_ACCEPTED  5

/* \u6570\u636e\u5305\u7ed3\u6784 */
typedef struct _packet {

  /* \u6570\u636e\u5305\u7c7b\u578b */
  char      type;

  /* \u6570\u636e\u5305\u5185\u5bb9\u957f\u5ea6 */
  long      lent;

  /* \u6570\u636e\u5305\u5185\u5bb9 */
  char *    text;

} Packet;

extern int startserver();
extern int locateserver();

extern Packet *recvpkt(int sd);
extern int sendpkt(int sd, char typ, long len, char *buf);
extern void freepkt(Packet *msg);
/*--------------------------------------------------------------------*/
