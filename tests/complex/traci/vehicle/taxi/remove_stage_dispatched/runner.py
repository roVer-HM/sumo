#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.dev/sumo
# Copyright (C) 2008-2025 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    runner.py
# @author  Jakob Erdmann
# @date    2017-01-23


from __future__ import print_function
from __future__ import absolute_import
import os
import sys

if "SUMO_HOME" in os.environ:
    sys.path.append(os.path.join(os.environ["SUMO_HOME"], "tools"))

import traci  # noqa
import sumolib  # noqa
import traci.constants as tc  # noqa


sumoBinary = sumolib.checkBinary('sumo')
traci.start([sumoBinary,
             "-n", "input_net4.net.xml",
             "-r", "input_routes.rou.xml",
             "--no-step-log",
             "--vehroute-output", "vehroutes.xml",
             "--vehroute-output.write-unfinished",
             "--tripinfo-output", "tripinfos.xml",
             "--stop-output", "stops.xml",
             "--device.taxi.dispatch-algorithm", "traci",
             ] + sys.argv[1:])


traci.simulationStep()


traci.simulationStep()
fleet = traci.vehicle.getTaxiFleet(0)
res = traci.person.getTaxiReservations(0)
print("taxiFleet", fleet)
print("reservations", res)
traci.vehicle.dispatchTaxi(fleet[0], [res[0].id])
traci.simulationStep()
traci.person.removeStage(res[0].persons[0], 0)

while traci.simulation.getMinExpectedNumber() > 0 and traci.simulation.getTime() < 100:
    print("%s reservations %s" % (
        traci.simulation.getTime(),
        traci.person.getTaxiReservations(0)))
    traci.simulationStep()
traci.close()
