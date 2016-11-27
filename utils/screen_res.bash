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
    screen_width=$(/usr/bin/osascript -e 'tell application "Finder" to get bounds of window of desktop' |sed 's/\,//g' | awk '{print $3}')
elif [ $my_uname == "Darwin" -a "$mode" == "height" ]; then
    screen_height=$(/usr/bin/osascript -e 'tell application "Finder" to get bounds of window of desktop' |sed 's/\,//g' | awk '{print $4}')

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

