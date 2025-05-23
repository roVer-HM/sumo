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
/// @file    GNEChange_Attribute.cpp
/// @author  Jakob Erdmann
/// @date    Mar 2011
///
// A network change in which something is changed (for undo/redo)
/****************************************************************************/

#include <netedit/GNENet.h>
#include <netedit/GNETagProperties.h>
#include <netedit/GNEViewNet.h>
#include <netedit/GNEViewParent.h>
#include <netedit/GNEApplicationWindow.h>
#include <netedit/GNEUndoList.h>
#include <netedit/elements/data/GNEDataSet.h>

#include "GNEChange_Attribute.h"

// ===========================================================================
// FOX-declarations
// ===========================================================================

FXIMPLEMENT_ABSTRACT(GNEChange_Attribute, GNEChange, nullptr, 0)

// ===========================================================================
// member method definitions
// ===========================================================================

void
GNEChange_Attribute::changeAttribute(GNEAttributeCarrier* AC, SumoXMLAttr key, const std::string& value, GNEUndoList* undoList, const bool force) {
    if (AC->getNet()->getViewNet()->getViewParent()->getGNEAppWindows()->isUndoRedoAllowed()) {
        // create change
        auto change = new GNEChange_Attribute(AC, key, value);
        // set force
        change->myForceChange = force;
        // check if process change
        if (change->trueChange()) {
            undoList->begin(AC, TLF("change '%' attribute in % '%' to '%'", toString(key), AC->getTagStr(), AC->getID(), value));
            undoList->add(change, true);
            undoList->end();
        } else {
            delete change;
        }
    } else {
        AC->setAttribute(key, value);
    }
}


void
GNEChange_Attribute::changeAttribute(GNEAttributeCarrier* AC, SumoXMLAttr key, const std::string& value, const std::string& originalValue, GNEUndoList* undoList, const bool force) {
    if (AC->getNet()->getViewNet()->getViewParent()->getGNEAppWindows()->isUndoRedoAllowed()) {
        // create change
        auto change = new GNEChange_Attribute(AC, key, value, originalValue);
        // set force
        change->myForceChange = force;
        // check if process change
        if (change->trueChange()) {
            undoList->begin(AC, TLF("change '%' attribute in % '%' to '%'", toString(key), AC->getTagStr(), AC->getID(), value));
            undoList->add(change, true);
            undoList->end();
        } else {
            delete change;
        }
    } else {
        AC->setAttribute(key, value);
    }
}


GNEChange_Attribute::~GNEChange_Attribute() {
    // only continue we have undo-redo mode enabled
    if (myAC->getNet()->getViewNet()->getViewParent()->getGNEAppWindows()->isUndoRedoAllowed()) {
        // decrease reference
        myAC->decRef("GNEChange_Attribute " + toString(myKey));
        // remove if is unreferenced
        if (myAC->unreferenced()) {
            // delete AC
            delete myAC;
        }
    }
}


void
GNEChange_Attribute::undo() {
    // set original value
    myAC->setAttribute(myKey, myOrigValue);
    // certain attributes needs extra operations
    if (myKey != GNE_ATTR_SELECTED) {
        // check if updated attribute requires a update geometry
        if (myAC->getTagProperty()->hasAttribute(myKey) && myAC->getTagProperty()->getAttributeProperties(myKey)->requireUpdateGeometry()) {
            myAC->updateGeometry();
        }
        // if is a dataelement, update attribute colors
        if (myAC->getTagProperty()->isGenericData()) {
            myAC->getNet()->getAttributeCarriers()->retrieveDataSet(myAC->getAttribute(GNE_ATTR_DATASET))->updateAttributeColors();
        } else if (myAC->getTagProperty()->getTag() == SUMO_TAG_DATASET) {
            myAC->getNet()->getAttributeCarriers()->retrieveDataSet(myAC->getAttribute(SUMO_ATTR_ID))->updateAttributeColors();
        }
        // check if networkElements, additional or shapes has to be saved (only if key isn't GNE_ATTR_SELECTED)
        if (myAC->getTagProperty()->isNetworkElement()) {
            myAC->getNet()->getSavingStatus()->requireSaveNetwork();
        } else if (myAC->getTagProperty()->isAdditionalElement()) {
            myAC->getNet()->getSavingStatus()->requireSaveAdditionals();
        } else if (myAC->getTagProperty()->isDemandElement()) {
            myAC->getNet()->getSavingStatus()->requireSaveDemandElements();
        } else if (myAC->getTagProperty()->isDataElement()) {
            myAC->getNet()->getSavingStatus()->requireSaveDataElements();
        } else if (myAC->getTagProperty()->isMeanData()) {
            myAC->getNet()->getSavingStatus()->requireSaveMeanDatas();
        }
    }
}


void
GNEChange_Attribute::redo() {
    // set new value
    myAC->setAttribute(myKey, myNewValue);
    // certain attributes needs extra operations
    if (myKey != GNE_ATTR_SELECTED) {
        // check if updated attribute requires a update geometry
        if (myAC->getTagProperty()->hasAttribute(myKey) && myAC->getTagProperty()->getAttributeProperties(myKey)->requireUpdateGeometry()) {
            myAC->updateGeometry();
        }
        // if is a dataelement, update attribute colors
        if (myAC->getTagProperty()->isGenericData()) {
            myAC->getNet()->getAttributeCarriers()->retrieveDataSet(myAC->getAttribute(GNE_ATTR_DATASET))->updateAttributeColors();
        } else if (myAC->getTagProperty()->getTag() == SUMO_TAG_DATASET) {
            myAC->getNet()->getAttributeCarriers()->retrieveDataSet(myAC->getAttribute(SUMO_ATTR_ID))->updateAttributeColors();
        }
        // check if networkElements, additional or shapes has to be saved (only if key isn't GNE_ATTR_SELECTED)
        if (myAC->getTagProperty()->isNetworkElement()) {
            myAC->getNet()->getSavingStatus()->requireSaveNetwork();
        } else if (myAC->getTagProperty()->isAdditionalElement()) {
            myAC->getNet()->getSavingStatus()->requireSaveAdditionals();
        } else if (myAC->getTagProperty()->isDemandElement()) {
            myAC->getNet()->getSavingStatus()->requireSaveDemandElements();
        } else if (myAC->getTagProperty()->isDataElement()) {
            myAC->getNet()->getSavingStatus()->requireSaveDataElements();
        } else if (myAC->getTagProperty()->isMeanData()) {
            myAC->getNet()->getSavingStatus()->requireSaveMeanDatas();
        }
    }
}


std::string
GNEChange_Attribute::undoName() const {
    return (TL("Undo change ") + myAC->getTagStr() + " attribute");
}


std::string
GNEChange_Attribute::redoName() const {
    return (TL("Redo change ") + myAC->getTagStr() + " attribute");
}


GNEChange_Attribute::GNEChange_Attribute(GNEAttributeCarrier* ac, SumoXMLAttr key, const std::string& value) :
    GNEChange(ac->getTagProperty()->getSupermode(), true, false),
    myAC(ac),
    myKey(key),
    myForceChange(false),
    myOrigValue(ac->getAttribute(key)),
    myNewValue(value) {
    myAC->incRef("GNEChange_Attribute " + toString(myKey));
}


GNEChange_Attribute::GNEChange_Attribute(GNEAttributeCarrier* ac, SumoXMLAttr key, const std::string& value, const std::string& origValue) :
    GNEChange(ac->getTagProperty()->getSupermode(), true, false),
    myAC(ac),
    myKey(key),
    myForceChange(false),
    myOrigValue(origValue),
    myNewValue(value) {
    myAC->incRef("GNEChange_Attribute " + toString(myKey));
}


bool
GNEChange_Attribute::trueChange() {
    // check if we're editing the value of an attribute or changing a disjoint attribute
    if (myForceChange) {
        return true;
    } else {
        return (myOrigValue != myNewValue);
    }
}

/****************************************************************************/
