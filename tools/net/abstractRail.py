#!/usr/bin/env python
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2007-2023 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    abstractRail.py
# @author  Jakob Erdmann
# @date    2023-02-22

"""
Convert a geodetical rail network in a abstract (schematic) rail network.
If the network is segmented (stationDistricts.py), the resulting network will be
a hybrid of multple schematic pieces being oriented in a roughly geodetical manner
"""
from __future__ import absolute_import
from __future__ import print_function
import os
import sys
import subprocess
import numpy as np
import math
import time
import scipy.optimize as opt
from collections import defaultdict
SUMO_HOME = os.environ.get('SUMO_HOME',
                           os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
sys.path.append(os.path.join(SUMO_HOME, 'tools'))
import sumolib  # noqa
from sumolib.options import ArgumentParser  # noqa
from sumolib.miscutils import Colorgen  # noqa
import sumolib.geomhelper as gh  # noqa

INTERSECT_RANGE = 1e5
COMPRESSION_WEIGHT = 0.01
STRAIGHT_WEIGHT = 2
OTHER_WEIGHT = 1


def get_options():
    ap = ArgumentParser()
    ap.add_option("-v", "--verbose", action="store_true", default=False,
                  help="tell me what you are doing")
    ap.add_option("-n", "--net-file", dest="netfile",
                  help="the network to read lane and edge permissions")
    ap.add_option("-s", "--stop-file", dest="stopfile",
                  help="the additional file with stops")
    ap.add_option("-a", "--region-file", dest="regionfile",
                  help="Load network regions from additional file (as taz elements)")
    ap.add_option("-o", "--output-prefix", dest="prefix",
                  help="output prefix for patch files")
    ap.add_option("--filter-regions", dest="filterRegions",
                  help="filter regions by name or id")
    ap.add_option("--keep-all", action="store_true", dest="keepAll", default=False,
                  help="keep original regions outside the filtered regions")
    ap.add_option("--horizontal", action="store_true", dest="horizontal", default=False,
                  help="output shapes roughly aligned along the horizontal")
    ap.add_option("--track-offset", type=float, default=20, dest="trackOffset",
                  help="default distance between parallel tracks")
    ap.add_option("--time-limit", type=float, dest="timeLimit",
                  help="time limit per region")
    ap.add_option("--max-iter", type=int, dest="maxIter",
                  help="maximum number of solver iterations per region")
    ap.add_option("--skip-large", type=int, dest="skipLarge",
                  help="skip regions require more than the given number of constraints")
    ap.add_option("--skip-yopt", action="store_true", dest="skipYOpt", default=False,
                  help="do not optimize the track offsets")
    ap.add_option("--skip-building", action="store_true", dest="skipBuilding", default=False,
                  help="do not call netconvert with the patch files")
    ap.add_option("--extra-verbose", action="store_true", default=False, dest="verbose2",
                  help="tell me more about what you are doing")
    options = ap.parse_args()

    if not options.netfile:
        ap.print_help()
        ap.exit("Error! setting net-file is mandatory")
    if not options.stopfile:
        ap.print_help()
        ap.exit("Error! setting stop-file is mandatory")
    if not options.prefix:
        ap.print_help()
        ap.exit("Error! setting output is mandatory")

    options.output_nodes = options.prefix + ".nod.xml"
    options.output_edges = options.prefix + ".edg.xml"
    options.output_net = options.prefix + ".net.xml"

    options.filterRegions = set(options.filterRegions.split(",")) if options.filterRegions else set()

    return options


def loadRegions(options, net):
    regions = dict()
    if options.regionfile:
        for taz in sumolib.xml.parse(options.regionfile, 'taz'):
            name = taz.attr_name
            if (options.filterRegions
                and name not in options.filterRegions
                    and taz.id not in options.filterRegions):
                continue
            edgeIDs = taz.edges.split()
            regions[name] = [net.getEdge(e) for e in edgeIDs if net.hasEdge(e)]
    else:
        regions['ALL'] = list(net.getEdges())
    return regions


def filterBidi(edges):
    return [e for e in edges if e.getBidi() is None or e.getID() < e.getBidi().getID()]


def initShapes(edges, nodeCoords, edgeShapes):
    nodes = getNodes(edges)

    for node in nodes:
        nodeCoords[node.getID()] = node.getCoord()

    for edge in edges:
        edgeShapes[edge.getID()] = edge.getShape(True)


def findMainline(options, net, edges):
    """use platforms to determine mainline orientation"""
    knownEdges = set([e.getID() for e in edges])

    angles = []
    for stop in sumolib.xml.parse(options.stopfile, ['busStop', 'trainStop']):
        # name = stop.getAttributeSecure("attr_name", stop.id)
        edgeID = stop.lane.rsplit('_', 1)[0]
        if edgeID not in knownEdges:
            continue
        edge = net.getEdge(edgeID)
        begCoord = gh.positionAtShapeOffset(edge.getShape(), float(stop.startPos))
        endCoord = gh.positionAtShapeOffset(edge.getShape(), float(stop.endPos))
        angles.append((gh.angleTo2D(begCoord, endCoord), (begCoord, endCoord)))

    angles.sort()
    mainLine = angles[int(len(angles) / 2)][1]

    if options.verbose2:
        print("mainLine=", shapeStr(mainLine))

    return mainLine


def getNodes(edges):
    nodes = set()
    for edge in edges:
        nodes.add(edge.getFromNode())
        nodes.add(edge.getToNode())
    return nodes


def rotateByMainLine(mainLine, edges, nodeCoords, edgeShapes, reverse):
    center = mainLine[0]
    angle = gh.angleTo2D(mainLine[0], mainLine[1])
    nodes = getNodes(edges)

    if reverse:
        def transform(coord):
            coord = gh.rotateAround2D(coord, angle, (0, 0))
            coord = gh.add(coord, center)
            return coord
    else:
        def transform(coord):
            coord = gh.sub(coord, center)
            coord = gh.rotateAround2D(coord, -angle, (0, 0))
            return coord

    for node in nodes:
        coord = nodeCoords[node.getID()]
        nodeCoords[node.getID()] = transform(coord)

    for edge in edges:
        shape = edgeShapes[edge.getID()]
        edgeShapes[edge.getID()] = [transform(coord) for coord in shape]


def computeTrackOrdering(options, mainLine, edges, nodeCoords, edgeShapes):
    """
    precondition: network is rotated so that the mainLine is on the horizontal
    for each x-value we imagine a vertical line and find all the edges that intersect
    this gives a list of track orderings
    Then we try to assign integers for each edge that are consistent with this ordering
    """

    # step 1: find ordering constraints
    orderings = []
    nodes = getNodes(edges)
    virtualNodes = defaultdict(list)

    xyNodes = [(nodeCoords[n.getID()], n) for n in nodes]
    xyNodes.sort(key=lambda x: x[0][0])

    for (x, y), node in xyNodes:
        node._newX = x
        vertical = [(x, -INTERSECT_RANGE), (x, INTERSECT_RANGE)]
        ordering = []
        for edge in edges:
            if edge.getFromNode() == node or edge.getToNode() == node:
                continue
            shape = edgeShapes[edge.getID()]
            intersects = gh.intersectsAtLengths2D(vertical, shape)
            intersects = [i - INTERSECT_RANGE for i in intersects]
            # intersects now holds y-values
            if len(intersects) == 1:
                virtualNode = (edge.getID(), x)
                ordering.append((intersects[0], virtualNode))
                virtualNodes[edge].append(virtualNode)
            elif len(intersects) > 1:
                sys.stderr.write(("Cannot compute track ordering for edge '%s'" +
                                  " because it runs orthogonal to the main line (intersects: %s)\n") % (
                    edge.getID(), intersects))
                # print("vertical=%s %s=%s" % (shapeStr(vertical), edge.getID(), shapeStr(shape)))

        if ordering:
            # also append the actual node before sorting
            ordering.append((y, node))
            ordering.sort(key=lambda x: x[0])
            ordering = [vn for y, vn in ordering]
            orderings.append((node.getID(), ordering))

    # for o in orderings: print(o)

    # step 3:
    nodeYValues = optimizeTrackOrder(options, edges, nodes, virtualNodes, orderings, nodeCoords)

    if options.verbose2:
        for k, v in nodeYValues.items():
            print(getVNodeID(k), v)
    return nodeYValues


def getVNodeX(vNode):
    try:
        return vNode._newX
    except:
        return vNode[1]


def getVNodeID(vNode):
    try:
        return vNode.getID()
    except:
        return vNode


def optimizeTrackOrder(options, edges, nodes, virtualNodes, orderings, nodeCoords):

    generalizedNodes = list(nodes)
    generalizedNodes.sort(key=lambda n: n.getID())
    nodeIndex = dict((n, i) for i, n in enumerate(generalizedNodes))

    # collect ordering constraints for nodes and virtual nodes
    yOrderConstraints = []
    for nodeID, ordering in orderings:
        for vNode in ordering:
            if vNode not in nodes:
                nodeIndex[vNode] = len(generalizedNodes)
                generalizedNodes.append(vNode)

        for vNode, vNode2 in zip(ordering[:-1], ordering[1:]):
            yOrderConstraints.append((nodeIndex[vNode], nodeIndex[vNode2]))

    # collect constraints for keeping edges parallel
    ySimilarConstraints = []
    ySameConstraints = []
    yPrios = []
    for edge in edges:
        vNodes = virtualNodes[edge]
        vNodes.append(edge.getFromNode())
        vNodes.append(edge.getToNode())
        vNodes.sort(key=getVNodeX)
        angle = gh.angleTo2D(nodeCoords[edge.getFromNode().getID()], nodeCoords[edge.getToNode().getID()])
        straight = min(abs(angle), abs(angle - math.pi)) < np.deg2rad(10)

        constraints = []
        # print(" ".join(["%s:%s" % (getVNodeID(vn), getVNodeX(vn)) for vn in vNodes]))
        for vNode, vNode2 in zip(vNodes[:-1], vNodes[1:]):
            # print("keepSame: %s | %s" % (getVNodeID(vNode), getVNodeID(vNode2)))
            constraints.append((nodeIndex[vNode], nodeIndex[vNode2]))

        numPrios = 2
        if len(constraints) > 2:
            ySameConstraints += constraints[1:-1]
            ySimilarConstraints.append(constraints[0])
            ySimilarConstraints.append(constraints[-1])
        else:
            ySimilarConstraints += constraints
            numPrios = len(constraints)
        yPrios += [2 if straight else 1] * numPrios
        if options.verbose2:
            print("edge=%s straight=%s vNodes=%s" % (edge.getID(), straight, [getVNodeID(vn) for vn in vNodes]))

    k = len(generalizedNodes)
    m = len(ySimilarConstraints)
    m2 = m * 2
    # n: number of variables
    n = k + m
    # q: number of equations:
    #   2 per ySame constraint to minimize the absolute difference and one per elementary ordering constraint
    q = m2 + len(yOrderConstraints)

    # we use m slack variables for the differences between y-values (one per edge)
    # x2 =  [x, s]

    b_ub = [0] * m2 + [-options.trackOffset] * len(yOrderConstraints)
    A_ub = np.zeros((q, n))

    row = 0
    # encode inequalities for slack variables (minimize differences)
    for slackI, (fromI, toI) in enumerate(ySimilarConstraints):
        slackI += k
        A_ub[row][fromI] = 1
        A_ub[row][toI] = -1
        A_ub[row][slackI] = -1
        row += 1
        A_ub[row][fromI] = -1
        A_ub[row][toI] = 1
        A_ub[row][slackI] = -1
        row += 1

    # encode inequalities for ordering
    for index1, index2 in yOrderConstraints:
        A_ub[row][index1] = 1
        A_ub[row][index2] = -1
        row += 1

    # inner points should all have the same y value
    q2 = len(ySameConstraints)
    A_eq = np.zeros((q2, n))
    b_eq = [0] * q2
    for row, (fromI, toI) in enumerate(ySameConstraints):
        A_eq[row][fromI] = 1
        A_eq[row][toI] = -1

    # minimization objective (only minimize slack variables)
    c = [COMPRESSION_WEIGHT] * k + yPrios

    if options.verbose2:
        print("k=%s" % k)
        print("m=%s" % m)
        print("q=%s" % q)
        print("q2=%s" % q2)
        print("A_ub (%s) %s" % (A_ub.shape, A_ub))
        print("b_ub (%s) %s" % (len(b_ub), b_ub))
        print("A_eq (%s) %s" % (A_eq.shape, A_eq))
        print("b_eq (%s) %s" % (len(b_eq), b_eq))
        print("c (%s) %s" % (len(c), c))

    if options.skipLarge and (q + q2) > options.skipLarge:
        sys.stderr.write("Skipping optimization with %s inequalities and %s equalities\n" % (q, q2))
        return dict()

    linProgOpts = {}
    started = time.time()
    if options.verbose:
        print("Starting optimization with %s inequalities and %s equalities" % (
            q, q2))

    if options.verbose2:
        linProgOpts["disp"] = True

    if options.timeLimit:
        linProgOpts["time_limit"] = options.timeLimit

    if options.maxIter:
        linProgOpts["maxiter"] = options.maxIter

    res = opt.linprog(c, A_ub=A_ub, b_ub=b_ub, A_eq=A_eq, b_eq=b_eq, options=linProgOpts)

    if not res.success:
        sys.stderr.write("Optimization failed\n")
        return dict()

    if options.verbose:
        if options.verbose:
            score = np.dot(res.x, c)
            runtime = time.time() - started
            print("Optimization succeeded after %ss (score=%s)" % (runtime, score))
        if options.verbose2:
            print(res.x)
    yValues = res.x[:k]  # cut of slack variables
    # print(yValues)

    nodeYValues = dict([(vNode, yValues[i]) for vNode, i in nodeIndex.items()])
    return nodeYValues


def patchShapes(options, edges, nodeCoords, edgeShapes, nodeYValues):
    nodes = getNodes(edges)
    edgeYValues = defaultdict(list)

    for node in nodes:
        coord = nodeCoords[node.getID()]
        nodeCoords[node.getID()] = (coord[0], nodeYValues[node])

    for vNode, y in nodeYValues.items():
        if vNode not in nodes:
            edgeID, x = vNode
            edgeYValues[edgeID].append((x, y))

    for edge in edges:
        edgeID = edge.getID()
        fromCoord = nodeCoords[edge.getFromNode().getID()]
        toCoord = nodeCoords[edge.getToNode().getID()]
        shape = [fromCoord, toCoord]
        if edgeID in edgeYValues:
            for coord in edgeYValues[edgeID]:
                if (abs(coord[0] - fromCoord[0]) < options.trackOffset or
                        abs(coord[0] - toCoord[0]) < options.trackOffset):
                    continue
                shape.append(coord)
            shape.sort()
            if fromCoord[0] > toCoord[0]:
                shape = list(reversed(shape))
        edgeShapes[edgeID] = shape


def cleanShapes(options, net, nodeCoords, edgeShapes):
    """Ensure consistent edge shape in case the same edge was part of multiple regions"""
    for edgeID, shape in edgeShapes.items():
        edge = net.getEdge(edgeID)
        fromID = edge.getFromNode().getID()
        toID = edge.getToNode().getID()
        shape[0] = nodeCoords[fromID]
        shape[-1] = nodeCoords[toID]


def shapeStr(shape):
    return ' '.join(["%.2f,%.2f" % coord for coord in shape])


def main(options):

    if options.verbose:
        print("Reading net")
    net = sumolib.net.readNet(options.netfile)

    regions = loadRegions(options, net)
    nodeCoords = dict()
    edgeShapes = dict()

    for name, edges in regions.items():
        edges = filterBidi(edges)
        if options.verbose:
            print("Processing region '%s' with %s edges" % (name, len(edges)))
        initShapes(edges, nodeCoords, edgeShapes)
        mainLine = findMainline(options, net, edges)
        rotateByMainLine(mainLine, edges, nodeCoords, edgeShapes, False)
        if not options.skipYOpt:
            nodeYValues = computeTrackOrdering(options, mainLine, edges, nodeCoords, edgeShapes)
            if nodeYValues:
                patchShapes(options, edges, nodeCoords, edgeShapes, nodeYValues)
        # computeDistinctHorizontalPoints()
        # squeezeHorizontal(edges)
        if not options.horizontal:
            rotateByMainLine(mainLine, edges, nodeCoords, edgeShapes, True)

    if len(regions) > 1:
        cleanShapes(options, net, nodeCoords, edgeShapes)

    with open(options.output_nodes, 'w') as outf_nod:
        sumolib.writeXMLHeader(outf_nod, "$Id$", "nodes", options=options)
        for nodeID in sorted(nodeCoords.keys()):
            coord = nodeCoords[nodeID]
            outf_nod.write('    <node id="%s" x="%.2f" y="%.2f"/>\n' % (
                nodeID, coord[0], coord[1]))
        outf_nod.write("</nodes>\n")

    with open(options.output_edges, 'w') as outf_edg:
        sumolib.writeXMLHeader(outf_edg, "$Id$", "edges", schemaPath="edgediff_file.xsd", options=options)
        for edgeID in sorted(edgeShapes.keys()):
            shape = edgeShapes[edgeID]
            edge = net.getEdge(edgeID)
            outf_edg.write('    <edge id="%s" shape="%s" length="%.2f"/>\n' % (
                edgeID, shapeStr(shape), edge.getLength()))
            if edge.getBidi():
                outf_edg.write('    <edge id="%s" shape="%s" length="%.2f"/>\n' % (
                    edge.getBidi().getID(), shapeStr(reversed(shape)), edge.getLength()))
        # remove the rest of the network
        if options.filterRegions and not options.keepAll:
            for edge in net.getEdges():
                if edge.getID() not in edgeShapes and edge.getBidi() not in edgeShapes:
                    outf_edg.write('    <delete id="%s"/>\n' % (edge.getID()))
        outf_edg.write("</edges>\n")

    if not options.skipBuilding:
        if options.verbose:
            print("Building new net")
        NETCONVERT = sumolib.checkBinary('netconvert')
        subprocess.call([NETCONVERT,
                         '-s', options.netfile,
                         '-n', options.output_nodes,
                         '-e', options.output_edges,
                         '-o', options.output_net,
                         ], stdout=subprocess.DEVNULL)


if __name__ == "__main__":
    main(get_options())
