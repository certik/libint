# Emacs should use -*- Perl -*- mode.

while ($arg = shift) {
    if ($arg eq "-x") {
        $excluded[$#excluded+1] = shift;
    }
    else {
	$srcdir = $arg;
    }
}

($mydev,$myino,$mymode,$mynlink,$myuid,$mygid,$myrdev,$mysize,
 $myatime,$mymtime,$myctime,$myblksize,$myblocks)
    = stat(".");

if (substr($srcdir,0,1) eq "/") {
    $topdir = "";
}
else {
    $topdir = ".";
}

$stubhead = "# Generated by objectdir.pl -- edit at own risk.\n";


if (-d "$srcdir/src") {
    local($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size,
          $atime,$mtime,$ctime,$blksize,$blocks)
        = stat("$srcdir/src");
    mkdir ("src", $mode);
}
else {
    die "couldn't find source distribution";
}
if (-d "$srcdir/src/bin") {
    local($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size,
          $atime,$mtime,$ctime,$blksize,$blocks)
        = stat("$srcdir/src/bin");
    mkdir ("src/bin", $mode);
}
else {
    die "couldn't find source distribution";
}
if (-d "$srcdir/src/lib") {
    local($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size,
          $atime,$mtime,$ctime,$blksize,$blocks)
        = stat("$srcdir/src/lib");
    mkdir ("src/lib", $mode);
}
else {
    die "couldn't find source distribution";
}

open(MAKEDIRLIST,">Makedirlist");
open(INCDIRLIST,">src/scdirlist.h");

&dodir("$srcdir",".",$topdir);

close(MAKEDIRLIST);
close(INCDIRLIST);

exit;

sub dodir {
    local($dir,$objdir,$topdir) = @_;
    local($file);
    local(@files);

    #print "In directory $dir\n";
    
    opendir(DIR, $dir) || (warn "Can't open $dir: $!\n", return);
    @files = readdir(DIR);
    closedir(DIR);

    foreach $file (@files) {
        if ($file eq "." || $file eq ".." || $file eq "CVS") {
            next;
        }

        local($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size,
         $atime,$mtime,$ctime,$blksize,$blocks)
            = stat("$dir/$file");

        if ($dev == $mydev && $ino == $myino) { next; }

	if (-d "$dir/$file") {
            mkdir ("$objdir/$file", $mode);
            local($nexttop);
            if ($topdir eq ".") {
                $nexttop = "../";
            }
            elsif ($topdir eq "" ) {
                $nexttop = "";
            }
            else {
                $nexttop = "$topdir../";
            }
            &dodir("$dir/$file", "$objdir/$file", $nexttop);
        }
        elsif ("$file" eq "Makefile" && isobjectdirmake("$dir/$file")) {
            #print "Found $dir/Makefile\n";
            local($nextdir);
            &domake("$topdir$dir", "$objdir/$file");
            &doconfigfiles("$objdir");
        }
    }
}

sub isobjectdirmake {
    local($file) = shift;
    open(MAKEFILE,"<$file");
    while (<MAKEFILE>) {
        if (/SRCDIR/) {
            return 1;
        }
    }
    return 0;
}

sub doconfigfiles {
    local($dir) = @_;
    $dir =~ s/\.//g;
    $dir =~ s/\//_/g;
    $dir = uc($dir);
    $dir =~ s/^_//;
    if ($dir ne "" && ! &excluded($dir)) {
        printf MAKEDIRLIST "HAVE_LIBINT_%s=yes\n", $dir;
        printf INCDIRLIST "#define HAVE_LIBINT_%s 1\n", $dir;
    }
}

sub domake {
    local($topdir, $stubmake) = @_;

    if (-f $stubmake) {
        open(STUBMAKE,"<$stubmake");
        local($line) = scalar(<STUBMAKE>);
        close(STUBMAKE);
        if ($line eq $stubhead) {
            print "Overwriting "
        }
        else {
            print "Skipping $stubmake\n";
            return;
        }
    }
    else {
        print "Writing ";
    }
    print "$stubmake\n";

    open(STUBMAKE,">$stubmake");
    print STUBMAKE "$stubhead";
    print STUBMAKE "SRCDIR = $topdir\n";
    print STUBMAKE "VPATH = \$(SRCDIR)\n";
    print STUBMAKE "include \$(SRCDIR)/Makefile\n";
    close(STUBMAKE);
}

sub excluded {
    my $dir = shift;
    my $i;
    foreach $i (0..($#excluded)) {
        if ($dir eq $excluded[$i]) { return 1; }
    }
    return 0;
}
