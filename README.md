# pdbadchan
Top-level analysis package to study bad channels in protoDUNE.

Bad channels are located by evaluating the RMS for multiple runs and
searching for channels where one or more channels fall below a
threshold of 4 ADC counts. The threshold is lowered to 2 for
cryostat-side wires or when the HV or purity are low.

## Processing data

The first step is to generate graphs of RMS vs. channel for the runs of interest.
Example command to generate these plots may be found in [runjobs.nxt].

The script [listBad.C] may then be used to generate plots of RMS vs. run for each APA plane.
The plots have green bands indicating the range where the central 90% of values for the plane lie.
Channels with any run below threshold are are shown in red if they are in the
current bad channel list and in blue if not.

The list of runs to plot and the label for each are taken from [runs.txt].
Near the end of the log is a table that lists all the channels identified as bad and gives
their ASIC channel identifiers and a pattern string indicating which runs failed.

## Categorizing bad channels.
The configuration file [myChannelStatus.fcl] assigns bad channels to categories whose
labels are in [listBad.C]. Those also appear in the channel table.
The log also indicates how many channels of each type were found.

