#!/bin/bash
# This script requires ImageMagick 7+
# https://github.com/SoftCreatR/imei/
#
#temp_dir=$(mktemp -d)
#echo "Temp dir: $temp_dir"
#icon256="$temp_dir/256.png"
#icon48="$temp_dir/48.png"
#icon32="$temp_dir/32.png"
#icon16="$temp_dir/16.png"

source="$1"
if [[ "$1" == *.ico ]]; then
  sizes=$(identify -format "%wx%h;%p\n" "$1")
  echo "$sizes"
  largest_info=$(echo "$sizes" | sort -n | tail -n 1)
  largest_size=$(echo "$largest_info" | cut -d ';' -f 1)
  frame_number=$(echo "$largest_info" | cut -d ';' -f 2)
  if [ -n "$frame_number" ]; then
    echo "Largest size: $largest_size Frame: $frame_number"
	source="$source[$frame_number]"
  else
    echo "Could not detect largest frame"
  fi
fi

magick -background transparent "$source" -define icon:auto-resize=16,32,48,256 "$2"