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

#define soname "libsystemd.so"
#define longname soname ".0.0"

void build_lib(void) {
	if(getenv("clean")) {
		unlink(longname) || puts("UNLINK " longname);
		return;
	}
	const char* cc = getenv("CC");
	if(cc == NULL) cc = "cc";
	puts("BUILD " longname);
	execlp(cc,cc,
				 "-Wl,--version-script,systemdsux.version",
				 "-ggdb",
				 "-shared",
				 "-Wl,-soname," longname,
				 "-fpic",
				 "-o",
				 soname,
				 "lib.c");
}

void build_stuff(void) {
	struct stat target, dep;
	if(0!=stat(soname,&target)) {
		assert(0==stat("lib.c",&dep));
		if(dep.st_mtime > target.st_mtime) {
			build_lib();
		}
	} else {
		build_lib();
	}
}

void install(int src, const char* name) {
	int dest = open("temp",O_WRONLY|O_CREAT|O_TRUNC,0755);
	if(dest<0)
		error(3,errno,"couldn't open temporary file");
	for(;;) {
		ssize_t amt = sendfile(dest,src,NULL,0x7ffff000);
		if(amt == 0) break;
		if(amt < 0) error(2,errno,"sendfile fail");
	}
	close(src);
	if(0!=rename("temp",name))
		error(4,errno,"couldn't rename temporary file to %s",name);;
	close(dest);
}

int main(void)
{
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

	if(getenv("clean")) {
		chdir(prefix);
		unlink(longname) || puts("UNLINK " longname);
		unlink(soname ".0") || puts("UNLINK " soname ".0");
		unlink(soname) || puts("UNLINK " soname);
		return 0;
	}

	struct stat target, dep;
	if(0!=stat(soname,&dep)) {
		error(1,0,"couldn't build " soname);
	}

	// it's either open this, or open the directory it's in,
	// disk read either way
	int src = open(soname,O_RDONLY);
	assert(src > -1);

	chdir(prefix);

	if(stat(longname,&target) == 0) {
		if(dep.st_mtime > target.st_mtime) {
			puts("INSTALL " longname);
			install(src, longname);
		}
	} else {
		puts("INSTALL " longname);
		install(src, longname);
	}

	if(stat(soname ".0",&target) != 0) {
		symlink(longname, soname ".0") || puts("SYMLINK " soname ".0");
	}
	if(stat(soname,&target) != 0) {
		symlink(longname, soname) || puts("SYMLINK " soname);;
	}
	return 0;
}
