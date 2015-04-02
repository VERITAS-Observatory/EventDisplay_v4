#!/bin/bash

# input arguments
SCIDIR="$1"
RAWOPTLINES="$2"

# load extra functions
source $EVNDISPSYS/scripts/VTS/helper_scripts/pipeline_functions.sh

# other variables
browseropt=$PIPELINE_BROWSER
html="$SCIDIR/options_browser.html"

# css and javascript files to pack into the html file
cssfiles=()
cssfiles+=("$EVNDISPSYS/scripts/VTS/helper_scripts/pipeline_optionsBrowser.css")
cssfiles+=("$EVNDISPSYS/scripts/VTS/helper_scripts/pipeline_optionsBrowser.jquery.dataTables.css")

# js files
jsfiles=()

# jquery plugin ( jquery.com )
jsfiles+=("$EVNDISPSYS/scripts/VTS/helper_scripts/pipeline_optionsBrowser.jquery.js")

# dataTables plugin ( datatables.net )
jsfiles+=("$EVNDISPSYS/scripts/VTS/helper_scripts/pipeline_optionsBrowser.jquery.dataTables.js")

# extra column data (for specific ordering and improved searching)
declare -A OPTIONCOMPLEXITYORDER
OPTIONCOMPLEXITYORDER["Common"]=0
OPTIONCOMPLEXITYORDER["Uncommon"]=1
OPTIONCOMPLEXITYORDER["Developer"]=2
declare -A STAGEORDER
STAGEORDER["evndisp"]=0
STAGEORDER["model3d"]=1
STAGEORDER["mscw"]=2
STAGEORDER["energy3d"]=3
STAGEORDER["frogs"]=4
STAGEORDER["anpar"]=5
STAGEORDER["anmer"]=6
STAGEORDER["mutate"]=7
declare -A STAGESEARCH
STAGESEARCH["evndisp"]="event display stage"
STAGESEARCH["model3d"]="model3d stage"
STAGESEARCH["mscw"]="mscw mean scaled width stage"
STAGESEARCH["energy3d"]="energy3d stage"
STAGESEARCH["frogs"]="frogs stage"
STAGESEARCH["anpar"]="anasum parallel stage"
STAGESEARCH["anmer"]="anasum merge stage"
STAGESEARCH["mutate"]="ctools mutate stage"

# arrays and variables for parsing the option lines
OPTFILE=()
OPTNAME=()
DEFAULTARG=()
ACTIVESTAGE=()
OPTIONCOMPLEXITY=()
OPTIONARGTYPE=()
OPTGROUPNAME=()
OPTDESCRIPTION=()
optfile=""
optionname=""
defaultarg=""
activestage=""
optioncomplexity=""
optionargtype=""
optiongroupname=""
optdescription=""

# parse raw option lines into a table of options
while read line ; do 
	#echo "'$line'"
	
	# if we get an empty line, then reset all options properties
	if [[ -z "$line" ]] ; then
		optionname=""
		defaultarg=""
		activestage=""
		optioncomplexity=""
		optionargtype=""
		optiongroupname=""
		optdescription=""
	fi
	
	# check if the line is a tag(#%) line with extra information
	if [[ "$line" =~ ^#%[[:space:]] ]] ; then
		#echo "tag! '$line'"
		tagname=$( echo "$line" | cut -d ' ' -f2  )
		#echo "  tagname:'$tagname'"
		if   [[ "$tagname" == "ActiveStage"      ]] ; then activestage=$(      echo "$line" | cut -d ' ' -f3  ) 
		elif [[ "$tagname" == "OptionComplexity" ]] ; then optioncomplexity=$( echo "$line" | cut -d ' ' -f3  ) 
		elif [[ "$tagname" == "OptionArgType"    ]] ; then optionargtype=$(    echo "$line" | cut -d ' ' -f3- ) 
		elif [[ "$tagname" == "GroupName"        ]] ; then optiongroupname=$(  echo "$line" | cut -d ' ' -f3- ) 
		elif [[ "$tagname" == "Description"      ]] ; then optdescription=$(   echo "$line" | cut -d ' ' -f3- ) 
		else echo "Warning, unrecognized tag '$tagname' in line '$line'"
		fi
		#tagcont=$( echo "$line" | cut -d ' ' -f3- )
		#echo "  tagcont:'$tagcont'"
	fi
	
	# if we see an actual export line, 
	# save the setting and its properties to the array
	if [[ "$line" =~ ^export* ]] ; then
		#echo "$line"
		optionname=$( echo "$line" | grep -Po "export .*?\=" | cut -d " " -f2- | tr -d '=')
		defaultarg=$( echo "$line" | grep -oP "=.*$" | sed -r 's/^.{1}//' ) 
		#echo "Read in $optionname"
		OPTFILE+=( "analysis.config" )
		OPTNAME+=(          "$optionname"       )
		DEFAULTARG+=(       "$defaultarg"       )
		ACTIVESTAGE+=(      "$activestage"      )
		OPTIONCOMPLEXITY+=( "$optioncomplexity" )
		OPTIONARGTYPE+=(    "$optionargtype"    )
		OPTIONGROUPNAME+=(  "$optiongroupname"  )
		OPTDESCRIPTION+=(   "$optdescription"   )
	fi

done <<< "$RAWOPTLINES"
#echo "$RAWOPTLINES"

if false ; then
	paramfilelist=()
	paramfilelist+=( "$VERITAS_EVNDISP_AUX_DIR/ParameterFiles/EVNDISP.reconstruction.runparameter" )
	paramfilelist+=( "$VERITAS_EVNDISP_AUX_DIR/ParameterFiles/EVNDISP.global.runparameter"         )
	paramfilelist+=( "$VERITAS_EVNDISP_AUX_DIR/ParameterFiles/TMVA.BoxCuts.runparameter"           )
	paramfilelist+=( "$VERITAS_EVNDISP_AUX_DIR/ParameterFiles/VERITAS.Epochs.runparameter"         )
	paramfilelist+=( "$VERITAS_EVNDISP_AUX_DIR/GammaHadronCutFiles/ANASUM.GammaHadron-Cut-NTel2-PointSource-Moderate.dat" )

	declare -A PROPFILE
	declare -A PROPDIR
	declare -A PROPNAME

	echo ; echo "Reading parameter files..."
	for pfilelong in "${paramfilelist[@]}" ; do
		pfile=$( basename "$pfilelong" )
		pdir=$(  basename `dirname  "$pfilelong"` )
		#echo "reading file '$pdir/$pfile'"
		pfiletext=$( cat "$pfilelong" )
		while read line ; do
			#echo "  '$line'"
			if [[ "$line" =~ ^"*" ]] ; then
				tagname=$( echo "$line" | cut -d ' ' -f2 )
				if  [[ "$tagname" == "-1" ]] || 
					[[ "$tagname" ==  "1" ]] || 
					[[ "$tagname" ==  "2" ]] ||
					[[ "$tagname" ==  "3" ]] ||
					[[ "$tagname" ==  "4" ]] ; then
					tagname=$( echo "$line" | cut -d ' ' -f3 )
				fi
				key="$pfile:$tagname"
				echo "$key"
				PROPFILE["$key"]="$pfile"
				PROPDIR["$key"]="$pdir"
				PROPNAME["$key"]="$tagname"
			fi
		done <<< "$pfiletext"
	done
fi

# generate html file
echo ; echo "creating html file at '$html'"
echo '
<!DOCTYPE html>
<html>
<head>
	<meta charset="utf-8">
' > $html

# add in the css files
for css in "${cssfiles[@]}" ; do
	echo "adding css file `simplifyToEVNDISPSYS $css`"
	echo '
	<style>
' >> $html
	cat "$css" >> $html
	echo '
	</style>
' >> $html
done
echo '
<style type="text/css" class="init"></style>
' >> $html

# add in js files
for js in "${jsfiles[@]}" ; do
	echo "adding js file `simplifyToEVNDISPSYS $js`"
	echo '
<script type="text/javascript">
' >> $html
	cat "$js" >> $html
	echo '
</script>
' >> $html
done

# setup our initial javascript table options
# a single-quote string can't contain single quotes,
# so we have to splice them in manually here
# hence, the '"'"' messes
echo '
<script type="text/javascript" language="javascript" class="init">
$(document).ready(function() {
    $('"'"'#example'"'"').DataTable( {
		"lengthMenu": [[ -1, 10, 25, 50, 100], ["All", 10, 25, 50, 100]],
    "aaSorting": [[3,'"'"'asc'"'"'],[0,'"'"'asc'"'"']]
	} );
} );
</script>
' >> $html

# initial content on the page
EVNDISPVERSION=$($EVNDISPSYS/bin/evndisp --version )
echo '
</head>
<body>
	<div class="container">
		<section>
			<h1>Event Display <span>Reconstruction Pipeline Options for '$EVNDISPVERSION'</span></h1>
			<h3>Assemble your analysis.config file:</h3>
			<pre class="lang-sh prettyprint prettyprinted"><code>#!/bin/bash
export RUNLIST="YourRunlistFileNameHere"
export CUTS="Moderate"
export BACKGROUND="reflected"
</code></pre>
' >> $html


#echo '
#			<h3>1) Assemble your analysis.config file</h3>
#			<pre><code id="analysisconfig">#!/bin/bash
#export RUNLIST=\"YourRunlistFileNameHere\"
#export CUTS=\"Moderate\"
#export BACKGROUND=\"reflected\"</code></pre>
#' >> $html

# print the beginning of the html table
echo '
	<table id="example" class="display" cellspacing="0" width="100%">
		<thead>
			<tr>
				<th>Option Name</th>
				<th>Option Group</th>
				<th>Active Stage</th>
				<th>Complexity</th>
				<th>Argument Type</th>
				<th>Description</th>
			</tr>
		</thead>
		<tbody>
' >> $html

# print each option to a row of the table
for i in "${!OPTNAME[@]}" ; do
	
	# figure out the data-order parameter for the Complexity column
	optcomplex="${OPTIONCOMPLEXITY[$i]}"
	optcomplexdataorder=""
	if [[ ${OPTIONCOMPLEXITYORDER[$optcomplex]} =~ 0|1|2 ]]  ; then
		optcomplexdataorder="data-order=\"${OPTIONCOMPLEXITYORDER[$optcomplex]}\""
	fi
	
	# figure out the data-order parameter for the Active Stage column
	actstage="${ACTIVESTAGE[$i]}"
	actstageorder=""
	actstagesearch=""
	if [[ ${STAGEORDER[$actstage]} =~ 0|1|2|3|4|5|6|7 ]] ; then
		actstageorder="data-order=\"${STAGEORDER[$actstage]}\""
		actsearch="data-search=\"${STAGESEARCH[$actstage]}\""
	else
		echo "Warning, unrecognized stage order '${STAGEORDER[$actstage]}' with option name '${OPTNAME[$i]}'"
	fi
	
	# print our table row to the html file
	echo "
			<tr>
				<td>${OPTNAME[$i]}</td>
				<td>${OPTIONGROUPNAME[$i]}</td>
				<td ${actstageorder} ${actsearch}>${actstage}</td>
				<td ${optcomplexdataorder}>${optcomplex}</td>
				<td>${OPTIONARGTYPE[$i]}</td>
				<td>${OPTDESCRIPTION[$i]}</td>
			</tr>
" >> $html
done

# finish up the table
echo "
		</tbody>
	</table>
" >> $html


echo "
	</section>
	</div>
</body>
</html>
" >> $html

cp $EVNDISPSYS/scripts/VTS/helper_scripts/pipeline_optionsBrowser.sort*png $SCIDIR/
chmod o+r $SCIDIR/pipeline_optionsBrowser.sort*png

# open the options page in a browser
echo ; echo "trying open html file '$html' in browser '$browseropt'"
#$browseropt "file://$html" &

