#!/bin/sh

# Function to show usage
function usage() {
    cat <<EOT
Usage:  $0

	-o|--out: specify output file
	-p|--page-size: specify page size (default size: 512)
	-c|--count: the number of bmp files
	-s|--source: specify source files
	-S|--splash-partition: specify a splash partition name

	By default, this will be set to 'splash'. DO NOT USE
	'--splash-partition' option in the SDK which contains a 'splash'
	partition only. This option should be used for the SDK for tcc805x
	above; each core uses its own splash partition (splash and
	subcore_splash).

	For multiple splash partition environment like TCC805x, you can specify
	partition as following:

	- to change boot-logo displayed via main-core
	  use --splash-partition splash (can be omitted)

	- to change parking guide line displayed via sub-core
	  use --splash-partition subcore_splash

Example:
	$0 -o mysplash.img -s bmpfile1.bmp bmpfile2.bmp bmpfile3.bmp
EOT
}

# Check the arguments count
if [ "$#" -eq "0" ]; then
    usage
    exit 1
fi

# Arguments type
## Parsing options
PARAMS=""
SOURCES=""
PARTITION="splash"

while (( "$#" )); do
    case "$1" in
	-p|--page-size)
	    PAGESIZE=$2
	    shift 2
	    ;;
	-l|--length)
	    LENGTH=$2
	    shift 2
	    ;;
	-c|--count)
	    LENGTH=$2
	    shift 2
	    ;;
	-o|--out)
	    OUTFILE=$2
	    shift 2
	    ;;
	-s|--source)
	    SOURCES="$SOURCES $2"
	    shift 2
	    ;;
	-S|--splash-partition)
		PARTITION=$2
		shift 2
		;;
	--) # end argument parsing
	    SOURCES="$SOURCES $1"
	    shift
	    ;;
	-*|--*=) # unsupported flags
	    echo "Error: Unsupported flag $1" >&2
	    exit 1
      	    ;;
	*) # preserve positional arguments
	    SOURCES="$SOURCES $1"
	    shift
	    ;;
    esac
done

# Adjust parameters

## LENGTH: the number of source image files
LENGTH=$(echo $SOURCES | tr ' ' '\n' | wc -l)

if [ -z "$SOURCES" ]; then
    echo "[ERROR] Source file has not been inputed";
    exit 2;
fi

## Check page size and use a default value if it is empty
if [ -z "$PAGESIZE" ] || [ "$PAGESIZE" -eq "0" ]; then
    PAGESIZE=512
fi

## Check outfile
if [ -z "$OUTFILE" ]; then
    echo "[ERROR] the name of output file is empty";
    exit 3;
fi

## Check page size
if [ "$PAGESIZE" -lt "512" ] ; then
    echo "[ERROR] Page size should be larger than 512."
    exit 4
fi

## Convert string to array
SRC_IMG=(${SOURCES// / })
for i in "${!SRC_IMG[@]}"
do
    echo "$i=>${SRC_IMG[i]}" >/dev/null
done

# Start to create a splash image file
echo "GENERATING SPLASH IMGAE ........"

# Remove temp files
rm -rf *.img *.tmp

for ((idx=0; idx<$LENGTH; idx++));
do
    IMG_ORI[$idx]=${SRC_IMG[$idx]}
    IMG_EXT[$idx]=${IMG_ORI[$idx]%%.*}.tmp
    IMG_FN[$idx]=${IMG_ORI[$idx]%%.*}.img

    FILE_FMT[$idx]=$(identify "${IMG_ORI[$idx]}" | cut -f 2 -d' ')
    IMG_RSL[$idx]=$(identify "${IMG_ORI[$idx]}" | cut -f 3 -d' ')
    IMG_BITS[$idx]=$(file "${IMG_ORI[$idx]}" | cut -f 11 -d' ')

    if [[ "${FILE_FMT[idx]}" == "BMP"* ]]; then
	echo "[CHECK] source image format is BMP"

	FMTSIZE=`./bmpbpp ${IMG_ORI[$idx]}`

	if [ "$FMTSIZE" -eq "32" ]; then
	    echo "[CHECK] Pixel size is 32 bit"
	    convert -depth 8 ${IMG_ORI[$idx]} rgba:${IMG_EXT[$idx]}
	    ./rgbto8888 < ${IMG_EXT[$idx]} > ${IMG_FN[$idx]}

	elif [ "$FMTSIZE" -eq "24" ];  then
	    echo "[CHECK] Pixel size is 24 bit"
	    convert -depth 8 ${IMG_ORI[$idx]} rgb:${IMG_EXT[$idx]}
	    ./rgbto888 < ${IMG_EXT[$idx]} > ${IMG_FN[$idx]}

	elif [ "$FMTSIZE" -eq "16" ]; then
	    echo "[CHECK] Pixel size 16 bit"
	    convert -depth 8 ${IMG_ORI[$idx]} rgb:${IMG_EXT[$idx]}
	    ./rgbto565 < ${IMG_EXT[$idx]} > ${IMG_FN[$idx]}

	else
	    echo "[ERROR] choose fmt 16 or 24 or 32"
	    exit 6
	fi

    elif [ "${FILE_FMT[idx]}" = "JPEG" ]; then
	echo "source image format is JPEG"
	cp ${IMG_ORI[$idx]} ${IMG_FN[$idx]}
    fi
done

# echo ${IMG_RSL[*]}
# echo ${IMG_BITS[*]}

MK_SPLASH="./mksplash $PAGESIZE $LENGTH $PARTITION ${IMG_FN[*]} ${IMG_RSL[*]} ${FILE_FMT[*]} $OUTFILE"
echo $MK_SPLASH

# Create splash image
$MK_SPLASH
