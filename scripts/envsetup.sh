#!/bin/bash

# SPDX-License-Identifier: GPL-2.0-or-later
#
# Copyright (C) 2023 Telechips Inc.

red=`tput setaf 1`
green=`tput setaf 2`
yellow=`tput setaf 3`
reset=`tput sgr0`

function _select_file_from_list()
{
	selected=
	list=("$@")
	desc=${list[-1]}

	unset list[-1]

	echo ""
	for i in ${!list[*]}; do
		echo ${red}[$((i + 1))] $(basename ${list[${i}]})${reset}
	done
	echo -n "${green}Select ${desc} (default ${#list[@]}):${reset} "
	read index

	if [ -z "${index}" ]; then
		index=${#list[@]}
	fi

	if [ "${index}" -lt "1" ] || [ -z "${list[$((index - 1))]}" ]; then
		echo "${red}Invalid ${desc}!${reset}"
		return 1
	fi

	selected=$(basename ${list[$((index - 1))]})
}

function select_build()
{
	echo "${red}[1] AArch64 build${reset}"
	echo -n "${green}Select build (default 1):${reset} "
	read index

	case ${index} in
"" |	1 )	ARCH=arm64
		CROSS_COMPILE=aarch64-none-linux-gnu-
		;;
	* )	echo "${red}Invalid build!${reset}"
		return 1
		;;
	esac
}

function select_chip()
{
	chips=($(ls -d arch/arm/dts/telechips/*/ | sort))

	if ! _select_file_from_list ${chips[@]} "chip"; then
		return 1
	fi

	dtsdir_r=telechips/${selected}
	dtsdir=arch/arm/dts/${dtsdir_r}
	dtslist=$(cat ${dtsdir}/Makefile | sed -n 's/^.* += \(.*\.dtb\)/\1/p')
}

function select_core()
{
	filter="grep ."

	if [[ ! "${dtslist}" =~ "-subcore-" ]]; then
		return
	fi

	echo ""
	echo "${red}[1] main core${reset}"
	echo "${red}[2] sub-core${reset}"
	echo -n "${green}Select core (default 1):${reset} "
	read index

	case ${index} in
"" |	1 )	filter="grep -Ev subcore"
		;;
	2 )	filter="grep -E subcore"
		;;
	* )	echo "${red}Invalid core!${reset}"
		return 1
		;;
	esac
}

function select_tree()
{
	trees=($(echo "${dtslist[@]}" | ${filter} | sort))

	if ! _select_file_from_list ${trees[@]} "device tree"; then
		return 1
	fi

	DEVICE_TREE=${dtsdir_r}/$(echo ${selected} | sed 's/\.dtb$//')
}

function report_and_set_env()
{
	echo "${yellow}"
	echo "### U-Boot Environment Setup Complete ###"
	echo "${reset}"

	echo "- ARCH=${ARCH}"
	echo "- CROSS_COMPILE=${CROSS_COMPILE}"
	echo "- DEVICE_TREE=${DEVICE_TREE}"
	echo ""

	export ARCH CROSS_COMPILE DEVICE_TREE
}

select_build && select_chip && select_core && select_tree && report_and_set_env
