# I hate myself...

my @names = ("systemd","gudev-1.0");

open(FUCKIT,"make.fuckit.c");
sysread(my $fuckit, FUCKIT, 0x1000);

my $code = "";
for my $name (@names) {
		my $soname = "lib$name.so";
		my $longname1 = "lib$name.so.0";
		my $longname = "lib$name.so.0.0";
		$_ = $fuckit;
		s/$NAME/$name/xg;
		s/$SONAME/$soname/xg;
		s/$LONGNAME/$longname/xg;
		s/$LONGNAME1/$longname1/xg;
		$code = $code . $_;
}

sysread($_, STDIN, 0x1000);
s/$FUCKIT/$fuckit/xg;

for my $name (@names) {
		my $soname = "lib$name.so";
		my $longname1 = "lib$name.so.0";
		my $longname = "lib$name.so.0.0";
while(<>) {
		s/
