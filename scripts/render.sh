#!/bin/bash

head -1 "board.wrl" > "board.front.wrl"
cat <<EOF >> "board.front.wrl"
Transform {
    children [
        DirectionalLight {
            on TRUE
            intensity 0.63
            ambientIntensity 0.21
            color 1.0 1.0 1.0
            direction 0.1 -0.1 -1
        }
EOF
cat "board.wrl" >> "board.front.wrl"
echo "] }" >> "board.front.wrl"

./rayhunder/rayhunter classic 7 \
    4320 4320 \
    "board.front.wrl" \
    "board.front.png" \
    --camera-pos 0 0 6 \
    --camera-dir 0 0 -1 \
    --scene-bg-color 1 1 1

convert \
    "board.front.png" \
    -trim \
    -resize 1060x1060 -normalize -density 600 \
    -bordercolor white -border 10 \
    -gravity center -extent 1080x1080 \
    "board.front.png"