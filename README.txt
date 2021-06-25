evaluation and plotting software for ROOT histograms

format of standard parameter-files is :

FILE /path/to .eding
HIST prefix suffix
file1 hist1
file2 hist2
file3 hist3
...

values can be omitted using '%' (without quotes)

-overlayer: standard parameter-file -> 
 takes 1D-histograms (ROOT::TH1 e.g. spectrums) and superimposes them
 additional options:
AXIS xAxisTitle yAxisTitle
RANGE xlow xhigh ylow yhigh
LOG logarithmicX logarithmicY 
 (specified by '0' [->false] or '1' [->true], without quotes)

-peakFitter: standard parameter-file ->
 takes 1D-histograms and fit specified peaks
 width (sigma and FWHM) and 
 peak-fit-center difference to nominal 
 is stored in ROOT::TGraphs
 required options per histogram
file1 hist1 peakPosition lowRangeFit highRangeFit
 additional options per histogram
additionalFunctionPart startParameter

-drawer: /path/to/file.root histname
 shows specified 2D-histogram (ROOT::TH2 e.g. heat-maps) from file

-comparator: standard parameter-file ->
 takes 2D-histograms 
 creates difference-histograms for each combination 
 -> stores overview
 calculates bin(/pixel)-wise mean for exclusivly one histogram
 subtracts this mean from bin-value of excluded histogram
 -> saves differences for each histogram separately (in distribution)

-plotter: standard parameter-file ->
 takes 2D-histograms calculates
 mean, standard-deviation, median, minimum, maximum
 of bin-values
 stores results in TGraphs as function of specified values
 and plots combined graphs
 required options per histogram
file1 hist1 value error(optional)
 additional options:
RANGE ylow yhigh
SKIP listOfValuesToBeSkipped <lowLimit >highLimit
 (low and high limit have to have '<' '>' preceding, respectivly)

-housekeeper: /path/to/files(.dat) (specifierList)
 stores data from text-files in root tree 
 and plots TGraphs of values with specifier from list
 format of text-files has to be:
unixtime quantity specifier value unit
