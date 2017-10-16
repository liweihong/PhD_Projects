#print "It matches \n" if "hello, world !" =~ /world/;

open INFO, "report.tex" or die "Error: NO report.tex found!\n";
open BIB, ">report.bib" or die "Error: Could not open file test.bib!\n";

my $start=0;
my $counter=0;
while (<INFO>) {   # file not eof
 my($line) = $_;   # current line
 chomp($line);     # remove \newline
 if ( $line =~ /bibitem\{(.*)\}/ ) {   # search for bibitem{....}
	if ($start) {
        	print BIB "}\n\n";
	} else {
		$start=1;
	}
	print BIB "\@ARTICLE{$1,\n";
	$counter ++;
 }
 elsif ( $line =~ /bbauth\{(.*)\}/ ) {
#	print BIB "    author = {$1},\n";
	my $news = join(' and', split(/,/, $1));   # split the name, replace "," with "and"
	print BIB "    author = {$news},\n";
 }
 elsif ( $line =~ /bbtitl\{(.*)\}/ ) {
	print BIB "    title = {$1},\n";
 }
 elsif ( $line =~ /bbjour\{(.*)\}/ ) {
	print BIB "    journal = {$1},\n";
 }
 elsif ( $line =~ /%\\bibitem\{/ ) {
	print "Found $line, quitting...\n";
	print BIB "}\n";

	print "Total Bib: $counter\n";
	close INFO;
	close BIB;
	exit 0;
 }
 elsif ( $start && $line =~ /\d/) {  # search for digit
	$line =~ s/^%//;             # remove the leading "%"
	$line =~ s/^\s+//;           # remove the leading " "
	foreach $item (split(/,/, $line)) {            # get items seperated by ","
	    if ( $item =~ /Vol/ || $item =~ /vol/ ) {
		$item =~ /(\d+)/;                      # get the volume #
		print BIB "    volume = {$1},\n";
	    }
	    elsif ( $item =~ /pp/ ) {                  # get the page #
		$item =~ /(\d+)\s-\s(\d+)/;
		print BIB "    pages = {$1--$2},\n";
	    }
	    elsif ( $item =~ /20/ || $item =~ /19/ ) {  # get the year
		$item =~ /(\d+)/;
		print BIB "    year = {$1},\n";
	    }
	    
	}
 }

}

