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
functionFitter
grapher
hister
housekeeper
logTOplot
mapper
overlayer
peakFitter
plotter
projector
setData
shower
slicer
superimposer
textTOtree

most of these require parameter-files as argument

arguments on commandline are specified by position
some of these can be omitted using '%' (without quotes)

///////////////////////////////////////////////////////////////////////////////

format of standard-parameter-files is :

FILE </path/to> <.ending>
HIST/GRAPH <prefix> <suffix>
<file1> <hist1>
<file2> <hist2>
<file3> <hist3>
...

uppercase words are specific
strings or values without <> 
most values can be omitted using '%' (without quotes)

///////////////////////////////////////////////////////////////////////////////

comparator

 arguments :
 standard-parameter-file

 optional argument :
 <skipPlotting>
 
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

 additional options in parameter-file (see plotter):
ROWS , COLUMNS , PIXELS

///////////////////////////////////////////////////////////////////////////////
 
correlator 

 arguments :

 standard-parameter-file

 or :

 <file1>
 <hist/graph1>
 <file2>
 <hist/graph2>
 
 optional arguments without parameter-file ('or'-case) :
 HIST/GRAPH/DIFF/RATIO(skip)
 <divisions1[low1,high1]>
 <divisions2[low2,high2]>
 <maxDistance>
 <binning[low,high]>
 <intervalList>
 
 output :
 root-file containing histogram , PDF
 
 takes two 2D-histograms (ROOT::TH2) or graphs and 
 plots corresponding (bin- or Y-)values agianst each other
 
 parameter-file-format :
<file1> <hist/graph1> (<low1> <high1> <divisions1>)
<file2> <hist/graph2> (<low2> <high2> <divisions2>)

 data from source 1 is on X-axis and data from source 2 on Y-axis
 of resulting correlations

 either the correlation is plotted as 2D-histogram (via option HIST)
 or as scatter-plot (via option GRAPH, default)

 if 'skip' (without quotes) is specified additionally the plot is not saved
 if only 'skip' (without quotes) is specified no canvas is shown

 if 'DIFF' or 'RATIO' (both without quotes) is specified
 a difference- or ratio-histogram is filled
 with data2-data1 or data2/data1 (Y-values, respectively),
 both only if also <binning>, <low> and <high> is specified
 (either as arguements or in the parameter-file)

 low and high values define correlation-histogram range 
 and which values are plotted in correlation-graph
 the divisions specifiy the correlation-histogram-binning (default=2000)
 
 <maxDistance> is the allowed difference of the X-values between the graphs
 without specification only X-value-combinations which match exactly are used
 if multiple values match no data is written, instead an error is given
 this can be omitted by specifying the value negative
 (same effect as USEFIRSTOCCURENCE in parameter-file)

 <intervalList> can contain any number of intervals (specified as [low,high])
 separated by spaces, which define the allowed X-values from the source-data

 additional options in parameter-file :
MAXDISTANCE <maxDistance>
USEFIRSTOCCURENCE
 if multiple X-values match the first occurence 
 in the second graph will be used
DIFFERENCES <low> <high> <binning>
RATIOS      <low> <high> <binning>
INTERVAL <lowLimit> <highLimit>
 can be specified multiple times
 only graph points with X-values within <lowLimit> and <highLimit>
 (included) will be used for correlations
 if one value is omitted via '%' (without quotes) 
 only other boundary is considered
 
 additional options in parameter-file (see plotter):
 FILE , HIST , AXIS , ROWS , COLUMNS , PIXELS , NOERRORS

///////////////////////////////////////////////////////////////////////////////
 
differentiator

 arguments :
 /path/to/file.root
 <graph/hist>
 
 output :
 root-file containing graphs
 
 takes either graph (ROOT::TGraph) or histogram (ROOT::TH1) 
 calculates differential (difference of adjacent Y-values) 
 and integral (accumulation of Y-values) in ascending X-value direction
 
 for both types output will be graphs
 a version weighted by the distance between the values will be also calculated

///////////////////////////////////////////////////////////////////////////////

drawer 

 arguments :
 /path/to/file.root
 histname
 
 optional arguments :
 <lowLimit>
 <highLimit>
 <nContours>
 <colorPalette>
 <statBox>
 (skip)
 
 output :
 PDF
 
 shows specified 2D-histogram (ROOT::TH2 e.g. heatmap) from file,
 if <lowLimit> and/or <highLimit> is specified Z-range is adjusted
 <nContours> specify number of color countours for Z-axis
 <colorPalette> can be chosen from
    https://root.cern.ch/doc/master/classTColor.html#C06 -> Color palettes
 <statBox> is drawn with entries according to 
    https://root.cern.ch/doc/master/classTPaveStats.html
 if at last (or as any positional-argument) 'skip' is specified 
 no canvas is drawn, but histogram is directly saved as PDF
 
 values can be omitted using '%' (without quotes) 

///////////////////////////////////////////////////////////////////////////////
 
functionFitter

 arguments :
 /path/to/file.root
 <graph/hist>
 <lowLimit>
 <highLimit>
 <function>
 
 optional arguments :
 <startParameterList>
 print/skip(_auto)
 
 output :
 fit-parameter (std-out) , PDF ,
 root-file containing fit-function and difference to data
 
 takes either graph (ROOT::TGraph) or histogram (ROOT::TH1) 
 fits function within specified limits to data
 shows data and fit in canvas
 
 function-formula has to be defined as specified in
 https://root.cern.ch/doc/master/classTFormula.html
 
 start-parameter should be specified in order according to formula,
 but can be skipped using '%' (without quotes)
 
 start-parameter can be given with range-limits, via :
 value[low,high] (all without withspaces) 
 
 with 'skip' no canvas is drawn
 for PDF 'print' should be the last argument
 using 'auto' combined with 'print' PDF is saved without interaction

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
PARAMETERARGUMENTS
 additional to FUNCTION
 the parameter for the function will be taken 
 from the <additionalDataList> as fixed numbers
ADJUSTX <scaleX> <offsetX>
 X-values of data are adjusted (after comparison)
 <scaleX> is applied before <offsetX> ( X_new = <scaleX> * X_old + <offsetX> )
AVERAGE <number>
 average over <number> of consecutive points of result-graph (X and Y)
 mean is writen as result, as well as standard-devation
 (so total number of points is practically divided by <number>)
 errors are also averaged
FLIP
 exchange x- and y-values of result-graphs
WRITEERRORS (X/Y)
 the Y-(or X-)error of the source-graph is used 
 for the Y-values of the output-graph
SETERRORS (X/Y)
 the values for the output-graph are taken from the source-graph
 and errors are assigned from the values of the reference-graphs 
 X or Y or both if not specified
 correct number of reference-graphs are required
SKIPRAW
 omit writing of source-graphs and reference-graphs

///////////////////////////////////////////////////////////////////////////////

hister

 arguments :
 parameter-file

 output :
 root-file containing histograms

 reads text-files and/or root-histograms 
 combines or manipulates these according defined function or specifications 
 values are matched by bin number

 arguments in parameter-file per output-histogram :
<resultsHistName1> <rawData1> (<additionalSpecifications1>)
<resultsHistName2> <rawData2> (<additionalSpecifications2>)
 the <additionalSpecifications> should contain 
 the histogram-name for root-files 
 
 mandatory options
WEIGHTS/VALUES (OUTFLOW)
 either sets the bin-contents to the values in the text-file 
 or fills these values into the histogram
 if OUTFLOW is specified end-range is set or filled as specified
DIMENSION <number>
 either 1 or 2 dimensional histograms are created or considered
 
 not mandatory options
BINNING <nXbins> <xlow> <xhigh> (<nYbins> <ylow> <yhigh>)
 y-binning has only to be provided if DIMENSION is 2
ROW    <first> <second>
COLUMN <first> <second>
 rows and columns to be used from a text-file
 numbering for these start at zero, whereas bin-numbers start at 1
 negativ numbers can be provided to skip certain rows/columns
 -> toskip = - ( line-number + 1 ) [e.g. for zero use -1]
FUNCTION <formula>
 function defined as specified in
 https://root.cern.ch/doc/master/classTFormula.html
 depending on number of parameters the histograms theirself are adjusted
 or if number of parameters plus one is equal to number of data-sets
 a combination is created called "result"
 value of "x" in formula is taken from first histogram/data-set
 parameters are filled with other data-set values (starting at 0)
 default is "x" (without quotes, understood as f(x)=x , no other parameter)
 with this no changes are applied
 if only one parameter is used and in <additionalSpecifications>
 another root-data-set is given for each data-set the function is
 evaluated separately (using these two sets)
PARAMETERARGUMENTS
 additional to FUNCTION
 the parameter for the function will be taken 
 from the <additionalSpecifications>
 also MEAN , STDV , MIN , MAX , MEDIAN can be defined as parameter
 the numbering starts at the fourth column 
 ( third column can be spared using '%' )
FLIP <flipX> <flipY>
 bin contents are swapped symmetrically along defined axis (=1)
 if no arguements are given both axis will be flipped
 additional data-sets for one-parameter-functions will NOT be flipped
WRITEERRORS
 the bin-contents of the output-histogram are set 
 to the errors of the source-histogram (no further processing)
SETERRORS
 the bin-errors of the output-histogram are set 
 to the bin-contetns of the reference-histogram 
 from the <additionalSpecifications> (no further processing)

///////////////////////////////////////////////////////////////////////////////

housekeeper 

 arguments :
 /path/to/files(.dat)

 optional arguments :
 <specifierList>
 show print write
 
 output :
 available specifier , root-file containing tree and graphs , PDFs
 
 stores data from text-files in root tree 
 and plots TGraphs of values with specifier from list
 storage is performed if 'write' (without quotes) is given as argument
 with 'show' (without quotes) plots are drawn sequentially
 with 'print' (without quotes) plots are saved also as PDFs
 output options can be combined (by adding them as arguments in any order)
 
 format of text-files has to be:
<unixtime> <quantity> <specifier> <value> <unit>

///////////////////////////////////////////////////////////////////////////////

logTOplot 

 arguments :
 /path/to/file.dat

 optional arguments :
 <timeOffset>
 <duration>
 <specifierListFile>
 <outputOptions>
 
 output :
 root-file containing TGraphs , PDF
 
 stores data from text-files or root-trees in graphs 
 and plots these in canvas against time
 
 format of text-files has to be the same as for housekeeper:
<unixtime> <quantity> <specifier> <value> <unit>

 each row in the specifier-list-file has to be 
 a specifier and its corresponding quantity, e.g. :
<specifier1> <quantity1>
<specifier2> <quantity1>
<specifier3> <quantity2>
<specifier4> <quantity2>
...
 specifier with the same quantity are plotted in the same pad
 (only specifier-quantity-combinations are plotted, which are found in data)
 
 timeOffset and duration should be given as number with unit
 (e.g. 3h or 5m , where "s"=second , "m"=minute , "h"=hour , "d"=day)
 
 additional options per specifier :
<markerStyle> <markerColor> <lineStyle> <column> <label>

 to adjust the plots and combine only certain specifier
 into quantity-pads one can use:
QUANTITY <quantity> (<name> <unit> <low> <high> <logarithmic> <nDivisions>)
 before specifiers with this quantity
 optional : the Y-axis will show "name ( unit )" (without quotes)
            and will be adjusted according to low and high,
            logarithmic scaling will be applied if requested ('1' or 'log')
            default <nDivisions> is 515

 other options in specifierListFile
TABLE <unixtimeColumn>
 if TABLE is used data will be treated as text in columns
 with specifier in given columns (see additional options per specifier)
DELIMITER <sign>
 for table-data delimiter is used to separate columns
XTITLE <xAxisTitle>
XAXIS <nDivisions> <xLabelFormat>
 set the divisions for x-axis-labels (default: 525) and their format
 default label-format is "%H:%M%F1970-01-01 00:00:00" (without quotes)
XNOTINTEGER
 for the x-axis double is used instead of unsigned int
SAVEAS <filetype>

 <outputOptions> can be a one or a combination of :
 - 'show'  : canvas is drawn and shown on screen
 - 'print' : canvas is saved
 - 'write' : graphs are saved in a root-file
 if at least one option is given others are set to false
 (all are true by default)

///////////////////////////////////////////////////////////////////////////////

mapper

 arguments :
 /path/to/file.txt

 optional arguments :
 <xTitle>
 <yTitle>
 <zTitle>
 <markerSize[colorPalette,nContours]>
 <xDivisions[xLow,xHigh](log)(grid)>
 <yDivisions[yLow,yHigh](log)(grid)>
 <zDivisions[yLow,yHigh](log)>
 graphname

 output :
 PDF

 shows 3D-data from text-file as points in x-y-plane
 with the third coordinate colored (z-axis)

 colorPalette can be chosen from
    https://root.cern.ch/doc/master/classTColor.html#C06
 using negativ values (i.e. minus in front) will invert the palette

 if graphname is given and filename ends with '.root' (without quotes)
 file is treated as root-file and a TGraph2D-object with graphname is searched
 from this the data-points are shown

///////////////////////////////////////////////////////////////////////////////

overlayer 
 
 arguments :
 standard-parameter-file
 
 output :
 root-file containing canvas , PDF
 
 takes 1D-histograms (ROOT::TH1 e.g. spectrums) and superimposes them
 
 additional options per histogram :
<color> <markerStyle>
 
 additional options in parameter-file :
AXIS <xAxisTitle> <yAxisTitle>
 in separate lines AXISX and AXISY with space separated titles
RANGE <xlow> <xhigh> <ylow> <yhigh>
LOG <logarithmicX> <logarithmicY> 
 (specified by '0' [->false] or '1' [->true], without quotes)
GRID <Xgridlines> <Ygridlines>
 (specified by '0' [->false] or '1' [->true], without quotes,
  default : X=false , Y=true )
SCALEMODE <mode>
 mode can be: 'bare', 'max', 'integral' (default) (all without quotes)
 addtional 'ranged' (added without withspace etc.) can be specified
 in order to enable maximum or integral calculation within [<ylow>,<yhigh>]
DIVISIONS <xDivisions> <yDivisions>
 divisions of axis-lables
PALETTE <number> (inverted)
 number of ROOT color palette which should be chosen for plotting
 optional 'inverted' (high/low color)
POINTS (<markerStyle>)
 points are used instead of bar-histograms
MARKERSIZE <size>
 global size of drawn point-marker (default=1)
STATBOX <mode> (<position>)
 statboxes for each histogram are drawn with entries according to mode
 (see : https://root.cern.ch/doc/master/classTPaveStats.html)
 position can be specified as top , bot , left or right
 statboxes are ordered column- or row-wise accordingly
LEGEND <text>
 adds <text> on top of legend
NARROWCANVAS
 same canvas aspect ratio as for superimposer (default)
SAVEAS <filetype>
REPLACENAN (<replacement>)
 search and replace bins with NaN content 
 default with zero, if specified with <replacement>

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
 
 optional argument :
 <skipShowing>/skip
 
 output :
 root-file containing graphs , PDF
 
 takes 2D-histograms (ROOT::TH2 e.g heatmaps) (or text-files) and calculates
 mean, standard-deviation, median, minimum, maximum
 of bin-values (numbers in rows and columns)
 stores results in TGraphs as function of specified values (or names)
 and plots combined graphs
 showing is omitted if additional commandline-arguments are given
 no PDF is written if skip is next argument
 
 arguments in parameter-file per histogram :
<file1> <hist1> <value> <error(optional)> <name(optional)>

 additional options in parameter-file :
AXIS <xAxisTitle> <yAxisTitle>
 in separate lines AXISX and AXISY with space separated titles
RANGE <ylow> <yhigh> / <xlow> <xhigh> <ylow> <yhigh>
 (either only ylow and yhigh or all four)
LOG <logarithmicX> <logarithmicY>
 (specified by '0' [->false] or '1' [->true], without quotes)
GRID <Xgridlines> <Ygridlines>
 (specified by '0' [->false] or '1' [->true], without quotes,
  default : X=false , Y=true )
SKIP <listOfValuesToBeSkipped> <<lowLimit> ><highLimit>
 (low and high limit have to have '<' '>' preceding, respectively)
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
DIVISIONS <xDivisions> <yDivisions>
 divisions of axis-lables
BROADCANVAS
 same canvas aspect ratio as for overlayer (default)

///////////////////////////////////////////////////////////////////////////////

printer

 arguments :
 /path/to/file.root
 histname

 optional arguments :
 <xDivisions[xLow,xHigh](log)(grid)>
 <yDivisions[yLow,yHigh](log)(grid)>
 <statBox>
 <lineStyle[lineColor,lineWidth]>
 <drawingOption>
 <markerStyle[markerColor,markerSize]>
 (skip)

 output :
 PDF

 shows specified histogram (ROOT::TH1) from file
 for stat-box-settings see
     https://root.cern.ch/doc/master/classTPaveStats.html
 for line-style-options see
     https://root.cern.ch/doc/master/classTAttLine.html
 for drawing-options see
     https://root.cern.ch/doc/master/classTHistPainter.html
     or
     https://root.cern.ch/root/htmldoc/guides/users-guide/Histograms.html
 for marker-style-options see
     https://root.cern.ch/doc/master/classTAttMarker.html
 if at last (or as any positional-argument) 'skip' is specified
 no canvas is drawn, but graph is directly saved as PDF

 values can be omitted using '%' (without quotes)

///////////////////////////////////////////////////////////////////////////////
 
projector

 arguments :
 /path/to/file.root
 histname
 
 optional arguments :
 <lowThresh>
 <highThresh>
 <nBins>
 <writeOptions>
 
 output :
 root-file containing histograms , PDFs , text-files
 
 bin-values from specified 2D-histogram (ROOT::TH2 e.g. heat-maps) in the file,
 are shown (and saved as ROOT::TH1) and histogram is projected
 onto X and Y axis (column and row average, respectively)
 
 bins, columns and rows deviating the standard-devation times
 <thresholdfactor> from mean are written to text-files
 or exceeding the <lowThresh> <highThresh> (one or both can be omitted using %)
 
 if <nBins> is specified bin-value-spectrum is ranged to lowThresh to highThresh

 at position of <thresholdfactor> or <writeOptions> additionally specifications
  - 'skip'       no canvas is draw
  - 'print'      spectrum and projections are saved as PDFs
  - 'accumulate' projections are not averaged
  - 'write'      bins, columns and rows deviating are written to text-files
 (all without quotes), options can be combined in single string (no whitespaces)

///////////////////////////////////////////////////////////////////////////////

pulseEvaluation

 arguments :
 parameter-file

 optional arguments :
 <plotOptions>
 
 output :
 root-file containing graphs with evaluation result of input-data like
 - offset    (baseline, also from fit)
 - maximum   (heighest values, also from fit)
 - variation (standard-devation around baseline)
 - risetime  (10% to 90%, also from fit)
 - falltime  (90% to 10%, also from fit)
 plotted against specified values per label (see parameter-file specifications)

 if an argument is given at <plotOptions> fits will be shown
 if 'ALL' (wihtout quotes) is specified individual peak-fits will be shown

 options per measurement (=TGraphErrors-Data) in parameter-file :
<name> <file> <graph> (<value1> <value2> ...)

 required additional specifications in parameter-file :
PERIODS <nPeriods>
LABELS <label1> <label2> ...
 for each label a value is required per measurement
 for each graphs will be created per label combination
RANGE low high nBins
 range and binning for the y-values of the data-graphs

 optional specifications in parameter-file :
CORRELATE <result1> <result2>
 creates additional correlation-plots from evaluation-data
 (can be specified multiple times)
PRINT <result1> <result2> ...
 output on stdout
 (can be specified multiple times)

///////////////////////////////////////////////////////////////////////////////

rootTOtext 

 arguments :
 /path/to/file.root  
 
 optional arguments :
 <rootObjectName>/WRITE
 WRITE
 
 output :
 text-files containing data from histograms and graphs
 
 iterates over objects in root-file and
 either lists all objects (and fillings) in stdout or
 writes data to text-file (via option 'WRITE')
 graphs are stored column-wise ( third and fourth column represent errors )
 for histograms also bin-edges are stored ( in 'X' and 'Y' row )
 with specified <rootObjectName> only the specified obejct
 will be written or shown

///////////////////////////////////////////////////////////////////////////////
 
setData 

 arguments :
 /path/to/file.root
 <objectName>
 <mode>
 <argumentList>
 
 output :
 root-file containing histogram or graph as specified
 
 depending on <mode> root objects can be manipulated or created
 <mode> can be :
 - HIST <nBins[lowLimit,highLimit]> ( <nYBins[lowYLimit,highYLimit]> )
 - FILL   <Xvalue> (<Yvalue>/ <weight>) 
 - WEIGHT <Xvalue> (<Yvalue>) <weight>
 - POINT (<index>) <Xvalue> <Yvalue> (<Xerror> <Yerror>)
 - REMOVE NAN/<index>/<xValue[xLow,xHigh]> ( <yValue[yLow,yHigh]> )
 difference between FILL and WEIGHT is by adding (for FILL)
 or setting (for WEIGHT) the respective bin-value
 for these two modes (FILL and WEIGHT) Xvalue and Yvalue can specify
 bin-numbers if 'BIN' (without quotes) is appended to the <mode>
 HIST creates new histograms (either one- or two-dimensional)
 POINT creates graph if no graph with specified name is found

///////////////////////////////////////////////////////////////////////////////

shower

 arguments :
 /path/to/file.root
 graphname

 optional arguments :
 <xDivisions[xLow,xHigh](log)(grid)>
 <yDivisions[yLow,yHigh](log)(grid)>
 <markerStyle[markerColor,markerSize]>
 <lineStyle[lineColor,lineWidth]>
 <xTitle>
 <yTitle>
 (skip)

 output :
 PDF

 shows specified graph (ROOT::TGraphErrors) from file
 for marker-style-options see
     https://root.cern.ch/doc/master/classTAttMarker.html
 for line-style-options see
     https://root.cern.ch/doc/master/classTAttLine.html
 if at last (or as any positional-argument) 'skip' is specified
 no canvas is drawn, but graph is directly saved as PDF

 values can be omitted using '%' (without quotes)

///////////////////////////////////////////////////////////////////////////////
 
slicer 

 arguments :
 /path/to/file.root
 histname
 
 optional arguments :
 <X/x/Y/y>
 <slicesList>
 
 output :
 root-file containing histograms
 
 slices (ROOT::TH1D) from specified 2D-histogram (ROOT::TH2 e.g. heat-maps) 
 are stored in new file
 
 if 'X' , 'x' , 'Y' or 'y' is specified 
 only slices along the respective direction are stored

 numbers in <sliceList> (on commandline) must be whitespace-separated 
 
 if a capital letter is used for the direction
 these numbers are treated as bin-numbers
 (so they should between 1 and total number of bins (both included) )
 
 if a lowercase letter is used for the direction
 these numbers are treated as axis-values 
 from which bin-numbers are derived
 
 for both cases out of range values are omitted

///////////////////////////////////////////////////////////////////////////////
 
superimposer 

 arguments :
 standard-parameter-file
 
 output :
 root-file containing canvas , PDF
 
 takes ROOT::TGraphs (or text-files) and superimposes the points in one plot

 options per graph in parameter-file :
<file1> <graph1> <title1> (<markerStyle> <markerColor> <lineStyle>)

 for style and color options see :
 https://root.cern.ch/doc/master/classTAttMarker.html

 if an additional commandline-argument is provided
 plot is directly saved without further request

 additional options in parameter-file :
AXIS <xAxisTitle> <yAxisTitle>
 in separate lines AXISX and AXISY with space separated titles
RANGE <xlow> <xhigh> <ylow> <yhigh>
LOG <logarithmicX> <logarithmicY> 
 (specified by '0' [->false] or '1' [->true], without quotes)
GRID <Xgridlines> <Ygridlines>
 (specified by '0' [->false] or '1' [->true], without quotes,
  default : X=false , Y=true )
NOERRORS
 omits error-bars (during drawing)
FORMAT <formatSpecifierList>
 c formatting for scanf used for text-data-input
 ( see : https://www.cplusplus.com/reference/cstdio/scanf/ )
DIVISIONS <xDivisions> <yDivisions>
 divisions of axis-lables
MARKERSIZE <size>
 global size of drawn point-marker (default=1)
PALETTE <number> (inverted)
 number of ROOT color palette which should be chosen for plotting
 optional 'inverted' (high/low color)
LEGEND <text>
 adds <text> on top of legend
LEGENDPOSITION (left/right) (top/bot)
BROADCANVAS
 same canvas aspect ratio as for overlayer (default)
SAVEAS <filetype>
REPLACENAN (<Xreplacement> <Yreplacement>)
 search and remove points with NaN values (X and Y) 
 if replacements are specified point will be overwritten
FUNCTION <formula> (<startParameterList>)
 function defined as specified in
 https://root.cern.ch/doc/master/classTFormula.html
 fit-results will be displayed on canvas and in console
 <startParameterList> could contain value[low,high] for
 the parameters of the function and their limits

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

