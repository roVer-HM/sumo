
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2013-2021 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    bonnmotion.py
# @author  Stefan Schuhbaeck
# @date    2023-05-23

"""
This module includes functions for converting SUMO's fcd-output into
a BonnMotion trace file as for instance in OMNeT++ [1].

Note1: For OMNeT++ the coordiante systems origin is in the upper right corner.
Therefore, the coordinates from the fcd-output have to me moved.  (use --bonnmotion-omnet-origin).

Note2: To ensure that the node numbering using bonnmotion traces and traci runs are identical the 
id's have to be sorted in ascending order (as numbers not strings.)

[1]: https://doc.omnetpp.org/inet/api-current/neddoc/inet.mobility.single.BonnMotionMobility.html
"""
from __future__ import print_function
from __future__ import absolute_import


def fcd2bonnmotion(inpFCD, outSTRM, further):
    omnet_orig = False
    if "bbox" in further:
        y_height = further["bbox"][1][1]
        omnet_orig = True
    
    lines = {}
    sumo_id_map = {}
    for timestep in inpFCD:
        for v in timestep.vehicle:
            _trace = lines.get(v.id, [])
            if omnet_orig:
                _y = y_height - v.y
                if _y < 0:
                    print("warning: negative y coordinate after switching to OMNeT origin. %s" % v)
            else:
                _y = v.y
            _trace.append("%s %s %s" % (timestep.time, v.x, _y)) 
            lines[v.id] = _trace
    # sort by numeric id if possible to ensure nodes are 
    try:
        _ids = [int(_id) for _id in lines.keys()]
        _ids.sort()
    except Exception as e:
        _ids = list(lines.keys())
        print("warning: Sorting node ids as numbers did not work. This is needed to ensure same node order if comparing trace and traci runs.")
    
    # print sumo ids as space separated list in comment of bonnMotion file to allow 
    # mapping between bonnmotion and fcd export. 
    print("# BonnMotion file (2D) time x y time x y ...", file=outSTRM)
    print("# %s nodes." % len(_ids), file=outSTRM)
    id_str = ' '.join([str(i) for i in _ids])
    print("# Sumo id map: %s" % id_str, file=outSTRM)
    for _id in _ids:
        _trace = lines[str(_id)]
        print(" ".join(_trace), file=outSTRM)
        
    
