VTS.getRunListFromDB

./VTS.getRunListFromDB 

Examples: 

VTS.getRunListFromDB -b 2009-09-01 -e 2009-10-01  -s Crab -z 60. -d 1200. -g -x
VTS.getRunListFromDB -l runlist.dat -t -x -v -g
VTS.getRunListFromDB -m laserlist.dat -x -g
VTS.getRunListFromDB -r 48930
VTS.getRunListFromDB -f 48931

Options:

Parameters Required:
	-l	<List of Runs>
		Download Runs from a runlist (ASCII file one column of run numbers)
	-m	<List of Laser Runs>
		Download LASER Runs from a runlist (ASCII file one column of run numbers)
		DO NOT use with -l: Use -l <runlist> followed by -x
	-r	<Run Number>
		Download this run number
	-s	<Source String>
		Download Runs for given source name e.g. Crab
		Wild Cards are allowed using % e.g.: 'TeVJ2032%'
	-b	<YYYY-MM-DD>
		Beginning Date
	-e	<YYY-MM-DD>
		End Date
	-d	<Duration>
		Minimum Duration [seconds]
	-z	<Angle>
		Minimum Zenith Angle [degrees]
	-a	<Anasum Filename>
		Output Anasum file runlist
		No star is added to the beginning of the line unless RUN is marked "good_run" by DQM

Optional Flags:
	-g	Get/Download Runs
		or
	-c	Checks to see if run(s) is(are) on disk
	-n	(With -g or -c) do *not* check md5 sums
	-x	Find Laser Runs - these will be downloaded too
	-t	Output ED style Time Mask
	-f	Also display/download UV Filter and Reduced HV Runs
	-v	Verbose - Output lots of DB information

Note: Using Laser Option extends run time.
      Cannot use Duration, Elevation and Date Cuts with runlist option!
