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
/// @file    GNEDemandElementPlan.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Sep 2023
///
// An auxiliar, asbtract class for plan elements
/****************************************************************************/

#include <netedit/GNENet.h>
#include <netedit/GNESegment.h>
#include <netedit/GNEUndoList.h>
#include <netedit/GNEViewNet.h>
#include <netedit/changes/GNEChange_Attribute.h>
#include <utils/gui/div/GLHelper.h>
#include <utils/gui/div/GUIDesigns.h>
#include <utils/gui/windows/GUIAppEnum.h>

#include "GNEDemandElementPlan.h"
#include "GNERoute.h"

// ===========================================================================
// static definitions
// ===========================================================================

const double GNEDemandElementPlan::myArrivalPositionDiameter = SUMO_const_halfLaneWidth;

// ===========================================================================
// GNEDemandElement method definitions
// ===========================================================================

GNEDemandElementPlan::GNEDemandElementPlan(GNEDemandElement* planElement, const double departPosition, const double arrivalPosition) :
    myDepartPosition(departPosition),
    myArrivalPosition(arrivalPosition),
    myPlanElement(planElement) {
}


GNEMoveOperation*
GNEDemandElementPlan::getPlanMoveOperation() {
    // get tag property
    const auto tagProperty = myPlanElement->getTagProperty();
    // only move personTrips defined over edges
    if (tagProperty->planToEdge() || tagProperty->planConsecutiveEdges() || tagProperty->planEdge()) {
        // get geometry end pos
        const Position geometryEndPos = getPlanAttributePosition(GNE_ATTR_PLAN_GEOMETRY_ENDPOS);
        // calculate circle width squared
        const double circleWidthSquared = myArrivalPositionDiameter * myArrivalPositionDiameter;
        // check if we clicked over a geometry end pos
        if (myPlanElement->myNet->getViewNet()->getPositionInformation().distanceSquaredTo2D(geometryEndPos) <= ((circleWidthSquared + 2))) {
            // continue depending of parent edges
            if (myPlanElement->getParentEdges().size() > 0) {
                return new GNEMoveOperation(myPlanElement, myPlanElement->getParentEdges().back()->getLaneByAllowedVClass(myPlanElement->getVClass()), myArrivalPosition, false);
            } else {
                return new GNEMoveOperation(myPlanElement, myPlanElement->getParentDemandElements().at(1)->getParentEdges().back()->getLaneByAllowedVClass(myPlanElement->getVClass()), myArrivalPosition, false);
            }
        }
    }
    return nullptr;
}


void
GNEDemandElementPlan::writeLocationAttributes(OutputDevice& device) const {
    const auto tagProperty = myPlanElement->getTagProperty();
    // write attributes depending of parent elements
    if (tagProperty->planConsecutiveEdges()) {
        device.writeAttr(SUMO_ATTR_EDGES, myPlanElement->parseIDs(myPlanElement->getParentEdges()));
    } else if (tagProperty->planRoute()) {
        device.writeAttr(SUMO_ATTR_ROUTE, myPlanElement->getParentDemandElements().at(1)->getID());
    } else if (tagProperty->planEdge()) {
        device.writeAttr(SUMO_ATTR_EDGE, myPlanElement->getParentEdges().front()->getID());
    } else if (tagProperty->planBusStop()) {
        device.writeAttr(SUMO_ATTR_BUS_STOP, myPlanElement->getParentStoppingPlaces().front()->getID());
    } else if (tagProperty->planTrainStop()) {
        device.writeAttr(SUMO_ATTR_TRAIN_STOP, myPlanElement->getParentStoppingPlaces().front()->getID());
    } else if (tagProperty->planContainerStop()) {
        device.writeAttr(SUMO_ATTR_CONTAINER_STOP, myPlanElement->getParentStoppingPlaces().front()->getID());
    } else if (tagProperty->planChargingStation()) {
        device.writeAttr(SUMO_ATTR_CHARGING_STATION, myPlanElement->getParentStoppingPlaces().front()->getID());
    } else if (tagProperty->planParkingArea()) {
        device.writeAttr(SUMO_ATTR_PARKING_AREA, myPlanElement->getParentStoppingPlaces().front()->getID());
    } else {
        // write from attribute (if this is the first element)
        if (myPlanElement->getParentDemandElements().at(0)->getPreviousChildDemandElement(myPlanElement) == nullptr) {
            // check if write edge or junction
            if (tagProperty->planFromEdge()) {
                device.writeAttr(SUMO_ATTR_FROM, myPlanElement->getParentEdges().front()->getID());
            } else if (tagProperty->planFromTAZ()) {
                device.writeAttr(SUMO_ATTR_FROM_TAZ, myPlanElement->getParentTAZs().front()->getID());
            } else if (tagProperty->planFromJunction()) {
                device.writeAttr(SUMO_ATTR_FROM_JUNCTION, myPlanElement->getParentJunctions().front()->getID());
            }
            // origin stopping places are transformed into an intial stop stage (see writeOriginStop)
        }
        // continue writting to attribute
        if (tagProperty->planToEdge()) {
            device.writeAttr(SUMO_ATTR_TO, myPlanElement->getParentEdges().back()->getID());
        } else if (tagProperty->planToJunction()) {
            device.writeAttr(SUMO_ATTR_TO_JUNCTION, myPlanElement->getParentJunctions().back()->getID());
        } else if (tagProperty->planToTAZ()) {
            device.writeAttr(SUMO_ATTR_TO_TAZ, myPlanElement->getParentTAZs().back()->getID());
        } else if (tagProperty->planToBusStop()) {
            device.writeAttr(SUMO_ATTR_BUS_STOP, myPlanElement->getParentStoppingPlaces().back()->getID());
        } else if (tagProperty->planToTrainStop()) {
            device.writeAttr(SUMO_ATTR_TRAIN_STOP, myPlanElement->getParentStoppingPlaces().back()->getID());
        } else if (tagProperty->planToContainerStop()) {
            device.writeAttr(SUMO_ATTR_CONTAINER_STOP, myPlanElement->getParentStoppingPlaces().back()->getID());
        } else if (tagProperty->planToChargingStation()) {
            device.writeAttr(SUMO_ATTR_CHARGING_STATION, myPlanElement->getParentStoppingPlaces().back()->getID());
        } else if (tagProperty->planToParkingArea()) {
            device.writeAttr(SUMO_ATTR_PARKING_AREA, myPlanElement->getParentStoppingPlaces().back()->getID());
        }
    }
    // check if write depart position
    if (tagProperty->hasAttribute(SUMO_ATTR_DEPARTPOS) && (myDepartPosition > 0)) {
        device.writeAttr(SUMO_ATTR_DEPARTPOS, myDepartPosition);
    }
    // check if write arrival position
    if (tagProperty->hasAttribute(SUMO_ATTR_ARRIVALPOS) && (myArrivalPosition > 0)) {
        device.writeAttr(SUMO_ATTR_ARRIVALPOS, myArrivalPosition);
    }
    // check if write end position
    if (tagProperty->hasAttribute(SUMO_ATTR_ENDPOS)) {
        device.writeAttr(SUMO_ATTR_ENDPOS, myArrivalPosition);
    }
}


void
GNEDemandElementPlan::writeOriginStop(OutputDevice& device) const {
    const auto tagProperty = myPlanElement->getTagProperty();
    // write an extra stop element with duration 0 over a stopping place (if this is the first element)
    if (tagProperty->planFromStoppingPlace()
            && myPlanElement->getParentDemandElements().at(0)->getPreviousChildDemandElement(myPlanElement) == nullptr) {
        device.openTag(SUMO_TAG_STOP);
        const std::string stopID = myPlanElement->getParentStoppingPlaces().front()->getID();
        if (tagProperty->planFromBusStop()) {
            device.writeAttr(SUMO_ATTR_BUS_STOP, stopID);
        } else if (tagProperty->planFromTrainStop()) {
            device.writeAttr(SUMO_ATTR_TRAIN_STOP, stopID);
        } else if (tagProperty->planFromContainerStop()) {
            device.writeAttr(SUMO_ATTR_CONTAINER_STOP, stopID);
        } else if (tagProperty->planFromChargingStation()) {
            device.writeAttr(SUMO_ATTR_CHARGING_STATION, stopID);
        } else if (tagProperty->planFromParkingArea()) {
            device.writeAttr(SUMO_ATTR_PARKING_AREA, stopID);
        }
        device.writeAttr(SUMO_ATTR_DURATION, 0);
        device.closeTag();
    }
}


GUIGLObjectPopupMenu*
GNEDemandElementPlan::getPlanPopUpMenu(GUIMainWindow& app, GUISUMOAbstractView& parent) {
    // create popup
    GUIGLObjectPopupMenu* ret = new GUIGLObjectPopupMenu(app, parent, myPlanElement);
    // build common options
    myPlanElement->buildPopUpMenuCommonOptions(ret, app, myPlanElement->myNet->getViewNet(), myPlanElement->getTagProperty()->getTag(), myPlanElement->isAttributeCarrierSelected());
    GUIDesigns::buildFXMenuCommand(ret, ("Cursor position in view: " + toString(getPlanPositionInView().x()) + "," + toString(getPlanPositionInView().y())).c_str(), nullptr, nullptr, 0);
    return ret;
}


GNELane*
GNEDemandElementPlan::getFirstPlanPathLane() const {
    // get tag property
    const auto tagProperty = myPlanElement->getTagProperty();
    // get vclass
    auto vClass = myPlanElement->getVClass();
    // continue depending of parents
    if (tagProperty->planRoute()) {
        // route
        return myPlanElement->getParentDemandElements().at(1)->getParentEdges().front()->getLaneByAllowedVClass(vClass);
    } else if (tagProperty->planConsecutiveEdges() || tagProperty->planFromEdge() || tagProperty->planEdge()) {
        // edges
        return myPlanElement->getParentEdges().front()->getLaneByAllowedVClass(vClass);
    } else if (tagProperty->planStoppingPlace() || tagProperty->planFromStoppingPlace() || tagProperty->planToStoppingPlace()) {
        // additional
        return myPlanElement->getParentStoppingPlaces().front()->getParentLanes().front();
    } else {
        // in other cases (TAZ, junctions, etc.) return null
        return nullptr;
    }
}


GNELane*
GNEDemandElementPlan::getLastPlanPathLane() const {
    // get tag property
    const auto tagProperty = myPlanElement->getTagProperty();
    // get vclass
    auto vClass = myPlanElement->getVClass();
    // check parents
    if (tagProperty->planRoute()) {
        // route
        return myPlanElement->getParentDemandElements().at(1)->getParentEdges().back()->getLaneByAllowedVClass(vClass);
    } else if (tagProperty->planConsecutiveEdges() || tagProperty->planToEdge() || tagProperty->planEdge()) {
        // edges
        return myPlanElement->getParentEdges().back()->getLaneByAllowedVClass(vClass);
    } else if (tagProperty->planStoppingPlace() || tagProperty->planFromStoppingPlace() || tagProperty->planToStoppingPlace()) {
        // additional
        return myPlanElement->getParentStoppingPlaces().back()->getParentLanes().front()->getParentEdge()->getLaneByAllowedVClass(vClass);
    } else {
        // in other cases (TAZ, junctions, etc.) return null
        return nullptr;
    }
}


void
GNEDemandElementPlan::computePlanPathElement() {
    // get tag property
    const auto tagProperty = myPlanElement->getTagProperty();
    // get vClass
    auto vClass = myPlanElement->getVClass();
    // get path manager
    auto pathManager = myPlanElement->getNet()->getDemandPathManager();
    // continue depending of parents
    if (tagProperty->planRoute()) {
        // calculate consecutive path using route edges
        pathManager->calculateConsecutivePathEdges(myPlanElement, vClass, myPlanElement->getParentDemandElements().at(1)->getParentEdges());
    } else if (tagProperty->planConsecutiveEdges()) {
        // calculate consecutive path using edges
        pathManager->calculateConsecutivePathEdges(myPlanElement, vClass, myPlanElement->getParentEdges());
    } else if (myPlanElement->myTagProperty->planFromJunction() && myPlanElement->myTagProperty->planToJunction()) {
        // calculate path using junctions
        pathManager->calculatePath(myPlanElement, vClass, myPlanElement->getParentJunctions().front(), myPlanElement->getParentJunctions().back());
    } else if (myPlanElement->myTagProperty->planFromJunction()) {
        // declare last lane
        GNELane* lastLane = nullptr;
        if (myPlanElement->myTagProperty->planStoppingPlace()) {
            lastLane = myPlanElement->getParentStoppingPlaces().back()->getParentLanes().back();
        } else if (myPlanElement->myTagProperty->planToStoppingPlace()) {
            lastLane = myPlanElement->getParentStoppingPlaces().back()->getParentLanes().front();
        } else if (myPlanElement->myTagProperty->planToEdge()) {
            lastLane = myPlanElement->getParentEdges().back()->getLaneByAllowedVClass(vClass);
        }
        // calculate path
        if (lastLane) {
            pathManager->calculatePath(myPlanElement, vClass, myPlanElement->getParentJunctions().front(), lastLane);
        }
    } else if (myPlanElement->myTagProperty->planToJunction()) {
        // declare first lane
        GNELane* firstLane = nullptr;
        if (myPlanElement->myTagProperty->planStoppingPlace()) {
            firstLane = myPlanElement->getParentStoppingPlaces().front()->getParentLanes().back();
        } else if (myPlanElement->myTagProperty->planFromStoppingPlace()) {
            firstLane = myPlanElement->getParentStoppingPlaces().front()->getParentLanes().front();
        } else if (myPlanElement->myTagProperty->planFromEdge()) {
            firstLane = myPlanElement->getParentEdges().front()->getLaneByAllowedVClass(vClass);
        }
        // calculate path
        if (firstLane) {
            pathManager->calculatePath(myPlanElement, vClass, firstLane, myPlanElement->getParentJunctions().back());
        }
    } else {
        // declare first edge
        GNELane* firstLane = nullptr;
        if (myPlanElement->myTagProperty->planFromEdge()) {
            firstLane = myPlanElement->getParentEdges().front()->getLaneByAllowedVClass(vClass);
        } else if (myPlanElement->myTagProperty->planStoppingPlace()) {
            firstLane = myPlanElement->getParentStoppingPlaces().front()->getParentLanes().back();
        } else if (myPlanElement->myTagProperty->planFromStoppingPlace()) {
            firstLane = myPlanElement->getParentStoppingPlaces().front()->getParentLanes().front();
        }
        // declare last lane
        GNELane* lastLane = nullptr;
        if (myPlanElement->myTagProperty->planToEdge()) {
            lastLane = myPlanElement->getParentEdges().back()->getLaneByAllowedVClass(vClass);
        } else if (myPlanElement->myTagProperty->planStoppingPlace()) {
            lastLane = myPlanElement->getParentStoppingPlaces().back()->getParentLanes().back();
        } else if (myPlanElement->myTagProperty->planToStoppingPlace()) {
            lastLane = myPlanElement->getParentStoppingPlaces().back()->getParentLanes().front();
        }
        if (firstLane && lastLane) {
            pathManager->calculatePath(myPlanElement, vClass, firstLane, lastLane);
        } else if (firstLane) {
            pathManager->calculateConsecutivePathLanes(myPlanElement, {firstLane});
        } else if (lastLane) {
            pathManager->calculateConsecutivePathLanes(myPlanElement, {lastLane});
        }
    }
    // update geometry
    updatePlanGeometry();
}


void
GNEDemandElementPlan::updatePlanGeometry() {
    // get tag property
    const auto tagProperty = myPlanElement->getTagProperty();
    // check if plan start or end in a TAZ (becase in this case has to be inserted in RTREE
    if (tagProperty->planFromTAZ() || tagProperty->planToTAZ()) {
        // declare first and last positions
        Position firstPos = Position::INVALID;
        Position lastPos = Position::INVALID;
        // set first position
        if (tagProperty->planFromEdge()) {
            // from junction
            firstPos = myPlanElement->getFirstPathLane()->getLaneShape().back();
        } else if (tagProperty->planFromJunction()) {
            // from junction
            firstPos = myPlanElement->getParentJunctions().front()->getPositionInView();
        } else if (tagProperty->planFromStoppingPlace()) {
            // end of stoppingPlace lane shape
            firstPos = myPlanElement->getParentStoppingPlaces().front()->getParentLanes().front()->getLaneShape().back();
        } else if (tagProperty->planFromTAZ()) {
            // from TAZ
            if (myPlanElement->getParentTAZs().front()->getAttribute(SUMO_ATTR_CENTER).empty()) {
                firstPos = myPlanElement->getParentTAZs().front()->getAttributePosition(GNE_ATTR_TAZ_CENTROID);
            } else {
                firstPos = myPlanElement->getParentTAZs().front()->getAttributePosition(SUMO_ATTR_CENTER);
            }
        }
        // set last position
        if (tagProperty->planToEdge()) {
            // from junction
            lastPos = myPlanElement->getLastPathLane()->getLaneShape().back();
        } else if (tagProperty->planToJunction()) {
            // from junction
            lastPos = myPlanElement->getParentJunctions().back()->getPositionInView();
        } else if (tagProperty->planToStoppingPlace()) {
            // end of stoppingPlace lane shape
            lastPos = myPlanElement->getParentStoppingPlaces().back()->getParentLanes().front()->getLaneShape().front();
        } else if (tagProperty->planToTAZ()) {
            // from TAZ
            if (myPlanElement->getParentTAZs().back()->getAttribute(SUMO_ATTR_CENTER).empty()) {
                lastPos = myPlanElement->getParentTAZs().back()->getAttributePosition(GNE_ATTR_TAZ_CENTROID);
            } else {
                lastPos = myPlanElement->getParentTAZs().back()->getAttributePosition(SUMO_ATTR_CENTER);
            }
        }
        // use both position to calculate a line
        if ((firstPos != Position::INVALID) && (lastPos != Position::INVALID)) {
            myPlanElement->myDemandElementGeometry.updateGeometry({firstPos, lastPos});
        } else {
            myPlanElement->myDemandElementGeometry.clearGeometry();
        }
    }
    // update centering boundary
    updatePlanCenteringBoundary(true);
    // update child demand elements
    for (const auto& demandElement : myPlanElement->getChildDemandElements()) {
        demandElement->updateGeometry();
    }
}


Boundary
GNEDemandElementPlan::getPlanCenteringBoundary() const {
    return myPlanBoundary;
}


void
GNEDemandElementPlan::updatePlanCenteringBoundary(const bool updateGrid) {
    // remove additional from grid
    if (myPlanBoundary.isInitialised() && updateGrid) {
        myPlanElement->getNet()->removeGLObjectFromGrid(myPlanElement);
    }
    myPlanBoundary = myPlanElement->getDemandElementGeometry().getShape().getBoxBoundary();
    // if this element is over route, add their boundary
    if (myPlanElement->getParentDemandElements().size() > 1) {
        myPlanBoundary.add(myPlanElement->getParentDemandElements().at(1)->getCenteringBoundary());
    }
    // add the combination of all parent edges's boundaries
    for (const auto& edge : myPlanElement->getParentEdges()) {
        myPlanBoundary.add(edge->getCenteringBoundary());
    }
    // add the combination of all parent edges's boundaries
    for (const auto& junction : myPlanElement->getParentJunctions()) {
        myPlanBoundary.add(junction->getCenteringBoundary());
    }
    // add the combination of all parent additional's boundaries (stoppingPlaces and TAZs)
    for (const auto& additional : myPlanElement->getParentAdditionals()) {
        if (additional->getTagProperty()->getTag() == SUMO_TAG_TAZ) {
            if (additional->getAttribute(SUMO_ATTR_CENTER).empty()) {
                myPlanBoundary.add(additional->getAttributePosition(GNE_ATTR_TAZ_CENTROID));
            } else {
                myPlanBoundary.add(additional->getAttributePosition(SUMO_ATTR_CENTER));
            }
        } else {
            myPlanBoundary.add(additional->getCenteringBoundary());
        }
    }
    // check if is valid
    if (myPlanBoundary.isInitialised()) {
        myPlanBoundary.grow(5);
    }
    // add additional into RTREE again
    if (myPlanBoundary.isInitialised() && updateGrid) {
        myPlanElement->getNet()->addGLObjectIntoGrid(myPlanElement);
    }
}


Position
GNEDemandElementPlan::getPlanPositionInView() const {
    // get tag property
    const auto tagProperty = myPlanElement->getTagProperty();
    // continue depending of parents
    if (tagProperty->planRoute()) {
        // route
        return myPlanElement->getParentDemandElements().at(1)->getPositionInView();
    } else if (tagProperty->isPlanStop()) {
        return myPlanElement->getDemandElementGeometry().getShape().front();
    } else if (tagProperty->planFromEdge() || tagProperty->planConsecutiveEdges() || tagProperty->planEdge()) {
        // first edge
        return myPlanElement->getParentEdges().front()->getPositionInView();
    } else if (tagProperty->planFromJunction()) {
        // first junction
        return myPlanElement->getParentJunctions().front()->getPositionInView();
    } else if (tagProperty->planStoppingPlace() || tagProperty->planFromStoppingPlace()) {
        // first additional
        return myPlanElement->getParentStoppingPlaces().front()->getPositionInView();
    } else if (tagProperty->planFromTAZ()) {
        if (myPlanElement->getParentTAZs().front()->getAttribute(SUMO_ATTR_CENTER).empty()) {
            return myPlanElement->getParentTAZs().front()->getAttributePosition(GNE_ATTR_TAZ_CENTROID);
        } else {
            return myPlanElement->getParentTAZs().front()->getAttributePosition(SUMO_ATTR_CENTER);
        }
    } else {
        // return parent position
        return Position(0, 0);
    }
}


std::string
GNEDemandElementPlan::getPlanAttribute(SumoXMLAttr key) const {
    // continue depending of key
    switch (key) {
        // Common plan attributes
        case SUMO_ATTR_ID:
        case GNE_ATTR_PARENT:
            return myPlanElement->getParentDemandElements().at(0)->getID();
        case SUMO_ATTR_DEPARTPOS:
            if (myDepartPosition < 0) {
                return "";
            } else {
                return toString(myDepartPosition);
            }
        case SUMO_ATTR_ENDPOS:
        case SUMO_ATTR_ARRIVALPOS:
            if (myArrivalPosition < 0) {
                return "";
            } else {
                return toString(myArrivalPosition);
            }
        // route
        case SUMO_ATTR_ROUTE:
            return myPlanElement->getParentDemandElements().at(1)->getID();
        // edges
        case SUMO_ATTR_EDGE:
        case SUMO_ATTR_EDGES:
            return myPlanElement->parseIDs(myPlanElement->getParentEdges());
        // stoppingPlaces (single and back)
        case SUMO_ATTR_BUS_STOP:
        case SUMO_ATTR_TRAIN_STOP:
        case SUMO_ATTR_CONTAINER_STOP:
        case SUMO_ATTR_CHARGING_STATION:
        case SUMO_ATTR_PARKING_AREA:
            return myPlanElement->getParentStoppingPlaces().back()->getID();
        // from elements
        case SUMO_ATTR_FROM:
            return myPlanElement->getParentEdges().front()->getID();
        case SUMO_ATTR_FROM_JUNCTION:
            return myPlanElement->getParentJunctions().front()->getID();
        case SUMO_ATTR_FROM_TAZ:
            return myPlanElement->getParentTAZs().front()->getID();
        case GNE_ATTR_FROM_BUSSTOP:
        case GNE_ATTR_FROM_TRAINSTOP:
        case GNE_ATTR_FROM_CONTAINERSTOP:
        case GNE_ATTR_FROM_CHARGINGSTATION:
        case GNE_ATTR_FROM_PARKINGAREA:
            return myPlanElement->getParentStoppingPlaces().front()->getID();
        // to elements
        case SUMO_ATTR_TO:
            return myPlanElement->getParentEdges().back()->getID();
        case SUMO_ATTR_TO_JUNCTION:
            return myPlanElement->getParentJunctions().back()->getID();
        case SUMO_ATTR_TO_TAZ:
            return myPlanElement->getParentTAZs().back()->getID();
        default:
            return myPlanElement->getCommonAttribute(dynamic_cast<Parameterised*>(myPlanElement), key);
    }
}


double
GNEDemandElementPlan::getPlanAttributeDouble(SumoXMLAttr key) const {
    // get tag property
    const auto tagProperty = myPlanElement->getTagProperty();
    // declare plan parent
    const auto planParent = myPlanElement->getParentDemandElements().at(0);
    // continue depending of key
    switch (key) {
        case GNE_ATTR_PLAN_GEOMETRY_STARTPOS: {
            if (tagProperty->planStoppingPlace()) {
                // use startpos of stoppingPlace parent (stops)
                const auto factor = myPlanElement->getParentStoppingPlaces().front()->getParentLanes().front()->getLengthGeometryFactor();
                return myPlanElement->getParentStoppingPlaces().front()->getAttributeDouble(SUMO_ATTR_STARTPOS) * factor;
            } else if (tagProperty->planFromStoppingPlace()) {
                // use end position of stoppingPlace parent (for plans that starts in stoppingPlaces)
                const auto factor = myPlanElement->getParentStoppingPlaces().front()->getParentLanes().front()->getLengthGeometryFactor();
                return myPlanElement->getParentStoppingPlaces().front()->getAttributeDouble(SUMO_ATTR_ENDPOS) * factor;
            } else if (tagProperty->planFromTAZ()) {
                return 0;
            } else if (tagProperty->planFromJunction()) {
                return -1;
            } else {
                // get previous plan element
                const auto previousPlan = planParent->getPreviousChildDemandElement(myPlanElement);
                // continue depending of previous plan
                if (previousPlan) {
                    // use previous plan end position (normally the arrival position)
                    const auto posOverLane = previousPlan->getAttributeDouble(GNE_ATTR_PLAN_GEOMETRY_ENDPOS);
                    // if posOverLane is -1, means that previousPlan ends in the end of lane.
                    if (posOverLane == -1) {
                        // INVALID_DOUBLE will put the startPositio at the end of line
                        return INVALID_DOUBLE;
                    } else {
                        return previousPlan->getAttributeDouble(GNE_ATTR_PLAN_GEOMETRY_ENDPOS);
                    }
                } else {
                    // use depart position defined in parent (person or container)
                    return planParent->getAttributeDouble(SUMO_ATTR_DEPARTPOS);
                }
            }
        }
        case GNE_ATTR_PLAN_GEOMETRY_ENDPOS:
            // continue depending of parents
            if (tagProperty->planStoppingPlace()) {
                // use end position of the stoppingPlace (stops)
                const auto factor = myPlanElement->getParentStoppingPlaces().back()->getParentLanes().front()->getLengthGeometryFactor();
                return myPlanElement->getParentStoppingPlaces().back()->getAttributeDouble(SUMO_ATTR_ENDPOS) * factor;
            } else if (tagProperty->planToStoppingPlace()) {
                // use start position of the stoppingPlace (for elements that ends in stoppingPlaces)
                const auto factor = myPlanElement->getParentStoppingPlaces().back()->getParentLanes().front()->getLengthGeometryFactor();
                return myPlanElement->getParentStoppingPlaces().back()->getAttributeDouble(SUMO_ATTR_STARTPOS) * factor;
            } else if (tagProperty->planToJunction() || tagProperty->planToTAZ()) {
                // junctions and TAZs return always -1
                return -1;
            } else if ((tagProperty->isPlanStopPerson() || tagProperty->isPlanStopContainer()) && tagProperty->planEdge()) {
                // elements that ends in stop always uses the end (arrival) position of the stops over edges
                return myArrivalPosition;
            } else {
                // check if next plan is a stop over edge
                const auto nextPlan = planParent->getNextChildDemandElement(myPlanElement);
                if (nextPlan && (nextPlan->getTagProperty()->isPlanStopPerson() ||
                                 nextPlan->getTagProperty()->isPlanStopContainer()) &&
                        nextPlan->getTagProperty()->planEdge()) {
                    // if next plan is an stop over stoppingPlaces, use ends of stoppingPlace
                    return nextPlan->getAttributeDouble(GNE_ATTR_PLAN_GEOMETRY_ENDPOS);
                } else {
                    // use arrival position
                    return myArrivalPosition;
                }
            }
        case SUMO_ATTR_DEPARTPOS:
            return myDepartPosition;
        case SUMO_ATTR_ENDPOS:
        case SUMO_ATTR_ARRIVALPOS:
            return myArrivalPosition;
        default:
            throw InvalidArgument(myPlanElement->getTagStr() + " doesn't have a doubleattribute of type '" + toString(key) + "'");
    }
}


Position
GNEDemandElementPlan::getPlanAttributePosition(SumoXMLAttr key) const {
    // get tag property
    const auto tagProperty = myPlanElement->getTagProperty();
    // declare plan parent
    const auto planParent = myPlanElement->getParentDemandElements().at(0);
    // continue depending of key
    switch (key) {
        case GNE_ATTR_PLAN_GEOMETRY_STARTPOS: {
            // get previous plan
            const auto previousPlan = planParent->getPreviousChildDemandElement(myPlanElement);
            if (previousPlan && previousPlan->getTagProperty()->isPlanStop() && previousPlan->getTagProperty()->planStoppingPlace()) {
                return previousPlan->getParentStoppingPlaces().front()->getAdditionalGeometry().getShape().back();
            }
            // continue depending of from element
            if (tagProperty->planStoppingPlace()) {
                return myPlanElement->getParentStoppingPlaces().front()->getAdditionalGeometry().getShape().front();
            } else if (tagProperty->planFromStoppingPlace()) {
                return myPlanElement->getParentStoppingPlaces().front()->getAdditionalGeometry().getShape().back();
            } else if (tagProperty->planFromJunction()) {
                // junction view position
                return myPlanElement->getParentJunctions().front()->getPositionInView();
            } else if (tagProperty->planFromTAZ()) {
                if (myPlanElement->getParentTAZs().front()->getAttribute(SUMO_ATTR_CENTER).empty()) {
                    return myPlanElement->getParentTAZs().front()->getAttributePosition(GNE_ATTR_TAZ_CENTROID);
                } else {
                    return myPlanElement->getParentTAZs().front()->getAttributePosition(SUMO_ATTR_CENTER);
                }
            } else if (tagProperty->planConsecutiveEdges() || tagProperty->planRoute() || tagProperty->planFromEdge()) {
                // get first path lane
                const auto firstLane = myPlanElement->getFirstPathLane();
                // check if first lane exists
                if (firstLane == nullptr) {
                    return Position::INVALID;
                }
                // declare lane position
                double lanePosition = 0;
                // continue depending of conditions
                if (previousPlan) {
                    // use previous geometry end position
                    lanePosition = previousPlan->getAttributeDouble(GNE_ATTR_PLAN_GEOMETRY_ENDPOS);
                } else {
                    // use departPos defined in planParent
                    lanePosition = planParent->getAttributeDouble(SUMO_ATTR_DEPARTPOS);
                }
                // get lane shape
                const auto& laneShape = firstLane->getLaneShape();
                // continue depending of lane position
                if (lanePosition <= 0) {
                    return laneShape.front();
                } else if (lanePosition >= laneShape.length2D()) {
                    return laneShape.back();
                } else {
                    return laneShape.positionAtOffset2D(lanePosition);
                }
            } else {
                return Position::INVALID;
            }
        }
        case GNE_ATTR_PLAN_GEOMETRY_ENDPOS: {
            // check parents
            if (tagProperty->planToJunction()) {
                // junctions
                return myPlanElement->getParentJunctions().back()->getPositionInView();
            } else if (tagProperty->planStoppingPlace()) {
                // get additional back shape (stops)
                return myPlanElement->getParentStoppingPlaces().back()->getAdditionalGeometry().getShape().back();
            } else if (tagProperty->planToStoppingPlace()) {
                // get additional front shape
                return myPlanElement->getParentStoppingPlaces().back()->getAdditionalGeometry().getShape().front();
            } else if (tagProperty->planToTAZ()) {
                // taz
                if (myPlanElement->getParentTAZs().back()->getAttribute(SUMO_ATTR_CENTER).empty()) {
                    return myPlanElement->getParentTAZs().back()->getAttributePosition(GNE_ATTR_TAZ_CENTROID);
                } else {
                    return myPlanElement->getParentTAZs().back()->getAttributePosition(SUMO_ATTR_CENTER);
                }
            } else if (tagProperty->planConsecutiveEdges() || tagProperty->planRoute() || tagProperty->planFromEdge()) {
                // get next plan
                const auto nextPlan = planParent->getNextChildDemandElement(myPlanElement);
                // if next plan exist, then use their first lane (needed to maintain connectivity with rides)
                const auto lastLane = nextPlan ? nextPlan->getFirstPathLane() : myPlanElement->getLastPathLane();
                // check if last lane exists
                if (lastLane == nullptr) {
                    return Position::INVALID;
                }
                // get lane shape
                const auto& laneShape = lastLane->getLaneShape();
                // continue depending of arrival position
                if (nextPlan && nextPlan->getTagProperty()->isPlanStop()) {
                    return nextPlan->getAttributePosition(GNE_ATTR_PLAN_GEOMETRY_ENDPOS);
                } else if (myArrivalPosition == 0) {
                    return laneShape.front();
                } else if ((myArrivalPosition == -1) || (myArrivalPosition >= laneShape.length2D())) {
                    return laneShape.back();
                } else {
                    return laneShape.positionAtOffset2D(myArrivalPosition);
                }
            } else {
                return Position::INVALID;
            }
        }
        default:
            throw InvalidArgument(myPlanElement->getTagStr() + " doesn't have a position attribute of type '" + toString(key) + "'");
    }
}


void
GNEDemandElementPlan::setPlanAttribute(SumoXMLAttr key, const std::string& value, GNEUndoList* undoList) {
    // continue depending of key
    switch (key) {
        // common attributes
        case SUMO_ATTR_DEPARTPOS:
        case SUMO_ATTR_ARRIVALPOS:
        case SUMO_ATTR_ENDPOS:
        case GNE_ATTR_PARENT:
            GNEChange_Attribute::changeAttribute(myPlanElement, key, value, undoList);
            break;
        default:
            myPlanElement->setCommonAttribute(key, value, undoList);
            break;
    }
}


bool
GNEDemandElementPlan::isPlanValid(SumoXMLAttr key, const std::string& value) {
    // continue depending of key
    switch (key) {
        // common attributes
        case GNE_ATTR_PARENT:
            return false;
        case SUMO_ATTR_DEPARTPOS:
        case SUMO_ATTR_ARRIVALPOS:
            if (value.empty()) {
                return true;
            } else if (GNEAttributeCarrier::canParse<double>(value)) {
                return GNEAttributeCarrier::parse<double>(value) >= 0;
            } else {
                return false;
            }
        case SUMO_ATTR_ENDPOS:
            return GNEAttributeCarrier::canParse<double>(value);
        default:
            return myPlanElement->isCommonValid(key, value);
    }
}


bool
GNEDemandElementPlan::isPlanAttributeEnabled(SumoXMLAttr key) const {
    switch (key) {
        // edges
        case SUMO_ATTR_EDGES:
        // edge
        case SUMO_ATTR_EDGE:
        // route
        case SUMO_ATTR_ROUTE:
        // from
        case SUMO_ATTR_FROM:
        case SUMO_ATTR_FROM_JUNCTION:
        case SUMO_ATTR_FROM_TAZ:
        case GNE_ATTR_FROM_BUSSTOP:
        case GNE_ATTR_FROM_TRAINSTOP:
        case GNE_ATTR_FROM_CONTAINERSTOP:
        case GNE_ATTR_FROM_CHARGINGSTATION:
        case GNE_ATTR_FROM_PARKINGAREA:
        // to
        case SUMO_ATTR_TO:
        case SUMO_ATTR_TO_JUNCTION:
        case SUMO_ATTR_TO_TAZ:
        case SUMO_ATTR_BUS_STOP:
        case SUMO_ATTR_TRAIN_STOP:
        case SUMO_ATTR_CONTAINER_STOP:
        case SUMO_ATTR_CHARGING_STATION:
        case SUMO_ATTR_PARKING_AREA:
        // depart pos (temporal, probably will be removed)
        case SUMO_ATTR_DEPARTPOS:
            return false;
        default:
            return true;
    }
}


void
GNEDemandElementPlan::setPlanAttribute(SumoXMLAttr key, const std::string& value) {
    bool recompute = false;
    switch (key) {
        // from-to attributes (needed if we're replacing junction by geometry points and similar operations)
        case SUMO_ATTR_FROM:
            myPlanElement->replaceFirstParentEdge(value);
            recompute = true;
            break;
        case SUMO_ATTR_TO:
            myPlanElement->replaceLastParentEdge(value);
            recompute = true;
            break;
        // Common plan attributes
        case GNE_ATTR_PARENT:
            replacePlanParent(value);
            break;
        case SUMO_ATTR_DEPARTPOS:
            if (value.empty()) {
                myDepartPosition = -1;
            } else {
                myDepartPosition = GNEAttributeCarrier::parse<double>(value);
            }
            recompute = true;
            break;
        case SUMO_ATTR_ENDPOS:
        case SUMO_ATTR_ARRIVALPOS:
            if (value.empty()) {
                myArrivalPosition = -1;
            } else {
                myArrivalPosition = GNEAttributeCarrier::parse<double>(value);
            }
            recompute = true;
            break;
        default:
            myPlanElement->setCommonAttribute(dynamic_cast<Parameterised*>(myPlanElement), key, value);
            break;
    }
    // check if compute geometry and path
    if (recompute && !myPlanElement->isTemplate()) {
        myPlanElement->updateGeometry();
        myPlanElement->computePathElement();
    }
}


std::string
GNEDemandElementPlan::getPlanHierarchyName() const {
    // get tag property
    const auto tagProperty = myPlanElement->getTagProperty();
    // declare result
    std::string result;
    // clear result
    int index = 0;
    while (tagProperty->getTagStr().at(index) != ':') {
        result.push_back(tagProperty->getTagStr().at(index));
        index++;
    }
    result += ": ";
    // continue depending of attributes
    if (tagProperty->planConsecutiveEdges()) {
        // edges
        return result + myPlanElement->getParentEdges().front()->getID() + " ... " + myPlanElement->getParentEdges().back()->getID();
    } else if (tagProperty->planRoute()) {
        // route
        return result + myPlanElement->getParentDemandElements().at(1)->getID();
    } else if (tagProperty->planEdge()) {
        // edge
        return result + myPlanElement->getParentEdges().front()->getID();
    } else if (tagProperty->planStoppingPlace()) {
        // stoppingPlace
        return myPlanElement->getParentStoppingPlaces().front()->getID();
    } else {
        // stoppingPlace
        if (tagProperty->planFromStoppingPlace()) {
            result += myPlanElement->getParentStoppingPlaces().front()->getID();
        }
        // TAZ
        if (tagProperty->planFromTAZ()) {
            result += myPlanElement->getParentTAZs().front()->getID();
        }
        // junction
        if (tagProperty->planFromJunction()) {
            result += myPlanElement->getParentJunctions().front()->getID();
        }
        // edge
        if (tagProperty->planFromEdge()) {
            result += myPlanElement->getParentEdges().front()->getID();
        }
        // arrow
        result += " -> ";
        // stoppingPlace
        if (tagProperty->planToStoppingPlace()) {
            result += myPlanElement->getParentStoppingPlaces().back()->getID();
        }
        // TAZ
        if (tagProperty->planToTAZ()) {
            result += myPlanElement->getParentTAZs().back()->getID();
        }
        // junction
        if (tagProperty->planToJunction()) {
            result += myPlanElement->getParentJunctions().back()->getID();
        }
        // edge
        if (tagProperty->planToEdge()) {
            result += myPlanElement->getParentEdges().back()->getID();
        }
        return result;
    }
}


bool
GNEDemandElementPlan::checkDrawPersonPlan() const {
    const auto viewNet = myPlanElement->getNet()->getViewNet();
    const auto& inspectedElements = viewNet->getInspectedElements();
    // check conditions
    if (viewNet->getEditModes().isCurrentSupermodeNetwork() &&
            viewNet->getNetworkViewOptions().showDemandElements() &&
            viewNet->getDemandViewOptions().showAllPersonPlans()) {
        // show all person plans in network mode
        return true;
    } else if (viewNet->getEditModes().isCurrentSupermodeDemand() &&
               viewNet->getDemandViewOptions().showAllPersonPlans()) {
        // show all person plans
        return true;
    } else if (viewNet->getEditModes().isCurrentSupermodeDemand() && myPlanElement->isAttributeCarrierSelected()) {
        // show selected
        return true;
    } else if (inspectedElements.isACInspected(myPlanElement->getParentDemandElements().front())) {
        // person parent is inspected
        return true;
    } else if (viewNet->getDemandViewOptions().getLockedPerson() == myPlanElement->getParentDemandElements().front()) {
        // person parent is locked
        return true;
    } else {
        // check if parent
        if (inspectedElements.getFirstAC() && inspectedElements.getFirstAC()->getTagProperty()->isPlanPerson() &&
                (inspectedElements.getFirstAC()->getAttribute(GNE_ATTR_PARENT) == myPlanElement->getAttribute(GNE_ATTR_PARENT))) {
            // common person parent
            return true;
        } else {
            // all conditions are false
            return false;
        }
    }
}


bool
GNEDemandElementPlan::checkDrawContainerPlan() const {
    const auto viewNet = myPlanElement->getNet()->getViewNet();
    const auto& inspectedElements = viewNet->getInspectedElements();
    // check conditions
    if (viewNet->getEditModes().isCurrentSupermodeNetwork() &&
            viewNet->getNetworkViewOptions().showDemandElements() &&
            viewNet->getDemandViewOptions().showAllContainerPlans()) {
        // show all container plans in network mode
        return true;
    } else if (viewNet->getEditModes().isCurrentSupermodeDemand() &&
               viewNet->getDemandViewOptions().showAllContainerPlans()) {
        // show all container plans
        return true;
    } else if (viewNet->getEditModes().isCurrentSupermodeDemand() && myPlanElement->isAttributeCarrierSelected()) {
        // show selected
        return true;
    } else if (inspectedElements.isACInspected(myPlanElement->getParentDemandElements().front())) {
        // container parent is inspected
        return true;
    } else if (viewNet->getDemandViewOptions().getLockedContainer() == myPlanElement->getParentDemandElements().front()) {
        // container parent is locked
        return true;
    } else {
        // check if parent is inspected
        if (inspectedElements.getFirstAC() && inspectedElements.getFirstAC()->getTagProperty()->isPlanContainer() &&
                (inspectedElements.getFirstAC()->getAttribute(GNE_ATTR_PARENT) == myPlanElement->getAttribute(GNE_ATTR_PARENT))) {
            // common container parent
            return true;
        } else {
            // all conditions are false
            return false;
        }
    }
}


void
GNEDemandElementPlan::drawPlanGL(const bool drawPlan, const GUIVisualizationSettings& s, const RGBColor& planColor, const RGBColor& planSelectedColor) const {
    const auto viewNet = myPlanElement->getNet()->getViewNet();
    const auto& inspectedElements = viewNet->getInspectedElements();
    // get plan parent
    const GNEDemandElement* planParent = myPlanElement->getParentDemandElements().front();
    // get tag property
    const auto tagProperty = myPlanElement->getTagProperty();
    // get plan geometry
    auto& planGeometry = myPlanElement->myDemandElementGeometry;
    // draw relations between TAZs
    if (drawPlan && (planGeometry.getShape().size() > 0)) {
        // draw boundary
        if (s.drawBoundaries) {
            GLHelper::drawBoundary(s, getPlanCenteringBoundary());
        }
        // get detail level
        const auto d = s.getDetailLevel(1);
        // check if draw with double width
        const bool drawHalfWidth = ((inspectedElements.getFirstAC() != myPlanElement) && (inspectedElements.getFirstAC() != planParent) && !gViewObjectsHandler.isObjectSelected(myPlanElement));
        // calculate path width
        double pathWidth = s.widthSettings.walkWidth;
        if (tagProperty->isPlanRide()) {
            pathWidth = s.widthSettings.rideWidth;
        } else if (tagProperty->isPlanPersonTrip()) {
            pathWidth = s.widthSettings.personTripWidth;
        }
        // draw geometry only if we'rent in drawForObjectUnderCursor mode
        if ((tagProperty->isPlanPerson() && s.checkDrawPerson(d, myPlanElement->isAttributeCarrierSelected())) ||
                (tagProperty->isPlanContainer() && s.checkDrawContainer(d, myPlanElement->isAttributeCarrierSelected()))) {
            // push matrix
            GLHelper::pushMatrix();
            // translate to front
            myPlanElement->drawInLayer(GLO_TAZ + 1);
            // set color
            GLHelper::setColor(myPlanElement->drawUsingSelectColor() ? planSelectedColor : planColor);
            // draw line
            GUIGeometry::drawGeometry(d, planGeometry, pathWidth * (drawHalfWidth ? 1 : 2));
            if (drawHalfWidth) {
                GLHelper::drawTriangleAtEnd(planGeometry.getShape().front(), planGeometry.getShape().back(), 0.5, 0.5, 0.5);
            } else {
                GLHelper::drawTriangleAtEnd(planGeometry.getShape().front(), planGeometry.getShape().back(), 1, 1, 1);
            }
            // pop matrix
            GLHelper::popMatrix();
            // draw dotted contour
            myPlanContour.drawDottedContours(s, d, myPlanElement, s.dottedContourSettings.segmentWidth, true);
        }
        // calculate contour and draw dotted geometry
        myPlanContour.calculateContourExtrudedShape(s, d, myPlanElement, planGeometry.getShape(), myPlanElement->getType(), pathWidth * 2,
                1, true, true, 0, nullptr, nullptr);
        // calculate contour for end
        myPlanContourEnd.calculateContourCircleShape(s, d, myPlanElement, planGeometry.getShape().back(), 1, myPlanElement->getType(), 1, nullptr);
    }
    // check if draw plan parent
    if (planParent->getPreviousChildDemandElement(myPlanElement) == nullptr) {
        planParent->drawGL(s);
    }
}


void
GNEDemandElementPlan::drawPlanLanePartial(const bool drawPlan, const GUIVisualizationSettings& s, const GNESegment* segment,
        const double offsetFront, const double planWidth, const RGBColor& planColor, const RGBColor& planSelectedColor) const {
    const auto viewNet = myPlanElement->getNet()->getViewNet();
    const auto& inspectedElements = viewNet->getInspectedElements();
    // get tag property
    const auto tagProperty = myPlanElement->getTagProperty();
    // get plan parent
    const GNEDemandElement* planParent = myPlanElement->getParentDemandElements().front();
    // check if draw plan element can be drawn
    if (drawPlan && segment->getLane() && myPlanElement->getNet()->getDemandPathManager()->getPathDraw()->checkDrawPathGeometry(s, segment->getLane(), tagProperty->getTag(), true)) {
        // draw boundary
        if (tagProperty->isPlacedInRTree() && s.drawBoundaries) {
            GLHelper::drawBoundary(s, getPlanCenteringBoundary());
        }
        // get detail level
        const auto d = s.getDetailLevel(1);
        // declare path geometry
        GUIGeometry planGeometry;
        // update pathGeometry depending of first and last segment
        if (segment->isFirstSegment() && segment->isLastSegment()) {
            if (tagProperty->planFromTAZ()) {
                planGeometry.updateGeometry(segment->getLane()->getLaneGeometry().getShape(),
                                            getPlanAttributeDouble(GNE_ATTR_PLAN_GEOMETRY_STARTPOS),
                                            Position::INVALID,
                                            getPlanAttributeDouble(GNE_ATTR_PLAN_GEOMETRY_ENDPOS),
                                            getPlanAttributePosition(GNE_ATTR_PLAN_GEOMETRY_ENDPOS));
            } else if (tagProperty->planToTAZ()) {
                planGeometry.updateGeometry(segment->getLane()->getLaneGeometry().getShape(),
                                            getPlanAttributeDouble(GNE_ATTR_PLAN_GEOMETRY_STARTPOS),
                                            getPlanAttributePosition(GNE_ATTR_PLAN_GEOMETRY_STARTPOS),
                                            getPlanAttributeDouble(GNE_ATTR_PLAN_GEOMETRY_ENDPOS),
                                            Position::INVALID);
            } else {
                planGeometry.updateGeometry(segment->getLane()->getLaneGeometry().getShape(),
                                            getPlanAttributeDouble(GNE_ATTR_PLAN_GEOMETRY_STARTPOS),
                                            getPlanAttributePosition(GNE_ATTR_PLAN_GEOMETRY_STARTPOS),
                                            getPlanAttributeDouble(GNE_ATTR_PLAN_GEOMETRY_ENDPOS),
                                            getPlanAttributePosition(GNE_ATTR_PLAN_GEOMETRY_ENDPOS));
            }
        } else if (segment->isFirstSegment()) {
            planGeometry.updateGeometry(segment->getLane()->getLaneGeometry().getShape(),
                                        getPlanAttributeDouble(GNE_ATTR_PLAN_GEOMETRY_STARTPOS),
                                        getPlanAttributePosition(GNE_ATTR_PLAN_GEOMETRY_STARTPOS),
                                        -1,
                                        Position::INVALID);
        } else if (segment->isLastSegment()) {
            planGeometry.updateGeometry(segment->getLane()->getLaneGeometry().getShape(),
                                        -1,
                                        Position::INVALID,
                                        getPlanAttributeDouble(GNE_ATTR_PLAN_GEOMETRY_ENDPOS),
                                        getPlanAttributePosition(GNE_ATTR_PLAN_GEOMETRY_ENDPOS));
        } else {
            planGeometry = segment->getLane()->getLaneGeometry();
        }
        // calculate path width double
        const double drawingWidth = s.addSize.getExaggeration(s, segment->getLane()) * planWidth * 2;
        // check if draw with double width
        const bool drawHalfWidth = ((inspectedElements.getFirstAC() != myPlanElement) && (inspectedElements.getFirstAC() != planParent) && !gViewObjectsHandler.isObjectSelected(myPlanElement));
        // get end pos radius
        const double endPosRadius = getEndPosRadius(s, segment, drawHalfWidth);
        // draw geometry only if we'rent in drawForObjectUnderCursor mode
        if ((tagProperty->isPlanPerson() && s.checkDrawPerson(d, myPlanElement->isAttributeCarrierSelected())) ||
                (tagProperty->isPlanContainer() && s.checkDrawContainer(d, myPlanElement->isAttributeCarrierSelected()))) {
            // Add a draw matrix
            GLHelper::pushMatrix();
            // Start with the drawing of the area traslating matrix to origin
            myPlanElement->drawInLayer(myPlanElement->getType(), offsetFront);
            // Set color
            GLHelper::setColor(myPlanElement->drawUsingSelectColor() ? planSelectedColor : planColor);
            // draw geometry depending of drawWithDoubleWidth
            GUIGeometry::drawGeometry(d, planGeometry, drawingWidth * (drawHalfWidth ? 0.5 : 1));
            // draw red arrows
            drawFromArrow(s, segment->getLane(), segment);
            drawToArrow(s, segment->getLane(), segment);
            // Pop last matrix
            GLHelper::popMatrix();
            // Draw name if isn't being drawn for selecting
            myPlanElement->drawName(myPlanElement->getCenteringBoundary().getCenter(), s.scale, s.addName);
            // draw dotted contour
            segment->getContour()->drawDottedContours(s, d, myPlanElement, s.dottedContourSettings.segmentWidth, true);
            // draw TAZ Center dotted contour
            myPlanContourEnd.drawDottedContours(s, d, myPlanElement, s.dottedContourSettings.segmentWidth, true);
        }
        // declare trim geometry to draw
        const auto& shape = (segment->isFirstSegment() || segment->isLastSegment()) ? planGeometry.getShape() : segment->getLane()->getLaneShape();
        // calculate contour and draw dotted geometry (always with double width)
        if (segment->isFirstSegment()) {
            segment->getContour()->calculateContourExtrudedShape(s, d, myPlanElement, shape, myPlanElement->getType(), drawingWidth, 1, true, false,
                    0, segment, segment->getLane()->getParentEdge());
        } else if (segment->isLastSegment()) {
            segment->getContour()->calculateContourExtrudedShape(s, d, myPlanElement, shape, myPlanElement->getType(), drawingWidth, 1, false, false,
                    0, segment, segment->getLane()->getParentEdge());
            // calculate contour for end
            myPlanContourEnd.calculateContourCircleShape(s, d, myPlanElement, getPlanAttributePosition(GNE_ATTR_PLAN_GEOMETRY_ENDPOS), 2 * endPosRadius,
                    myPlanElement->getType(), 1, segment->getLane());
        } else {
            segment->getContour()->calculateContourExtrudedShape(s, d, myPlanElement, shape, myPlanElement->getType(), drawingWidth, 1, false, false, 0,
                    segment, segment->getLane()->getParentEdge());
        }
        // check if add this path element to redraw buffer
        if (!gViewObjectsHandler.isPathElementMarkForRedraw(myPlanElement) && segment->getContour()->checkDrawPathContour(s, d, myPlanElement)) {
            gViewObjectsHandler.addToRedrawPathElements(myPlanElement);
        }
    }
    // check if draw plan parent
    if (planParent->getPreviousChildDemandElement(myPlanElement) == nullptr) {
        planParent->drawGL(s);
    }
}


void
GNEDemandElementPlan::drawPlanJunctionPartial(const bool drawPlan, const GUIVisualizationSettings& s, const GNESegment* segment,
        const double offsetFront, const double planWidth, const RGBColor& planColor, const RGBColor& planSelectedColor) const {
    const auto viewNet = myPlanElement->getNet()->getViewNet();
    const auto& inspectedElements = viewNet->getInspectedElements();
    // get tag property
    const auto tagProperty = myPlanElement->getTagProperty();
    // get plan parent
    const GNEDemandElement* planParent = myPlanElement->getParentDemandElements().front();
    // check if draw plan elements can be drawn
    if (drawPlan && myPlanElement->getNet()->getDemandPathManager()->getPathDraw()->checkDrawPathGeometry(s, segment, tagProperty->getTag(), false)) {
        // draw boundary
        if (tagProperty->isPlacedInRTree() && s.drawBoundaries) {
            GLHelper::drawBoundary(s, getPlanCenteringBoundary());
        }
        // get detail level
        const auto d = s.getDetailLevel(1);
        // calculate path width double
        const double pathWidthDouble = s.addSize.getExaggeration(s, segment->getLane()) * planWidth * 2;
        // check if draw with double width
        const bool drawWithDoubleWidth = ((inspectedElements.getFirstAC() == myPlanElement) || (inspectedElements.getFirstAC() == planParent) || gViewObjectsHandler.isObjectSelected(myPlanElement));
        // draw geometry only if we'rent in drawForObjectUnderCursor mode
        if (!s.drawForViewObjectsHandler) {
            // push a draw matrix
            GLHelper::pushMatrix();
            // Start with the drawing of the area traslating matrix to origin
            myPlanElement->drawInLayer(myPlanElement->getType(), offsetFront);
            // Set plan color
            GLHelper::setColor(myPlanElement->drawUsingSelectColor() ? planSelectedColor : planColor);
            // check if draw lane2lane connection or a red line
            if (segment->getPreviousLane() && segment->getNextLane()) {
                if (segment->getPreviousLane()->getLane2laneConnections().exist(segment->getNextLane())) {
                    // obtain lane2lane geometry
                    const GUIGeometry& lane2laneGeometry = segment->getPreviousLane()->getLane2laneConnections().getLane2laneGeometry(segment->getNextLane());
                    // draw lane2lane
                    GUIGeometry::drawGeometry(d, lane2laneGeometry, pathWidthDouble * (drawWithDoubleWidth ? 1 : 0.5));
                } else {
                    // Set invalid plan color
                    GLHelper::setColor(RGBColor::RED);
                    // draw line between end of first shape and first position of second shape
                    GLHelper::drawBoxLines({segment->getPreviousLane()->getLaneShape().back(), segment->getNextLane()->getLaneShape().front()}, (0.5 * pathWidthDouble * (drawWithDoubleWidth ? 1 : 0.5)));
                }
            } else if (segment->getPreviousLane()) {
                // draw line between center of junction and last lane shape
                GLHelper::drawBoxLines({segment->getPreviousLane()->getLaneShape().back(), myPlanElement->getParentJunctions().back()->getPositionInView()}, pathWidthDouble * (drawWithDoubleWidth ? 1 : 0.5));
            } else if (segment->getNextLane()) {
                // draw line between center of junction and first lane shape
                GLHelper::drawBoxLines({myPlanElement->getParentJunctions().front()->getPositionInView(), segment->getNextLane()->getLaneShape().front()}, pathWidthDouble * (drawWithDoubleWidth ? 1 : 0.5));
            }
            // Pop last matrix
            GLHelper::popMatrix();
            // draw lock icon
            GNEViewNetHelper::LockIcon::drawLockIcon(d, myPlanElement, myPlanElement->getType(), myPlanElement->getPositionInView(), 0.5);
            // draw dotted contour
            segment->getContour()->drawDottedContours(s, d, myPlanElement, s.dottedContourSettings.segmentWidth, true);
        }
        // check if shape dotted contour has to be drawn
        if (segment->getPreviousLane() && segment->getNextLane()) {
            if (segment->getPreviousLane()->getLane2laneConnections().exist(segment->getNextLane())) {
                // get shape
                const auto& shape = segment->getPreviousLane()->getLane2laneConnections().getLane2laneGeometry(segment->getNextLane()).getShape();
                // calculate contour and draw dotted geometry (always with double width)
                segment->getContour()->calculateContourExtrudedShape(s, d, myPlanElement, shape, myPlanElement->getType(), pathWidthDouble, 1, false, false, 0, segment, segment->getJunction());
            }
        } else if (segment->getPreviousLane()) {
            segment->getContour()->calculateContourExtrudedShape(s, d, myPlanElement, {segment->getPreviousLane()->getLaneShape().back(), myPlanElement->getParentJunctions().back()->getPositionInView()},
                    myPlanElement->getType(), pathWidthDouble, 1, false, true, 0, segment, segment->getJunction());
        } else if (segment->getNextLane()) {
            segment->getContour()->calculateContourExtrudedShape(s, d, myPlanElement, {myPlanElement->getParentJunctions().front()->getPositionInView(), segment->getNextLane()->getLaneShape().front()},
                    myPlanElement->getType(), pathWidthDouble, 1, true, false, 0, segment, segment->getJunction());
        }
        // check if add this path element to redraw buffer
        if (!gViewObjectsHandler.isPathElementMarkForRedraw(myPlanElement) && segment->getContour()->checkDrawPathContour(s, d, myPlanElement)) {
            gViewObjectsHandler.addToRedrawPathElements(myPlanElement);
        }
    }
    // check if draw plan parent
    if (planParent->getPreviousChildDemandElement(myPlanElement) == nullptr) {
        planParent->drawGL(s);
    }
}


GNEDemandElement::Problem
GNEDemandElementPlan::isPlanPersonValid() const {
    // get previous plan
    const auto previousPlan = myPlanElement->getParentDemandElements().at(0)->getPreviousChildDemandElement(myPlanElement);
    if (previousPlan) {
        // get previous lane
        const auto previousLastLane = previousPlan->getLastPathLane();
        // get first lane
        const auto firstLane = myPlanElement->getFirstPathLane();
        // compare edges
        if (previousLastLane && firstLane && (previousLastLane->getParentEdge() != firstLane->getParentEdge())) {
            return GNEDemandElement::Problem::DISCONNECTED_PLAN;
        }
        // in the future, check more elements
    }
    // get next child
    const auto nextPlan = myPlanElement->getParentDemandElements().at(0)->getNextChildDemandElement(myPlanElement);
    if (nextPlan) {
        // get previous lane
        const auto nextFirstLane = nextPlan->getFirstPathLane();
        // get first lane
        const auto lastLane = myPlanElement->getLastPathLane();
        // compare edges
        if (nextFirstLane && lastLane && (nextFirstLane->getParentEdge() != lastLane->getParentEdge())) {
            return GNEDemandElement::Problem::DISCONNECTED_PLAN;
        }
        // in the future, check more elements
    }
    // all ok, then return true
    return GNEDemandElement::Problem::OK;
}


std::string
GNEDemandElementPlan::getPersonPlanProblem() const {
    // get previous plan
    const auto previousPlan = myPlanElement->getParentDemandElements().at(0)->getPreviousChildDemandElement(myPlanElement);
    if (previousPlan) {
        // get previous lane
        const auto previousLastLane = previousPlan->getLastPathLane();
        // get first lane
        const auto firstLane = myPlanElement->getLastPathLane();
        // compare edges
        if (previousLastLane && firstLane && (previousLastLane->getParentEdge() != firstLane->getParentEdge())) {
            return TLF("Edge '%' is not consecutive with edge '%'", previousLastLane->getParentEdge()->getID(), firstLane->getParentEdge()->getID());
        }
        // in the future, check more elements
    }
    // get next child
    const auto nextPlan = myPlanElement->getParentDemandElements().at(0)->getNextChildDemandElement(myPlanElement);
    if (nextPlan) {
        // get previous lane
        const auto nextFirstLane = nextPlan->getFirstPathLane();
        // get first lane
        const auto lastLane = myPlanElement->getLastPathLane();
        // compare edges
        if (nextFirstLane && lastLane && (nextFirstLane->getParentEdge() != lastLane->getParentEdge())) {
            return TLF("Edge '%' is not consecutive with edge '%'", nextFirstLane->getParentEdge()->getID(), lastLane->getParentEdge()->getID());
        }
        // in the future, check more elements
    }
    // undefined problem
    return "undefined problem";
}


double
GNEDemandElementPlan::getEndPosRadius(const GUIVisualizationSettings& s, const GNESegment* segment, const bool drawHalfWidth) const {
    // check if myPlanElement is the last segment
    if (segment->isLastSegment()) {
        // calculate circle width
        const double circleRadius = (drawHalfWidth ? myArrivalPositionDiameter * 0.5 : myArrivalPositionDiameter);
        return circleRadius * MIN2((double)0.5, s.laneWidthExaggeration);
    } else {
        return -1;
    }
}


void
GNEDemandElementPlan::drawFromArrow(const GUIVisualizationSettings& s, const GNELane* lane, const GNESegment* segment) const {
    // draw ifcurrent amd next segment is placed over lanes
    if (segment->getNextLane()) {
        // get firstPosition (last position of current lane shape)
        const Position from = lane->getLaneShape().back();
        // get lastPosition (first position of next lane shape)
        const Position to = segment->getNextLane()->getLaneShape().front();
        // push draw matrix
        GLHelper::pushMatrix();
        // move front
        glTranslated(0, 0, 4);
        // draw child line
        GUIGeometry::drawChildLine(s, from, to, RGBColor::RED, myPlanElement->isAttributeCarrierSelected(), .05);
        // pop draw matrix
        GLHelper::popMatrix();
    }
}


void
GNEDemandElementPlan::drawToArrow(const GUIVisualizationSettings& s, const GNELane* lane, const GNESegment* segment) const {
    // draw the line if previos segment and current segment is placed over lanes
    if (segment->getPreviousLane()) {
        // get firstPosition (last position of current lane shape)
        const Position from = lane->getLaneShape().front();
        // get lastPosition (first position of next lane shape)
        const Position to = segment->getPreviousLane()->getLaneShape().back();
        // push draw matrix
        GLHelper::pushMatrix();
        // move front
        glTranslated(0, 0, 4);
        // draw child line
        GUIGeometry::drawChildLine(s, from, to, RGBColor::RED, myPlanElement->isAttributeCarrierSelected(), .05);
        // pop draw matrix
        GLHelper::popMatrix();
    }
}


void
GNEDemandElementPlan::drawEndPosition(const GUIVisualizationSettings& /* s */, const GUIVisualizationSettings::Detail d, const double endPosRadius) const {
    // check if myPlanElement is the last segment
    if (endPosRadius > 0) {
        const Position geometryEndPos = getPlanAttributePosition(GNE_ATTR_PLAN_GEOMETRY_ENDPOS);
        // push draw matrix
        GLHelper::pushMatrix();
        // translate to pos and move to
        glTranslated(geometryEndPos.x(), geometryEndPos.y(), 4);
        // resolution of drawn circle depending of the zoom (To improve smothness)
        GLHelper::drawFilledCircleDetailled(d, endPosRadius);
        // pop draw matrix
        GLHelper::popMatrix();
    }
}


bool
GNEDemandElementPlan::replacePlanParent(const std::string& newParentID) {
    std::vector<SumoXMLTag> tags;
    if (myPlanElement->myTagProperty->isPlanPerson()) {
        tags.push_back(SUMO_TAG_PERSON);
        tags.push_back(SUMO_TAG_PERSONFLOW);
    } else {
        tags.push_back(SUMO_TAG_CONTAINER);
        tags.push_back(SUMO_TAG_CONTAINERFLOW);
    }
    // search new parent and set it
    for (const auto& tag : tags) {
        if (myPlanElement->getNet()->getAttributeCarriers()->retrieveDemandElement(tag, newParentID, false) != nullptr) {
            myPlanElement->replaceDemandElementParent(tag, newParentID, 0);
            return true;
        }
    }
    throw ProcessError("Invalid new parent ID");
}

/****************************************************************************/
