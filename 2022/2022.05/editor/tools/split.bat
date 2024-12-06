#!/bin/bash 2>nul

python editor/tools/split.py --input ./fwk.h --output-path ./
python editor/tools/split.py --input ./fwk.c --output-path ./
python editor/tools/split.py --input ./fwk   --output-path ./
