#!/bin/bash 2>nul

python tools/bin/split.py --input ./fwk.h --output-path ./
python tools/bin/split.py --input ./fwk.c --output-path ./
python tools/bin/split.py --input ./fwk   --output-path ./
