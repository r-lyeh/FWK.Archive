#!/bin/bash 2>nul

python tools/join.py --template tools/fwk.h.inl --path ./ --output ./split/fwk.h
python tools/join.py --template tools/fwk.c.inl --path ./ --output ./split/fwk.c
python tools/join.py --template tools/fwk.x.inl --path ./ --output ./split/fwk
