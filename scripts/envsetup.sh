#!/bin/sh

# SPDX-License-Identifier: GPL-2.0-or-later
#
# Copyright (C) Telechips Inc.

red=`tput setaf 1`
green=`tput setaf 2`
yellow=`tput setaf 3`
reset=`tput sgr0`

function select_platform()
{
	echo "${red}[1] Telechips Android Platform"
	echo "[2] Telechips Android Platform (64bit)"
	echo "[3] Telechips Linux Platform"
	echo "[4] Telechips Linux Platform (64bit)${reset}"
	echo -n "${green}Select platform (default 4):${reset} "
	read index

	case $index in
	1 )	export ARCH=arm
		export CROSS_COMPILE=arm-none-linux-gnueabihf-
		;;
	2 )	export ARCH=arm64
		export CROSS_COMPILE=aarch64-none-linux-gnu-
		;;
	3 )	export ARCH=arm
		export CROSS_COMPILE=arm-none-linux-gnueabihf-
		;;
"" |	4 )	export ARCH=arm64
		export CROSS_COMPILE=aarch64-none-linux-gnu-
		;;
	* )	export CROSS_COMPILE=
		echo "${red}Invalid platform !!${reset}"
		return 1
		;;
	esac
}

function select_soc_family()
{
	echo "${red}"
	echo "[1] TCC805x"
	echo "[2] TCC803x"
	echo -n "${green}Select SoC family (default 1):${reset} "
	read index

	case $index in
"" |	1 )	select_tcc805x_board ;;
	2 )	select_tcc803x_board ;;
	* )	echo "${red}Invalid SoC family !!${reset}"
		return 1
		;;
	esac
}

function select_tcc805x_board()
{
	echo "${red}"
	echo "[1] TCC8050 EVB 1.0"
	echo "[2] TCC8053 EVB 1.0"
	echo "[3] TCC8050 EVB 0.1"
	echo "[4] TCC8053 EVB 0.1"
	echo "[5] TCC8059 EVB 0.1"
	echo -n "${green}Select board (default 1):${reset} "
	read index

	case $index in
"" |	1 )	chip=tcc8050
		board=evb_sv1.0
		;;
	2 )	chip=tcc8053
		board=evb_sv1.0
		;;
	3 )	chip=tcc8050
		board=evb_sv0.1
		;;
	4 )	chip=tcc8053
		board=evb_sv0.1
		;;
	5 )	chip=tcc8059
		board=evb_sv0.1
		;;
	* )	echo "${red}Invalid board !!${reset}"
		return 1
		;;
	esac

	echo "${red}"
	echo "[1] Maincore"
	echo "[2] Subcore"
	echo -n "${green}Select core (default 1):${reset} "
	read index

	case $index in
"" |	1 )	export DEVICE_TREE=${chip}-${board} ;;
	2 )	export DEVICE_TREE=${chip}-subcore-${board} ;;
	* )	echo "${red}Invalid core !!${reset}"
		return 1
		;;
	esac
}

function select_tcc803x_board()
{
	echo "${red}"
	echo "[1] TCC803XP EVB 1.0"
	echo "[2] TCC8030 EVB 0.1"
	echo -n "${green}Select board (default 1):${reset} "
	read index

	case $index in
"" |	1 )	chip=tcc803xp
		board=evb_sv1.0
		;;
	2 )	chip=tcc8030
		board=evb_sv0.1
		;;
	* )	echo "${red}Invalid board !!${reset}"
		return 1
		;;
	esac

	echo "${red}"
	echo "[1] Maincore"
	echo -n "${green}Select core (default 1):${reset} "
	read index

	case $index in
"" |	1 )	export DEVICE_TREE=${chip}-${board} ;;
	* )	echo "${red}Invalid core !!${reset}"
		return 1
		;;
	esac
}

select_platform && select_soc_family

echo "${yellow}"
echo "### U-Boot Environment Setup Complete ###"
echo "${reset}"

echo "- ARCH=$ARCH"
echo "- CROSS_COMPILE=$CROSS_COMPILE"
echo "- DEVICE_TREE=$DEVICE_TREE"
echo ""
