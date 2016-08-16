#include <unistd.h> // execlp, fork, close
#include <stdlib.h> // getenv
#include <sys/sendfile.h>
#include <fcntl.h> // open
#include <sys/stat.h>
#include <assert.h>
#include <error.h>
#include <stdio.h> // rename...?
#include <errno.h>
#include <sys/wait.h> // waitpid

#define soname(name) "lib" name ".so"
#define longname1(name) soname(name) ".0"
#define longname(name) soname(name) ".0.0"

struct path {
	int dirfd;
	const char* name;
};

int mystat(struct path p, struct stat* buf) {
	if(p.dirfd != AT_FDCWD) {
		return fstatat(p.dirfd, p.name, buf);
	}
	return stat(p.name,buf);
}

bool need_update(struct path target, struct path dep) {
	struct stat tstat, dstat;
	if(0 != mystat(target, &tstat)) {
		return true;
	}
	if(0 != mystat(dep, &dstat)) {
		error(23,errno,"Couldn't stat a dependency!");
	}
	if(dstat.st_mtime > tstat.st_mtime) return true;
	return false;
}

void build_lib(struct path lib, struct path source) {
	const char* cc = getenv("CC");
	if(cc == NULL) cc = "cc";
	puts("BUILD " lib.name);
	int pid = fork();
	if(pid == 0) {
		execlp(cc,cc,
					 "-Wl,--version-script,systemdsux.version",
					 "-ggdb",
					 "-shared",
					 "-Wl,-soname," longname,
					 "-fpic",
					 "-o",
					 lib.name,
					 source.name,
					 NULL);
		error(23,errno,"wat");
	}
	int status;
	waitpid(pid,&status,0);
	if(!WIFEXITED(status) || 0 != WEXITSTATUS(status))
		exit(status);
}

void build_thingy(struct path lib, struct path source) {
	if(getenv("clean")) {
		unlink(soname) || puts("UNLINK " soname);
		return;
	}

	if(need_update(lib,source,NULL)) {
			build_lib(lib,source);
	}
	struct stat derp;
	assert(0 == mystat(lib,&derp));
}

struct path path(const char* name) {
	path ret = { 0, name };
	return ret;
}

void build_stuff(void) {
	build_thingy(path(longname("systemd")),path("lib.c"));
	build_thingy(path(longname("gudev-1.0")),path("gudev.c"));
}

void install(struct path target, struct path dep) {
	int source;
	if(dep.dirfd != AT_FDCWD) {
		source = openat(dep.dirfd, dep.name, O_RDONLY);
	} else {
		source = open(dep.name, O_RDONLY);
	}
	if(source<0)
		error(3,errno,"couldn't open source dep");
	
	int dest;
	if(target.dirfd != AT_FDCWD) {
		dest = openat(target.dirfd, "temp",O_WRONLY|O_CREAT|O_TRUNC,0755);
	} else {
		dest = open("temp",O_WRONLY|O_CREAT|O_TRUNC,0755);
	}
	if(dest<0)
		error(3,errno,"couldn't open temporary file");

	for(;;) {
		ssize_t amt = sendfile(dest,source,NULL,0x7ffff000);
		if(amt == 0) break;
		if(amt < 0) error(2,errno,"sendfile fail");
	}
	close(source);
	int res;
	if(target.dirfd != AT_FDCWD) {
		res = renameat(target.dirfd,"temp",target.dirfd,target.name);
	} else {
		res = rename("temp",name);
	}
	if(res != 0) {
		error(res,errno,"couldn't rename temporary file to %s",target.name);
	}

	close(dest);
}

int main(void)
{
	if(getenv("clean")) {
#define CLEANONE(name)																							\
		unlink(longname(name)) || puts("UNLINK " longname(name));				\
		unlink(longname1(name)) || puts("UNLINK " longname1(name)); \
		unlink(soname(name)) || puts("UNLINK " soname(name));
		CLEANONE("systemd");
		CLEANONE("gudev-1.0");
		chdir(prefix);
		CLEANONE("systemd");
		CLEANONE("gudev-1.0");
		return 0;
	}

	// drop privs to build if needed
	if(getuid()==0) {
		int builder = fork();
		if(builder == 0) {
			uid_t builder;
			struct stat info;
			assert(0==stat(".",&info));
			if(info.st_uid > 0)
				builder = info.st_uid;
			else
				builder = 0xffff;
			setregid(0xffff,0xffff);
			setreuid(builder,builder);
			build_stuff();
			exit(0);
		}
		waitpid(builder,NULL,0);
	} else {
		build_stuff();
		puts("run this as root to install: prefix=/usr/lib ./make");
		return 0;
	}

	// we're still root
	puts("installing");
	const char* prefix = getenv("prefix");
	if(prefix==NULL)
		prefix = "/usr/lib/";

	int dest = open(prefix,O_RDONLY | O_SEARCH);
	assert(dest >= 0);

#define FUCKIT(NAME)																			\
	struct path target = { AT_FDCWD, longname(NAME) };			\
	struct path source = { AT_FDCWD, NAME ".c" };						\
	build_thingy(target,source);														\
	target.dirfd = dest;																		\
	source.name = longname(NAME);														\
	if(need_update1(target,source)) {												\
		install(target,source);																\
	}																												\
	if(fstatat(target.dirfd,longname1(NAME), &derp) != 0) {	\
		symlinkat(target.name, target.dirfd, longname1(NAME))	\
			|| puts("SYMLINK" longname1(NAME));								\
		symlinkat(target.name, target.dirfd, soname(NAME))				\
			|| puts("SYMLINK" soname(NAME));										\
	}

	FUCKIT("systemd");
	FUCKIT("gudev-1.0");

	return 0;
}
