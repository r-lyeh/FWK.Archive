#!/bin/bash 2>nul

python tools/join.py --template tools/fwk.h.inl --path ./ --output ./tools/fwk.h
python tools/join.py --template tools/fwk.c.inl --path ./ --output ./tools/fwk.c
python tools/join.py --template tools/fwk.x.inl --path ./ --output ./tools/fwk
