#!/bin/bash 2>nul

python art/editor/tools/join.py --template art/editor/tools/fwk.h.inl --path ./ --output ./fwk.h
python art/editor/tools/join.py --template art/editor/tools/fwk.c.inl --path ./ --output ./fwk.c
python art/editor/tools/join.py --template art/editor/tools/fwk.x.inl --path ./ --output ./fwk
