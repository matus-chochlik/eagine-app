#!/bin/bash
grep -H -c -e '\([A-F0-9]\)\1' "$(dirname ${0})/"*.txt |\
grep -v ':0$' |\
cut -d: -f1
