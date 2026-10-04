#!/bin/zsh
# Historical channel exclusions below are preserved. Run IDs encode run*100+subrun.
if [[ $# < 1 || ! -f $1 ]]; then
    echo "Usage: $0 <run/subrun list> [calib dir]" >&2
    exit 1
fi
list=$(realpath "$1") || exit 1
dir=$(realpath "${2:-$VERITAS_EVNDISP_AUX_DIR/Calibration}") || exit 1
script_dir=$(dirname "$(realpath "$0")")
output="$PWD/LowGainPedestals.lped"
output_tmp=$(mktemp "$PWD/.LowGainPedestals.XXXXXX") || exit 1
scratch=$(mktemp -d "${TMPDIR:-/tmp}/lped-XXXXXX") || exit 1
trap 'rm -rf -- "$scratch"; rm -f -- "$output_tmp"' EXIT
mkdir -p "$dir/lpedfiles" || exit 1
names=()
while read -r run rest || [[ -n $run ]]; do
    [[ -z $run || $run == \#* ]] && continue
    [[ $run =~ ^[0-9]+$ ]] || exit 1
    if [[ ! -s "$dir/lpedfiles/$run.lped" ]]; then
        "$script_dir/mutate_lped.pl" "$run" "$dir" || exit 1
    fi
    [[ -s "$dir/lpedfiles/$run.lped" ]] || exit 1
    names+=("$dir/lpedfiles/$run.lped")
done < "$list"
((${#names[@]})) || exit 1
cd "$scratch" || exit 1
sort -n -o temp.lped "${names[@]}" || exit 1
sed -i -e 's/\t/ /g' temp.lped || exit 1
# swap modules 18 and 26 in that one run

                                              sed -i -e "s/\t/ /g" temp.lped || exit 1
                                              echo "spaced"

                                              for (( i = 0; i < 10; i++ ))
                                                  do
                                                      sed -i -e "s/26 $i 6947576 3 2/18XX $i 6947576 3 2/" temp.lped || exit 1
                                                      sed -i -e "s/18 $i 6947576 3 2/26XX $i 6947576 3 2/" temp.lped || exit 1
                                                      done

                                                      sed -i -e "s/XX//" temp.lped || exit 1

                                                      echo "swapped"
# remove some runs.
# 7083132 T4: completely wrong window.
# 5769394, 5811314, 5880607 T2: Pulse very late -> not reliable for lpeds
# 5423738, 5423941, 5508384, 5625859, 5625860 -> 64 sample readout

                                                      sed -i -e "/7083132 3/d" -e "/5769394 1/d" -e "/5811314 1/d" -e "/5880607 1/d" temp.lped || exit 1
                                                      sed -i -e "/5423738/d" -e "/5423941/d" -e "/5508384/d" -e "/5625859/d" -e "/5625860/d" temp.lped || exit 1
                                                      echo "removed runs"
# remove bogus lped of L2 channels
# T3, 443 became parasitic L2 channel after 52186
                                                      sed -i -e "/^190 3 [56789]...... 2 443/d" temp.lped || exit 1
# T1 channels 110/255/404, T2, c173, T3 c 37/159/319, T4 c 99/259 became L2 channel after 36254
                                                      sed -i -e "/^142 7 [56789]...... 2 37/d" temp.lped || exit 1
                                                      sed -i -e "/^63 3 [56789]...... 1 173/d" temp.lped || exit 1
                                                      echo "removed L2"
# remove channels that have no pulse in LG
# what about  "203 9", "89 3", "192 3", "24 3", "38 8"
# 111 5: T1 c15 -> very small pulse? Fixed in summer 2014 (run 74220)
# 196 4: T1 c294 -> swapped out in summer 2014
#  37 1: T1 c441 Fixed
#  34 4: T1 c464 Fixed
# 104 2: T2 c452 Fixed
# 108 8: T2 c328, T3 c188 Fixed
# 177 2: T3 c312 Fixed
# 169 1: T3 c371 Fixed
# 142 8: T3 c38 broke between runs 7309495 and 7348488.

                                                      awk '{if ( \
	( $1==111 && $2==5 && $3 < 7422000 ) || \
	( $1==196 && $2==4 && $3 > 5000000 ) || \
	( $1==37 && $2==1 &&  $3 < 7422000 ) || \
	( $1==34 && $2==4 &&  $3 < 7422000 ) || \
	( $1==104 && $2==2 &&  $3 < 7422000 ) || \
	( $1==108 && $2==8 &&  $3 < 7422000 ) || \
	( $1==177 && $2==2 && $3>5709596 && $3 < 7422000 ) || \
	( $1==169 && $2==1 && $3 < 7422000 ) || \
	( $1==142 && $2==8 && $3>7309495  ) \
   ) \
{ print $1, $2, $3, $4, $5, 999, $7, $8, $9, $10, $11, $12, $13, $14, $15, $16, $17, $18, $19, $20, $21, $22, $23, $24, $25, $26; }
                                                      else
                                {
                                    print $0
                                }
}' temp.lped > "$output_tmp" || exit 1
mv -f -- "$output_tmp" "$output" || exit 1
echo "awked"

exit
