struct dest {
	int out;
	const char* identifier;
	ssize_t len;
};

struct dests {
	struct dest* d;
	ssize_t n;
};

struct dests dests[0x100] = {};

struct sourceinfo {
	const char* identifier;
	ssize_t len;
	int priority;
};

struct growbuf {
	char* base;
	ssize_t len;
	ssize_t space;
};

int efd = -1;

void* transmogrifier(void* arg) {
	char buf[0x100];
	snprintf(buf,0x100,"%s/.local/systemd-dummy.log",getpwuid(getuid).pw_dir);
	FILE* logfd = fopen(buf,"at");
	assert(logfd > 0);
	struct epoll_event events[0x10];
	for(;;) {
		int n = epoll_wait(efd, events, 0x10, -1);
		if(n < 0) {
			error(0,errno,"transmogrifier epoll error");
		}
		struct growbuf ibuf = {};
		for(i=0;i<n;++i) {
			// check if it's EPOLLIN?
			// this must be multi-threaded...
			struct sourceinfo* info = (struct sourceinfo*)events[i].data.ptr;
			char buf[0x100];
			for(;;) {
				ssize_t amt = read(events[i].data.fd,&buf,0x100);
				if(amt < 0) {
					if(errno == EAGAIN) break;
					error(0,errno,"transmogrifier read error %s:%d",info->identifier,info->priority);
				}
				buf[amt] = '\0';
				fprintf(logfd, "%d ", info->priority);
				fwrite(info->identifier, info->len, 1, logfd);
				fputc(' ', logfd);
				// >0x100 messages will wrap
				fwrite(buf, amt, 1, logfd);
				fputc('\n',logfd);
				fflush(logfd);
			}
		}
	}
}

void add_source(int fd, const char* identifier, int priority) {
	int need_thread = 0;
	if(efd == -1) {
		efd = epoll_create1(EPOLL_CLOEXEC);
		need_thread = 1;
	}

	struct sourceinfo* info = malloc(sizeof(struct sourceinfo));
	struct epoll_event ev = {
		events = EPOLLIN | EPOLLET,
		data = {
			ptr = info,
			fd = fd
		}
	};

	int res = epoll_ctl(efd, EPOLL_CTL_ADD, fd, &ev);
	assert(res == 0);
	
	if(need_thread) {
		pthread_t thread;
		int res = pthread_create(&thread, &attr, transmogrifier, NULL);
		assert(res >= 0);
		pthread_detach(thread);
	}
}
	
	

int sd_journal_stream_fd(const char* identifier, int priority, int level_prefix) {
	ssize_t len = strlen(identifier);
	int i;
	for(i=0;i<dests[priority].n;++i) {
		if(len != dests[priority].d->len) continue;
		if(0!=memcmp(dests[priority].d->identifier,identifier,len)) continue;
		return dests[priority].d.out;
	}
	int io[2];
	pipe(io);
	dests[priority].d = realloc(dests[priority].d,sizeof(struct dest)*(++dests[priority].n));
	struct dest* dest = dests[priority].d + dests[priority].n - 1;
	dest->out = io[1];
	dest->identifier = malloc(len);
	memcpy(dest->identifier,identifier,len);
	dest->len = len;
	add_source(io[0], identifier, priority);
	return 2;
}
