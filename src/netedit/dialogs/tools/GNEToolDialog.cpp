/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2001-2023 German Aerospace Center (DLR) and others.
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
/// @file    GNEToolDialog.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Jun 2022
///
// Abstract dialog for tools
/****************************************************************************/

#include <netedit/GNEApplicationWindow.h>
#include <utils/gui/div/GUIDesigns.h>
#include <utils/handlers/TemplateHandler.h>

#include "GNEToolDialog.h"


// ===========================================================================
// FOX callback mapping
// ===========================================================================

FXDEFMAP(GNEToolDialog) GNEToolDialogMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_BUTTON_RUN,     GNEToolDialog::onCmdRun),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_BUTTON_CANCEL,  GNEToolDialog::onCmdCancel),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_BUTTON_RESET,   GNEToolDialog::onCmdReset),
};

// Object implementation
FXIMPLEMENT(GNEToolDialog, FXTopWindow, GNEToolDialogMap, ARRAYNUMBER(GNEToolDialogMap))

// ============================================-===============================
// member method definitions
// ===========================================================================

GNEToolDialog::GNEToolDialog(GNEApplicationWindow* GNEApp, const std::string& name, const std::string& templateToolStr) :
    FXTopWindow(GNEApp->getApp(), name.c_str(), GUIIconSubSys::getIcon(GUIIcon::EMPTY), 
        GUIIconSubSys::getIcon(GUIIcon::EMPTY), GUIDesignDialogBoxExplicit(400, 400)),
    myGNEApp(GNEApp) {
    // parse tool options
    TemplateHandler::parseTemplate(myToolsOptions, templateToolStr);
    // create main frame
    FXVerticalFrame* mainFrame = new FXVerticalFrame(this, GUIDesignAuxiliarFrame);
    // build horizontalFrame for content
    myContentFrame = new FXVerticalFrame(mainFrame, GUIDesignContentsFrame);
    // build arguments
    buildArguments();
    // add separator
    new FXSeparator(mainFrame);
    // create buttons centered
    FXHorizontalFrame* buttonsFrame = new FXHorizontalFrame(mainFrame, GUIDesignHorizontalFrame);
    new FXHorizontalFrame(buttonsFrame, GUIDesignAuxiliarHorizontalFrame);
    new FXButton(buttonsFrame, (TL("Run") + std::string("\t\t") + TL("close accepting changes")).c_str(),  GUIIconSubSys::getIcon(GUIIcon::ACCEPT), this, MID_GNE_BUTTON_RUN, GUIDesignButtonAccept);
    new FXButton(buttonsFrame, (TL("Cancel") + std::string("\t\t") + TL("close discarding changes")).c_str(), GUIIconSubSys::getIcon(GUIIcon::CANCEL), this, MID_GNE_BUTTON_CANCEL, GUIDesignButtonCancel);
    new FXButton(buttonsFrame, (TL("Reset") + std::string("\t\t") + TL("reset to previous values")).c_str(),  GUIIconSubSys::getIcon(GUIIcon::RESET),  this, MID_GNE_BUTTON_RESET,  GUIDesignButtonReset);
    new FXHorizontalFrame(buttonsFrame, GUIDesignAuxiliarHorizontalFrame);
}


GNEToolDialog::~GNEToolDialog() {}


void
GNEToolDialog::openToolDialog() {
    // show dialog
    show(PLACEMENT_SCREEN);
    // open as modal dialog (will block all windows until stop() or stopModal() is called)
    myGNEApp->getApp()->runModalFor(this);
}


void
GNEToolDialog::hideToolDialog() {
}


bool
GNEToolDialog::shown() const {
    return FXWindow::shown();
}


FXVerticalFrame*
GNEToolDialog::getContentFrame() const {
    return myContentFrame;
}


long
GNEToolDialog::onCmdRun(FXObject*, FXSelector, void*) {
    // RUN

    // stop modal
    myGNEApp->getApp()->stopModal(this);
    // hide dialog
    hide();
    return 1;
}


long
GNEToolDialog::onCmdCancel(FXObject*, FXSelector, void*) {
    // stop modal
    myGNEApp->getApp()->stopModal(this);
    // hide dialog
    hide();
    return 1;
}


long
GNEToolDialog::onCmdReset(FXObject*, FXSelector, void*) {
    // iterate over all arguments and reset values
    for (const auto& argument : myArguments) {
        argument->reset();
    }
    return 1;
}


FXint
GNEToolDialog::openAsModalDialog(FXuint placement) {
    // create Dialog
    create();
    // show in the given position
    show(placement);
    // refresh APP
    getApp()->refresh();
    // open as modal dialog (will block all windows until stop() or stopModal() is called)
    return getApp()->runModalFor(this);
}


void
GNEToolDialog::buildArguments() {
    int i = 4;
    // iterate over options
    for (const auto &option : myToolsOptions) {
        // build argument
        myArguments.push_back(new GNEToolDialogElements::FileNameArgument(this, option.first, option.second));
        i += 2;
    }
    setHeight(GUIDesignHeight * i);
}

/****************************************************************************/
