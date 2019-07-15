/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2001-2019 German Aerospace Center (DLR) and others.
// This program and the accompanying materials
// are made available under the terms of the Eclipse Public License v2.0
// which accompanies this distribution, and is available at
// http://www.eclipse.org/legal/epl-v20.html
// SPDX-License-Identifier: EPL-2.0
/****************************************************************************/
/// @file    GNEBusStop.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Nov 2015
/// @version $Id$
///
// A lane area vehicles can halt at (GNE version)
/****************************************************************************/

// ===========================================================================
// included modules
// ===========================================================================

#include <foreign/fontstash/fontstash.h>
#include <netedit/GNENet.h>
#include <netedit/GNEUndoList.h>
#include <netedit/GNEViewNet.h>
#include <netedit/changes/GNEChange_Attribute.h>
#include <netedit/demandelements/GNEDemandElement.h>
#include <netedit/netelements/GNEEdge.h>
#include <netedit/netelements/GNELane.h>
#include <utils/options/OptionsCont.h>
#include <utils/gui/globjects/GLIncludes.h>
#include <utils/gui/div/GLHelper.h>
#include <utils/geom/GeomHelper.h>

#include "GNEBusStop.h"
#include "GNEAccess.h"
#include "GNEAdditionalHandler.h"

// ===========================================================================
// method definitions
// ===========================================================================

GNEBusStop::GNEBusStop(const std::string& id, GNELane* lane, GNEViewNet* viewNet, const std::string& startPos, const std::string& endPos, const std::string& name, const std::vector<std::string>& lines, int personCapacity, bool friendlyPosition, bool blockMovement) :
    GNEStoppingPlace(id, viewNet, GLO_BUS_STOP, SUMO_TAG_BUS_STOP, lane, startPos, endPos, name, friendlyPosition, blockMovement),
    myLines(lines),
    myPersonCapacity(personCapacity) {
}


GNEBusStop::~GNEBusStop() {}


void
GNEBusStop::updateGeometry() {
    // Get value of option "lefthand"
    double offsetSign = OptionsCont::getOptions().getBool("lefthand") ? -1 : 1;

    // Update common geometry of stopping place
    setStoppingPlaceGeometry(getLaneParents().front()->getParentEdge().getNBEdge()->getLaneWidth(getLaneParents().front()->getIndex()) / 2);

    // Obtain a copy of the shape
    PositionVector tmpShape = myGeometry.shape;

    // Move shape to side
    tmpShape.move2side(1.5 * offsetSign);

    // Get position of the sign
    mySignPos = tmpShape.getLineCenter();

    // Set block icon position
    myBlockIcon.position = myGeometry.shape.getLineCenter();

    // Set block icon rotation, and using their rotation for sign
    myBlockIcon.setRotation(getLaneParents().front());

    // update demand element children (GNEStops)
    for (const auto& i : getDemandElementChildren()) {
        i->updateGeometry();
    }
}


Boundary
GNEBusStop::getCenteringBoundary() const {
    return myGeometry.shape.getBoxBoundary().grow(10); 
}


void
GNEBusStop::drawGL(const GUIVisualizationSettings& s) const {
    // Obtain exaggeration of the draw
    const double exaggeration = s.addSize.getExaggeration(s, this);
    // Start drawing adding an gl identificator
    glPushName(getGlID());
    // Add a draw matrix
    glPushMatrix();
    // Start with the drawing of the area traslating matrix to origin
    glTranslated(0, 0, getType());
    // Set color of the base
    if (mySpecialColor) {
        GLHelper::setColor(*mySpecialColor);
    } else if (drawUsingSelectColor()) {
        GLHelper::setColor(s.colorSettings.selectedAdditionalColor);
    } else {
        GLHelper::setColor(s.colorSettings.busStop);
    }
    // Draw the area using shape, shapeRotations, shapeLengths and value of exaggeration
    GLHelper::drawBoxLines(myGeometry.shape, myGeometry.shapeRotations, myGeometry.shapeLengths, exaggeration);
    // Check if the distance is enought to draw details and if is being drawn for selecting
    if (s.drawForSelecting) {
        // only draw circle depending of distance between sign and mouse cursor
        if (myViewNet->getPositionInformation().distanceSquaredTo2D(mySignPos) <= (myCircleWidthSquared + 2)) {
            // Add a draw matrix for details
            glPushMatrix();
            // Start drawing sign traslating matrix to signal position
            glTranslated(mySignPos.x(), mySignPos.y(), 0);
            // scale matrix depending of the exaggeration
            glScaled(exaggeration, exaggeration, 1);
            // set color
            GLHelper::setColor(s.colorSettings.busStop);
            // Draw circle
            GLHelper::drawFilledCircle(myCircleWidth, s.getCircleResolution());
            // pop draw matrix
            glPopMatrix();
        }
    } else if (s.drawDetail(s.detailSettings.stoppingPlaceDetails, exaggeration)) {
        // draw lines between BusStops and Acces
        for (auto i : getAdditionalChildren()) {
            GLHelper::drawBoxLine(i->getShape()[0], RAD2DEG(mySignPos.angleTo2D(i->getShape()[0])) - 90, mySignPos.distanceTo2D(i->getShape()[0]), .05);
        }
        // Add a draw matrix for details
        glPushMatrix();
        // draw lines depending of detailSettings
        if (s.drawDetail(s.detailSettings.stoppingPlaceText, exaggeration)) {
            // Iterate over every line
            for (int i = 0; i < (int)myLines.size(); ++i) {
                // push a new matrix for every line
                glPushMatrix();
                // Rotate and traslaste
                glTranslated(mySignPos.x(), mySignPos.y(), 0);
                glRotated(-1 * myBlockIcon.rotation, 0, 0, 1);
                // draw line with a color depending of the selection status
                if (drawUsingSelectColor()) {
                    GLHelper::drawText(myLines[i].c_str(), Position(1.2, (double)i), .1, 1.f, s.colorSettings.selectionColor, 0, FONS_ALIGN_LEFT);
                } else {
                    GLHelper::drawText(myLines[i].c_str(), Position(1.2, (double)i), .1, 1.f, s.colorSettings.busStop, 0, FONS_ALIGN_LEFT);
                }
                // pop matrix for every line
                glPopMatrix();
            }
        }
        // Start drawing sign traslating matrix to signal position
        glTranslated(mySignPos.x(), mySignPos.y(), 0);
        // scale matrix depending of the exaggeration
        glScaled(exaggeration, exaggeration, 1);
        // Set color of the externe circle
        if (drawUsingSelectColor()) {
            GLHelper::setColor(s.colorSettings.selectedAdditionalColor);
        } else {
            GLHelper::setColor(s.colorSettings.busStop);
        }
        // Draw circle
        GLHelper::drawFilledCircle(myCircleWidth, s.getCircleResolution());
        // Traslate to front
        glTranslated(0, 0, .1);
        // Set color of the interne circle
        if (drawUsingSelectColor()) {
            GLHelper::setColor(s.colorSettings.selectionColor);
        } else {
            GLHelper::setColor(s.colorSettings.busStop_sign);
        }
        // draw another circle in the same position, but a little bit more small
        GLHelper::drawFilledCircle(myCircleInWidth, s.getCircleResolution());
        // draw H depending of detailSettings
        if (s.drawDetail(s.detailSettings.stoppingPlaceText, exaggeration)) {
            if (drawUsingSelectColor()) {
                GLHelper::drawText("H", Position(), .1, myCircleInText, s.colorSettings.selectedAdditionalColor, myBlockIcon.rotation);
            } else {
                GLHelper::drawText("H", Position(), .1, myCircleInText, s.colorSettings.busStop, myBlockIcon.rotation);
            }
        }
        // pop draw matrix
        glPopMatrix();
        // Show Lock icon depending of the Edit mode
        myBlockIcon.drawIcon(s, exaggeration);
    }
    // pop draw matrix
    glPopMatrix();
    // Draw name if isn't being drawn for selecting
    drawName(getPositionInView(), s.scale, s.addName);
    if (s.addFullName.show && (myAdditionalName != "") && !s.drawForSelecting) {
        GLHelper::drawText(myAdditionalName, mySignPos, GLO_MAX - getType(), s.addFullName.scaledSize(s.scale), s.addFullName.color, myBlockIcon.rotation);
    }
    // check if dotted contour has to be drawn
    if (myViewNet->getDottedAC() == this) {
        GLHelper::drawShapeDottedContourAroundShape(s, getType(), myGeometry.shape, exaggeration);
    }
    // Pop name
    glPopName();
    // draw demand element children
    for (const auto &i : getDemandElementChildren()) {
        if (!i->getTagProperty().isPlacedInRTree()) {
            i->drawGL(s);
        }
    }
}


std::string
GNEBusStop::getAttribute(SumoXMLAttr key) const {
    switch (key) {
        case SUMO_ATTR_ID:
            return getAdditionalID();
        case SUMO_ATTR_LANE:
            return getLaneParents().front()->getID();
        case SUMO_ATTR_STARTPOS:
            return toString(myStartPosition);
        case SUMO_ATTR_ENDPOS:
            return myEndPosition;
        case SUMO_ATTR_NAME:
            return myAdditionalName;
        case SUMO_ATTR_FRIENDLY_POS:
            return toString(myFriendlyPosition);
        case SUMO_ATTR_LINES:
            return joinToString(myLines, " ");
        case SUMO_ATTR_PERSON_CAPACITY:
            return toString(myPersonCapacity);
        case GNE_ATTR_BLOCK_MOVEMENT:
            return toString(myBlockMovement);
        case GNE_ATTR_SELECTED:
            return toString(isAttributeCarrierSelected());
        case GNE_ATTR_GENERIC:
            return getGenericParametersStr();
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


void
GNEBusStop::setAttribute(SumoXMLAttr key, const std::string& value, GNEUndoList* undoList) {
    if (value == getAttribute(key)) {
        return; //avoid needless changes, later logic relies on the fact that attributes have changed
    }
    switch (key) {
        case SUMO_ATTR_ID: {
            // change ID of BusStop
            undoList->p_add(new GNEChange_Attribute(this, myViewNet->getNet(), key, value));
            // Change Ids of all Acces children
            for (auto i : getAdditionalChildren()) {
                i->setAttribute(SUMO_ATTR_ID, generateChildID(SUMO_TAG_ACCESS), undoList);
            }
            break;
        }
        case SUMO_ATTR_LANE:
        case SUMO_ATTR_STARTPOS:
        case SUMO_ATTR_ENDPOS:
        case SUMO_ATTR_NAME:
        case SUMO_ATTR_FRIENDLY_POS:
        case SUMO_ATTR_LINES:
        case SUMO_ATTR_PERSON_CAPACITY:
        case GNE_ATTR_BLOCK_MOVEMENT:
        case GNE_ATTR_SELECTED:
        case GNE_ATTR_GENERIC:
            undoList->p_add(new GNEChange_Attribute(this, myViewNet->getNet(), key, value));
            break;
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


bool
GNEBusStop::isValid(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_ID:
            return isValidAdditionalID(value);
        case SUMO_ATTR_LANE:
            if (myViewNet->getNet()->retrieveLane(value, false) != nullptr) {
                return true;
            } else {
                return false;
            }
        case SUMO_ATTR_STARTPOS:
            if (value.empty()) {
                return true;
            } else if (canParse<double>(value)) {
                return checkStoppinPlacePosition(value, myEndPosition, getLaneParents().front()->getParentEdge().getNBEdge()->getFinalLength(), myFriendlyPosition);
            } else {
                return false;
            }
        case SUMO_ATTR_ENDPOS:
            if (value.empty()) {
                return true;
            } else if (canParse<double>(value)) {
                return checkStoppinPlacePosition(myStartPosition, value, getLaneParents().front()->getParentEdge().getNBEdge()->getFinalLength(), myFriendlyPosition);
            } else {
                return false;
            }
        case SUMO_ATTR_NAME:
            return SUMOXMLDefinitions::isValidAttribute(value);
        case SUMO_ATTR_FRIENDLY_POS:
            return canParse<bool>(value);
        case SUMO_ATTR_LINES:
            return canParse<std::vector<std::string> >(value);
        case SUMO_ATTR_PERSON_CAPACITY:
            return canParse<int>(value) && (parse<int>(value) > 0 || parse<int>(value) == -1);
        case GNE_ATTR_BLOCK_MOVEMENT:
            return canParse<bool>(value);
        case GNE_ATTR_SELECTED:
            return canParse<bool>(value);
        case GNE_ATTR_GENERIC:
            return isGenericParametersValid(value);
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}

// ===========================================================================
// private
// ===========================================================================

void
GNEBusStop::setAttribute(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_ID:
            changeAdditionalID(value);
            break;
        case SUMO_ATTR_LANE:
            changeLaneParents(this, value);
            break;
        case SUMO_ATTR_STARTPOS:
            myStartPosition = value;
            break;
        case SUMO_ATTR_ENDPOS:
            myEndPosition = value;
            break;
        case SUMO_ATTR_NAME:
            myAdditionalName = value;
            break;
        case SUMO_ATTR_FRIENDLY_POS:
            myFriendlyPosition = parse<bool>(value);
            break;
        case SUMO_ATTR_LINES:
            myLines = GNEAttributeCarrier::parse<std::vector<std::string> >(value);
            break;
        case SUMO_ATTR_PERSON_CAPACITY:
            myPersonCapacity = GNEAttributeCarrier::parse<int>(value);
            break;
        case GNE_ATTR_BLOCK_MOVEMENT:
            myBlockMovement = parse<bool>(value);
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
