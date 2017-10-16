# preprocessing:
#  *. grep -e pdf -e png -e jpg *.tex |grep -v % > figure_pdf_jpg_png_no_comment.txt
#  *. aa=`ls`; for i in $aa; do in=`grep $i ../../figure_pdf_jpg_png_no_comment.txt`; if [ ! "$in" == "" ]; then cp $i ../../dissertation/figures/; fi done;
#  *. copy the above line to a file, say tt.txt. run "sh tt.txt" under cygwin. Note, two places to be changed when folder is changed.
#
# usage for this file:
#  *. perl dot.pl paper.tex
#

use strict;

if ($#ARGV < 0) {
    die "USAGE: dot dot_filename;\n" ;
}

my $fn = $ARGV[0];
open (PRP, "<$fn") or die ("Error opening $fn\n $!\n");
open (OUT, ">temp.dot") or die ("Error opening temp.dot\n $!\n");

my $line_no = 1;
my $all_line = 1;
while (<PRP>) {
    my $line = $_;
    chomp $line;
    if ( $line =~ /^%/ ) {

	$line_no ++;	
    } elsif ( $line =~ /^\s\s*%/ ) {
    	$line_no ++;
    } elsif ( $line =~ /iffalse/ ) {
    	print "WARNING: \iffalse is detected!\n";
	print OUT "$line\n";
    } else {
	print OUT "$line\n";
    }
    
    $all_line ++;

}

print "$line_no over $all_line are removed!\n";

close(PRP);
close(OUT);
system ("move", "temp.dot", $fn);

exit;
