///////////////////////////////////////////////////////////////////////////////
evaluation and plotting software using CERN's ROOT C++ library 
///////////////////////////////////////////////////////////////////////////////

compilation using make
requires g++ with c++11 and active ROOT environment
(https://root.cern.ch/)

///////////////////////////////////////////////////////////////////////////////

after successful compilation several commandline programs are available

comparator
correlator
differentiator
drawer
grapher
housekeeper
logTOplot
overlayer
peakFitter
plotter
projector
superimposer
textTOtree

most of these require parameter-files as argument

///////////////////////////////////////////////////////////////////////////////

format of standard-parameter-files is :

FILE </path/to> <.ending>
HIST <prefix> <suffix>
<file1> <hist1>
<file2> <hist2>
<file3> <hist3>
...

uppercase words are specific
strings or values without <> 
values can be omitted using '%' (without quotes)

///////////////////////////////////////////////////////////////////////////////

comparator

 arguments :
 standard-parameter-file (<skipPlotting>)
 
 output :
 root-file containing histograms , PDF
 
 takes 2D-histograms (ROOT::TH2 e.g. heatmaps)
 creates difference-histograms for each combination and stores overview
 calculates bin(/pixel)-wise mean for exclusivly one histogram
 subtracts this mean from bin-value of excluded histogram
 saves differences for each histogram separately (in distribution)
 
 additional options in parameter-file :
 for each histogram
<file1> <hist1> <identifier1>
 identifiers are used on X-axis and for map-differences-names 
LABELOPTIONS <mode>
 see description at plotter
RANGE <type> <nbins> <low> <high>
 specifies the number of bins (<nbins>) and range (frome <low> to <high>)
 of the Y-axis for histograms of <type>
 "difference_relative" , "difference_absolute" ,
 "variation_relative"  , "variation_absolute"
  or "spectra" (all without quotes)

///////////////////////////////////////////////////////////////////////////////
 
correlator 

 arguments :
 <file1> <hist1> <file2> <hist2> (<low1> <high1> <low2> <high2>)
 or
 standard-parameter-file
 
 output :
 root-file containing histogram , PDF
 
 takes two 2D-histograms (ROOT::TH2) and 
 plots corresponding bin-values agianst each other
 
 parameter-file-format :
<file1> <hist1> (<low1> <high1>)
<file2> <hist2> (<low2> <high2>)

 low and high values define correlation-histogram range 
 and which values are plotted in correlation-graph
 
 additional options in parameter-file (see plotter):
 FILE , HIST , AXIS , ROWS , COLUMNS , PIXELS

///////////////////////////////////////////////////////////////////////////////
 
differentiator

 arguments :
 <file1> <graph1> <file2> <graph2>
 or 
 standard-parameter-file 
 
 output :
 root-file containing graph
 
 takes two graphs (ROOT::TGraph) and 
 calculates differences (absolute and relative) for each equal x-position
 only root-file output, no plotting
 
 parameter-file-format (HIST has to be GRAPH) :
<file1> <graph1>
<file2> <graph2>

///////////////////////////////////////////////////////////////////////////////

drawer 

 arguments :
 /path/to/file.root histname (<lowLimit> <highLimit> <nContours>)
 
 output :
 PDF
 
 shows specified 2D-histogram (ROOT::TH2 e.g. heatmap) from file,
 if <lowLimit> <highLimit> is specified Z-range is adjusted
 <nContours> specify number of color countours 
 values can be omitted using '%' (without quotes) 
 if only one ore more than three optional arguments are given
 plot is not shown but only saved as PDF

///////////////////////////////////////////////////////////////////////////////

grapher

 arguments :
 parameter-file

 output :
 root-file containing graphs

 reads text-files and/or root-graphs and combines these
 according defined function 
 values are matched with index and X-value (first column)

 arguments in parameter-file per output-graph :
<resultsGraphName1> <rawData1> (<additionalDataList1>)
<resultsGraphName2> <rawData2> (<additionalDataList2>)
 the <additionalDataList> can be also text or root-files
 for root-files the name of the corresponding graph has to specified next
 raw- and additional-data is also stored
 
FORMAT <formatSpecifierList>
 c formatting for scanf used for text-data-input
 ( see : https://www.cplusplus.com/reference/cstdio/scanf/ )
FUNCTION <formula>
 function defined as specified in
 https://root.cern.ch/doc/master/classTFormula.html
 value of "x" in formula is taken from <rawData> Y-values
 parameters are filled from <additionalDataList> (for each point separat)
 default is "x" (without quotes, understood as f(x)=x , no other parameter)
ADJUSTX <scaleX> <offsetX>
 X-values of data are adjusted (after comparison)
 <scaleX> is applied before <offsetX> ( X_new = <scaleX> * X_old + <offsetX> )

///////////////////////////////////////////////////////////////////////////////

housekeeper 

 arguments :
 /path/to/files(.dat) (<specifierList>)
 
 output :
 root-file containing tree , PDF
 
 stores data from text-files in root tree 
 and (optional) plots TGraphs of values with specifier from list
 
 format of text-files has to be:
<unixtime> <quantity> <specifier> <value> <unit>

///////////////////////////////////////////////////////////////////////////////

logTOplot 

 arguments :
 /path/to/file.dat (<timeOffset> <duration> <specifierListFile>)
 
 output :
 root-file containing canvas , PDF
 
 stores data from text-files or root-trees in graphs 
 and plots these in canvas against time
 
 format of text-files has to be the same as for housekeeper

 each row in the specifier-list-file has to be 
 a specifier and its corresponding unit, e.g. :
<specifier1> <unit1>
<specifier2> <unit1>
<specifier3> <unit2>
<specifier4> <unit2>
...
 specifier with the same unit are plotted in the same pad
 (only specifier-unit-combinations are plotted, which are found in data)
 
 timeOffset and duration should be given as number with unit
 (e.g. 3h or 3m , where "s"=second , "m"=minute , "h"=hour , "d"=day)

///////////////////////////////////////////////////////////////////////////////

overlayer 
 
 arguments :
 standard-parameter-file
 
 output :
 root-file containing canvas , PDF
 
 takes 1D-histograms (ROOT::TH1 e.g. spectrums) and superimposes them
 
 additional options in parameter-file :
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
POINTS (<markerStyle>)
 points are used instead of bar-histograms
STATBOX <mode>
 statboxes for each histogram are drawn with entries according to mode
 (see : https://root.cern.ch/doc/master/classTPaveStats.html)

///////////////////////////////////////////////////////////////////////////////

peakFitter 

 arguments :
 standard-parameter-file
 
 optional arguments :
 <skipDrawing>/print
 
 output :
 root-file containing graphs
 
 takes 1D-histograms and fit specified peaks
 width (sigma and FWHM) and peak-fit-center difference to nominal 
 are stored in ROOT::TGraphs
 fits are shown in canvas (double-click required for progress)
 fit-specifications are written to std-out
 
 required arguments in parameter-file per histogram :
<file1> <hist1> <peakPosition> <lowFitLimit> <highFitLimit>

 additional options in parameter-file per histogram :
<additionalFunctionPart> <startParameter>

 the additional function part should be specified as ROOT::TFormula
 addable to the standard gaussian
 <startParameter> should be a list with size of additional parameters
 
 if additional arguments besides the parameter-file are provided
 the fits are not drawn unless the first argument reads "print"
 then single fits are stored as PDF

///////////////////////////////////////////////////////////////////////////////
 
plotter 

 arguments :
 standard-parameter-file
 
 output :
 root-file containing graphs , PDF
 
 takes 2D-histograms (ROOT::TH2 e.g heatmaps) (or text-files) and calculates
 mean, standard-deviation, median, minimum, maximum
 of bin-values (numbers in rows and columns)
 stores results in TGraphs as function of specified values (or names)
 and plots combined graphs
 
 arguments in parameter-file per histogram :
<file1> <hist1> <value> <error(optional)> <name(optional)>

 additional options in parameter-file :
AXIS <xAxisTitle> <yAxisTitle>
RANGE <ylow> <yhigh> / <xlow> <xhigh> <ylow> <yhigh>
 (either only ylow and yhigh or all four)
SKIP <listOfValuesToBeSkipped> <<lowLimit> ><highLimit>
 (low and high limit have to have '<' '>' preceding, respectivly)
ROWS <rowsList>
COLUMNS <columnsList>
 lists to use or to omit during calculation (bin-numbers required)
 lists can either be directly given or specified in text-file (separate lines)
 negativ values will be omitted, positive values will be used 
 (only one sign allowed)
PIXELS <pixelList> (select)
 <pixelList> should be name of text-file containing list of pixels (x y)
 if select is specified only for given pixels values are calculated
 else pixels in list are excluded  
DELIMITER <string>
 if text-files are specified columns are separated using specified string
LABELSOPTION <mode>
 X-axis labels can be manipulated according to 
 https://root.cern.ch/doc/v608/classTH1.html -> LabelsOption

///////////////////////////////////////////////////////////////////////////////
 
projector 

 arguments :
 /path/to/file.root histname 
 
 optional arguments :
 <thresholdfactor> 
 or
 <lowThresh> <highThresh> <nBins> (<skipPlotting>)
 
 output :
 root-file containing histograms , PDFs , text-files
 
 bin-values from specified 2D-histogram (ROOT::TH2 e.g. heat-maps) in the file,
 are shown (and saved as ROOT::TH1) and histogram is projected
 onto X and Y axis (column and row average, respectivly)
 
 bins, columns and rows deviating the standard-devation times
 <thresholdfactor> from mean are written to text-files
 or exceeding the <lowThresh> <highThresh> (one or both can be omitted using %)
 
 if <nBins> is specified bin-value-spectrum is ranged to lowThresh to highThresh

///////////////////////////////////////////////////////////////////////////////
 
superimposer 

 arguments :
 standard parameter-file
 
 takes ROOT::TGraphs (or text-files) and superimposes the points in one plot
 
 output :
 root-file containing canvas , PDF
 
 options per graph in parameter-file :
<file1> <graph1> <title1> (<markerStyle> <markerColor> <lineStyle>)

 for style and color options see :
 https://root.cern.ch/doc/master/classTAttMarker.html

  additional options in parameter-file :
AXIS <xAxisTitle> <yAxisTitle>
RANGE <xlow> <xhigh> <ylow> <yhigh>
LOG <logarithmicX> <logarithmicY> 
 (specified by '0' [->false] or '1' [->true], without quotes)
NOERRORS
 omits error-bars (during drawing)
FORMAT <formatSpecifierList>
 c formatting for scanf used for text-data-input
 ( see : https://www.cplusplus.com/reference/cstdio/scanf/ )
MARKERSIZE <size>
 global size of drawn point-marker (default=1)

///////////////////////////////////////////////////////////////////////////////

textTOtree 

 arguments :
 parameter-file
 
 output :
 root-file containing tree
 
 stores columns of text-files in specified leafs of ROOT:TTree
 
 format of parameter-file :
FILE </path/to> <specifier> <.ending>
<columnIndex1> <variableName1> <variableType1>
<columnIndex2> <variableName2> <variableType2>
<columnIndex3> <variableName3> <variableType3>
...
 column-indices are counted starting from zero
 variableType should be 'int' , 'double' , 'string' without quotes
 'string's are stored as ROOT::TString (for postprocessing)

