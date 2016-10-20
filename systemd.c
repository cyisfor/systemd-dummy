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

STUB(sd_listen_fds)
END_STUB

int sd_is_socket() {
  return 0;
}

struct log_dest {
	int out;
	const char* identifier;
	ssize_t len;
};

struct log_dests {
	struct log_dest* d;
	ssize_t n;
};

int sd_journal_stream_fd(const char* identifier, int priority, int level_prefix) {
	return 2;
}
