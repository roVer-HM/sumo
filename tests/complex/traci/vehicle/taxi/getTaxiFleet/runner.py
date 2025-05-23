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
             "-a", "input_routes.rou.xml",
             "--no-step-log",
             "--vehroute-output", "vehroutes.xml",
             "--tripinfo-output", "tripinfos.xml",
             "--device.taxi.dispatch-algorithm", "traci",
             ] + sys.argv[1:])


while traci.simulation.getMinExpectedNumber() > 0:

    print("%s all=%s empty=%s pickup=%s dropoff=%s pickup+dropoff=%s" % (
        traci.simulation.getTime(),
        traci.vehicle.getTaxiFleet(-1),
        traci.vehicle.getTaxiFleet(0),
        traci.vehicle.getTaxiFleet(1),
        traci.vehicle.getTaxiFleet(2),
        traci.vehicle.getTaxiFleet(3),))

    if traci.simulation.getDepartedNumber() > 0:
        reservations = traci.person.getTaxiReservations(0)
        fleet = traci.vehicle.getTaxiFleet(0)
        if reservations and fleet:
            traci.vehicle.dispatchTaxi(fleet[0], [reservations[0].id])

    traci.simulationStep()
traci.close()
