#include "stub.h"

#include <stdlib.h> // unsetenv
#include <stdio.h>


STUB(sd_notify)
  unsetenv("NOTIFY_SOCKET");
END_STUB

STUB(sd_notifyf)
	unsetenv("NOTIFY_SOCKET");
END_STUB
	
int sd_booted(void) {
	return 0;
}

STUB(sd_journal_send_with_location)
END_STUB
