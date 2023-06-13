
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
    min_len = further["bonnmotion_min_trace_len"]
    skip_min_len = further["bonnmotion_min_trace_skip"]
    if "bbox" in further:
        y_max = further["bbox"][1][1]
        x_min = further["bbox"][0][0]
        omnet_orig = True
    
    lines = {}
    sumo_id_map = {}
    for timestep in inpFCD:
        for v in timestep.vehicle:
            _trace = lines.get(v.id, [])
            if omnet_orig:
                _x = v.x - x_min
                _y = y_max - v.y
                if _y < 0:
                    print("warning: negative y coordinate after switching to OMNeT origin. %s" % v)
                if _x < 0:
                    print("warning: negative x coordinate after switching to OMNeT origin. %s" % v)
            else:
                _x = v.x
                _y = v.y
            _trace.append("%s %s %s" % (timestep.time, _x, _y)) 
            lines[v.id] = _trace
    # sort by numeric id if possible to ensure nodes are 
    try:
        _ids = [int(_id) for _id in lines.keys()]
        _ids.sort()
    except Exception as e:
        _ids = list(lines.keys())
        print("warning: Sorting node ids as numbers did not work. This is needed to ensure same node order if comparing trace and traci runs.")

    # clear traces with to few data points.
    _remove_ids = []
    for id_index, _id in enumerate(_ids):
        _trace = lines[str(_id)]
        if len(_trace) <= min_len:
            if skip_min_len:
                _remove_ids.append(id_index)
                del lines[str(_id)]
                print("warning: removed trace for id %i as it has to few data points. Expected at least %i points in trace, got %i." % (_id, min_len, len(_trace)))
                continue
            else:
                raise ValueError("Expected at least %s points in trace, got %s." % (min_len, len(_trace)))
    # remove id's in reverse order. (trace lines are already removed.)
    _remove_ids.sort(reverse=True)
    for i in _remove_ids:
        del _ids[i]


    # print sumo ids as space separated list in comment of bonnMotion file to allow 
    # mapping between bonnmotion and fcd export. 
    print("# BonnMotion file (2D) time x y time x y ...", file=outSTRM)
    print("# %s nodes." % len(_ids), file=outSTRM)
    id_str = ' '.join([str(i) for i in _ids])
    print("# Sumo id map: %s" % id_str, file=outSTRM)
    for _id in _ids:
        _trace = lines[str(_id)]
        print(" ".join(_trace), file=outSTRM)
        
    
