evaluation and plotting software for ROOT histograms

format of standard parameter-files is :

FILE </path/to> <.ending>
HIST <prefix> <suffix>
<file1> <hist1>
<file2> <hist2>
<file3> <hist3>
...

(strings or values without <> , uppercase words are specific)
values can be omitted using '%' (without quotes)

-overlayer: standard parameter-file -> 
 takes 1D-histograms (ROOT::TH1 e.g. spectrums) and superimposes them
 additional options:
AXIS <xAxisTitle> <yAxisTitle>
RANGE <xlow> <xhigh> <ylow> <yhigh>
LOG <logarithmicX> <logarithmicY> 
 (specified by '0' [->false] or '1' [->true], without quotes)
SCALEMODE <mode>
 mode can be: 'bare', 'max', 'integral' (default) (all without quotes)
 addtional 'ranged' (added without withspace etc.) can be specified
 in order to enable maximum or integral calculation within [<ylow>,<yhigh>]
PALETTE <number> (inverted)
 number of ROOT color palette which should be chosen for plotting
 optional 'inverted' (high/low color)

-peakFitter: standard parameter-file ->
 takes 1D-histograms and fit specified peaks
 width (sigma and FWHM) and 
 peak-fit-center difference to nominal 
 is stored in ROOT::TGraphs
 required options per histogram
<file1> <hist1> <peakPosition> <lowFitLimit> <highFitLimit>
 additional options per histogram
<additionalFunctionPart> <startParameter>
 the additional function part should be specified as ROOT::TFormula
 addable to the standard gaussian
 <startParameter> should be a list with size of additional parameter

-drawer: /path/to/file.root histname (<lowLimit> <highLimit> <nContours>)
 shows specified 2D-histogram (ROOT::TH2 e.g. heat-maps) from file,
 if <lowLimit> <highLimit> is specified both Z-range is adjusted
 <nContours> specify number of color countours 
 values can be omitted using '%' (without quotes) 
 if only one ore more than three options are given
 plot is not shown but only saved as PDF
 
-projector: /path/to/file.root histname 
 options : <thresholdfactor> / <lowThresh> <highThresh> <nBins> (<skipPlotting>)
 bin-values from specified 2D-histogram (ROOT::TH2 e.g. heat-maps) in the file,
 are shown and histogram is projected
 onto X and Y axis (column and row average, respectivly)
 bins, columns and rows deviating the standard-devation times
 <thresholdfactor> from mean are written to text-files
 or exceeding the <lowThresh> <highThresh> (one or both can be omitted using %)
 if <nBins> is specified bin-value-spectrum is ranged to lowThresh to highThresh

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
 either standard parameter-file 
 with file-names and hist-names has to be specified
<file1> <hist1> (<low1> <high1>)
<file2> <hist2> (<low2> <high2>)
 or commandline values can be given
<file1> <hist1> <file2> <hist2> (<low1> <high1> <low2> <high2>)
 low and high values define correlation-histogram range 
 and which values are plotted in correlation-graph
 additional options:
 FILE , HIST , AXIS , ROWS , COLUMNS : as for plotter
 
-plotter: standard parameter-file ->
 takes 2D-histograms calculates
 mean, standard-deviation, median, minimum, maximum
 of bin-values
 stores results in TGraphs as function of specified values
 and plots combined graphs
 required options per histogram
<file1> <hist1> <value> <error(optional)>
 additional options:
AXIS <xAxisTitle> <yAxisTitle>
RANGE <ylow> <yhigh> / <xlow> <xhigh> <ylow> <yhigh>
 (either only ylow and yhigh or all four)
SKIP <listOfValuesToBeSkipped> <<lowLimit> ><highLimit>
 (low and high limit have to have '<' '>' preceding, respectivly)
ROWS <rowsList>
COLUMNS <columnsList>
 lists to use or to omit during calculation (bin-numbers required)
 lists can either be directly given or specified in textfile (separate lines)
 negativ values will be omitted, positive values will be used 
 (all with same sign required)
 
-superimposer: standard parameter-file (except HIST=GRAPH) ->
 takes ROOT::TGraphs and superimposes them in one plot
 stores result in canvas (.root-file) and as PDF
 required options per graph
<file1> <graph1> <title1>
  additional options:
AXIS <xAxisTitle> <yAxisTitle>
RANGE <xlow> <xhigh> <ylow> <yhigh>
LOG <logarithmicX> <logarithmicY> 
 (specified by '0' [->false] or '1' [->true], without quotes)
NOERRORS
 omits error-bars (during drawing)

-housekeeper: /path/to/files(.dat) (<specifierList>)
 stores data from text-files in root tree 
 and plots TGraphs of values with specifier from list
 format of text-files has to be:
unixtime quantity specifier value unit

-logTOplot: /path/to/file.dat (<timeOffset> <duration> <specifierListFile>)
 stores data from text-files or root-trees in graphs 
 and plots these in canvas against time
 format of text-files has to be the same as for housekeeper
 each row of in the specifier-list-file has to be a specifier
 and its corresponding unit, e.g. :
<specifier1> <unit1>
<specifier2> <unit1>
<specifier3> <unit2>
<specifier4> <unit2>
...
 specifier with the same unit are plotted in the same pad
 (only specifier-unit-combinations are plotted, which are found in data)

-textTOtree: parameter-file
 stores columns of textfiles in specified leafs of ROOT:TTree
 format of parameter-file
FILE </path/to> <specifier> <.ending>
<columnIndex1> <variableName1> <variableType1>
<columnIndex2> <variableName2> <variableType2>
<columnIndex3> <variableName3> <variableType3>
...
 variableType should be 'int' , 'double' , 'string' without quotes
 'string's are stored as ROOT::TString (for postprocessing)

