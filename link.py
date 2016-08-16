#!/usr/bin/env python
import os,sys
cc=os.environ["CC"];
target,source = sys.argv[1:]
def myexec(*a):
	print(' '.join(a))
	os.execvp(a[0],a)
myexec(cc,
     "-Wl,--version-script,systemdsux.version",
     "-ggdb",
     "-shared",
     "-Wl,-soname,longname,"+target,
     "-fpic",
     "-o",target,source)


