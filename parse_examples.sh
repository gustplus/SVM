#!/bin/sh

cur_path=$(pwd)
examples_path=${cur_path}"/examples"

function parse_dir() {
	filelist=`ls ${examples_path}`
	for file in $filelist; do
		file_full_name=$1"/"${file}
		if((test -f ${file_full_name})&&([ "${file#*.}"=="svm" ])); then
			echo "parse ${file_full_name}"
			`./parser ${file_full_name}>/dev/null`
		elif test -d $file_full_name; then
			parse_dir $file_full_name
		fi
	done
}

parse_dir $examples_path
