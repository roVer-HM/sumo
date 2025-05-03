# SUMO in CrowNet: Overview of Modifications
This fork of the Eclipse SUMO project includes several adaptations for CrowNet. 
While some are specific for CrowNet, we have and will continue to submit
pull requests for all adaptions which are of general use for the SUMO
community. 

Thanks to the DLR for making this great project publicly available!
Please refer to the Eclipse SUMO website for all general questions
regarding SUMO: https://github.com/eclipse-sumo/sumo

## Trace Export in BonnMotion format
We add a new trace export for exporting traces in BonnMotion format.

Commit: d7e813026d5bfa94532f646bdce4fe73856f7766 by Stefan Schuhbäck.

## Fixes for OMNeT++ Coordinate Transformation in BonnMontion trace export
In `tools/sumolib/output/convert/bonnmotion.py`, a small fix was added in fcd2bonnmotion(...) which checks for negative coordinates and converts x and y values. Furthermore, we remove traces with too few data paints.

Commit: fafa3cd11ebe8adfc3529e35afdf7478e6e85a25 by Stefan Schuhbäck.

## Export sumo id in BonnMontion trace export
In `tools/sumolib/output/convert/bonnmotion.py`, a new comment lists the SUMO IDs to allow mapping between bonnmotion and fcd export.

Commit: 9313d8ad03904f8f0c4e4802a802c23d9a0ee3c8 by Stefan Schuhbäck.

## Syntax compatibility for Python 2 in bonnmotion traceExporter
In `tools/sumolib/output/convert/bonnmotion.py`, print statements are adapted to be compatible with Python 2 syntax.

Commit: a6a27b83562eaf3907f20a6bc8616a579783041f by Stefan Schuhbäck.

## Added a new bound-box argument to create random trips in a specified bounding box
In `tools/randomTrips.py` a small patch adds a new command line parameter to create random trips in a specified bounding box.

Commit: c2d6892ff9a30ad09c296391b8e014a7d5ece587 by Stefan Schuhbäck.


## To be documented

in tools/traceExporter.py
