/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.dev/sumo
// Copyright (C) 2013-2024 German Aerospace Center (DLR) and others.
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
/// @file    MSTransportableDevice_FCDReplay.cpp
/// @author  Michael Behrisch
/// @date    04.03.2024
///
// A device which replays recorded floating car data
/****************************************************************************/
#include <config.h>

#include <utils/options/OptionsCont.h>
#include <libsumo/Person.h>
#include <microsim/MSEventControl.h>
#include <microsim/MSNet.h>
#include <microsim/transportables/MSPerson.h>
#include <microsim/transportables/MSStageWalking.h>
#include <microsim/transportables/MSTransportable.h>
#include <microsim/transportables/MSTransportableControl.h>
#include "MSTransportableDevice_FCDReplay.h"


// ===========================================================================
// static member initializations
// ===========================================================================
bool MSTransportableDevice_FCDReplay::myAmActive = false;


// ===========================================================================
// method definitions
// ===========================================================================
// ---------------------------------------------------------------------------
// static initialisation methods
// ---------------------------------------------------------------------------
void
MSTransportableDevice_FCDReplay::insertOptions(OptionsCont& oc) {
    insertDefaultAssignmentOptions("fcd-replay", "FCD Replay Device", oc, true);
}


void
MSTransportableDevice_FCDReplay::buildDevices(MSTransportable& t, std::vector<MSTransportableDevice*>& into) {
    OptionsCont& oc = OptionsCont::getOptions();
    if (equippedByDefaultAssignmentOptions(oc, "fcd-replay", t, oc.isSet("device.fcd-replay.file"), true)) {
        MSTransportableDevice_FCDReplay* device = new MSTransportableDevice_FCDReplay(t, "fcdReplay_" + t.getID());
        into.push_back(device);
        if (!myAmActive) {
            MSNet::getInstance()->getBeginOfTimestepEvents()->addEvent(new MovePedestrians(), SIMSTEP + DELTA_T);
            myAmActive = true;
        }
    }
}


// ---------------------------------------------------------------------------
// MSTransportableDevice_FCDReplay-methods
// ---------------------------------------------------------------------------
MSTransportableDevice_FCDReplay::MSTransportableDevice_FCDReplay(MSTransportable& holder, const std::string& id) :
    MSTransportableDevice(holder, id) {
}


MSTransportableDevice_FCDReplay::~MSTransportableDevice_FCDReplay() {
}


bool
MSTransportableDevice_FCDReplay::move(SUMOTime currentTime) {
    if (myTrajectory == nullptr || myTrajectory->empty()) {
        // removing person
        return true;
    }
    MSPerson* person = dynamic_cast<MSPerson*>(&myHolder);
    const auto& te = myTrajectory->front();
    if (person == nullptr || !person->hasDeparted() || te.time > currentTime) {
        return false;
    }
    libsumo::Person::moveToXY(person->getID(), te.edgeOrLane, te.pos.x(), te.pos.y(), te.angle, 7);
    // person->setPreviousSpeed(std::get<3>(p), std::numeric_limits<double>::min());
    myTrajectory->erase(myTrajectory->begin());
    return false;
}


SUMOTime
MSTransportableDevice_FCDReplay::MovePedestrians::execute(SUMOTime currentTime) {
    MSTransportableControl& c = MSNet::getInstance()->getPersonControl();
    std::vector<MSTransportable*> toRemove;
    for (MSTransportableControl::constVehIt i = c.loadedBegin(); i != c.loadedEnd(); ++i) {
        MSTransportableDevice_FCDReplay* device = static_cast<MSTransportableDevice_FCDReplay*>(i->second->getDevice(typeid(MSTransportableDevice_FCDReplay)));
        if (device != nullptr && device->move(currentTime)) {
            toRemove.push_back(&device->getHolder());
        }
    }
    for (MSTransportable* t : toRemove) {
        t->removeStage(0, false);
    }
    return DELTA_T;
}


/****************************************************************************/
