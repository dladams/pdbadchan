# pdbadchan
Top-level analysis package to study bad channels in protoDUNE.

Bad channels are located by evaluating the RMS for multiple runs and
searching for channels where one or more channels fall below a
threshold of 4 ADC counts. The threshold is lowered to 2 for
cryostat-side wires or when the HV or purity are low.

## Processing data

The first step is to generate graphs of RMS vs. channel for the runs of interest.
Example command to generate these plots may be found in [runjobs.nxt](runjobs.nxt).

The script [listBad.C](listBad.C) may then be used to generate plots of RMS vs. run for each APA plane.
The list of runs to plot and the label for each are taken from [runs.txt](runs.txt).
The plots have green bands indicating the range where the central 90% of values for the plane lie.
Channels with any run below threshold are are shown in these plots.
One file is produced for each APA with a separate plot for each APA plane.
Near the end of the standard output of the script is a table that lists all the channels identified
as bad along with their ASIC channel identifiers, a pattern string indicating which runs failed, and
a label indicating a category (seel below) for the channel. 
This is followed by table indicating the number of channels falling in each category.

## Categorizing bad channels.
The category assignments are obtained with an index map tool with configuration in
[myChannelStatus.fcl](myChannelStatus.fcl).
The labels for the categories are in the listBad fundction in [listBad.C](listBad.C).
The assignments were made by examining the plots and log tables described above.

## Results

Resuts obtained from a survey pereformed in October 2020 were presented at the DUNE CE
consortium meeting [October 19, 2020](https://indico.fnal.gov/event/46063).
