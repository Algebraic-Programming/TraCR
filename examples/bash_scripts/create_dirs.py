#!/usr/bin/env python3
import os
import sys

for d in sys.argv[1:]:
    os.makedirs(d, exist_ok=True)
