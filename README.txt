evaluation and plotting software for ROOT histograms

format of standard parameter-files is :

FILE /path/to .ending
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
file1 hist1 peakPosition lowFitLimit highFitLimit
 additional options per histogram
additionalFunctionPart startParameter
 the additional function part should be specified as ROOT::TFormula
 addable to the standard gaussian

-drawer: /path/to/file.root histname (thresholdfactor)
 shows specified 2D-histogram (ROOT::TH2 e.g. heat-maps) from file,
 if thresholdfactor is specified both projections (onto X and Y)
 are calculated (average over column or row, respectivly)
 and shown as canvases
 bins, columns and rows deviationg the standard-devation times
 thresholdfactor from mean are written to text-files

-comparator: standard parameter-file ->
 takes 2D-histograms 
 creates difference-histograms for each combination 
 -> stores overview
 calculates bin(/pixel)-wise mean for exclusivly one histogram
 subtracts this mean from bin-value of excluded histogram
 -> saves differences for each histogram separately (in distribution)
 
-correlator: 
 takes two 2D-histograms and 
 plot corresponding bin-values agianst each other
 either textfile with file-names and hist-names has to be specified
<file1> <hist1> (<low1> <high1>)
<file2> <hist2> (<low2> <high2>)
 or commandline values can be given
<file1> <hist1> <file2> <hist2> (<low1> <high1> <low2> <high2>)
 low and high values define correlation-histogram range 
 and which values are plotted in correlation-graph
 
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
 
-superimposer: standard parameter-file (except HIST=GRAPH) ->
 takes ROOT::TGraphs and superimposes them in one plot
 stores result in canvas (.root-file) and as PDF
 required options per graph
file1 graph1 title1
  additional options:
AXIS xAxisTitle yAxisTitle
RANGE xlow xhigh ylow yhigh
LOG logarithmicX logarithmicY 
 (specified by '0' [->false] or '1' [->true], without quotes)

-housekeeper: /path/to/files(.dat) (specifierList)
 stores data from text-files in root tree 
 and plots TGraphs of values with specifier from list
 format of text-files has to be:
unixtime quantity specifier value unit

-textTOtree: parameter-file
 stores columns of textfiles in specified leafs of ROOT:TTree
 format of parameter-file
FILE /path/to specifier .ending
columnIndex1 variableName1 variableType1
columnIndex2 variableName2 variableType2
columnIndex3 variableName3 variableType3
...
 variableType should be 'int' , 'double' , 'string' without quotes
 'string's are stored as ROOT::TString (for postprocessing)

