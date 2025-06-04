package lib::Config;
use Exporter;
@ISA	= qw(Exporter);
@EXPORT = qw(%CONFIG $debug %COUNTERS 
		@COUNTERS $refreshtime $lastmodpic $backlog
		$EOL $BLANK $confdir $conffile $remoteport 
		$delay
		&rindexa &dodebug &readconfig);
$version="2.0";
$confdir="etc/";
$conffile=$confdir . "labmon.conf";
$remoteport="finger(79)";
$backlog="0";
$logfile="";
$delay=0;

# Define configuration templates
%THTML = (					# HTML snippets
		'TITLE'		=> '',		# What to put inbetween TITLE tags
		'HEAD'		=> '',		# What to put between HEAD tags
		'BODY'		=> '',		# What to put between BODY tags after H1 and before our stuff
		'ADDRESS'	=> 		# What to put between ADDRESS tags at the end
			"<HR>LabMon $version by Schlomo Schapiro",
	);

%THTMLTAG = (					# HTML TAG attributes
		'HTML'		=> 'DIR=LTR',	# default to LTR documents
	);	
%TGROUP = (
		'outdir'	=> './out/',
		'gfxoutdir'	=> './out/gfx/',
		'numgfxoutdir'	=> './out/numgfx/',
		'urlgfx'	=> '/gfx/',
		'backlog'	=> 0,		# Create a backlog for this group
		'logdir'	=> './log/',	# Where to create the backlog
		'puipinpix'	=> 0,		# put the IP address under the picture in picpatch
		'HTML'		=> { %THTML },	# HTML bits for HTML files in this group
		'HTMLTAG'	=> { %THTMLTAG },	# HTM
		'DESCRIPTION'	=> 'Default',		# A longish description of this group
		'HOSTS'		=> [],		# list of hosts to query
		'numcomps'	=> 0,		# How many real computers are in this group. If there are more addresses in hosts that real computers, this is used to calculate the amount of offline computers 
		'MAP'		=> '',		# Use this map for picpatch
		'PICPATCH'	=> '',		# Picture to patch, relative path to conf dir
		'INFOS'		=> [],		# Collect these infos, e.g. apply these patterns to the output
		'numpic'	=> 0,		# Wether to create a numerical picture for this group
	);
	
# Define Configuration hash
%TCONFIG = (	'DEFAULT'	=> { %TGROUP },	# Default group configuration
		'GROUPS'	=> [ ],		# List of TGROUP Groups
	# Global options
		'GLOBAL'	=> {	'debug'	=> 0,		# Write debugging info to STDOUT
					'HTML'	=> { %THTML },	# Global HTML data
					},
	# Pattern definitions. should be a Perl m// regex pattern. use ONE pair of () to denote 
	# the information to extract. No need for () if the pattern will be used only for counters
		'PATTERNS'	=> {	free => "FrEe" ,
					busy => "BuSy",
					offline => "OfFlInE",
					},
	# Color definitions in HTML #RGB format
		'COLORS'	=> {	free => "00FF00",	# free computers in picpatch
					busy => "FF0000", 	# busy computers in picpatch
					offline => "000000",	# offline computers
					text=> "0000FF",	# Texts (e.g. IP address)
					numback=> "000000",	# background in numerical pics
					numfree=> "FFF000",	# free computers in numerical pics
					numtotal=> "000FFF",	# total computers in numerical pics
					},
	# Short names for objects (groups, colors, patterns, etc.). Will be used for displaying
	# to the user as table headers and in table fields.
		'NAMES'		=> {	free => "free",
					busy => "busy",
					offline => "offline",
					},
	);
					

#@EXPORT_OK = qw(%C &add_log_file);
# %NETS keeps the named list of networks and names to work on
# @NETS keeps the order of the networks list
# %NAMES is a dictionary between short name and display name
# $debug is 1 for debugging output to file
# %PATTERNS is a named list of patterns
# @INFOS is a list of pattern names to use as info to be extracted from the finger response
# $COUNTERS is a list of counters, includes the builtins named busy, free, offline
#		offline means the WS doesn't answer pings, 
#		free means we can't connect to it but it answers pings
#		busy means that we connected and read some data from it.
# 		The counters are initialized with some default patterns
# @COUNTERS keeps the order of the counters listed. First are free,offline,busy
# $refreshtime is the time after which the browser should reload the page, adapts itself automatically
# to the time the script needs the pass all networks
# @MAPS is a hash of IP<->coordinates map to know where to put the markings for busy/free/unknown
# @COLORS is a hash of colors in HTML #RGB format
# @PICPATCH is a hash of picture patches to be done for classes, lists the picture to be taken for each
# class. The pictures should be relative to the conf dir
# $putipinpix tells wether to put the IP number in the network picture
# %NUMCOMPS keeps the real number of computers in a class (if the IP range is larger than the actual amount of
# computers
# %NUMPIC keeps the names of files and networks for that a picture with the numerical result should
# be created
# $lastmodpic if yes, creates a file called lastmod.png with the current time in it at the end of each cycle
# $backlog if yes, create log of status.
# under $logdir create dirs for years/month and files for each day. In each file write one line
# for each network scanned of the format time\tfree\tbusy\toffline\t[customcounter=data\t...].
# The user has to make sure to remove old data
# $delay is the minimum amount of seconds that have to be between two visits to the same net, meaning
# that if labmon passes all nets faster than $delay it will sleep the remaining seconds of $delay to
# ensure that a new run is started every $delay seconds
# $graphwidth,$graphheight are the dimensions of the graphs to create

sub dodebug 
# Deal with debug message @, joining by space
{
	print STDERR "Debug: " . join(" ",@_) . "\n" ;
}


# config file structure
%CONFIGFILE = (
	'SECTION' => {
		'DEFAULT' => ""
		}
	);
	
sub readconfig
# Initializes all variables, reads in the config file. It is save to call this from within when
# the config file has changed
{
	undef %CONFIG if (defined %CONFIG);
	%CONFIG=%TCONFIG;
	$debug = $CONFIG{'GLOBAL'}{'debug'};
	$refreshtime = 5 ;
	$lastmodpic = 0 ;
	open CONF,"<$conffile" || die "Could not open config file $conffile: $@\n";
	my $line=0;
	my $sectiontype="";
	my $sectionname="none";
# @sectionstack is an array of anonymous hashes with name and type as keys
	my @sectionstack=();
	my %section=();
FILE:	while (<CONF>)		# Mark this while so that we can jump to it with next
	{
		chomp;
		$line++;
# If a line starts with a # or contains only whitespace, skip this line
		next if (m/^\s*$/ or m/^\s*#/);
# Look for </sectiontype> stuff
		if (m/^\s*<\/(\w+)>\s*$/)
		{
			die "Section of type $1 is not the last opened ($sectiontype) section at line $line !"
				unless ($1 eq $sectiontype);
			pop(@sectionstack);
			&dodebug("Closing section $sectiontype/$sectionname");
			%section = %{$sectionstack[-1]};
			$sectionname=$section{'name'};
			$sectiontype=$section{'type'};
		}
# Look for <sectiontype sectionname> stuff
		elsif (m/^\s*<(\w+)\s+(\w+)>\s*$/)
		{
			die "Section of type $1 can't be contained in another section of the same type at line $line !" 
				unless (rindexa($1,@sectionstack) == -1);
			$sectiontype=$1;
			$sectionname=$2;
			&dodebug("Opening section $sectiontype/$sectionname");
			push(@sectionstack,{'type'=>$sectiontype,'name'=>$sectionname});
		}
# Look for <sectiontype> stuff
		elsif (m/^\s*<(\w+)>\s*$/)
		{
			die "Section of type $1 can't be contained in another section of the same type at line $line !" 
				unless (rindexa($1,@sectionstack) == -1);
			$sectiontype=$1;
# We leave section name as it is. So it will be taken from an outerlying section, if exists.
			&dodebug("Opening section $sectiontype/$sectionname");
			push(@sectionstack,{'type'=>$sectiontype,'name'=>$sectionname});
		}
# separate each line into command and parameters, white space allowed. There are no commands without parameters.
# this should also filter out any leading or trailing whitespace
		($command,$parms) = m/^\s*(\w+)\s+(.*\S)\s*$/;
# parse commands

# deal first with commands that can be in any section and even outside of sections
		if ($command eq "debug")
# enable debugging output. The debug command can be used in any section.
 		{ 
			$debug = $parms;
			if ($debug)
			{
				&dodebug("Enabled Debugging");
			}
			else
			{
				&dodebug("Disabled Debugging");
			}
			next FILE;
		}
# Now deal with stuff that has to be enclosed by a section		
		if ($sectiontype == "group")
		{}
		elsif ($sectiontype == "html")
		{}
		elsif ($sectiontype == "htmltag")
		{}
		elsif ($sectiontype == "pattern")
		{}
		elsif ($sectiontype == "color")
		{}
		elsif ($sectiontype == "name")
		{}
		else
		{
			&dodebug("Warning: Unknown section encountered at line $line !");
		}
		
		
		
		
		
		next; # skip old stuff below
		
		
		
		
		if ($command eq "net")
# Define a new network
# net <name> <address range>
# - name should fit \w+
# - addtess range should be of type a.b.c.d-e (only class C's are supported right now)
		{
			my ($name,$iprange) = $parms =~ m/^(\w+)\s+(\d+\.\d+\.\d+\.\d+-\d+)/;
			die "Invalid net command in $conffile line $line: $parms\n" .
			    "Syntax should be net <name> <a.b.c.d-e>\n" unless ($name and $iprange);
			$NETS{$name}=$iprange;
			push @NETS,$name;
# Add default display name, can be changed by a later name command
			$NAMES{$name}=$name;
			&dodebug("Added net $name = $iprange from $conffile line $line") if ($debug);
		}
		elsif ($command eq "name")
# Define a new display name
# name <name> <Display Name>
# - name should fit \w+
# - Display Name is any text
		{
			my ($name,$display) = $parms =~ m/^(\w+)\s+(\S.*)$/;
			die "Invalid name command in $conffile line $line: $parms\n" .
			    "Syntax should be name <name> <Display Name>" unless ($name and ($display ne ""));
#			$display =~ s/"/\\"/g;
#			print "\$display=\"$display\"\n";
#			eval  "\$display=\"$display\"";
			$NAMES{$name}=$display;
			&dodebug("Added name $name = $display from $conffile line $line") if ($debug);
		}
		elsif ($command eq "numcomps")
# Define a new real amount of computers
# name <name> <Amount of computers>
# - name should fit \w+
# - Amount is a natural number
		{
			my ($name,$number) = $parms =~ m/^(\w+)\s+(\d+).*$/;
			die "Invalid numcomps command in $conffile line $line: $parms\n" .
			    "Syntax should be numcomps <name> <Amount of computers>" unless ($name and ($number ne ""));
			$NUMCOMPS{$name}=$number;
			&dodebug("Added numcomps $name = $number from $conffile line $line") if ($debug);
		}
		elsif ($command eq "numpic")
# Define a new numerical picture for a class
# name <name> <filename>
# - name should fit \w+
# - Filename is the name of the PNG file that will be created in $numgfxoutdir
		{
			my ($name,$file) = $parms =~ m/^(\w+)\s+(\S.*)$/;
			die "Invalid numpic command in $conffile line $line: $parms\n" .
			    "Syntax should be numpic <name> <Filename>" unless ($name and ($file ne ""));
			$NUMPIC{$name}=$file;
			&dodebug("Added numpic $name = $file from $conffile line $line") if ($debug);
		}
		elsif ($command eq "pattern")
# Define a new pattern
# pattern <name> <perl-regex pattern>
# - name should fit \w+
# - pattern should not start with a whitespace
		{
			my ($name,$pattern) = $parms =~ m/^(\w+)\s+(.*)$/;
			die "Invalid pattern command in $conffile line $line: $parms\n" . 
			    "Syntax should be pattern <name> <perl-regex pattern>\n" . 
			    "See the perlre man page for details\n" .
			    "The text to extract has to be enclosed by ()\n" unless ($name and $pattern);
			$PATTERNS{$name}=$pattern;
			$NAMES{$name}=$name;
			&dodebug("Added pattern $name = $pattern from $conffile line $line") if ($debug);
		}
		elsif ($command eq "counter")
# Define a new counter, increases if the pattern is found in the response
# pattern <name>
# - name should fit \w+
		{
			my ($name) = $parms =~ m/^(\w+)$/;
			die "Invalid counter command in $conffile line $line: $parms\n" . 
			    "Syntax should be counter <name>\n" . 
			    "Where name is also the name of a pattern that trigger the counter\n".
			    "The counter counts in how many hosts the pattern matched in the reponse\n" unless ($name);
			$COUNTERS{$name}=0;
			push @COUNTERS,$name;
			$NAMES{$name}=$name;
			&dodebug("Added counter $name from $conffile line $line") if ($debug);
		}
		elsif ($command eq "color")
# Define a color,
# color <name> <HTML-style RGB value>
# - name should fit \w+
# - color is like AABBCC for RGB
		{
			my ($name,$color) = $parms =~ m/^(\w+)\s+(.*)$/;
			die "Invalid color command in $conffile line $line: $parms\n" . 
			    "Syntax should be color <name> <HTML-style RGB value\n" . 
			    "Where name is also the name a color (busy, free, offline are predefined\n".
			    "" unless ($name and $color);
			$COLORS{$name}=$color;
			$NAMES{$name}=$name;
			&dodebug("Added color $name = $color from $conffile line $line") if ($debug);
		}
		elsif ($command eq "map")
# Define a new map that translates between IP and coordinates in a picture. The coordinates are floodfilled
# with a color
# map <name> <filename>
# - name should fit \w+
# - filename is relative to conf-dir
# fileformat:
# 192.168.1.2	20 30
		{
			my ($name,$filename) = $parms =~ m/^(\w+)\s+(.*)$/;
			die "Invalid map command in $conffile line $line: $parms\n" . 
			    "Syntax should be map <name> <filename>\n" . 
			    "Where name is also the name of a net and filename is a map file relative to\n".
			    "the configuration file\n" unless ($name and $filename);
			$MAPS{$name}=$filename;
			&dodebug("Added map $name = $filename from $conffile line $line") if ($debug);
		}
		elsif ($command eq "picpatch")
# Define a new picture to be patched for a network
# picpatch <name> <filename>
# - name should fit \w+
# - filename is relative to conf-dir
		{
			my ($name,$filename) = $parms =~ m/^(\w+)\s+(.*)$/;
			die "Invalid picpatch command in $conffile line $line: $parms\n" . 
			    "Syntax should be map <name> <filename>\n" . 
			    "Where name is also the name of a net and filename is a PNG/JPEG/BMP file relative to\n".
			    "the configuration file\n" unless ($name and $filename);
			$PICPATCH{$name}=$filename;
			&dodebug("Added picpatch $name = $filename from $conffile line $line") if ($debug);
		}
		elsif ($command eq "info")
# Add a new info item to be extraced, named by a pattern
# info <name>
# - name should fit \w+
		{
			my ($name) = $parms =~ m/^(\w+)$/;
			die "Invalid info command in $conffile line $line: $parms\n" .
			    "Syntax should be info <name>\n" .
			    "name is a named pattern\n" unless ($name);
			push @INFOS,$name;
			&dodebug("Added info $name from $conffile line $line") if ($debug);
		}
		elsif ($command eq "option")
# Change an option. Options are all scalar parameters used in the program
# option <name> <Value>
# - name should fit \w+
# - value can be anything
		{
			my ($name,$value) = $parms =~ m/^(\w+)\s+(.*)$/;
			die "Invalid option command in $conffile line $line: $parms\n" .
			    "Syntax should be option <name> <value>\n".
			    "Where name is the name of an option and the value it\'s value\n" unless ($name and $value);
			eval '$' . $name . '=\'' . $value . '\';' ;
			&dodebug("Set option $name = ${$name} from $conffile line $line") if ($debug);
		}
		else
# Warn about unknown things in the config file
		{
			&dodebug("Warning: Unknown command \"$command\" found in $conffile line $line: [$_]") if ($debug);
		}
	} # while <CONF>
	close CONF;
	
	
	
} # readconfig

# Return the rightmost index of $_[0] in an array or -1 if not found
sub rindexa
{
        while ($_[0] ne pop) {};
        return $#_;
}


# some really global (= neverchanging) stuff here
# find out some stuff by my own, set some constants
$EOL = "\015\012";
$BLANK = $EOL x 2;

return 1;
