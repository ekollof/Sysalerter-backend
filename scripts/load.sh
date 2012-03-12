#!/bin/sh


prstat -c -Z 1 1

echo
echo "Processes or zones that are on the top of the list are the ones"
echo "that generate the most load, and need to be investigated."
echo
