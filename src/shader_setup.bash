#!/bin/bash

_MAX_LIGHTS_=$1
build_shader_dir=build/shaders
shader_path="src/shaders"
fragment_shaders=$(ls "$shader_path" | grep .frag)
vertex_shaders=$(ls "$shader_path" | grep .vert)

mkdir -p "$build_shader_dir"

for shader in $fragment_shaders
do
  touch "$build_shader_dir/$shader"
  echo "#version 330 core" > "$build_shader_dir/$shader"
  echo "#define _MAX_LIGHTS_=$_MAX_LIGHTS_" >> "$build_shader_dir/$shader"
  cat "$shader_path/$shader" >> "$build_shader_dir/$shader"
done

for shader in $vertex_shaders
do
  touch "$build_shader_dir/$shader"
  echo "#version 330 core" > "$build_shader_dir/$shader"
  cat "$shader_path/$shader" >> "$build_shader_dir/$shader"
done
