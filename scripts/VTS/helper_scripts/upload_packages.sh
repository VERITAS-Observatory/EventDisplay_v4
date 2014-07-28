# script to upload packages for releases to UCLA

UCLA=`grep "\* VTSRAWDATA" $VERITAS_EVNDISP_AUX_DIR/ParameterFiles/EVNDISP.global.runparameter | awk '{print $3}'`

echo $UCLA

# upload version might be different from download version
UP="450"
VERSION="EVNDISP-447-auxv01"

bbftp -u bbftp -V -S -m -p 12 -e "put EVNDISP-${UP}.tar.gz /veritas/upload/EVNDISP/v${UP}/EVNDISP-${UP}.tar.gz" $UCLA

for N in calibration lookuptables runfiles dispBDTs Model3D effectiveareas radialacceptances
do
   FILE=${VERSION}.VTS.aux.${N}.tar.gz
   echo $FILE
   bbftp -u bbftp -V -S -m -p 12 -e "put $FILE /veritas/upload/EVNDISP/$UP/$FILE" $UCLA
done
