#!/bin/bash

mode="$1"

if [ "$mode" != "width" -a "$mode" != "height" ]; then
    echo "Run script with:"
    echo "  $0 mode"
    echo "Where mode is either width or height!"
    exit 1
fi

my_uname=$(uname -s)
screen_width=0
screen_height=0

# Mac
if [ $my_uname == "Darwin" -a "$mode" == "width" ]; then
    screen_width=$(system_profiler SPDisplaysDataType |grep Resolution | awk '{print $2}')
    # If multiple displays this will contain more than one ord 
    if [ $(echo "$screen_width" | wc -w) -gt 1 ];then
        # If more than one display, pick secondary (external on MacBooks)
        screen_width=$(echo $screen_width | awk '{print $2}')
    fi

elif [ $my_uname == "Darwin" -a "$mode" == "height" ]; then
    screen_height=$(system_profiler SPDisplaysDataType |grep Resolution | awk '{print $4}')
    # If multiple displays this will contain space
    if [ $(echo "$screen_height" | wc -w) -gt 1 ];then
        # If more than one display, pick secondary (external on MacBooks)
        screen_height=$(echo $screen_height | awk '{print $2}')
    fi

# Linux
elif [ $my_uname == "Linux" -a "$mode" == "width" ]; then
    screen_width=$(xrandr |grep '*' | head -n 1 | awk '{print $1}' | sed 's/x/ /g' | awk '{print $1}')

elif [ $my_uname == "Linux" -a "$mode" == "height" ]; then
    screen_height=$(xrandr |grep '*' | head -n 1 | awk '{print $1}' | sed 's/x/ /g' | awk '{print $2}')
fi

if [ "$mode" == "width" ]; then
    echo $screen_width
else
    echo $screen_height
fi

