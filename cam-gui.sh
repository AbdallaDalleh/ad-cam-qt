#!/bin/bash

export EPICS_DISPLAY_PATH=/opt/epics/support/areaDetector/core/op/adl:/opt/epics/support/areaDetector/genicam/op/adl:/opt/epics/support/areaDetector/aravis/op/adl:/opt/epics/support/areaDetector/genicam/op/adl

medm -x -macro "P=${1}:,R=${2}:,C=Basler-acA1300-30gm" ADAravis.adl
