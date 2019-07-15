/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2001-2019 German Aerospace Center (DLR) and others.
// This program and the accompanying materials
// are made available under the terms of the Eclipse Public License v2.0
// which accompanies this distribution, and is available at
// http://www.eclipse.org/legal/epl-v20.html
// SPDX-License-Identifier: EPL-2.0
/****************************************************************************/
/// @file    GNEParkingSpace.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Feb 2018
/// @version $Id$
///
// A lane area vehicles can halt at (GNE version)
/****************************************************************************/

// ===========================================================================
// included modules
// ===========================================================================

#include <netedit/GNENet.h>
#include <netedit/GNEUndoList.h>
#include <netedit/GNEViewNet.h>
#include <netedit/changes/GNEChange_Attribute.h>
#include <netedit/netelements/GNEEdge.h>
#include <utils/gui/div/GLHelper.h>
#include <utils/gui/globjects/GLIncludes.h>

#include "GNEParkingSpace.h"


// ===========================================================================
// method definitions
// ===========================================================================

GNEParkingSpace::GNEParkingSpace(GNEViewNet* viewNet, GNEAdditional* parkingAreaParent, const Position& pos, double width, double length, double angle, bool blockMovement) :
    GNEAdditional(parkingAreaParent, viewNet, GLO_PARKING_SPACE, SUMO_TAG_PARKING_SPACE, "", blockMovement,
{}, {}, {}, {parkingAreaParent}, {}, {}, {}, {}, {}, {}),
myPosition(pos),
myWidth(width),
myLength(length),
myAngle(angle) {
}


GNEParkingSpace::~GNEParkingSpace() {}


void
GNEParkingSpace::moveGeometry(const Position& offset) {
    // restore old position, apply offset and update Geometry
    myPosition = myMove.originalViewPosition;
    myPosition.add(offset);
    // filtern position using snap to active grid
    myPosition = myViewNet->snapToActiveGrid(myPosition);
    updateGeometry();
}


void
GNEParkingSpace::commitGeometryMoving(GNEUndoList* undoList) {
    // commit new position allowing undo/redo
    undoList->p_begin("position of " + getTagStr());
    undoList->p_add(new GNEChange_Attribute(this, myViewNet->getNet(), SUMO_ATTR_POSITION, toString(myPosition), true, toString(myMove.originalViewPosition)));
    undoList->p_end();
}


void
GNEParkingSpace::updateGeometry() {
    // Nothing to update
}


Position
GNEParkingSpace::getPositionInView() const {
    return myPosition;
}


Boundary
GNEParkingSpace::getCenteringBoundary() const {
    // Return Boundary depending if myMovingGeometryBoundary is initialised (important for move geometry)
    if (myMove.movingGeometryBoundary.isInitialised()) {
        return myMove.movingGeometryBoundary;
    } else {
        // calculate shape using a Position vector as reference
        PositionVector boundaryShape({
            {-(myWidth / 2), 0},
            { (myWidth / 2), 0},
            { (myWidth / 2), myLength},
            {-(myWidth / 2), myLength},
        });
        // rotate position vector (note: convert from degree to rads
        boundaryShape.rotate2D(myAngle*PI/180.0);
        // move to space position
        boundaryShape.add(myPosition);
        // return boundary associated to boundaryShape
        return boundaryShape.getBoxBoundary().grow(5);
    }
}


std::string
GNEParkingSpace::getParentName() const {
    return getAdditionalParents().at(0)->getMicrosimID();
}


void
GNEParkingSpace::drawGL(const GUIVisualizationSettings& s) const {
    // Set initial values
    const double exaggeration = s.addSize.getExaggeration(s, this);
    // check if boundary has to be drawn
    if(s.drawBoundaries) {
        GLHelper::drawBoundary(getCenteringBoundary());
    }
    // push name and matrix
    glPushName(getGlID());
    glPushMatrix();
    // Traslate matrix and draw green contour
    glTranslated(myPosition.x(), myPosition.y(), getType() + 0.1);
    glRotated(myAngle, 0, 0, 1);
    // only drawn small box if isn't being drawn for selecting
    if (!s.drawForSelecting) {
        // Set Color depending of selection
        if (drawUsingSelectColor()) {
            GLHelper::setColor(s.colorSettings.selectedAdditionalColor);
        } else {
            GLHelper::setColor(s.colorSettings.parkingSpace);
        }
        GLHelper::drawBoxLine(Position(0, myLength + 0.05), 0, myLength + 0.1, (myWidth / 2) + 0.05);
    }
    // Traslate matrix and draw blue innen
    glTranslated(0, 0, 0.1);
    // Set Color depending of selection
    if (drawUsingSelectColor()) {
        GLHelper::setColor(s.colorSettings.selectedAdditionalColor);
    } else {
        GLHelper::setColor(s.colorSettings.parkingSpaceInnen);
    }
    GLHelper::drawBoxLine(Position(0, myLength), 0, myLength, myWidth / 2);
    // Traslate matrix and draw lock icon if isn't being drawn for selecting
    glTranslated(0, myLength / 2, 0.1);
    myBlockIcon.drawIcon(s, exaggeration);
    // pop draw matrix
    glPopMatrix();
    // check if dotted contour has to be drawn
    if (myViewNet->getDottedAC() == this) {
        GLHelper::drawShapeDottedContourRectangle(s, getType(), myPosition, myWidth, myLength, myAngle, 0, myLength / 2);
    }
    // pop name
    glPopName();
}


std::string
GNEParkingSpace::getAttribute(SumoXMLAttr key) const {
    switch (key) {
        case SUMO_ATTR_ID:
            return getAdditionalID();
        case SUMO_ATTR_POSITION:
            return toString(myPosition);
        case SUMO_ATTR_WIDTH:
            return toString(myWidth);
        case SUMO_ATTR_LENGTH:
            return toString(myLength);
        case SUMO_ATTR_ANGLE:
            return toString(myAngle);
        case GNE_ATTR_BLOCK_MOVEMENT:
            return toString(myBlockMovement);
        case GNE_ATTR_PARENT:
            return getAdditionalParents().at(0)->getID();
        case GNE_ATTR_SELECTED:
            return toString(isAttributeCarrierSelected());
        case GNE_ATTR_GENERIC:
            return getGenericParametersStr();
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


void
GNEParkingSpace::setAttribute(SumoXMLAttr key, const std::string& value, GNEUndoList* undoList) {
    if (value == getAttribute(key)) {
        return; //avoid needless changes, later logic relies on the fact that attributes have changed
    }
    switch (key) {
        case SUMO_ATTR_ID:
        case SUMO_ATTR_POSITION:
        case SUMO_ATTR_WIDTH:
        case SUMO_ATTR_LENGTH:
        case SUMO_ATTR_ANGLE:
        case GNE_ATTR_BLOCK_MOVEMENT:
        case GNE_ATTR_PARENT:
        case GNE_ATTR_SELECTED:
        case GNE_ATTR_GENERIC:
            undoList->p_add(new GNEChange_Attribute(this, myViewNet->getNet(), key, value));
            break;
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


bool
GNEParkingSpace::isValid(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_ID:
            return isValidAdditionalID(value);
        case SUMO_ATTR_POSITION:
            return canParse<Position>(value);
        case SUMO_ATTR_WIDTH:
            return canParse<double>(value) && (parse<double>(value) >= 0);
        case SUMO_ATTR_LENGTH:
            return canParse<double>(value) && (parse<double>(value) >= 0);
        case SUMO_ATTR_ANGLE:
            return canParse<double>(value);
        case GNE_ATTR_BLOCK_MOVEMENT:
            return canParse<bool>(value);
        case GNE_ATTR_PARENT:
            return (myViewNet->getNet()->retrieveAdditional(SUMO_TAG_PARKING_AREA, value, false) != nullptr);
        case GNE_ATTR_SELECTED:
            return canParse<bool>(value);
        case GNE_ATTR_GENERIC:
            return isGenericParametersValid(value);
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


std::string
GNEParkingSpace::getPopUpID() const {
    return getTagStr();
}


std::string
GNEParkingSpace::getHierarchyName() const {
    return getTagStr() + ": " + getAttribute(SUMO_ATTR_POSITION);
}

// ===========================================================================
// private
// ===========================================================================

void
GNEParkingSpace::setAttribute(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_ID:
            changeAdditionalID(value);
            break;
        case SUMO_ATTR_POSITION:
            myViewNet->getNet()->removeGLObjectFromGrid(this);
            myPosition = parse<Position>(value);
            myViewNet->getNet()->addGLObjectIntoGrid(this);
            break;
        case SUMO_ATTR_WIDTH:
            myViewNet->getNet()->removeGLObjectFromGrid(this);
            myWidth = parse<double>(value);
            myViewNet->getNet()->addGLObjectIntoGrid(this);
            break;
        case SUMO_ATTR_LENGTH:
            myViewNet->getNet()->removeGLObjectFromGrid(this);
            myLength = parse<double>(value);
            myViewNet->getNet()->addGLObjectIntoGrid(this);
            break;
        case SUMO_ATTR_ANGLE:
            myViewNet->getNet()->removeGLObjectFromGrid(this);
            myAngle = parse<double>(value);
            myViewNet->getNet()->addGLObjectIntoGrid(this);
            break;
        case GNE_ATTR_BLOCK_MOVEMENT:
            myBlockMovement = parse<bool>(value);
            break;
        case GNE_ATTR_PARENT:
            changeAdditionalParent(this, value, 0);
            break;
        case GNE_ATTR_SELECTED:
            if (parse<bool>(value)) {
                selectAttributeCarrier();
            } else {
                unselectAttributeCarrier();
            }
            break;
        case GNE_ATTR_GENERIC:
            setGenericParametersStr(value);
            break;
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


/****************************************************************************/
