#!/bin/bash 2>nul

python editor/tools/join.py --template editor/tools/fwk.h.inl --path ./ --output ./fwk.h
python editor/tools/join.py --template editor/tools/fwk.c.inl --path ./ --output ./fwk.c
python editor/tools/join.py --template editor/tools/fwk.x.inl --path ./ --output ./fwk
