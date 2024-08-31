#!/bin/bash 2>nul

python tools/split.py --input ./split/fwk.h --output-path ./
python tools/split.py --input ./split/fwk.c --output-path ./
python tools/split.py --input ./split/fwk   --output-path ./
