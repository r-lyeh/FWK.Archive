#!/bin/bash 2>nul

python tools/bin/join.py --template tools/bin/fwk.h.inl --path ./ --output ./fwk.h
python tools/bin/join.py --template tools/bin/fwk.c.inl --path ./ --output ./fwk.c
python tools/bin/join.py --template tools/bin/fwk.x.inl --path ./ --output ./fwk
