#!/bin/bash 2>nul

python tools/split.py --input ./tools/fwk.h --output-path ./
python tools/split.py --input ./tools/fwk.c --output-path ./
python tools/split.py --input ./tools/fwk   --output-path ./
