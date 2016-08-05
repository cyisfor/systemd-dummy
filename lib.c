#include <stdlib.h> // unsetenv
#include <stdio.h>


#define STUB(name) int name() { fputs("systemd: " #name "\n",stderr);
#define END_STUB return 0; }

STUB(sd_notify)
  unsetenv("NOTIFY_SOCKET");
END_STUB

STUB(sd_notifyf)
	unsetenv("NOTIFY_SOCKET");
END_STUB
