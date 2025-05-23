#!/usr/bin/env python
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.dev/sumo
# Copyright (C) 2015-2025 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    templates.py
# @author  Pablo Alvarez Lopez
# @date    Dec 2022

"""
This script rebuilds "src/netedit/templates.h" the files
representing the templates.
It does this by parsing the data from the sumo data dir.
"""

from __future__ import print_function
from __future__ import absolute_import

import sys
import os
import re
import subprocess
import io
from os.path import dirname, join
from glob import glob

# list of folders and tools
TOOLS = [

    # detector
    "detector/edgeDataFromFlow.py",
    "detector/flowFromEdgeData.py",
    "detector/mapDetectors.py",

    # district
    "district/filterDistricts.py",
    "district/gridDistricts.py",
    "district/stationDistricts.py",

    # drt
    # "drt/darpSolvers.py",   NO CONFIG
    "drt/drtOrtools.py",
    # "drt/ortools_pdp.py",   NO CONFIG

    # emissions
    # "emissions/findMinDiffModel.py",    NO CONFIG
    # "emissions/nefz.py",                NO CONFIG

    # import
    "import/citybrain/citybrain_flow.py",
    "import/citybrain/citybrain_infostep.py",
    "import/citybrain/citybrain_road.py",
    "import/dxf/dxf2jupedsim.py",
    "import/gtfs/gtfs2fcd.py",
    # "import/gtfs/gtfs2osm.py",                                NO CONFIG
    "import/gtfs/gtfs2pt.py",
    "import/vissim/vissim_parseRoutes.py",
    "import/visum/visum_convertEdgeTypes.py",

    # net
    "net/abstractRail.py",
    "net/createRoundaboutConnections.py",
    "net/net2geojson.py",
    "net/net2jpsgeometry.py",
    "net/net2kml.py",
    "net/net2poly.py",
    "net/netcheck.py",
    "net/netdiff.py",
    # "net/netduplicate.py",                  NO CONFIG
    "net/netmatch.py",
    "net/network_statistics.py",
    # "net/odConnectionsCheck.py",            NO CONFIG
    # "net/prepareVISUM.py",                  NO CONFIG
    "net/reduceLanes.py",
    # "net/reprojectpolys.py",                NO CONFIG
    "net/split_at_stops.py",
    # "net/visum_mapDistricts.py",            NO CONFIG
    # "net/xmlconnections_mapEdges.py",       NO CONFIG
    # "net/xmledges_applyOffset.py",          NO CONFIG
    # "net/xmlnodes_applyOffset.py",          NO CONFIG

    # route
    "route/addStopDelay.py",
    "route/addStops2Routes.py",
    "route/addTAZ.py",
    "route/checkStopOrder.py",
    "route/implausibleRoutes.py",
    "route/route2OD.py",
    "route/route2poly.py",
    "route/route2sel.py",
    "route/routecheck.py",
    "route/routecompare.py",
    "route/routeStats.py",
    "route/scaleTimeLine.py",
    "route/sort_routes.py",
    "route/splitRandom.py",
    "route/splitRouteFiles.py",
    "route/tracegenerator.py",
    "route/tracemapper.py",
    # "route/vehicle2flow.py",                NO CONFIG

    # output
    "output/aggregateBatteryOutput.py",
    "output/analyze_pedestrian_jam.py",
    "output/analyze_teleports.py",
    "output/attributeStats.py",
    "output/attributeDiff.py",
    "output/attributeCompare.py",
    "output/computeCoordination.py",
    "output/computePassengerCounts.py",
    "output/computeStoppingPlaceUsage.py",
    "output/countLaneChanges.py",
    "output/edgeDataDiff.py",
    # "output/edgeDataStatistics.py",                 NO CONFIG
    "output/fcdDiff.py",
    # "output/generateITetrisIntersectionMetrics.py", NO CONFIG
    # "output/generateITetrisNetworkMetrics.py",      NO CONFIG
    # "output/generateTLSE1Detectors.py",             NO CONFIG
    "output/generateTLSE2Detectors.py",
    # "output/generateTLSE3Detectors.py",             NO CONFIG
    # "output/netdumpdiff.py",                        NO CONFIG
    # "output/netdumpmean.py",                        NO CONFIG
    "output/parkingSearchTraffic.py",
    "output/scheduleStats.py",
    # "output/statisticsElements.py",                 NO CONFIG
    "output/stopOrder.py",
    # "output/tables.py",                             NO CONFIG
    # "output/timingStats.py",                        NO CONFIG
    "output/tripinfoByType.py",
    # "output/tripStatistics.py",                     NO CONFIG
    "output/vehLanes.py",
    "output/vehrouteCountValidation.py",
    "output/vehrouteDiff.py",
    "output/walkFactor.py",

    # shapes
    "shapes/circlePolygon.py",
    # "shapes/CSV2polyconvertXML.py", NO CONFIG
    # "shapes/poi_alongRoads.py",     NO CONFIG
    # "shapes/poi_at_stops.py",       NO CONFIG
    # "shapes/poi_atTLS.py",          NO CONFIG

    # tls
    # "tls/buildTransitions.py",      NO CONFIG
    "tls/createTlsCsv.py",
    # "tls/tls_check.py",             NO CONFIG
    "tls/tls_csvSignalGroups.py",

    # turn-defs
    # "turn-defs/collectinghandler.py",       NO CONFIG
    # "turn-defs/connections.py",             NO CONFIG
    # "turn-defs/generateTurnDefs.py",        NO CONFIG
    "turn-defs/generateTurnRatios.py",
    "turn-defs/turnCount2EdgeCount.py",
    # "turn-defs/turndefinitions.py",         NO CONFIG
    "turn-defs/turnFile2EdgeRelations.py",

    # visualization
    # "visualization/mpl_dump_onNet.py",              NO CONFIG
    # "visualization/mpl_dump_timeline.py",           NO CONFIG
    # "visualization/mpl_dump_twoAgainst.py",         NO CONFIG
    # "visualization/mpl_tripinfos_twoAgainst.py",    NO CONFIG
    "visualization/plot_csv_bars.py",
    "visualization/plot_csv_pie.py",
    "visualization/plot_csv_timeline.py",
    "visualization/plot_net_dump.py",
    "visualization/plot_net_selection.py",
    "visualization/plot_net_speeds.py",
    "visualization/plot_net_trafficLights.py",
    "visualization/plot_summary.py",
    "visualization/plotXMLAttributes.py",

    # xml
    # "xml/addSchema.py",         IGNORE
    # "xml/binary2plain.py",      NO CONFIG
    "xml/csv2xml.py",
    "xml/xml2csv.py",
    "xml/changeAttribute.py",
    "xml/filterElements.py",

    # other
    "countEdgeUsage.py",
    "createVehTypeDistribution.py",
    "edgesInDistricts.py",
    # "extractTest.py",                 NOT_SUITABLE
    "fcdReplay.py",
    "findAllRoutes.py",
    "generateBidiDistricts.py",
    "generateContinuousRerouters.py",
    "generateParkingAreaRerouters.py",
    "generateParkingAreas.py",
    "generateRailSignalConstraints.py",
    "generateRerouters.py",
    # "osmBuild.py",                    NOT_SUITABLE
    "osmGet.py",
    "plot_trajectories.py",
    "ptlines2flows.py",
    "randomTrips.py",
    "routeSampler.py",
    "runSeeds.py",
    "stateReplay.py",
    "tileGet.py",
    "tlsCoordinator.py",
    "tlsCycleAdaptation.py",


    "jtcrouter.py",
    "distributeChargingStations.py",
    "traceExporter.py",
    "createScreenshotSequence.py",
    "averageTripStatistics.py",
    "generateLandmarks.py",
    # "osmWebWizard.py",         NOT_WORKING
    "assign/duaIterateMix.py",
    "assign/duaIterate_analysis.py",
    "assign/duaIterate_reroutingAnalysis.py",
    "assign/one-shot.py",
    # "assign/cadytsIterate.py", NOT_WORKING (on windows to due classpath default)
    "assign/costFunctionChecker.py",
    "assign/duaIterate.py",
    "detector/aggregateFlows.py",
    "detector/filterFlows.py",
    "detector/flowFromRoutes.py",
    "detector/flowrouter.py",
    "detector/routeUsage.py",
    "detector/validate.py",
    "detector/plotFlows.py",
    "drt/drtOnline.py",
    "import/matsim/matsim_importPlans.py",
    "import/osm/osmTaxiStop.py",
    "import/vissim/convert_detectors2SUMO.py",
    "import/vissim/convert_vissimXML_flows_statRoutes.py",
    "import/vissim/tls_vissimXML2SUMOnet_update.py",
    "import/vissim/vissim2poly.py",
    "import/vissim/vissim_parseBusStops.py",
    "import/visum/visum_convertRoutes.py",
    "import/visum/visum_convertTurnPercentages.py",
    "import/visum/visum_parseZaehlstelle.py",
    "import/opendrive/signal_POIs_from_xodr.py",
    "output/generateMeanDataDefinitions.py",
    "output/edgeDepartDelay.py",
    "output/vehroute2amitranOD.py",
    "output/tripinfoByTAZ.py",
    "output/tripinfoDiff.py",
    "route/route_departOffset.py",
    "route/cutTrips.py",
    "route/driveways2poly.py",
    "route/addParkingAreaStops2Trips.py",
    "route/analyzePersonPlans.py",
    "route/addParkingAreaStops2Routes.py",
    "route/cutRoutes.py",
    "route/route2alts.py",
    "route/route_1htoDay.py",
    "route/geoTrip2POI.py",
    "visualization/macrOutput.py",
    "visualization/plotWKT.py",
    "xml/protobuf2xml.py",
    "xml/xml2protobuf.py",
    "district/aggregateAndSplitMatrices.py",
    "district/countConnectionsInDistricts.py",
    "district/districtMapper.py",
    "emissions/hbefa2sumo.py",
    "net/buildFullGraph.py",
    "net/generateStationEdges.py",
    "net/patchVClasses.py",
    "shapes/poly2edgedata.py",
    "tls/buildTransitions.py",
    "tls/tls_csv2SUMO.py",
    "trigger/csv2vss.py",
    "trigger/meandata2vss.py",
]


PATH_MAPPING = {
    "import/dxf": "import",
    "import/osm": "import",
    "import/matsim": "import",
    "import/opendrive": "import",
}


SOURCE_DEPS = [
    "microsim/MSFrame.cpp", "microsim/devices/*.cpp",
    "utils/common/RandHelper.cpp", "utils/common/SystemFrame.cpp",
    "utils/geom/GeoConvHelper.cpp",
]


def buildTemplateToolHeader(templateHeaderFile):
    print(
        u"#pragma once\n"
        "#include <string>\n"
        "#include <vector>\n"
        "\n"
        "// @brief template tool\n"
        "struct TemplateTool {\n"
        "\n"
        "    // @brief constructor\n"
        "    TemplateTool(const std::string name_, const std::string pythonPath_,\n"
        "        const std::string subfolder_, const std::string templateStr_) :\n"
        "        name(name_),\n"
        "        pythonPath(pythonPath_),\n"
        "        subfolder(subfolder_),\n"
        "        templateStr(templateStr_) {\n"
        "    }\n"
        "\n"
        "    // @brief tool name\n"
        "    const std::string name;\n"
        "\n"
        "    // @brief python path\n"
        "    const std::string pythonPath;\n"
        "\n"
        "    // @brief subfolder path\n"
        "    const std::string subfolder;\n"
        "\n"
        "    // @brief tool template\n"
        "    const std::string templateStr;\n"
        "};\n",
        file=templateHeaderFile)


def formatBinTemplate(templateStr):
    """
    @brief parse binary of a bin template (sumo, netconvert, etc.)
    """
    # remove endlines in Windows
    templateStr = templateStr.replace("\\r", '')
    # replace " with \"
    templateStr = templateStr.replace('"', '\\"')
    # split lines
    return templateStr.replace("\n", '"\n    "')


def formatToolTemplate(templateStr):
    """
    @brief format python tool template
    """
    if templateStr == '' or templateStr[0] != "<":
        return '""'
    # replace all current directory values (src/netedit)
    templateStr = re.sub("(?<=value)(.*)(?=netedit)", "", templateStr)
    templateStr = templateStr.replace('netedit', '="')
    # replace " with \"
    templateStr = templateStr.replace('"', '\\"')
    # add quotes and end lines
    return templateStr.replace("<", '"<').replace("\n", '"\n')


def generateTemplate(app, appBin):
    """
    @brief generate template for the given app
    """
    print("Obtaining " + app + " template")
    env = dict(os.environ)
    # the *SAN_OPTIONS are only relevant for the clang build but should not hurt others
    env["LSAN_OPTIONS"] = "suppressions=%s/../../build_config/clang_memleak_suppressions.txt" % dirname(__file__)
    env["UBSAN_OPTIONS"] = "suppressions=%s/../../build_config/clang_ubsan_suppressions.txt" % dirname(__file__)
    # obtain template piping stdout using check_output
    try:
        template = formatBinTemplate(subprocess.check_output(
            [appBin, "--save-template", "stdout"], env=env, universal_newlines=True))
    except subprocess.CalledProcessError as e:
        sys.stderr.write("Error when generating template for " + app + ": '%s'" % e)
        template = ""
    # join variable and formatted template
    return u'const std::string %sTemplate = "%s";\n' % (app, template)


def _collectOutput(procs, toolPaths, failed, verbose, testFailure):
    result = u""
    for p, toolPath in zip(procs, toolPaths):
        toolName = os.path.basename(toolPath)[:-3]
        d = os.path.dirname(toolPath)
        result += 'TemplateTool("%s", "tools/%s", "%s",\n' % (toolName, toolPath, PATH_MAPPING.get(d, d))
        stdout, stderr = p.communicate()
        if p.returncode:
            failed.append(toolPath if testFailure else toolName)
            if verbose:
                print("Cannot generate tool template for %s: '%s'." % (toolName, stderr), file=sys.stderr)
            result += '""'
        else:
            result += formatToolTemplate(stdout)
        result += '),\n'
    return result


def generateToolTemplates(toolDir, toolPaths, verbose, testFailure=False):
    """
    @brief generate tool template
    """
    print("Obtaining tool templates.")
    procs = []
    result = u""
    failed = []
    paths = []
    env = dict(os.environ)
    if "SUMO_HOME" not in env:
        env["SUMO_HOME"] = join(dirname(__file__), '..', '..')
    for toolPath in toolPaths:
        toolName = os.path.basename(toolPath)[:-3]
        if verbose:
            print("Obtaining '" + toolName + "' tool template.")
        procs.append(subprocess.Popen([sys.executable, join(toolDir, toolPath), "--save-template", "stdout"], env=env,
                                      stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True))
        paths.append(toolPath)
        if len(procs) == 32:
            result += _collectOutput(procs, paths, failed, verbose, testFailure)
            procs = []
            paths = []
    result += _collectOutput(procs, paths, failed, verbose, testFailure)
    if testFailure:
        return failed
    if failed:
        print("Could not generate tool templates for %s." % (", ".join(failed)), file=sys.stderr)
    return result


def checkMod(toolDir, reference):
    mtime = os.path.getmtime(reference)
    for root, _, files in os.walk(toolDir):
        for f in files:
            if f[-3:] == ".py" and os.path.getmtime(join(root, f)) > mtime:
                return True
    srcDir = join(dirname(toolDir), 'src')
    for p in SOURCE_DEPS:
        for f in glob(join(srcDir, p)):
            if os.path.getmtime(f) > mtime:
                return True
    return False


def main():
    if len(sys.argv) != 3:
        sys.exit("Arguments: <pathToSumo> <pathToNetgenerate>")
    # get tool dir path (SUMO/tools)
    toolDir = join(dirname(__file__), '..')
    if not os.path.exists("templates.h") or checkMod(toolDir, "templates.h"):
        # write templates.h
        with io.open("templates.h", 'w', encoding='utf8') as templateHeaderFile:
            buildTemplateToolHeader(templateHeaderFile)
            is_debug = sys.argv[1].endswith("D") or sys.argv[1].endswith("D.exe")
            print(u"const std::vector<TemplateTool> templateTools {\n", file=templateHeaderFile)
            print(generateToolTemplates(toolDir, TOOLS, is_debug), file=templateHeaderFile)
            print(u"};\n", file=templateHeaderFile)
            # generate sumo Template
            print(generateTemplate("sumo", sys.argv[1]), file=templateHeaderFile)
            # generate netgenerate Template
            print(generateTemplate("netgenerate", sys.argv[2]), file=templateHeaderFile)


if __name__ == "__main__":
    main()
