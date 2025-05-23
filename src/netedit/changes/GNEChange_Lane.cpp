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
/// @file    GNEChange_Lane.cpp
/// @author  Jakob Erdmann
/// @date    April 2011
///
// A network change in which a single lane is created or deleted
/****************************************************************************/

#include <netedit/GNENet.h>
#include <netedit/GNEViewNet.h>
#include <netedit/GNEViewParent.h>
#include <netedit/GNEApplicationWindow.h>

#include "GNEChange_Lane.h"

// ===========================================================================
// FOX-declarations
// ===========================================================================

FXIMPLEMENT_ABSTRACT(GNEChange_Lane, GNEChange, nullptr, 0)

// ===========================================================================
// member method definitions
// ===========================================================================

GNEChange_Lane::GNEChange_Lane(GNEEdge* edge, const NBEdge::Lane& laneAttrs):
    GNEChange(Supermode::NETWORK, true, false),
    myEdge(edge),
    myLane(nullptr),
    myLaneAttrs(laneAttrs),
    myRecomputeConnections(true) {
    myEdge->incRef("GNEChange_Lane");
}


GNEChange_Lane::GNEChange_Lane(GNEEdge* edge, GNELane* lane, const NBEdge::Lane& laneAttrs, bool forward, bool recomputeConnections):
    GNEChange(Supermode::NETWORK, lane, forward, lane->isAttributeCarrierSelected()),
    myEdge(edge),
    myLane(lane),
    myLaneAttrs(laneAttrs),
    myRecomputeConnections(recomputeConnections) {
    // include both references (To edge and lane)
    myEdge->incRef("GNEChange_Lane");
    myLane->incRef("GNEChange_Lane");
}


GNEChange_Lane::~GNEChange_Lane() {
    // only continue we have undo-redo mode enabled
    if (myEdge->getNet()->getViewNet()->getViewParent()->getGNEAppWindows()->isUndoRedoAllowed()) {
        myEdge->decRef("GNEChange_Lane");
        if (myEdge->unreferenced()) {
            delete myEdge;
        }
        if (myLane) {
            myLane->decRef("GNEChange_Lane");
            if (myLane->unreferenced()) {
                // delete lane
                delete myLane;
            }
        }
    }
}


void
GNEChange_Lane::undo() {
    if (myForward) {
        // remove lane from edge (note: myLane can be nullptr)
        myEdge->removeLane(myLane, false);
        // special case if lane exist
        if (myLane && mySelectedElement) {
            myLane->unselectAttributeCarrier();
        }
    } else {
        // show extra information for tests
        if (myLane && mySelectedElement) {
            myLane->selectAttributeCarrier();
        }
        // add lane and their attributes to edge (lane removal is reverted, no need to recompute connections)
        myEdge->addLane(myLane, myLaneAttrs, false);
    }
    // enable save networkElements
    myEdge->getNet()->getSavingStatus()->requireSaveNetwork();
}


void
GNEChange_Lane::redo() {
    if (myForward) {
        // show extra information for tests
        if (myLane && mySelectedElement) {
            myLane->selectAttributeCarrier();
        }
        // add lane and their attributes to edge
        myEdge->addLane(myLane, myLaneAttrs, myRecomputeConnections);
    } else {
        // special case if lane exist
        if (myLane && mySelectedElement) {
            myLane->unselectAttributeCarrier();
        }
        // remove lane from edge
        myEdge->removeLane(myLane, myRecomputeConnections);
    }
    // enable save networkElements
    myEdge->getNet()->getSavingStatus()->requireSaveNetwork();
}


std::string
GNEChange_Lane::undoName() const {
    if (myForward) {
        return (TL("Undo create lane '") + myLane->getID() + "'");
    } else {
        return (TL("Undo delete lane '") + myLane->getID() + "'");
    }
}


std::string
GNEChange_Lane::redoName() const {
    if (myForward) {
        return (TL("Redo create lane '") + myLane->getID() + "'");
    } else {
        return (TL("Redo delete lane '") + myLane->getID() + "'");
    }
}
