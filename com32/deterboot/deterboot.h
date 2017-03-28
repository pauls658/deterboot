#include <netinet/in.h>

struct NetInfo
{
  struct in_addr myAddr,
                 bossAddr;
};

int getNetInfo(struct NetInfo *);
