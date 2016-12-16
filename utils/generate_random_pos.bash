#!/bin/bash

if [ "$#" -ne 5 ]
then
    echo "Usage: "
    echo "  $0 file_to_load height scale nr_of_instances limit"
    exit 1
fi

file_to_load=$1
height=$2
scale=$3
nr_instances=$4
limit=$5
rotation=""
x_pos=""
z_pos=""

for i in $(seq 1 $nr_instances); do
    if [ "$(uname -s)" == "Darwin" ]; then
        rotation=$(jot -r 1 0 360)
        x_pos=$(jot -r 1 -$limit $limit)
        z_pos=$(jot -r 1 -$limit $limit)
    else
        rotation=$(shuf -i0-360 -n1)
        x_pos=$(seq -$limit $limit | shuf -n1)
        z_pos=$(seq -$limit $limit | shuf -n1)
    fi
    echo "$file_to_load | rel | 0 $rotation 0 | $x_pos $height $z_pos | $scale | -1 -1 | -1"
done


