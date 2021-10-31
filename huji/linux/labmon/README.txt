LABMon v. 1.0 by Schlomo Schapiro (huji at schlomo dot schapiro dot org)

This is a perl script which scans ranges of IP addresses to find which
computers are online and in use. It relies on a FINGER daemon running on
each computer and reporing the logged in user. If no user is logged in, then
the finger daemon should NOT run (e.g. if the computer answers pings but not
finger requests then it is online but not in use).

The aim is to create usage statistics for the system administrators and a 
web-based tool for users to find and locate available computers

About computers that answer neither ping nor finger we can't say much so
the script assumes that they are offline (switched off), meaning also 
available (you can set the total of computers in each range if need and if
it is more than IP addresses in the range).

The information returned by finger is matched against REGEX patterns to
extract information about the computer which is then shown on the resulting
webpages (e.g. user, load, type, ...).

If you want to include some non-standard information you have to just write
your own finger daemon (which is not complicated at all) to return whatever
information you want to have returned.

All computers are scanned sequentially with an option to give a minimum time
between 2 consecutive checks of a computer (in order not to produce too much
network load or trafick).

LABMon primarily creates web-orientated output. It builds a structure of
directories and files that represent the ranges (called "net" or "class")
and the computers in them with the information gathered about them. Since this
information is not neccessarily considered public, LABMon in addition creates
PNG pictures of the free/total number for each class. It can also patch an
existing PNG file with a map of the class to reflect in a graphical manner the
status of computers in the class.

The latter part (the PNG pictures) are intended to be included in some kind
of information pages about the computer labs, with only statistics per class
or also with a detailed map where to find an available computer.

In addition LABMon can also create a backlog of statistics, organized by year,
month and day. You can then create aggregates of this data and show nice graphs
to the managers about computer usage (The backlog evaluation and statistical 
manipulation part is not yet implemented, but if s.b. will write some nice 
module which creates web-based statistics pages like for a web-server I will
be more than glad to include it in the LABMon distribution).


Requirements:
-------------
* Perl (I use 5.6, did not test it on older versions) on Unix or Win32/Cygwin 
(see below about Cygwin note)
* GD library
* A webserver to serve out the results
* diskspace if you want to create a backlog. I get about 200k per day for 140
computers which are scanned once per minute.

Installation:
-------------
- Open the archive where you want to install the program. In the etc/ subdir
is the configuration file and PNG source files, in the lib/ subdir is a 
library needed by the script.

- Edit etc/labmon.conf to suit your needs, it has an explanation about
each paramter and you can look at my examples.

- Run labmon so that it will run all the time. For example I run it from
inittab like this:
LAB:23:respawn:/bin/bash -c "cd /usr/local/labmon ; exec ./labmon 2>>/tmp/labmon.log >>/tmp/labmon.log"

labmon will run significantly faster as root because then it can use ICMP 
pings instead of TCP pings.

labmon will monitor its configuration file and reload the configuration if
the configuration file changes (you can play with the debug option in the
file to get debug info for some time (or only for the configuration parsing
part) and then disable it and it will reload the config and stop printing 
debugging info.

- The included mkgraphs script is my unsuccessfl try at writing something 
that interprets the backlog. I ran out of time and if you find it useful, it's
yours.

- Configure your webserver to serve out the pages created by labmon.


Cygwin Note:
------------
The script makes use of alarm() for timeouts and ping. In order to do fast
ICMP pings it has to be run as root. I did not manage to let it run as root
under Windows, I don't know why. In general performance was much less on
Windows.

You have to use a Perl compiled with SIGNAL support. The ActiveState version
does not include the alarm() function, that is why I had to compile my own
perl under Cygwin (and you have to tell it to include SIGNAL support !).

So much for Windows ...

Schlomo Schapiro
