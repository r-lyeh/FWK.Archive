#!/bin/bash 2>nul

python art/editor/tools/split.py --input ./fwk.h --output-path ./
python art/editor/tools/split.py --input ./fwk.c --output-path ./
python art/editor/tools/split.py --input ./fwk   --output-path ./
