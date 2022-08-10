/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2001-2022 German Aerospace Center (DLR) and others.
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
/// @file    GNETLSEditorFrame.cpp
/// @author  Jakob Erdmann
/// @date    May 2011
///
// The Widget for modifying traffic lights
/****************************************************************************/
#include <config.h>

#include <netbuild/NBLoadedSUMOTLDef.h>
#include <netedit/GNENet.h>
#include <netedit/GNEUndoList.h>
#include <netedit/GNEViewNet.h>
#include <netedit/changes/GNEChange_TLS.h>
#include <netedit/dialogs/GNESingleParametersDialog.h>
#include <netedit/elements/network/GNEInternalLane.h>
#include <netedit/frames/GNETLSTable.h>
#include <netimport/NIXMLTrafficLightsHandler.h>
#include <utils/gui/div/GUIDesigns.h>
#include <utils/gui/windows/GUIAppEnum.h>
#include <utils/options/OptionsCont.h>
#include <utils/xml/XMLSubSys.h>

#include "GNETLSEditorFrame.h"

// ===========================================================================
// FOX callback mapping
// ===========================================================================

FXDEFMAP(GNETLSEditorFrame::TLSDefinition) TLSDefinitionMap[] = {
    FXMAPFUNC(SEL_COMMAND,    MID_GNE_TLSFRAME_DEFINITION_CREATE,   GNETLSEditorFrame::TLSDefinition::onCmdCreate),
    FXMAPFUNC(SEL_UPDATE,     MID_GNE_TLSFRAME_DEFINITION_CREATE,   GNETLSEditorFrame::TLSDefinition::onUpdCreateButton),
    FXMAPFUNC(SEL_COMMAND,    MID_GNE_TLSFRAME_DEFINITION_DELETE,   GNETLSEditorFrame::TLSDefinition::onCmdDelete),
    FXMAPFUNC(SEL_UPDATE,     MID_GNE_TLSFRAME_DEFINITION_DELETE,   GNETLSEditorFrame::TLSDefinition::onUpdTLSModified),
    FXMAPFUNC(SEL_COMMAND,    MID_GNE_TLSFRAME_DEFINITION_RESET,    GNETLSEditorFrame::TLSDefinition::onCmdReset),
    FXMAPFUNC(SEL_UPDATE,     MID_GNE_TLSFRAME_DEFINITION_RESET,    GNETLSEditorFrame::TLSDefinition::onUpdTLSModified),
};

FXDEFMAP(GNETLSEditorFrame::TLSAttributes) TLSAttributesMap[] = {
    FXMAPFUNC(SEL_COMMAND,    MID_GNE_TLSFRAME_ATTRIBUTES_SWITCH,           GNETLSEditorFrame::TLSAttributes::onCmdDefSwitchTLSProgram),
    FXMAPFUNC(SEL_UPDATE,     MID_GNE_TLSFRAME_ATTRIBUTES_SWITCH,           GNETLSEditorFrame::TLSAttributes::onUpdTLSModified),
    FXMAPFUNC(SEL_COMMAND,    MID_GNE_TLSFRAME_ATTRIBUTES_OFFSET,           GNETLSEditorFrame::TLSAttributes::onCmdSetOffset),
    FXMAPFUNC(SEL_UPDATE,     MID_GNE_TLSFRAME_ATTRIBUTES_OFFSET,           GNETLSEditorFrame::TLSAttributes::onUpdNeedsTLSDef),
    FXMAPFUNC(SEL_COMMAND,    MID_GNE_TLSFRAME_ATTRIBUTES_PARAMETERS,       GNETLSEditorFrame::TLSAttributes::onCmdSetParameters),
    FXMAPFUNC(SEL_UPDATE,     MID_GNE_TLSFRAME_ATTRIBUTES_PARAMETERS,       GNETLSEditorFrame::TLSAttributes::onUpdNeedsTLSDef),
    FXMAPFUNC(SEL_COMMAND,    MID_GNE_TLSFRAME_ATTRIBUTES_RENAME,           GNETLSEditorFrame::TLSAttributes::onCmdDefRename),
    FXMAPFUNC(SEL_COMMAND,    MID_GNE_TLSFRAME_ATTRIBUTES_SUBRENAME,        GNETLSEditorFrame::TLSAttributes::onCmdDefSubRename),
    FXMAPFUNC(SEL_COMMAND,    MID_GNE_TLSFRAME_ATTRIBUTES_ADDOFF,           GNETLSEditorFrame::TLSAttributes::onCmdDefAddOff),
    FXMAPFUNC(SEL_COMMAND,    MID_GNE_TLSFRAME_ATTRIBUTES_GUESSPROGRAM,     GNETLSEditorFrame::TLSAttributes::onCmdGuess),
    FXMAPFUNC(SEL_COMMAND,    MID_GNE_TLSFRAME_ATTRIBUTES_PARAMETERSDIALOG, GNETLSEditorFrame::TLSAttributes::onCmdEditParameters),
};

FXDEFMAP(GNETLSEditorFrame::TLSModifications) TLSModificationsMap[] = {
    FXMAPFUNC(SEL_COMMAND,    MID_GNE_TLSFRAME_MODIFICATIONS_SAVE,      GNETLSEditorFrame::TLSModifications::onCmdSaveChanges),
    FXMAPFUNC(SEL_UPDATE,     MID_GNE_TLSFRAME_MODIFICATIONS_SAVE,      GNETLSEditorFrame::TLSModifications::onUpdModified),
    FXMAPFUNC(SEL_COMMAND,    MID_GNE_TLSFRAME_MODIFICATIONS_DISCARD,   GNETLSEditorFrame::TLSModifications::onCmdDiscardChanges),
    FXMAPFUNC(SEL_UPDATE,     MID_GNE_TLSFRAME_MODIFICATIONS_DISCARD,   GNETLSEditorFrame::TLSModifications::onUpdModified),
};

FXDEFMAP(GNETLSEditorFrame::TLSPhases) TLSPhasesMap[] = {
    FXMAPFUNC(SEL_COMMAND,    MID_GNE_TLSFRAME_PHASES_CLEANUP,          GNETLSEditorFrame::TLSPhases::onCmdCleanStates),
    FXMAPFUNC(SEL_UPDATE,     MID_GNE_TLSFRAME_PHASES_CLEANUP,          GNETLSEditorFrame::TLSPhases::onUpdNeedsSingleDef),
    FXMAPFUNC(SEL_COMMAND,    MID_GNE_TLSFRAME_PHASES_ADDUNUSED,        GNETLSEditorFrame::TLSPhases::onCmdAddUnusedStates),
    FXMAPFUNC(SEL_UPDATE,     MID_GNE_TLSFRAME_PHASES_ADDUNUSED,        GNETLSEditorFrame::TLSPhases::onUpdNeedsDef),
    FXMAPFUNC(SEL_COMMAND,    MID_GNE_TLSFRAME_PHASES_GROUPSTATES,      GNETLSEditorFrame::TLSPhases::onCmdGroupStates),
    FXMAPFUNC(SEL_UPDATE,     MID_GNE_TLSFRAME_PHASES_GROUPSTATES,      GNETLSEditorFrame::TLSPhases::onUpdNeedsSingleDef),
    FXMAPFUNC(SEL_COMMAND,    MID_GNE_TLSFRAME_PHASES_UNGROUPSTATES,    GNETLSEditorFrame::TLSPhases::onCmdUngroupStates),
    FXMAPFUNC(SEL_UPDATE,     MID_GNE_TLSFRAME_PHASES_UNGROUPSTATES,    GNETLSEditorFrame::TLSPhases::onUpdUngroupStates),
};

FXDEFMAP(GNETLSEditorFrame::TLSFile) TLSFileMap[] = {
    FXMAPFUNC(SEL_COMMAND,    MID_GNE_TLSFRAME_FILE_LOADPROGRAM,    GNETLSEditorFrame::TLSFile::onCmdLoadTLSProgram),
    FXMAPFUNC(SEL_UPDATE,     MID_GNE_TLSFRAME_FILE_LOADPROGRAM,    GNETLSEditorFrame::TLSFile::onUpdNeedsDef),
    FXMAPFUNC(SEL_COMMAND,    MID_GNE_TLSFRAME_FILE_SAVEPROGRAM,    GNETLSEditorFrame::TLSFile::onCmdSaveTLSProgram),
    FXMAPFUNC(SEL_UPDATE,     MID_GNE_TLSFRAME_FILE_SAVEPROGRAM,    GNETLSEditorFrame::TLSFile::onUpdNeedsDef),
};

// Object implementation
FXIMPLEMENT(GNETLSEditorFrame::TLSDefinition,       MFXGroupBoxModule,  TLSDefinitionMap,       ARRAYNUMBER(TLSDefinitionMap))
FXIMPLEMENT(GNETLSEditorFrame::TLSAttributes,       MFXGroupBoxModule,  TLSAttributesMap,       ARRAYNUMBER(TLSAttributesMap))
FXIMPLEMENT(GNETLSEditorFrame::TLSModifications,    MFXGroupBoxModule,  TLSModificationsMap,    ARRAYNUMBER(TLSModificationsMap))
FXIMPLEMENT(GNETLSEditorFrame::TLSPhases,           MFXGroupBoxModule,  TLSPhasesMap,           ARRAYNUMBER(TLSPhasesMap))
FXIMPLEMENT(GNETLSEditorFrame::TLSFile,             MFXGroupBoxModule,  TLSFileMap,             ARRAYNUMBER(TLSFileMap))


// ===========================================================================
// method definitions
// ===========================================================================

GNETLSEditorFrame::GNETLSEditorFrame(FXHorizontalFrame* horizontalFrameParent, GNEViewNet* viewNet):
    GNEFrame(horizontalFrameParent, viewNet, "Edit Traffic Light"),
    myEditedDef(nullptr) {

    // Create Overlapped Inspection modul
    myOverlappedInspection = new GNEOverlappedInspection(this, SUMO_TAG_JUNCTION);

    // create TLSJunction modul
    myTLSJunction = new GNETLSEditorFrame::TLSJunction(this);

    // create TLSDefinition modul
    myTLSDefinition = new GNETLSEditorFrame::TLSDefinition(this);

    // create TLSAttributes modul
    myTLSAttributes = new GNETLSEditorFrame::TLSAttributes(this);

    // create TLSModifications modul
    myTLSModifications = new GNETLSEditorFrame::TLSModifications(this);

    // create TLSPhases modul
    myTLSPhases = new GNETLSEditorFrame::TLSPhases(this);

    // create TLSFile modul
    myTLSFile = new GNETLSEditorFrame::TLSFile(this);

    // "Add 'off' program"
    /*
    new FXButton(myContentFrame, "Add \"Off\"-Program\t\tAdds a program for switching off this traffic light",
            0, this, MID_GNE_TLSFRAME_ATTRIBUTES_ADDOFF, GUIDesignButton);
    */
}


GNETLSEditorFrame::~GNETLSEditorFrame() {
    cleanup();
}


void
GNETLSEditorFrame::show() {
    // hide myOverlappedInspection
    myOverlappedInspection->hideOverlappedInspection();
    // show
    GNEFrame::show();
}


void
GNETLSEditorFrame::editTLS(const Position& clickedPosition, const GNEViewNetHelper::ObjectsUnderCursor& objectsUnderCursor) {
    // first check if in objectsUnderCursor there is a junction
    if (objectsUnderCursor.getJunctionFront()) {
        // show objects under cursor
        myOverlappedInspection->showOverlappedInspection(objectsUnderCursor, clickedPosition);
        // hide if we inspect only one junction
        if (myOverlappedInspection->getNumberOfOverlappedACs() == 1) {
            myOverlappedInspection->hideOverlappedInspection();
        }
        // set junction
        editJunction(objectsUnderCursor.getJunctionFront());
    } else {
        myViewNet->setStatusBarText("Click over a junction to edit a TLS");
    }
}


bool
GNETLSEditorFrame::isTLSSaved() {
    if (myTLSModifications->checkHaveModifications()) {
        // write warning if netedit is running in testing mode
        WRITE_DEBUG("Opening question FXMessageBox 'save TLS'");
        // open question box
        FXuint answer = FXMessageBox::question(this, MBOX_YES_NO_CANCEL,
                                               "Save TLS Changes", "%s",
                                               "There is unsaved changes in current edited traffic light.\nDo you want to save it before changing mode?");
        if (answer == MBOX_CLICKED_YES) { //1:yes, 2:no, 4:esc/cancel
            // write warning if netedit is running in testing mode
            WRITE_DEBUG("Closed FXMessageBox 'save TLS' with 'YES'");
            // save modifications
            myTLSModifications->onCmdSaveChanges(nullptr, 0, nullptr);
            return true;
        } else if (answer == MBOX_CLICKED_NO) {
            // write warning if netedit is running in testing mode
            WRITE_DEBUG("Closed FXMessageBox 'save TLS' with 'No'");
            // cancel modifications
            myTLSModifications->onCmdSaveChanges(nullptr, 0, nullptr);
            return true;
        } else {
            // write warning if netedit is running in testing mode
            WRITE_DEBUG("Closed FXMessageBox 'save TLS' with 'Cancel'");
            // abort changing mode
            return false;
        }
    } else {
        return true;
    }
}


bool
GNETLSEditorFrame::parseTLSPrograms(const std::string& file) {
    NBTrafficLightLogicCont& tllCont = myViewNet->getNet()->getTLLogicCont();
    NBTrafficLightLogicCont tmpTLLCont;
    NIXMLTrafficLightsHandler tllHandler(tmpTLLCont, myViewNet->getNet()->getEdgeCont());
    // existing definitions must be available to update their programs
    std::set<NBTrafficLightDefinition*> origDefs;
    for (NBTrafficLightDefinition* def : tllCont.getDefinitions()) {
        // make a copy of every program
        NBTrafficLightLogic* logic = tllCont.getLogic(def->getID(), def->getProgramID());
        if (logic != nullptr) {
            NBTrafficLightDefinition* copy = new NBLoadedSUMOTLDef(*def, *logic);
            std::vector<NBNode*> nodes = def->getNodes();
            for (auto it_node : nodes) {
                GNEJunction* junction = myViewNet->getNet()->getAttributeCarriers()->retrieveJunction(it_node->getID());
                myViewNet->getUndoList()->add(new GNEChange_TLS(junction, def, false, false), true);
                myViewNet->getUndoList()->add(new GNEChange_TLS(junction, copy, true), true);
            }
            tmpTLLCont.insert(copy);
            origDefs.insert(copy);
        } else {
            WRITE_WARNING("tlLogic '" + def->getID() + "', program '" + def->getProgramID() + "' could not be built");
        }
    }
    //std::cout << " initialized tmpCont with " << origDefs.size() << " defs\n";
    XMLSubSys::runParser(tllHandler, file);

    std::vector<NBLoadedSUMOTLDef*> loadedTLS;
    for (NBTrafficLightDefinition* def : tmpTLLCont.getDefinitions()) {
        NBLoadedSUMOTLDef* sdef = dynamic_cast<NBLoadedSUMOTLDef*>(def);
        if (sdef != nullptr) {
            loadedTLS.push_back(sdef);
        }
    }
    myViewNet->setStatusBarText("Loaded " + toString(loadedTLS.size()) + " programs");
    for (auto def : loadedTLS) {
        if (origDefs.count(def) != 0) {
            // already add to undolist before
            //std::cout << " skip " << def->getDescription() << "\n";
            continue;
        }
        std::vector<NBNode*> nodes = def->getNodes();
        //std::cout << " add " << def->getDescription() << " for nodes=" << toString(nodes) << "\n";
        for (auto it_node : nodes) {
            GNEJunction* junction = myViewNet->getNet()->getAttributeCarriers()->retrieveJunction(it_node->getID());
            //myViewNet->getUndoList()->add(new GNEChange_TLS(junction, myTLSEditorParent->myEditedDef, false), true);
            myViewNet->getUndoList()->add(new GNEChange_TLS(junction, def, true), true);
        }
    }
    // clean up temporary container to avoid deletion of defs when it's destruct is called
    for (NBTrafficLightDefinition* def : tmpTLLCont.getDefinitions()) {
        tmpTLLCont.removeProgram(def->getID(), def->getProgramID(), false);
    }
    return true;
}


void
GNETLSEditorFrame::selectedOverlappedElement(GNEAttributeCarrier* AC) {
    editJunction(dynamic_cast<GNEJunction*>(AC));
}


GNETLSEditorFrame::TLSModifications* 
GNETLSEditorFrame::getTLSModifications() {
    return myTLSModifications;
}


void
GNETLSEditorFrame::cleanup() {
    if (myTLSJunction->getCurrentJunction()) {
        myTLSJunction->getCurrentJunction()->selectTLS(false);
        if (myTLSAttributes->getNumberOfTLSDefinitions() > 0) {
            for (NBNode* node : myTLSAttributes->getCurrentTLSDefinition()->getNodes()) {
                myViewNet->getNet()->getAttributeCarriers()->retrieveJunction(node->getID())->selectTLS(false);
            }
        }
    }
    // clean data structures
    myTLSJunction->setCurrentJunction(nullptr);
    myTLSModifications->setHaveModifications(false);
    delete myEditedDef;
    myEditedDef = nullptr;
    // clear internal lanes
    buildInternalLanes(nullptr);
    // clean up controls
    myTLSAttributes->clearTLSAttributes();
    // only clears when there are no definitions
    myTLSPhases->initPhaseTable();
    myTLSJunction->updateJunctionDescription();
}


void
GNETLSEditorFrame::buildInternalLanes(NBTrafficLightDefinition* tlDef) {
    // clean up previous internal lanes
    for (const auto& internalLanes : myInternalLanes) {
        for (const auto& internalLane : internalLanes.second) {
            // remove internal lane from grid
            myViewNet->getNet()->getGrid().removeAdditionalGLObject(internalLane);
            // delete internal lane
            delete internalLane;
        }
    }
    // clear container
    myInternalLanes.clear();
    // create new internal lanes
    if (tlDef != nullptr) {
        const int NUM_POINTS = 10;
        const NBNode* nbnCurrentJunction = myTLSJunction->getCurrentJunction()->getNBNode();
        // get innerID NWWriter_SUMO::writeInternalEdges
        const std::string innerID = ":" + nbnCurrentJunction->getID();
        const NBConnectionVector& links = tlDef->getControlledLinks();
        // iterate over links
        for (const auto& link : links) {
            int tlIndex = link.getTLIndex();
            PositionVector shape;
            try {
                const NBEdge::Connection& con = link.getFrom()->getConnection(link.getFromLane(), link.getTo(), link.getToLane());
                shape = con.shape;
                shape.append(con.viaShape);
            } catch (ProcessError&) {
                shape = link.getFrom()->getToNode()->computeInternalLaneShape(link.getFrom(), NBEdge::Connection(link.getFromLane(),
                        link.getTo(), link.getToLane()), NUM_POINTS);
            }
            if (shape.length() < 2) {
                // enlarge shape to ensure visibility
                shape.clear();
                const PositionVector laneShapeFrom = link.getFrom()->getLaneShape(link.getFromLane());
                const PositionVector laneShapeTo = link.getTo()->getLaneShape(link.getToLane());
                shape.push_back(laneShapeFrom.positionAtOffset(MAX2(0.0, laneShapeFrom.length() - 1)));
                shape.push_back(laneShapeTo.positionAtOffset(MIN2(1.0, laneShapeFrom.length())));
            }
            GNEInternalLane* internalLane = new GNEInternalLane(this, myTLSJunction->getCurrentJunction(), innerID + '_' + toString(tlIndex),  shape, tlIndex);
            // due GNEInternalLane aren't attribute carriers, we need to use the net grid
            myViewNet->getNet()->getGrid().addAdditionalGLObject(internalLane);
            myInternalLanes[tlIndex].push_back(internalLane);
        }
        // iterate over crossings
        for (const auto& nbn : tlDef->getNodes()) {
            for (const auto& crossing : nbn->getCrossings()) {
                if (crossing->tlLinkIndex2 > 0 && crossing->tlLinkIndex2 != crossing->tlLinkIndex) {
                    // draw both directions
                    PositionVector forward = crossing->shape;
                    forward.move2side(crossing->width / 4);
                    GNEInternalLane* internalLane = new GNEInternalLane(this, myTLSJunction->getCurrentJunction(), crossing->id, forward, crossing->tlLinkIndex);
                    // due GNEInternalLane aren't attribute carriers, we need to use the net grid
                    myViewNet->getNet()->getGrid().addAdditionalGLObject(internalLane);
                    myInternalLanes[crossing->tlLinkIndex].push_back(internalLane);
                    PositionVector backward = crossing->shape.reverse();
                    backward.move2side(crossing->width / 4);
                    GNEInternalLane* internalLaneReverse = new GNEInternalLane(this, myTLSJunction->getCurrentJunction(), crossing->id + "_r", backward, crossing->tlLinkIndex2);
                    // due GNEInternalLane aren't attribute carriers, we need to use the net grid
                    myViewNet->getNet()->getGrid().addAdditionalGLObject(internalLaneReverse);
                    myInternalLanes[crossing->tlLinkIndex2].push_back(internalLaneReverse);
                } else {
                    // draw only one lane for both directions
                    GNEInternalLane* internalLane = new GNEInternalLane(this, myTLSJunction->getCurrentJunction(), crossing->id, crossing->shape, crossing->tlLinkIndex);
                    // due GNEInternalLane aren't attribute carriers, we need to use the net grid
                    myViewNet->getNet()->getGrid().addAdditionalGLObject(internalLane);
                    myInternalLanes[crossing->tlLinkIndex].push_back(internalLane);
                }
            }
        }
    }
}


std::string
GNETLSEditorFrame::varDurString(SUMOTime dur) {
    return (dur == NBTrafficLightDefinition::UNSPECIFIED_DURATION)? "" : getSteps2Time(dur);
}


const NBTrafficLightLogic::PhaseDefinition&
GNETLSEditorFrame::getPhase(const int index) {
    if ((index >= 0) || (index < (int)myEditedDef->getLogic()->getPhases().size())) {
        return myEditedDef->getLogic()->getPhases().at(index);
    } else {
        throw ProcessError("Invalid phase index");
    }
}


void
GNETLSEditorFrame::handleChange(GNEInternalLane* lane) {
    myTLSModifications->setHaveModifications(true);
    // get current selected row
    const auto selectedRow = myTLSPhases->getPhaseTable()->getCurrentSelectedRow();
    if (myViewNet->changeAllPhases()) {
        for (int row = 0; row < (int)myEditedDef->getLogic()->getPhases().size(); row++) {
            myEditedDef->getLogic()->setPhaseState(row, lane->getTLIndex(), lane->getLinkState());
        }
    } else {
        myEditedDef->getLogic()->setPhaseState(myTLSPhases->getPhaseTable()->getCurrentSelectedRow(), lane->getTLIndex(), lane->getLinkState());
    }
    // init phaseTable
    myTLSPhases->initPhaseTable();
    // select row
    myTLSPhases->getPhaseTable()->selectRow(selectedRow);
    // focus table
    myTLSPhases->getPhaseTable()->setFocus();
}


void
GNETLSEditorFrame::handleMultiChange(GNELane* lane, FXObject* obj, FXSelector sel, void* eventData) {
    if (myEditedDef != nullptr) {
        myTLSModifications->setHaveModifications(true);
        const NBConnectionVector& links = myEditedDef->getControlledLinks();
        std::set<std::string> fromIDs;
        fromIDs.insert(lane->getMicrosimID());
        // if neither the lane nor its edge are selected, apply changes to the whole edge
        if (!lane->getParentEdge()->isAttributeCarrierSelected() && !lane->isAttributeCarrierSelected()) {
            for (auto it_lane : lane->getParentEdge()->getLanes()) {
                fromIDs.insert(it_lane->getMicrosimID());
            }
        } else {
            // if the edge is selected, apply changes to all lanes of all selected edges
            if (lane->getParentEdge()->isAttributeCarrierSelected()) {
                const auto selectedEdge = myViewNet->getNet()->getAttributeCarriers()->getSelectedEdges();
                for (const auto& edge : selectedEdge) {
                    for (auto it_lane : edge->getLanes()) {
                        fromIDs.insert(it_lane->getMicrosimID());
                    }
                }
            }
            // if the lane is selected, apply changes to all selected lanes
            if (lane->isAttributeCarrierSelected()) {
                const auto selectedLanes = myViewNet->getNet()->getAttributeCarriers()->getSelectedLanes();
                for (auto it_lane : selectedLanes) {
                    fromIDs.insert(it_lane->getMicrosimID());
                }
            }

        }
        // set new state for all connections from the chosen lane IDs
        for (auto it : links) {
            if (fromIDs.count(it.getFrom()->getLaneID(it.getFromLane())) > 0) {
                std::vector<GNEInternalLane*> lanes = myInternalLanes[it.getTLIndex()];
                for (auto it_lane : lanes) {
                    it_lane->onDefault(obj, sel, eventData);
                }
            }
        }
    }
}


bool
GNETLSEditorFrame::controlsEdge(GNEEdge* edge) const {
    if (myEditedDef != nullptr) {
        const NBConnectionVector& links = myEditedDef->getControlledLinks();
        for (auto it : links) {
            if (it.getFrom()->getID() == edge->getMicrosimID()) {
                return true;
            }
        }
    }
    return false;
}


void
GNETLSEditorFrame::editJunction(GNEJunction* junction) {
    if ((myTLSJunction->getCurrentJunction() == nullptr) || (!myTLSModifications->checkHaveModifications() && (junction != myTLSJunction->getCurrentJunction()))) {
        myTLSModifications->onCmdDiscardChanges(nullptr, 0, nullptr);
        myViewNet->getUndoList()->begin(GUIIcon::MODETLS, "modifying traffic light definition");
        myTLSJunction->setCurrentJunction(junction);
        myTLSAttributes->initTLSAttributes(myTLSJunction->getCurrentJunction());
        myTLSJunction->updateJunctionDescription();
        // only select TLS if getCurrentJunction exist
        if (myTLSJunction->getCurrentJunction()) {
            myTLSJunction->getCurrentJunction()->selectTLS(true);
        }
        if (myTLSAttributes->getNumberOfTLSDefinitions() > 0) {
            for (NBNode* node : myTLSAttributes->getCurrentTLSDefinition()->getNodes()) {
                myViewNet->getNet()->getAttributeCarriers()->retrieveJunction(node->getID())->selectTLS(true);
            }
        }
    } else {
        myViewNet->setStatusBarText("Unsaved modifications. Abort or Save");
    }
}


SUMOTime
GNETLSEditorFrame::getSUMOTime(const std::string& string) {
    return TIME2STEPS(GNEAttributeCarrier::parse<double>(string));
}

const std::string
GNETLSEditorFrame::getSteps2Time(const SUMOTime value) {
    return toString(STEPS2TIME(value));
}

// ---------------------------------------------------------------------------
// GNETLSEditorFrame::TLSAttributes - methods
// ---------------------------------------------------------------------------

GNETLSEditorFrame::TLSAttributes::TLSAttributes(GNETLSEditorFrame* TLSEditorParent) :
    MFXGroupBoxModule(TLSEditorParent, "Traffic light Attributes"),
    myTLSEditorParent(TLSEditorParent) {

    // create frame, label and textfield for name (By default disabled)
    FXHorizontalFrame* typeFrame = new FXHorizontalFrame(getCollapsableFrame(), GUIDesignAuxiliarHorizontalFrame);
    new FXLabel(typeFrame, toString(SUMO_ATTR_TYPE).c_str(), nullptr, GUIDesignLabelAttribute);
    myTLSType = new FXTextField(typeFrame, GUIDesignTextFieldNCol, this, 0, GUIDesignTextField);
    myTLSType->disable();

    // create frame, label and textfield for name (By default disabled)
    FXHorizontalFrame* idFrame = new FXHorizontalFrame(getCollapsableFrame(), GUIDesignAuxiliarHorizontalFrame);
    new FXLabel(idFrame, toString(SUMO_ATTR_ID).c_str(), nullptr, GUIDesignLabelAttribute);
    myIDTextField = new FXTextField(idFrame, GUIDesignTextFieldNCol, this, MID_GNE_TLSFRAME_ATTRIBUTES_SWITCH, GUIDesignTextField);
    myIDTextField->disable();

    // create frame, label and comboBox for Program (By default hidden)
    FXHorizontalFrame* programFrame = new FXHorizontalFrame(getCollapsableFrame(), GUIDesignAuxiliarHorizontalFrame);
    new FXLabel(programFrame, "program", nullptr, GUIDesignLabelAttribute);
    myProgramComboBox = new FXComboBox(programFrame, GUIDesignComboBoxNCol, this, MID_GNE_TLSFRAME_ATTRIBUTES_SWITCH, GUIDesignComboBoxAttribute);
    myProgramComboBox->disable();

    // create frame, label and TextField for Offset (By default disabled)
    FXHorizontalFrame* offsetFrame = new FXHorizontalFrame(getCollapsableFrame(), GUIDesignAuxiliarHorizontalFrame);
    new FXLabel(offsetFrame, toString(SUMO_ATTR_OFFSET).c_str(), nullptr, GUIDesignLabelAttribute);
    myOffsetTextField = new FXTextField(offsetFrame, GUIDesignTextFieldNCol, this, MID_GNE_TLSFRAME_ATTRIBUTES_OFFSET, GUIDesignTextField);
    myOffsetTextField->disable();

    // create frame, label and TextField for Offset (By default disabled)
    FXHorizontalFrame* parametersFrame = new FXHorizontalFrame(getCollapsableFrame(), GUIDesignAuxiliarHorizontalFrame);
    myButtonEditParameters = new FXButton(parametersFrame, "parameters", nullptr, this, MID_GNE_TLSFRAME_ATTRIBUTES_PARAMETERSDIALOG, GUIDesignButtonAttribute);
    myParametersTextField = new FXTextField(parametersFrame, GUIDesignTextFieldNCol, this, MID_GNE_TLSFRAME_ATTRIBUTES_PARAMETERS, GUIDesignTextField);
    myButtonEditParameters->disable();
    myParametersTextField->disable();
}


GNETLSEditorFrame::TLSAttributes::~TLSAttributes() {}


void
GNETLSEditorFrame::TLSAttributes::initTLSAttributes(GNEJunction* junction) {
    if (junction == nullptr) {
        throw ProcessError("Junction cannot be NULL");
    } else {
        // clear definitions
        myTLSDefinitions.clear();
        // set TLS type
        myTLSType->setText(junction->getAttribute(SUMO_ATTR_TLTYPE).c_str());
        // enable id TextField
        myIDTextField->enable();
        // enable Offset
        myOffsetTextField->enable();
        myOffsetTextField->setTextColor(MFXUtils::getFXColor(RGBColor::BLACK));
        // enable parameters
        myButtonEditParameters->enable();
        myParametersTextField->enable();
        myParametersTextField->setTextColor(MFXUtils::getFXColor(RGBColor::BLACK));
        // obtain TLSs
        for (const auto& TLS : junction->getNBNode()->getControllingTLS()) {
            myTLSDefinitions.push_back(TLS);
            myIDTextField->setText(TLS->getID().c_str());
            myIDTextField->enable();
            myProgramComboBox->appendItem(TLS->getProgramID().c_str());
        }
        if (myTLSDefinitions.size() > 0) {
            myProgramComboBox->enable();
            myProgramComboBox->setCurrentItem(0);
            myProgramComboBox->setNumVisible(myProgramComboBox->getNumItems());
            // switch TLS Program
            myTLSEditorParent->myTLSAttributes->onCmdDefSwitchTLSProgram(nullptr, 0, nullptr);
        }
    }
}


void
GNETLSEditorFrame::TLSAttributes::clearTLSAttributes() {
    // clear definitions
    myTLSDefinitions.clear();
    // clear TLS type
    myTLSType->setText("");
    // clear and disable name TextField
    myIDTextField->setText("");
    myIDTextField->disable();
    // clear and disable myProgramComboBox
    myProgramComboBox->clearItems();
    myProgramComboBox->disable();
    // clear and disable Offset TextField
    myOffsetTextField->setText("");
    myOffsetTextField->disable();
    myOffsetTextField->setTextColor(MFXUtils::getFXColor(RGBColor::BLACK));
    // clear and disable parameters TextField
    myButtonEditParameters->disable();
    myParametersTextField->setText("");
    myParametersTextField->disable();
    myParametersTextField->setTextColor(MFXUtils::getFXColor(RGBColor::BLACK));
}


NBTrafficLightDefinition*
GNETLSEditorFrame::TLSAttributes::getCurrentTLSDefinition() const {
    return myTLSDefinitions.at(myProgramComboBox->getCurrentItem());
}


const std::string
GNETLSEditorFrame::TLSAttributes::getCurrentTLSProgramID() const {
    if (myProgramComboBox->getNumItems() == 0) {
        return "";
    } else {
        return myProgramComboBox->getText().text();
    }
}


int
GNETLSEditorFrame::TLSAttributes::getNumberOfTLSDefinitions() const {
    return (int)myTLSDefinitions.size();
}


int
GNETLSEditorFrame::TLSAttributes::getNumberOfPrograms() const {
    return myProgramComboBox->getNumItems();
}


SUMOTime
GNETLSEditorFrame::TLSAttributes::getOffset() const {
    return getSUMOTime(myOffsetTextField->getText().text());
}


void
GNETLSEditorFrame::TLSAttributes::setOffset(const SUMOTime& offset) {
    myOffsetTextField->setText(getSteps2Time(offset).c_str());
    myOffsetTextField->setTextColor(MFXUtils::getFXColor(RGBColor::BLACK));
}


bool
GNETLSEditorFrame::TLSAttributes::isValidOffset() {
    if (GNEAttributeCarrier::canParse<SUMOTime>(myOffsetTextField->getText().text())) {
        myOffsetTextField->setTextColor(MFXUtils::getFXColor(RGBColor::BLACK));
        return true;
    } else {
        myOffsetTextField->setTextColor(MFXUtils::getFXColor(RGBColor::RED));
        return false;
    }
}


std::string
GNETLSEditorFrame::TLSAttributes::getParameters() const {
    return myParametersTextField->getText().text();
}


void
GNETLSEditorFrame::TLSAttributes::setParameters(const std::string& parameters) {
    myParametersTextField->setText(parameters.c_str());
    myParametersTextField->setTextColor(MFXUtils::getFXColor(RGBColor::BLACK));
}


bool
GNETLSEditorFrame::TLSAttributes::isValidParameters() {
    if (Parameterised::areParametersValid(myParametersTextField->getText().text())) {
        myParametersTextField->setTextColor(MFXUtils::getFXColor(RGBColor::BLACK));
        return true;
    } else {
        myParametersTextField->setTextColor(MFXUtils::getFXColor(RGBColor::RED));
        return false;
    }
}


long
GNETLSEditorFrame::TLSAttributes::onUpdTLSModified(FXObject* sender, FXSelector, void*) {
    if (myTLSEditorParent->myTLSAttributes->getNumberOfTLSDefinitions() == 0) {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_DISABLE), nullptr);
    } else if (myTLSEditorParent->myTLSModifications->checkHaveModifications()) {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_DISABLE), nullptr);
    } else {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_ENABLE), nullptr);
    }
    return 1;
}


long
GNETLSEditorFrame::TLSAttributes::onUpdNeedsTLSDef(FXObject* o, FXSelector, void*) {
    if (myTLSDefinitions.size() > 0) {
        o->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), nullptr);
    } else {
        o->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), nullptr);
    }
    return 1;
}


long
GNETLSEditorFrame::TLSAttributes::onCmdDefSwitchTLSProgram(FXObject*, FXSelector, void*) {
    if (myTLSEditorParent->myTLSJunction->getCurrentJunction() == nullptr) {
        throw ProcessError("Junction cannot be NULL");
    } else if ((int)myTLSDefinitions.size() != myProgramComboBox->getNumItems()) {
        throw ProcessError("myProgramComboBox must habe the same number of TLSDefinitions");
    } else {
        // get current definition
        NBTrafficLightDefinition* tlDef = getCurrentTLSDefinition();
        // logic may not have been recomputed yet. recompute to be sure
        NBTrafficLightLogicCont& tllCont = myTLSEditorParent->getViewNet()->getNet()->getTLLogicCont();
        // compute junction
        myTLSEditorParent->getViewNet()->getNet()->computeJunction(myTLSEditorParent->myTLSJunction->getCurrentJunction());
        // obtain TrafficLight logic vinculated with tlDef
        NBTrafficLightLogic* tllogic = tllCont.getLogic(tlDef->getID(), tlDef->getProgramID());
        // check that tllLogic exist
        if (tllogic != nullptr) {
            // now we can be sure that the tlDef is up to date (i.e. re-guessed)
            myTLSEditorParent->buildInternalLanes(tlDef);
            // create working copy from original def
            delete myTLSEditorParent->myEditedDef;
            myTLSEditorParent->myEditedDef = new NBLoadedSUMOTLDef(*tlDef, *tllogic);
            // set values
            setOffset(myTLSEditorParent->myEditedDef->getLogic()->getOffset());
            setParameters(myTLSEditorParent->myEditedDef->getLogic()->getParametersStr());
            // init phaseTable with the new TLS
            myTLSEditorParent->myTLSPhases->initPhaseTable();
        } else {
            // tlDef has no valid logic (probably because id does not control any links
            myTLSEditorParent->myTLSModifications->onCmdDiscardChanges(nullptr, 0, nullptr);
            myTLSEditorParent->getViewNet()->setStatusBarText("Traffic light does not control any links");
        }
    }
    return 1;
}


long
GNETLSEditorFrame::TLSAttributes::onCmdSetOffset(FXObject*, FXSelector, void*) {
    if (isValidOffset()) {
        myTLSEditorParent->myTLSModifications->setHaveModifications(true);
        myTLSEditorParent->myEditedDef->setOffset(getOffset());
    }
    return 1;
}


long
GNETLSEditorFrame::TLSAttributes::onCmdSetParameters(FXObject*, FXSelector, void*) {
    if (isValidParameters()) {
        myTLSEditorParent->myTLSModifications->setHaveModifications(true);
        myTLSEditorParent->myEditedDef->setParametersStr(getParameters());
    }
    return 1;
}


long
GNETLSEditorFrame::TLSAttributes::onCmdDefRename(FXObject*, FXSelector, void*) {
    throw ProcessError("Currently unused");
}


long
GNETLSEditorFrame::TLSAttributes::onCmdDefSubRename(FXObject*, FXSelector, void*) {
    throw ProcessError("Currently unused");
}


long
GNETLSEditorFrame::TLSAttributes::onCmdDefAddOff(FXObject*, FXSelector, void*) {
    throw ProcessError("Currently unused");
}


long
GNETLSEditorFrame::TLSAttributes::onCmdGuess(FXObject*, FXSelector, void*) {
    throw ProcessError("Currently unused");
}


long
GNETLSEditorFrame::TLSAttributes::onCmdEditParameters(FXObject*, FXSelector, void*) {
    // continue depending of myEditedDef
    if (myTLSEditorParent->myEditedDef) {
        // get previous parameters
        const auto previousParameters = getParameters();
        // write debug information
        WRITE_DEBUG("Open single parameters dialog");
        if (GNESingleParametersDialog(myTLSEditorParent->getViewNet()->getApp(), myTLSEditorParent->myEditedDef).execute()) {
            // write debug information
            WRITE_DEBUG("Close single parameters dialog");
            // set parameters in textfield
            setParameters(myTLSEditorParent->myEditedDef->getParametersStr());
            // only mark as modified if parameters are different
            if (getParameters() != previousParameters) {
                myTLSEditorParent->myTLSModifications->setHaveModifications(true);
            }
        } else {
            // write debug information
            WRITE_DEBUG("Cancel single parameters dialog");
        }
    }
    return 1;
}

// ---------------------------------------------------------------------------
// GNETLSEditorFrame::TLSJunction - methods
// ---------------------------------------------------------------------------

GNETLSEditorFrame::TLSJunction::TLSJunction(GNETLSEditorFrame* tlsEditorParent) :
    MFXGroupBoxModule(tlsEditorParent, "Junction"),
    myCurrentJunction(nullptr) {
    // Create frame for junction ID
    FXHorizontalFrame* junctionIDFrame = new FXHorizontalFrame(getCollapsableFrame(), GUIDesignAuxiliarHorizontalFrame);
    myLabelJunctionID = new FXLabel(junctionIDFrame, "Junction ID", nullptr, GUIDesignLabelAttribute);
    myTextFieldJunctionID = new FXTextField(junctionIDFrame, GUIDesignTextFieldNCol, this, MID_GNE_TLSFRAME_SELECT_JUNCTION, GUIDesignTextField);
    myTextFieldJunctionID->setEditable(false);
    // update junction description after creation
    updateJunctionDescription();
    // show TLS Junction
    show();
}


GNETLSEditorFrame::TLSJunction::~TLSJunction() {}


GNEJunction*
GNETLSEditorFrame::TLSJunction::getCurrentJunction() const {
    return myCurrentJunction;
}


void
GNETLSEditorFrame::TLSJunction::setCurrentJunction(GNEJunction* junction) {
    myCurrentJunction = junction;
}


void
GNETLSEditorFrame::TLSJunction::updateJunctionDescription() const {
    if (myCurrentJunction == nullptr) {
        myTextFieldJunctionID->setText("");
    } else {
        NBNode* nbn = myCurrentJunction->getNBNode();
        myTextFieldJunctionID->setText(nbn->getID().c_str());
    }
}

// ---------------------------------------------------------------------------
// GNETLSEditorFrame::TLSDefinition - methods
// ---------------------------------------------------------------------------

GNETLSEditorFrame::TLSDefinition::TLSDefinition(GNETLSEditorFrame* TLSEditorParent) :
    MFXGroupBoxModule(TLSEditorParent, "Traffic Light Programs"),
    myTLSEditorParent(TLSEditorParent) {
    // create auxiliar frames
    FXHorizontalFrame* horizontalFrameAux = new FXHorizontalFrame(getCollapsableFrame(), GUIDesignAuxiliarHorizontalFrameUniform);
    FXVerticalFrame* verticalFrameAuxA = new FXVerticalFrame(horizontalFrameAux, GUIDesignAuxiliarHorizontalFrame);
    FXVerticalFrame* verticalFrameAuxB = new FXVerticalFrame(horizontalFrameAux, GUIDesignAuxiliarHorizontalFrame);
    // create create tlDef button
    myNewTLProgram = new FXButton(verticalFrameAuxA, "Create\t\tCreate a new traffic light program",
                                  GUIIconSubSys::getIcon(GUIIcon::MODETLS), this, MID_GNE_TLSFRAME_DEFINITION_CREATE, GUIDesignButton);
    // create delete tlDef button
    myDeleteTLProgram = new FXButton(verticalFrameAuxB, "Delete\t\tDelete a traffic light program. If all programs are deleted the junction turns into a priority junction.",
                                     GUIIconSubSys::getIcon(GUIIcon::REMOVE), this, MID_GNE_TLSFRAME_DEFINITION_DELETE, GUIDesignButton);
    // create regenerate tlDef button
    myRegenerateTLProgram = new FXButton(verticalFrameAuxA, "Reset\t\tRegenerate TLS Program.",
                                         GUIIconSubSys::getIcon(GUIIcon::RELOAD), this, MID_GNE_TLSFRAME_DEFINITION_RESET, GUIDesignButton);
    // show TLS TLSDefinition
    show();
}


GNETLSEditorFrame::TLSDefinition::~TLSDefinition() {}


long
GNETLSEditorFrame::TLSDefinition::onCmdCreate(FXObject*, FXSelector, void*) {
    GNEJunction* junction = myTLSEditorParent->myTLSJunction->getCurrentJunction();
    // get current TLS program id
    const auto currentTLS = myTLSEditorParent->myTLSAttributes->getCurrentTLSProgramID();
    // abort because we onCmdOk assumes we wish to save an edited definition
    myTLSEditorParent->myTLSModifications->onCmdDiscardChanges(nullptr, 0, nullptr);
    // check that current junction has two or more edges
    if ((junction->getGNEIncomingEdges().size() > 0) && (junction->getGNEOutgoingEdges().size() > 0)) {
        if (junction->getAttribute(SUMO_ATTR_TYPE) != toString(SumoXMLNodeType::TRAFFIC_LIGHT)) {
            junction->setAttribute(SUMO_ATTR_TYPE, toString(SumoXMLNodeType::TRAFFIC_LIGHT), myTLSEditorParent->getViewNet()->getUndoList());
        } else {
            if (junction->getNBNode()->isTLControlled()) {
                // use existing traffic light as template for type, signal groups, controlled nodes etc
                NBTrafficLightDefinition* tpl = nullptr;
                for (const auto& TLS : junction->getNBNode()->getControllingTLS()) {
                    if (TLS->getProgramID() == currentTLS) {
                        tpl = TLS;
                    }
                }
                if (tpl == nullptr) {
                    tpl = *junction->getNBNode()->getControllingTLS().begin();
                }
                NBTrafficLightLogic* newLogic = tpl->compute(OptionsCont::getOptions());
                NBLoadedSUMOTLDef* newDef = new NBLoadedSUMOTLDef(*tpl, *newLogic);
                delete newLogic;
                myTLSEditorParent->getViewNet()->getUndoList()->add(new GNEChange_TLS(junction, newDef, true, true), true);
            } else {
                // for some reason the traffic light was not built, try again
                myTLSEditorParent->getViewNet()->getUndoList()->add(new GNEChange_TLS(junction, nullptr, true, true), true);
            }
        }
        myTLSEditorParent->editJunction(junction);
    } else {
        // write warning if netedit is running in testing mode
        WRITE_DEBUG("Opening warning FXMessageBox 'invalid TLS'");
        // open question box
        FXMessageBox::warning(this, MBOX_OK,
                              "TLS cannot be created", "%s",
                              "Traffic Light cannot be created because junction must have\n at least one incoming edge and one outgoing edge.");
        // write warning if netedit is running in testing mode
        WRITE_DEBUG("Closed FXMessageBox 'invalid TLS'");
    }
    return 1;
}


long
GNETLSEditorFrame::TLSDefinition::onCmdDelete(FXObject*, FXSelector, void*) {
    GNEJunction* junction = myTLSEditorParent->myTLSJunction->getCurrentJunction();
    const bool changeType = myTLSEditorParent->myTLSAttributes->getNumberOfTLSDefinitions() == 1;
    NBTrafficLightDefinition* tlDef = myTLSEditorParent->myTLSAttributes->getCurrentTLSDefinition();
    myTLSEditorParent->myTLSModifications->onCmdDiscardChanges(nullptr, 0, nullptr); // abort because onCmdOk assumes we wish to save an edited definition
    if (changeType) {
        junction->setAttribute(SUMO_ATTR_TYPE, toString(SumoXMLNodeType::PRIORITY), myTLSEditorParent->getViewNet()->getUndoList());
    } else {
        myTLSEditorParent->getViewNet()->getUndoList()->add(new GNEChange_TLS(junction, tlDef, false), true);
    }
    return 1;
}


long
GNETLSEditorFrame::TLSDefinition::onCmdReset(FXObject*, FXSelector, void*) {
    // make a copy of the junction
    GNEJunction* junction = myTLSEditorParent->myTLSJunction->getCurrentJunction();
    // begin undo
    myTLSEditorParent->getViewNet()->getUndoList()->begin(GUIIcon::MODETLS, "regenerate TLS");
    // delete junction
    onCmdDelete(nullptr, 0, nullptr);
    // set junction again
    myTLSEditorParent->myTLSJunction->setCurrentJunction(junction);
    // create junction
    onCmdCreate(nullptr, 0, nullptr);
    // end undo
    myTLSEditorParent->getViewNet()->getUndoList()->end();
    return 1;
}


long
GNETLSEditorFrame::TLSDefinition::onUpdCreateButton(FXObject* sender, FXSelector, void*) {
    // get current junction
    const auto junction = myTLSEditorParent->myTLSJunction->getCurrentJunction();
    // check conditions
    if (junction == nullptr) {
        // no junction, disable button
        sender->handle(this, FXSEL(SEL_COMMAND, ID_DISABLE), nullptr);
    } else if (myTLSEditorParent->myTLSModifications->checkHaveModifications()) {
        // wait for modifications, disable button
        sender->handle(this, FXSEL(SEL_COMMAND, ID_DISABLE), nullptr);
    } else {
        // enable button
        sender->handle(this, FXSEL(SEL_COMMAND, ID_ENABLE), nullptr);
        // update button text
        if (junction->getNBNode()->isTLControlled()) {
            static_cast<FXButton*>(sender)->setText("Copy");
        } else {
            static_cast<FXButton*>(sender)->setText("Create");
        }
    }
    return 1;
}


long
GNETLSEditorFrame::TLSDefinition::onUpdTLSModified(FXObject* sender, FXSelector, void*) {
    if (myTLSEditorParent->myTLSAttributes->getNumberOfTLSDefinitions() == 0) {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_DISABLE), nullptr);
    } else if (myTLSEditorParent->myTLSModifications->checkHaveModifications()) {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_DISABLE), nullptr);
    } else {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_ENABLE), nullptr);
    }
    return 1;
}

// ---------------------------------------------------------------------------
// GNETLSEditorFrame::TLSPhases - methods
// ---------------------------------------------------------------------------

GNETLSEditorFrame::TLSPhases::TLSPhases(GNETLSEditorFrame* TLSEditorParent) :
    MFXGroupBoxModule(TLSEditorParent, "Phases", MFXGroupBoxModule::Options::COLLAPSIBLE | MFXGroupBoxModule::Options::EXTENSIBLE),
    myTLSEditorParent(TLSEditorParent) {
    // create GNETLSTable
    myPhaseTable = new GNETLSTable(this);
    // hide phase table
    myPhaseTable->hide();
    FXHorizontalFrame* phaseButtons = new FXHorizontalFrame(getCollapsableFrame(), GUIDesignAuxiliarHorizontalFrameUniform);
    FXVerticalFrame* col1 = new FXVerticalFrame(phaseButtons, GUIDesignAuxiliarHorizontalFrame); // left button columm
    FXVerticalFrame* col2 = new FXVerticalFrame(phaseButtons, GUIDesignAuxiliarHorizontalFrame); // right button column
    // create cleanup states button
    new MFXButtonTooltip(col1, "Clean States\tClean unused states from all phase\tClean unused states from all phase. (Not allowed for multiple programs)", nullptr, this, MID_GNE_TLSFRAME_PHASES_CLEANUP, GUIDesignButton);
    // add unused states button
    new MFXButtonTooltip(col2, "Add States\tExtend the state vector for all phases by one entry\tExtend the state vector for all phases by one entry (unused until a connection or crossing is assigned to the new index).", nullptr, this, MID_GNE_TLSFRAME_PHASES_ADDUNUSED, GUIDesignButton);
    // group states button
    new MFXButtonTooltip(col1, "Group Signals\tShorten state definition by letting connections with the same signal states use the same index\tShorten state definition by letting connections with the same signal states use the same index. (Not allowed for multiple programs)", nullptr, this, MID_GNE_TLSFRAME_PHASES_GROUPSTATES, GUIDesignButton);
    // ungroup states button
    new MFXButtonTooltip(col2, "Ungroup Signals\tLet every connection use a distinct index (reverse state grouping)\tLet every connection use a distinct index (reverse state grouping). (Not allowed for multiple programs)", nullptr, this, MID_GNE_TLSFRAME_PHASES_UNGROUPSTATES, GUIDesignButton);
    // show TLSFile
    show();
}


GNETLSEditorFrame::TLSPhases::~TLSPhases() {
}


GNETLSEditorFrame*
GNETLSEditorFrame::TLSPhases::getTLSEditorParent() const {
    return myTLSEditorParent;
}


GNETLSTable*
GNETLSEditorFrame::TLSPhases::getPhaseTable() const {
    return myPhaseTable;
}


void
GNETLSEditorFrame::TLSPhases::initPhaseTable() {
    // first clear table
    myPhaseTable->clearTable();
    if (myTLSEditorParent->myTLSAttributes->getNumberOfTLSDefinitions() > 0) {
        if (myTLSEditorParent->myEditedDef->getType() == TrafficLightType::STATIC) {
            initStaticPhaseTable();
        } else if (myTLSEditorParent->myEditedDef->getType() == TrafficLightType::ACTUATED) {
            initActuatedPhaseTable();
        } else if (myTLSEditorParent->myEditedDef->getType() == TrafficLightType::DELAYBASED) {
            initDelayBasePhaseTable();
        } else if (myTLSEditorParent->myEditedDef->getType() == TrafficLightType::NEMA) {
            initNEMAPhaseTable();
        }
        // select first element (by default)
        myPhaseTable->selectRow(0);
        // recalc width and show
        myPhaseTable->recalcTableWidth();
        myPhaseTable->show();
    } else {
        myPhaseTable->hide();
    }
    update();
}


void
GNETLSEditorFrame::TLSPhases::switchPhase() {
    // get current selected phase in phaseTable
    const auto row = myPhaseTable->getCurrentSelectedRow();
    const NBTrafficLightLogic::PhaseDefinition& phase = myTLSEditorParent->getPhase(row);
    myPhaseTable->selectRow(row);
    // need not hold since links could have been deleted somewhere else and indices may be reused
    for (const auto &internalLane : myTLSEditorParent->myInternalLanes) {
        int tlIndex = internalLane.first;
        std::vector<GNEInternalLane*> lanes = internalLane.second;
        LinkState state = LINKSTATE_DEADEND;
        if (tlIndex >= 0 && tlIndex < (int)phase.state.size()) {
            state = (LinkState)phase.state[tlIndex];
        }
        for (const auto &lane : lanes) {
            lane->setLinkState(state);
        }
    }
    myTLSEditorParent->getViewNet()->updateViewNet();
}


bool
GNETLSEditorFrame::TLSPhases::changePhaseValue(const int col, const int row, const std::string &value) {
    // Declare columns
    int colDuration = 1;
    int colState = -1;
    int colNext = -1;
    int colName = -1;
    int colMinDur = -1;
    int colMaxDur = -1;
    int colEarliestEnd = -1;
    int colLatestEnd = -1;
    int colVehExt = -1;
    int colYellow = -1;
    int colRed = -1;
    // set columns depending of traffic light type
    if (myTLSEditorParent->myEditedDef->getType() == TrafficLightType::STATIC) {
        colState = 2;
        colNext = 3;
        colName = 4;
    } else if (myTLSEditorParent->myEditedDef->getType() == TrafficLightType::ACTUATED) {
        colMinDur = 2;
        colMaxDur = 3;
        colState = 4;
        colEarliestEnd = 5;
        colLatestEnd = 6;
        colNext = 7;
        colName = 8;
    } else if (myTLSEditorParent->myEditedDef->getType() == TrafficLightType::DELAYBASED) {
        colMinDur = 2;
        colMaxDur = 3;
        colState = 4;
        colNext = 5;
    } else if (myTLSEditorParent->myEditedDef->getType() == TrafficLightType::NEMA) {
        colMinDur = 2;
        colMaxDur = 3;
        colState = 4;
        colVehExt = 5;
        colYellow = 6;
        colRed = 7;
        colNext = 8;
        colName = 9;
    }
    // check column
    if (col == colDuration) {
        return setDuration(col, row, value);
    } else if (col == colState) {
        return setState(col, row, value);
    } else if (col == colNext) {
        return setNext(col, row, value);
    } else if (col == colName) {
        return setName(col, row, value);
    } else if (col == colMinDur) {
        return setMinDur(col, row, value);
    } else if (col == colMaxDur) {
        return setMaxDur(col, row, value);
    } else if (col == colEarliestEnd) {
        return setEarliestEnd(col, row, value);
    } else if (col == colLatestEnd) {
        return setLatestEnd(col, row, value);
    } else if (col == colVehExt) {
        return setVehExt(col, row, value);
    } else if (col == colYellow) {
        return setYellow(col, row, value);
    } else if (col == colRed) {
        return setRed(col, row, value);
    } else {
        throw ProcessError("invalid column");
    }
}


void
GNETLSEditorFrame::TLSPhases::addPhase(const int row) {
    // get option container
    const OptionsCont& oc = OptionsCont::getOptions();
    // mark TLS as modified
    myTLSEditorParent->myTLSModifications->setHaveModifications(true);
    // check if TLS is static
    const bool TLSStatic = (myTLSEditorParent->myEditedDef->getType() == TrafficLightType::STATIC);
    // calculate new index
    const int newIndex = row + 1;
    // copy current row
    auto duration = getSUMOTime(myPhaseTable->getItemText(row, 1));
    const auto oldState = myPhaseTable->getItemText(row, TLSStatic ? 2 : 4);
    auto state = oldState;
    // update crossingINdices
    std::set<int> crossingIndices;
    for (const auto &node : myTLSEditorParent->myEditedDef->getNodes()) {
        for (const auto &crossing : node->getCrossings()) {
            crossingIndices.insert(crossing->tlLinkIndex);
            crossingIndices.insert(crossing->tlLinkIndex2);
        }
    }
    // smart adapations for new state
    bool haveGreen = false;
    bool haveYellow = false;
    for (const auto &linkStateChar : state) {
        if ((linkStateChar == LINKSTATE_TL_GREEN_MAJOR) || (linkStateChar == LINKSTATE_TL_GREEN_MINOR)) {
            haveGreen = true;
        } else if ((linkStateChar == LINKSTATE_TL_YELLOW_MAJOR) || (linkStateChar == LINKSTATE_TL_YELLOW_MINOR)) {
            haveYellow = true;
        }
    }
    if (haveGreen && haveYellow) {
        // guess left-mover state
        duration = TIME2STEPS(oc.getInt("tls.left-green.time"));
        for (int i = 0; i < (int)state.size(); i++) {
            if ((state[i] == LINKSTATE_TL_YELLOW_MAJOR) || (state[i] == LINKSTATE_TL_YELLOW_MINOR)) {
                state[i] = LINKSTATE_TL_RED;
            } else if (state[i] == LINKSTATE_TL_GREEN_MINOR) {
                state[i] = LINKSTATE_TL_GREEN_MAJOR;
            }
        }
    } else if (haveGreen) {
        // guess yellow state
        myTLSEditorParent->myEditedDef->setParticipantsInformation();
        duration = TIME2STEPS(myTLSEditorParent->myEditedDef->computeBrakingTime(oc.getFloat("tls.yellow.min-decel")));
        for (int i = 0; i < (int)state.size(); i++) {
            if ((state[i] == LINKSTATE_TL_GREEN_MAJOR) || (state[i] == LINKSTATE_TL_GREEN_MINOR)) {
                if (crossingIndices.count(i) == 0) {
                    state[i] = LINKSTATE_TL_YELLOW_MINOR;
                } else {
                    state[i] = LINKSTATE_TL_RED;
                }
            }
        }
    } else if (haveYellow) {
        duration = TIME2STEPS(oc.isDefault("tls.allred.time") ? 2 :  oc.getInt("tls.allred.time"));
        // guess all-red state
        for (int i = 0; i < (int)state.size(); i++) {
            if ((state[i] == LINKSTATE_TL_YELLOW_MAJOR) || (state[i] == LINKSTATE_TL_YELLOW_MINOR)) {
                state[i] = LINKSTATE_TL_RED;
            }
        }
    }
    // fix continuous green states
    const int nextIndex = (myPhaseTable->getNumRows() > newIndex)? newIndex : 0;
    const std::string state2 = myPhaseTable->getItemText(nextIndex, (TLSStatic ? 2 : 4));
    for (int i = 0; i < (int)state.size(); i++) {
        if (((oldState[i] == LINKSTATE_TL_GREEN_MAJOR) || (oldState[i] == LINKSTATE_TL_GREEN_MINOR)) &&
            ((state2[i] == LINKSTATE_TL_GREEN_MAJOR) || (state2[i] == LINKSTATE_TL_GREEN_MINOR))) {
            state[i] = oldState[i];
        }
    }
    // add new step
    myTLSEditorParent->myEditedDef->getLogic()->addStep(duration, state, std::vector<int>(), "", newIndex);
    // int phase table again
    initPhaseTable();
    // mark new row as selected
    myPhaseTable->selectRow(newIndex);
    // set focus in table
    getPhaseTable()->setFocus();
}


void
GNETLSEditorFrame::TLSPhases::removePhase(const int row) {
    // mark TLS ad modified
    myTLSEditorParent->myTLSModifications->setHaveModifications(true);
    // calculate new row
    const auto newRow = MAX2(0, (row - 1));
    // delete selected row
    myTLSEditorParent->myEditedDef->getLogic()->deletePhase(row);
    // int phase table again
    initPhaseTable();
    // mark new row as selected
    myPhaseTable->selectRow(newRow);
    // set focus in table
    getPhaseTable()->setFocus();
}


long
GNETLSEditorFrame::TLSPhases::onUpdNeedsDef(FXObject* o, FXSelector, void*) {
    if (myTLSEditorParent->myTLSAttributes->getNumberOfTLSDefinitions() > 0) {
        o->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), nullptr);
    } else {
        o->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), nullptr);
    }
    return 1;
}


long
GNETLSEditorFrame::TLSPhases::onUpdNeedsDefAndPhase(FXObject* o, FXSelector, void*) {
    if (myTLSEditorParent->myTLSAttributes->getNumberOfTLSDefinitions() == 0) {
        o->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), nullptr);
    } else if (myPhaseTable->getNumRows() <= 1) {
        o->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), nullptr);
    } else {
        o->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), nullptr);
    }
    return 1;
}


long
GNETLSEditorFrame::TLSPhases::onCmdCleanStates(FXObject*, FXSelector, void*) {
    myTLSEditorParent->myTLSModifications->setHaveModifications(myTLSEditorParent->myEditedDef->cleanupStates());
    myTLSEditorParent->buildInternalLanes(myTLSEditorParent->myEditedDef);
    initPhaseTable();
    myPhaseTable->setFocus();
    myTLSEditorParent->myTLSModifications->setHaveModifications(true);
    return 1;
}


long
GNETLSEditorFrame::TLSPhases::onCmdAddUnusedStates(FXObject*, FXSelector, void*) {
    myTLSEditorParent->myEditedDef->getLogic()->setStateLength(myTLSEditorParent->myEditedDef->getLogic()->getNumLinks() + 1);
    myTLSEditorParent->myTLSModifications->setHaveModifications(true);
    initPhaseTable();
    myPhaseTable->setFocus();
    return 1;
}


long
GNETLSEditorFrame::TLSPhases::onCmdGroupStates(FXObject*, FXSelector, void*) {
    myTLSEditorParent->myEditedDef->groupSignals();
    myTLSEditorParent->myTLSModifications->setHaveModifications(true);
    myTLSEditorParent->buildInternalLanes(myTLSEditorParent->myEditedDef);
    initPhaseTable();
    myPhaseTable->setFocus();
    return 1;
}


long
GNETLSEditorFrame::TLSPhases::onCmdUngroupStates(FXObject*, FXSelector, void*) {
    myTLSEditorParent->myEditedDef->setParticipantsInformation();
    myTLSEditorParent->myEditedDef->ungroupSignals();
    myTLSEditorParent->myTLSModifications->setHaveModifications(true);
    myTLSEditorParent->buildInternalLanes(myTLSEditorParent->myEditedDef);
    initPhaseTable();
    myPhaseTable->setFocus();
    return 1;
}


long
GNETLSEditorFrame::TLSPhases::onUpdNeedsSingleDef(FXObject* o, FXSelector, void*) {
    if (myTLSEditorParent->myTLSAttributes->getNumberOfTLSDefinitions() == 1) {
        o->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), nullptr);
    } else {
        o->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), nullptr);
    }
    return 1;
}


long
GNETLSEditorFrame::TLSPhases::onUpdUngroupStates(FXObject* o, FXSelector, void*) {
    if (myTLSEditorParent->myTLSAttributes->getNumberOfTLSDefinitions() != 1) {
        o->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), nullptr);
    } else if (myTLSEditorParent->myEditedDef == nullptr) {
        o->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), nullptr);
    } else if (myTLSEditorParent->myEditedDef->usingSignalGroups()) {
        o->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), nullptr);
    } else {
        o->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), nullptr);
    }
    return 1;
}


void
GNETLSEditorFrame::TLSPhases::initStaticPhaseTable() {
    // declare constants for columns
    const int colDuration = 1;
    const int colState = 2;
    const int colNext = 3;
    const int colName = 4;
    // get phases
    const auto &phases = myTLSEditorParent->myEditedDef->getLogic()->getPhases();
    // adjust table
    myPhaseTable->setTableSize("sup--id", (int)phases.size());
    // fill rows
    for (int row = 0; row < (int)phases.size(); row++) {
        myPhaseTable->setItemText(row, colDuration, getSteps2Time(phases.at(row).duration).c_str());
        myPhaseTable->setItemText(row, colState, phases.at(row).state.c_str());
        myPhaseTable->setItemText(row, colNext, phases.at(row).next.size() > 0 ? toString(phases.at(row).next).c_str() : "");
        myPhaseTable->setItemText(row, colName, phases.at(row).name.c_str());
    }
    // set columns
    myPhaseTable->setColumnLabelTop(colDuration, "dur");
    myPhaseTable->setColumnLabelTop(colState, "state");
    myPhaseTable->setColumnLabelTop(colNext, "next");
    myPhaseTable->setColumnLabelTop(colName, "name");
    // set bot labels
    updateCycleDuration(colDuration);
    updateStateSize(colState);
    // set focus
    myPhaseTable->setFocus();
}


void
GNETLSEditorFrame::TLSPhases::initActuatedPhaseTable() {
    // declare constants for columns
    const int colDuration = 1;
    const int colMinDur = 2;
    const int colMaxDur = 3;
    const int colState = 4;
    const int colEarliestEnd = 5;
    const int colLatestEnd = 6;
    const int colNext = 7;
    const int colName = 8;
    // get phases
    const auto &phases = myTLSEditorParent->myEditedDef->getLogic()->getPhases();
    // adjust table
    myPhaseTable->setTableSize("suffpff--id", (int)phases.size());
    // fill rows
    for (int row = 0; row < (int)phases.size(); row++) {
        myPhaseTable->setItemText(row, colDuration, getSteps2Time(phases.at(row).duration).c_str());
        myPhaseTable->setItemText(row, colMinDur, varDurString(phases.at(row).minDur).c_str());
        myPhaseTable->setItemText(row, colMaxDur, varDurString(phases.at(row).maxDur).c_str());
        myPhaseTable->setItemText(row, colState, phases.at(row).state.c_str());
        myPhaseTable->setItemText(row, colEarliestEnd, varDurString(phases.at(row).earliestEnd).c_str());
        myPhaseTable->setItemText(row, colLatestEnd, varDurString(phases.at(row).latestEnd).c_str());
        myPhaseTable->setItemText(row, colNext, phases.at(row).next.size() > 0 ? toString(phases.at(row).next).c_str() : "");
        myPhaseTable->setItemText(row, colName, phases.at(row).name.c_str());
    }
    // set columns
    myPhaseTable->setColumnLabelTop(colDuration, "dur");
    myPhaseTable->setColumnLabelTop(colMinDur, "min");
    myPhaseTable->setColumnLabelTop(colMaxDur, "max");
    myPhaseTable->setColumnLabelTop(colEarliestEnd, "ear.end", "earlyEnd");
    myPhaseTable->setColumnLabelTop(colLatestEnd, "lat.end", "latestEnd");
    myPhaseTable->setColumnLabelTop(colState, "state");
    myPhaseTable->setColumnLabelTop(colNext, "next");
    myPhaseTable->setColumnLabelTop(colName, "name");
    // set bot labels
    updateCycleDuration(colDuration);
    updateStateSize(colState);
    // set focus
    myPhaseTable->setFocus();
}


void
GNETLSEditorFrame::TLSPhases::initDelayBasePhaseTable() {
    // declare constants for columns
    const int colDuration = 1;
    const int colMinDur = 2;
    const int colMaxDur = 3;
    const int colState = 4;
    const int colNext = 5;
    const int colName = 6;
    // get phases
    const auto &phases = myTLSEditorParent->myEditedDef->getLogic()->getPhases();
    // adjust table
    myPhaseTable->setTableSize("suffp--id", (int)phases.size());
    // fill rows
    for (int row = 0; row < (int)phases.size(); row++) {
        myPhaseTable->setItemText(row, colDuration, getSteps2Time(phases.at(row).duration).c_str());
        myPhaseTable->setItemText(row, colMinDur, varDurString(phases.at(row).minDur).c_str());
        myPhaseTable->setItemText(row, colMaxDur, varDurString(phases.at(row).maxDur).c_str());
        myPhaseTable->setItemText(row, colState, phases.at(row).state.c_str());
        myPhaseTable->setItemText(row, colNext, phases.at(row).next.size() > 0 ? toString(phases.at(row).next).c_str() : "");
        myPhaseTable->setItemText(row, colName, phases.at(row).name.c_str());
    }
    // set columns
    myPhaseTable->setColumnLabelTop(colDuration, "dur");
    myPhaseTable->setColumnLabelTop(colMinDur, "min");
    myPhaseTable->setColumnLabelTop(colMaxDur, "max");
    myPhaseTable->setColumnLabelTop(colState, "state");
    myPhaseTable->setColumnLabelTop(colNext, "next");
    myPhaseTable->setColumnLabelTop(colName, "name");
    // set bot labels
    updateCycleDuration(colDuration);
    updateStateSize(colState);
    // set focus
    myPhaseTable->setFocus();
}


void
GNETLSEditorFrame::TLSPhases::initNEMAPhaseTable() {
    // declare constants for columns
    const int colDuration = 1;
    const int colMinDur = 2;
    const int colMaxDur = 3;
    const int colState = 4;
    const int colVehExt = 5;
    const int colYellow = 6;
    const int colRed = 7;
    const int colNext = 8;
    const int colName = 9;
    // get phases
    const auto &phases = myTLSEditorParent->myEditedDef->getLogic()->getPhases();
    // adjust table
    myPhaseTable->setTableSize("suffpff---id", (int)phases.size());
    // fill rows
    for (int row = 0; row < (int)phases.size(); row++) {
        myPhaseTable->setItemText(row, colDuration, getSteps2Time(phases.at(row).duration).c_str());
        myPhaseTable->setItemText(row, colMinDur, varDurString(phases.at(row).minDur).c_str());
        myPhaseTable->setItemText(row, colMaxDur, varDurString(phases.at(row).maxDur).c_str());
        myPhaseTable->setItemText(row, colState, phases.at(row).state.c_str());
        myPhaseTable->setItemText(row, colVehExt, varDurString(phases.at(row).vehExt).c_str());
        myPhaseTable->setItemText(row, colYellow, varDurString(phases.at(row).yellow).c_str());
        myPhaseTable->setItemText(row, colRed, varDurString(phases.at(row).red).c_str());
        myPhaseTable->setItemText(row, colNext, phases.at(row).next.size() > 0 ? toString(phases.at(row).next).c_str() : "");
        myPhaseTable->setItemText(row, colName, phases.at(row).name.c_str());
    }
    // set columns
    myPhaseTable->setColumnLabelTop(colDuration, "dur");
    myPhaseTable->setColumnLabelTop(colMinDur, "min");
    myPhaseTable->setColumnLabelTop(colMaxDur, "max");
    myPhaseTable->setColumnLabelTop(colState, "state");
    myPhaseTable->setColumnLabelTop(colVehExt, "vehExt", "vehicle extension");
    myPhaseTable->setColumnLabelTop(colYellow, "yellow");
    myPhaseTable->setColumnLabelTop(colRed, "red");
    myPhaseTable->setColumnLabelTop(colNext, "next");
    myPhaseTable->setColumnLabelTop(colName, "name");
    // set bot labels
    updateCycleDuration(colDuration);
    updateStateSize(colState);
    // set focus
    myPhaseTable->setFocus();
}


bool
GNETLSEditorFrame::TLSPhases::setDuration(const int col, const int row, const std::string &value) {
    // check value
    if (value.empty()) {
        // input empty, reset
        getPhaseTable()->setItemText(row, col, getSteps2Time(myTLSEditorParent->getPhase(row).duration).c_str());
        return true;
    } else if (GNEAttributeCarrier::canParse<double>(value)) {
        const auto duration = getSUMOTime(value);
        // check that duration > 0
        if (duration > 0) {
            myTLSEditorParent->myEditedDef->getLogic()->setPhaseDuration(row, duration);
            myTLSEditorParent->myTLSModifications->setHaveModifications(true);
            // update Cycle duration
            updateCycleDuration(col);
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}


bool
GNETLSEditorFrame::TLSPhases::setState(const int col, const int row, const std::string &value) {
    // get state
    const auto &phase = myTLSEditorParent->getPhase(row);
    // declare new state. If value is empty, use previous value (reset)
    const auto newState = value.empty()? phase.state : value;
    // insert phase
    try {
        myTLSEditorParent->myEditedDef->getLogic()->addStep(phase.duration, newState, phase.next, phase.name, row);
    } catch (ProcessError&) {
        // invalid character in newState
        return false;
    }
    // delete next phase
    try {
        myTLSEditorParent->myEditedDef->getLogic()->deletePhase(row + 1);
    } catch (InvalidArgument&) {
        WRITE_ERROR("Error deleting phase '" + toString(row + 1) + "'");
        return false;
    }
    // mark TLS as modified depending of value
    if (value.size() > 0) {
        myTLSEditorParent->myTLSModifications->setHaveModifications(true);
        // switch to new phase
        switchPhase();
    } else {
        // input empty, reset
        getPhaseTable()->setItemText(row, col, newState);
    }
    // update state size
    updateStateSize(col);
    return true;
}


bool
GNETLSEditorFrame::TLSPhases::setNext(const int col, const int row, const std::string &value) {
    // check value
    if (value.empty()) {
        // input empty, reset value
        myPhaseTable->setItemText(row, col, toString(myTLSEditorParent->getPhase(row).next));
        return true;
    } else {
        // check next
        if (GNEAttributeCarrier::canParse<std::vector<int> >(value)) {
            const auto nextEdited = GNEAttributeCarrier::parse<std::vector<int> >(value);
            for (const auto nextPhase : nextEdited) {
                if ((nextPhase < 0) || (nextPhase >= myPhaseTable->getNumRows())) {
                    return false;
                }
            }
            // set new next
            myTLSEditorParent->myEditedDef->getLogic()->setPhaseNext(row, nextEdited);
            myTLSEditorParent->myTLSModifications->setHaveModifications(true);
            return true;
        } else {
            return false;
        }
    }
}


bool
GNETLSEditorFrame::TLSPhases::setName(const int /*col*/, const int row, const std::string &value) {
    // update name (currently no check needed)
    myTLSEditorParent->myEditedDef->getLogic()->setPhaseName(row, value);
    myTLSEditorParent->myTLSModifications->setHaveModifications(true);
    return true;
}


bool
GNETLSEditorFrame::TLSPhases::setMinDur(const int col, const int row, const std::string &value) {
    // check value
    if (value.empty()) {
        // input empty, reset value
        myPhaseTable->setItemText(row, col, toString(myTLSEditorParent->getPhase(row).minDur));
        return true;
    } else if (GNEAttributeCarrier::canParse<double>(value)) {
        const auto minDur = getSUMOTime(value);
        // check that minDur > 0
        if (minDur > 0) {
            myTLSEditorParent->myEditedDef->getLogic()->setPhaseMinDuration(row, minDur);
            myTLSEditorParent->myTLSModifications->setHaveModifications(true);
            return true;
        } else {
            return false;
        }
    } else if (StringUtils::prune(value).empty()) {
        myTLSEditorParent->myEditedDef->getLogic()->setPhaseMinDuration(row, NBTrafficLightDefinition::UNSPECIFIED_DURATION);
        myTLSEditorParent->myTLSModifications->setHaveModifications(true);
        return true;
    } else {
        return false;
    }
}


bool
GNETLSEditorFrame::TLSPhases::setMaxDur(const int col, const int row, const std::string &value) {
    // check value
    if (value.empty()) {
        // input empty, reset value
        myPhaseTable->setItemText(row, col, toString(myTLSEditorParent->getPhase(row).maxDur));
        return true;
    } else if (GNEAttributeCarrier::canParse<double>(value)) {
        const auto maxDur = getSUMOTime(value);
        // check that minDur > 0
        if (maxDur > 0) {
            myTLSEditorParent->myEditedDef->getLogic()->setPhaseMaxDuration(row, maxDur);
            myTLSEditorParent->myTLSModifications->setHaveModifications(true);
            return true;
        } else {
            return false;
        }
    } else if (StringUtils::prune(value).empty()) {
        myTLSEditorParent->myEditedDef->getLogic()->setPhaseMaxDuration(row, NBTrafficLightDefinition::UNSPECIFIED_DURATION);
        myTLSEditorParent->myTLSModifications->setHaveModifications(true);
        return true;
    } else {
        return false;
    }
}


bool
GNETLSEditorFrame::TLSPhases::setEarliestEnd(const int col, const int row, const std::string &value) {
    // check value
    if (value.empty()) {
        // input empty, reset value
        myPhaseTable->setItemText(row, col, toString(myTLSEditorParent->getPhase(row).earliestEnd));
        return true;
    } else if (GNEAttributeCarrier::canParse<double>(value)) {
        const auto earliestEnd = getSUMOTime(value);
        // check that earliestEnd > 0
        if (earliestEnd > 0) {
            myTLSEditorParent->myEditedDef->getLogic()->setPhaseEarliestEnd(row, earliestEnd);
            myTLSEditorParent->myTLSModifications->setHaveModifications(true);
            return true;
        } else {
            return false;
        }
    } else if (StringUtils::prune(value).empty()) {
        myTLSEditorParent->myEditedDef->getLogic()->setPhaseEarliestEnd(row, NBTrafficLightDefinition::UNSPECIFIED_DURATION);
        myTLSEditorParent->myTLSModifications->setHaveModifications(true);
        return true;
    } else {
        return false;
    }
}


bool
GNETLSEditorFrame::TLSPhases::setLatestEnd(const int col, const int row, const std::string &value) {
    // check value
    if (value.empty()) {
        // input empty, reset value
        myPhaseTable->setItemText(row, col, toString(myTLSEditorParent->getPhase(row).latestEnd));
        return true;
    } else if (GNEAttributeCarrier::canParse<double>(value)) {
        const auto latestEnd = getSUMOTime(value);
        // check that latestEnd > 0
        if (latestEnd > 0) {
            myTLSEditorParent->myEditedDef->getLogic()->setPhaseLatestEnd(row, latestEnd);
            myTLSEditorParent->myTLSModifications->setHaveModifications(true);
            return true;
        } else {
            return false;
        }
    } else if (StringUtils::prune(value).empty()) {
        myTLSEditorParent->myEditedDef->getLogic()->setPhaseLatestEnd(row, NBTrafficLightDefinition::UNSPECIFIED_DURATION);
        myTLSEditorParent->myTLSModifications->setHaveModifications(true);
        return true;
    } else {
        return false;
    }
}


bool
GNETLSEditorFrame::TLSPhases::setVehExt(const int col, const int row, const std::string &value) {
    // check value
    if (value.empty()) {
        // input empty, reset value
        myPhaseTable->setItemText(row, col, toString(myTLSEditorParent->getPhase(row).vehExt));
        return true;
    } else if (GNEAttributeCarrier::canParse<double>(value)) {
        const auto vehExt = getSUMOTime(value);
        // check that vehExt > 0
        if (vehExt > 0) {
            myTLSEditorParent->myEditedDef->getLogic()->setPhaseVehExt(row, vehExt);
            myTLSEditorParent->myTLSModifications->setHaveModifications(true);
            return true;
        } else {
            return false;
        }
    } else if (StringUtils::prune(value).empty()) {
        myTLSEditorParent->myEditedDef->getLogic()->setPhaseVehExt(row, NBTrafficLightDefinition::UNSPECIFIED_DURATION);
        myTLSEditorParent->myTLSModifications->setHaveModifications(true);
        return true;
    } else {
        return false;
    }
}


bool
GNETLSEditorFrame::TLSPhases::setYellow(const int col, const int row, const std::string &value) {
    // check value
    if (value.empty()) {
        // input empty, reset value
        myPhaseTable->setItemText(row, col, toString(myTLSEditorParent->getPhase(row).yellow));
        return true;
    } else if (GNEAttributeCarrier::canParse<double>(value)) {
        const auto yellow = getSUMOTime(value);
        // check that yellow > 0
        if (yellow > 0) {
            myTLSEditorParent->myEditedDef->getLogic()->setPhaseYellow(row, yellow);
            myTLSEditorParent->myTLSModifications->setHaveModifications(true);
            return true;
        } else {
            return false;
        }
    } else if (StringUtils::prune(value).empty()) {
        myTLSEditorParent->myEditedDef->getLogic()->setPhaseYellow(row, NBTrafficLightDefinition::UNSPECIFIED_DURATION);
        myTLSEditorParent->myTLSModifications->setHaveModifications(true);
        return true;
    } else {
        return false;
    }
}


bool
GNETLSEditorFrame::TLSPhases::setRed(const int col, const int row, const std::string &value) {
    // check value
    if (value.empty()) {
        // input empty, reset value
        myPhaseTable->setItemText(row, col, toString(myTLSEditorParent->getPhase(row).red));
        return true;
    } else if (GNEAttributeCarrier::canParse<double>(value)) {
        const auto red = getSUMOTime(value);
        // check that red > 0
        if (red > 0) {
            myTLSEditorParent->myEditedDef->getLogic()->setPhaseRed(row, red);
            myTLSEditorParent->myTLSModifications->setHaveModifications(true);
            return true;
        } else {
            return false;
        }
    } else if (StringUtils::prune(value).empty()) {
        myTLSEditorParent->myEditedDef->getLogic()->setPhaseRed(row, NBTrafficLightDefinition::UNSPECIFIED_DURATION);
        myTLSEditorParent->myTLSModifications->setHaveModifications(true);
        return true;
    } else {
        return false;
    }
}


void
GNETLSEditorFrame::TLSPhases::updateCycleDuration(const int col) {
    SUMOTime cycleDuration = 0;
    for (const auto &phase : myTLSEditorParent->myEditedDef->getLogic()->getPhases()) {
        cycleDuration += phase.duration;
    }
    // update bot label with cycle duration
    myPhaseTable->setColumnLabelBot(col, getSteps2Time(cycleDuration));
}


void 
GNETLSEditorFrame::TLSPhases::updateStateSize(const int col) {
    // update bot label with number of links
    myPhaseTable->setColumnLabelBot(col, "Links: " + toString(myTLSEditorParent->myEditedDef->getLogic()->getNumLinks()));
}

// ---------------------------------------------------------------------------
// GNETLSEditorFrame::TLSModifications - methods
// ---------------------------------------------------------------------------

GNETLSEditorFrame::TLSModifications::TLSModifications(GNETLSEditorFrame* TLSEditorParent) :
    MFXGroupBoxModule(TLSEditorParent, "Modifications"),
    myTLSEditorParent(TLSEditorParent),
    myHaveModifications(false) {
    FXHorizontalFrame* buttonsFrame = new FXHorizontalFrame(getCollapsableFrame(), GUIDesignAuxiliarHorizontalFrame);
    // create save modifications button
    mySaveModificationsButtons = new FXButton(buttonsFrame, "Save\t\tSave program modifications (Enter)",
            GUIIconSubSys::getIcon(GUIIcon::OK), this, MID_GNE_TLSFRAME_MODIFICATIONS_SAVE, GUIDesignButton);
    // create discard modifications buttons
    myDiscardModificationsButtons = new FXButton(buttonsFrame, "Cancel\t\tDiscard program modifications (Esc)",
            GUIIconSubSys::getIcon(GUIIcon::CANCEL), this, MID_GNE_TLSFRAME_MODIFICATIONS_DISCARD, GUIDesignButton);
    // show TLSModifications
    show();
}


GNETLSEditorFrame::TLSModifications::~TLSModifications() {}


bool
GNETLSEditorFrame::TLSModifications::checkHaveModifications() const {
    return myHaveModifications;
}


void
GNETLSEditorFrame::TLSModifications::setHaveModifications(bool value) {
    myHaveModifications = value;
}


long
GNETLSEditorFrame::TLSModifications::onCmdSaveChanges(FXObject*, FXSelector, void*) {
    if (myTLSEditorParent->myTLSJunction->getCurrentJunction() != nullptr) {
        if (myHaveModifications) {
            const auto oldDefinition = myTLSEditorParent->myTLSAttributes->getCurrentTLSDefinition();
            std::vector<NBNode*> nodes = oldDefinition->getNodes();
            for (const auto& node : nodes) {
                GNEJunction* junction = myTLSEditorParent->getViewNet()->getNet()->getAttributeCarriers()->retrieveJunction(node->getID());
                myTLSEditorParent->getViewNet()->getUndoList()->add(new GNEChange_TLS(junction, oldDefinition, false), true);
                myTLSEditorParent->getViewNet()->getUndoList()->add(new GNEChange_TLS(junction, myTLSEditorParent->myEditedDef, true), true);
            }
            myTLSEditorParent->myEditedDef = nullptr;
            myTLSEditorParent->getViewNet()->getUndoList()->end();
            myTLSEditorParent->cleanup();
            myTLSEditorParent->getViewNet()->updateViewNet();
        } else {
            onCmdDiscardChanges(nullptr, 0, nullptr);
        }
    }
    return 1;
}


long
GNETLSEditorFrame::TLSModifications::onCmdDiscardChanges(FXObject*, FXSelector, void*) {
    if (myTLSEditorParent->myTLSJunction->getCurrentJunction() != nullptr) {
        myTLSEditorParent->getViewNet()->getUndoList()->abortAllChangeGroups();
        myTLSEditorParent->cleanup();
        myTLSEditorParent->getViewNet()->updateViewNet();
    }
    return 1;
}


long
GNETLSEditorFrame::TLSModifications::onUpdModified(FXObject* o, FXSelector, void*) {
    if (myHaveModifications) {
        o->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), nullptr);
    } else {
        o->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), nullptr);
    }
    return 1;
}

// ---------------------------------------------------------------------------
// GNETLSEditorFrame::TLSFile - methods
// ---------------------------------------------------------------------------

GNETLSEditorFrame::TLSFile::TLSFile(GNETLSEditorFrame* TLSEditorParent) :
    MFXGroupBoxModule(TLSEditorParent, "TLS Program File"),
    myTLSEditorParent(TLSEditorParent) {
    FXHorizontalFrame* buttonsFrame = new FXHorizontalFrame(getCollapsableFrame(), GUIDesignAuxiliarHorizontalFrame);
    // create create tlDef button
    myLoadTLSProgramButton = new FXButton(buttonsFrame, "Load\t\tLoad TLS program from additional file", GUIIconSubSys::getIcon(GUIIcon::OPEN_CONFIG), this, MID_GNE_TLSFRAME_FILE_LOADPROGRAM, GUIDesignButton);
    // create create tlDef button
    mySaveTLSProgramButton = new FXButton(buttonsFrame, "Save\t\tSave TLS program to additional file", GUIIconSubSys::getIcon(GUIIcon::SAVE), this, MID_GNE_TLSFRAME_FILE_SAVEPROGRAM, GUIDesignButton);
    // show TLSFile
    show();
}


GNETLSEditorFrame::TLSFile::~TLSFile() {}


long
GNETLSEditorFrame::TLSFile::onCmdLoadTLSProgram(FXObject*, FXSelector, void*) {
    FXFileDialog opendialog(getCollapsableFrame(), "Load TLS Program");
    opendialog.setIcon(GUIIconSubSys::getIcon(GUIIcon::MODETLS));
    opendialog.setSelectMode(SELECTFILE_EXISTING);
    opendialog.setPatternList("XML files (*.xml,*.xml.gz)\nAll files (*)");
    if (gCurrentFolder.length() != 0) {
        opendialog.setDirectory(gCurrentFolder);
    }
    if (opendialog.execute()) {
        // run parser
        NBTrafficLightLogicCont tmpTLLCont;
        NIXMLTrafficLightsHandler tllHandler(tmpTLLCont, myTLSEditorParent->getViewNet()->getNet()->getEdgeCont(), true);
        tmpTLLCont.insert(myTLSEditorParent->myEditedDef);
        XMLSubSys::runParser(tllHandler, opendialog.getFilename().text());

        NBLoadedSUMOTLDef* newDefSameProgram = nullptr;
        std::set<NBLoadedSUMOTLDef*> newDefsOtherProgram;
        for (auto item : tmpTLLCont.getPrograms(myTLSEditorParent->myEditedDef->getID())) {
            if (item.second != myTLSEditorParent->myEditedDef) {
                NBLoadedSUMOTLDef* sdef = dynamic_cast<NBLoadedSUMOTLDef*>(item.second);
                if (item.first == myTLSEditorParent->myEditedDef->getProgramID()) {
                    newDefSameProgram = sdef;
                } else {
                    newDefsOtherProgram.insert(sdef);
                }
            }
        }
        const int newPrograms = (int)newDefsOtherProgram.size();
        if (newPrograms > 0 || newDefSameProgram != nullptr) {
            std::vector<NBNode*> nodes = myTLSEditorParent->myEditedDef->getNodes();
            for (auto newProg : newDefsOtherProgram) {
                for (auto it_node : nodes) {
                    GNEJunction* junction = myTLSEditorParent->getViewNet()->getNet()->getAttributeCarriers()->retrieveJunction(it_node->getID());
                    myTLSEditorParent->getViewNet()->getUndoList()->add(new GNEChange_TLS(junction, newProg, true), true);
                }
            }
            if (newPrograms > 0) {
                WRITE_MESSAGE("Loaded " + toString(newPrograms) + " new programs for tlLogic '" + myTLSEditorParent->myEditedDef->getID() + "'");
            }
            if (newDefSameProgram != nullptr) {
                // replace old program when loading the same program ID
                myTLSEditorParent->myEditedDef = newDefSameProgram;
                WRITE_MESSAGE("Updated program '" + newDefSameProgram->getProgramID() +  "' for tlLogic '" + myTLSEditorParent->myEditedDef->getID() + "'");
            }
        } else {
            if (tllHandler.getSeenIDs().count(myTLSEditorParent->myEditedDef->getID()) == 0)  {
                myTLSEditorParent->getViewNet()->setStatusBarText("No programs found for traffic light '" + myTLSEditorParent->myEditedDef->getID() + "'");
            }
        }

        // clean up temporary container to avoid deletion of defs when it's destruct is called
        for (NBTrafficLightDefinition* def : tmpTLLCont.getDefinitions()) {
            tmpTLLCont.removeProgram(def->getID(), def->getProgramID(), false);
        }

        myTLSEditorParent->myTLSPhases->initPhaseTable();
        myTLSEditorParent->myTLSModifications->setHaveModifications(true);
    }
    return 0;
}


long
GNETLSEditorFrame::TLSFile::onCmdSaveTLSProgram(FXObject*, FXSelector, void*) {
    FXString file = MFXUtils::getFilename2Write(this,
                    "Save TLS Program as", ".xml",
                    GUIIconSubSys::getIcon(GUIIcon::MODETLS),
                    gCurrentFolder);
    // check file
    if (file != "") {
        // add xml extension
        file = FileHelpers::addExtension(file.text(), ".xml").c_str();
        OutputDevice& device = OutputDevice::getDevice(file.text());
        // save program
        device.writeXMLHeader("additional", "additional_file.xsd");
        device.openTag(SUMO_TAG_TLLOGIC);
        device.writeAttr(SUMO_ATTR_ID, myTLSEditorParent->myEditedDef->getLogic()->getID());
        device.writeAttr(SUMO_ATTR_TYPE, myTLSEditorParent->myEditedDef->getLogic()->getType());
        device.writeAttr(SUMO_ATTR_PROGRAMID, myTLSEditorParent->myEditedDef->getLogic()->getProgramID());
        device.writeAttr(SUMO_ATTR_OFFSET, writeSUMOTime(myTLSEditorParent->myEditedDef->getLogic()->getOffset()));
        myTLSEditorParent->myEditedDef->writeParams(device);
        // write the phases
        const bool TLSActuated = (myTLSEditorParent->myEditedDef->getLogic()->getType() == TrafficLightType::ACTUATED);
        const bool TLSDelayBased = (myTLSEditorParent->myEditedDef->getLogic()->getType() == TrafficLightType::DELAYBASED);
        const bool TLSNEMA = (myTLSEditorParent->myEditedDef->getLogic()->getType() == TrafficLightType::NEMA);
        // write the phases
        const auto &phases = myTLSEditorParent->myEditedDef->getLogic()->getPhases();
        for (const auto& phase : phases) {
            device.openTag(SUMO_TAG_PHASE);
            device.writeAttr(SUMO_ATTR_DURATION, writeSUMOTime(phase.duration));
            device.writeAttr(SUMO_ATTR_STATE, phase.state);
            // write specific actuated parameters
            if (TLSActuated || TLSDelayBased) {
                if (phase.minDur != NBTrafficLightDefinition::UNSPECIFIED_DURATION) {
                    device.writeAttr(SUMO_ATTR_MINDURATION, writeSUMOTime(phase.minDur));
                }
                if (phase.maxDur != NBTrafficLightDefinition::UNSPECIFIED_DURATION) {
                    device.writeAttr(SUMO_ATTR_MAXDURATION, writeSUMOTime(phase.maxDur));
                }
                if (phase.earliestEnd != NBTrafficLightDefinition::UNSPECIFIED_DURATION) {
                    device.writeAttr(SUMO_ATTR_MAXDURATION, writeSUMOTime(phase.maxDur));
                }
                if (phase.earliestEnd != NBTrafficLightDefinition::UNSPECIFIED_DURATION) {
                    device.writeAttr(SUMO_ATTR_EARLIEST_END, writeSUMOTime(phase.maxDur));
                }
                if (phase.latestEnd != NBTrafficLightDefinition::UNSPECIFIED_DURATION) {
                    device.writeAttr(SUMO_ATTR_LATEST_END, writeSUMOTime(phase.maxDur));
                }
            }
            // write specific NEMA parameters
            if (TLSNEMA) {
                if (phase.minDur != NBTrafficLightDefinition::UNSPECIFIED_DURATION) {
                    device.writeAttr(SUMO_ATTR_MINDURATION, writeSUMOTime(phase.minDur));
                }
                if (phase.maxDur != NBTrafficLightDefinition::UNSPECIFIED_DURATION) {
                    device.writeAttr(SUMO_ATTR_MAXDURATION, writeSUMOTime(phase.maxDur));
                }
                if (phase.vehExt != NBTrafficLightDefinition::UNSPECIFIED_DURATION) {
                    device.writeAttr(SUMO_ATTR_MINDURATION, writeSUMOTime(phase.vehExt));
                }
                if (phase.red != NBTrafficLightDefinition::UNSPECIFIED_DURATION) {
                    device.writeAttr(SUMO_ATTR_MAXDURATION, writeSUMOTime(phase.red));
                }
                if (phase.yellow != NBTrafficLightDefinition::UNSPECIFIED_DURATION) {
                    device.writeAttr(SUMO_ATTR_MAXDURATION, writeSUMOTime(phase.yellow));
                }
            }
            device.closeTag();
        }
        device.close();
    }
    return 1;
}


std::string
GNETLSEditorFrame::TLSFile::writeSUMOTime(SUMOTime steps) {
    const double time = STEPS2TIME(steps);
    if (time == std::floor(time)) {
        return toString(int(time));
    } else {
        return toString(time);
    }
}


long
GNETLSEditorFrame::TLSFile::onUpdNeedsDef(FXObject* o, FXSelector, void*) {
    if (myTLSEditorParent->myTLSAttributes->getNumberOfTLSDefinitions() > 0) {
        o->handle(getCollapsableFrame(), FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), nullptr);
    } else {
        o->handle(getCollapsableFrame(), FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), nullptr);
    }
    return 1;
}

/****************************************************************************/
