/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.dev/sumo
// Copyright (C) 2001-2025 German Aerospace Center (DLR) and others.
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// https://www.eclipse.org/legal/epl-2.0/
// This Source Code may also be made available under the following Secondary
// Licenses when the conditions for such availability set forth in the Eclipse
// Public License 2.0 are satisfied: GNU General Public License, version 2
// or later which is available at
// https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
// SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later
/****************************************************************************/
/// @file    ROMARouteHandler.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @date    Mon, 9 Jul 2001
///
// Parser and container for routes during their loading
/****************************************************************************/
#include <config.h>

#include <utils/common/MsgHandler.h>
#include <utils/options/OptionsCont.h>
#include <utils/vehicle/SUMOVehicleParameter.h>
#include <utils/vehicle/SUMOVehicleParserHelper.h>
#include <od/ODMatrix.h>
#include "ROMARouteHandler.h"


// ===========================================================================
// method definitions
// ===========================================================================
ROMARouteHandler::ROMARouteHandler(ODMatrix& matrix) :
    SUMOSAXHandler(""), myMatrix(matrix),
    myIgnoreTaz(OptionsCont::getOptions().getBool("ignore-taz")),
    myScale(OptionsCont::getOptions().getFloat("scale")),
    myNumLoaded(0) {
    if (OptionsCont::getOptions().isSet("taz-param")) {
        myTazParamKeys = OptionsCont::getOptions().getStringVector("taz-param");
    }
}


ROMARouteHandler::~ROMARouteHandler() {
}


void
ROMARouteHandler::myStartElement(int element, const SUMOSAXAttributes& attrs) {
    if (element == SUMO_TAG_TRIP || element == SUMO_TAG_VEHICLE || element == SUMO_TAG_FLOW) {
        myVehicleParameter = (element == SUMO_TAG_FLOW
                              ? SUMOVehicleParserHelper::parseFlowAttributes(SUMO_TAG_FLOW, attrs, true, true, 0, TIME2STEPS(3600))
                              : SUMOVehicleParserHelper::parseVehicleAttributes(element, attrs, true));
        if (!myVehicleParameter->wasSet(VEHPARS_FROM_TAZ_SET) || myIgnoreTaz) {
            if (attrs.hasAttribute(SUMO_ATTR_FROM)) {
                myVehicleParameter->fromTaz = attrs.getString(SUMO_ATTR_FROM);
            } else if (attrs.hasAttribute(SUMO_ATTR_FROM_JUNCTION)) {
                myVehicleParameter->fromTaz = attrs.getString(SUMO_ATTR_FROM_JUNCTION) + "-source";
            }

        }
        if (!myVehicleParameter->wasSet(VEHPARS_TO_TAZ_SET) || myIgnoreTaz) {
            if (attrs.hasAttribute(SUMO_ATTR_TO)) {
                myVehicleParameter->toTaz = attrs.getString(SUMO_ATTR_TO);
            } else if (attrs.hasAttribute(SUMO_ATTR_TO_JUNCTION)) {
                myVehicleParameter->toTaz = attrs.getString(SUMO_ATTR_TO_JUNCTION) + "-sink";
            }
        }
    } else if (element == SUMO_TAG_PARAM && !myTazParamKeys.empty()) {
        if (attrs.getString(SUMO_ATTR_KEY) == myTazParamKeys[0]) {
            myVehicleParameter->fromTaz = attrs.getString(SUMO_ATTR_VALUE);
            myVehicleParameter->parametersSet |= VEHPARS_FROM_TAZ_SET;
        }
        if (myTazParamKeys.size() > 1 && attrs.getString(SUMO_ATTR_KEY) == myTazParamKeys[1]) {
            myVehicleParameter->toTaz = attrs.getString(SUMO_ATTR_VALUE);
            myVehicleParameter->parametersSet |= VEHPARS_TO_TAZ_SET;
        }
    }
}


void
ROMARouteHandler::myEndElement(int element) {
    if (element == SUMO_TAG_TRIP || element == SUMO_TAG_VEHICLE || element == SUMO_TAG_FLOW) {
        if (myVehicleParameter->fromTaz == "" || myVehicleParameter->toTaz == "") {
            WRITE_WARNINGF(TL("No origin or no destination given, ignoring '%'!"), myVehicleParameter->id);
        } else {
            int quota = 1;
            SUMOTime departOffset = 0;
            if (element == SUMO_TAG_FLOW) {
                int flowSize = 1;
                double flowDur = STEPS2TIME(myVehicleParameter->repetitionEnd - myVehicleParameter->depart);
                if (myVehicleParameter->repetitionNumber != std::numeric_limits<int>::max()) {
                    flowSize = myVehicleParameter->repetitionNumber;
                } else if (myVehicleParameter->poissonRate > 0) {
                    flowSize = (int)(flowDur * myVehicleParameter->poissonRate);
                } else if (myVehicleParameter->repetitionProbability > 0) {
                    flowSize = (int)(flowDur * myVehicleParameter->repetitionProbability);
                }
                quota = (int)(flowSize * myScale + 0.5);
                myNumLoaded += flowSize;
                departOffset = TIME2STEPS(flowDur) / quota;
            } else {
                quota = getScalingQuota(myScale, myNumLoaded);
                myNumLoaded += 1;
            }
            SUMOTime depart = myVehicleParameter->depart;
            for (int i = 0; i < quota; i++) {
                SUMOVehicleParameter veh = *myVehicleParameter;
                veh.id = i == 0 ? myVehicleParameter->id : myVehicleParameter->id + "." + toString(i);
                veh.depart = depart;
                myMatrix.add(veh,
                             !myVehicleParameter->wasSet(VEHPARS_FROM_TAZ_SET) || myIgnoreTaz,
                             !myVehicleParameter->wasSet(VEHPARS_TO_TAZ_SET) || myIgnoreTaz);
                depart += departOffset;
            }
        }
        delete myVehicleParameter;
    }
}


/****************************************************************************/
