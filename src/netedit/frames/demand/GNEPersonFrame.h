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
/// @file    GNEPersonFrame.h
/// @author  Pablo Alvarez Lopez
/// @date    May 2019
///
// The Widget for add person elements
/****************************************************************************/
#pragma once
#include <config.h>

#include <netedit/frames/GNEFrame.h>

// ===========================================================================
// class declaration
// ===========================================================================

class GNEAttributesEditor;
class GNEDemandElementSelector;
class GNEPlanCreator;
class GNEPlanCreatorLegend;
class GNEPlanSelector;
class GNETagSelector;

// ===========================================================================
// class definitions
// ===========================================================================

class GNEPersonFrame : public GNEFrame {

public:
    /**@brief Constructor
     * @brief viewParent GNEViewParent in which this GNEFrame is placed
     * @brief viewNet viewNet that uses this GNEFrame
     */
    GNEPersonFrame(GNEViewParent* viewParent, GNEViewNet* viewNet);

    /// @brief Destructor
    ~GNEPersonFrame();

    /// @brief show Frame
    void show();

    /// @brief hide Frame
    void hide();

    /**@brief add vehicle element
     * @param viewObjects collection of objects under cursor after click over view
     * @return true if vehicle was successfully added
     */
    bool addPerson(const GNEViewNetHelper::ViewObjectsSelector& viewObjects);

    /// @brief get plan creator module
    GNEPlanCreator* getPlanCreator() const;

    /// @brief get Type selectors
    GNEDemandElementSelector* getTypeSelector() const;

    /// @brief get personPlan selector
    GNEPlanSelector* getPlanSelector() const;

    /// @brief get attributes creator
    GNEAttributesEditor* getPersonAttributesEditor() const;

protected:
    /// @brief Tag selected in GNETagSelector
    void tagSelected();

    /// @brief selected demand element in DemandElementSelector
    void demandElementSelected();

    /// @brief create path
    bool createPath(const bool useLastRoute);

private:
    /// @brief person base object
    CommonXMLStructure::SumoBaseObject* myPersonBaseObject = nullptr;

    /// @brief person tag selector (used to select diffent kind of persons)
    GNETagSelector* myPersonTagSelector = nullptr;

    /// @brief Person Type selectors
    GNEDemandElementSelector* myTypeSelector = nullptr;

    /// @brief personPlan selector
    GNEPlanSelector* myPlanSelector = nullptr;

    /// @brief person attributes editor
    GNEAttributesEditor* myPersonAttributesEditor = nullptr;

    /// @brief person plan attributes editor
    GNEAttributesEditor* myPersonPlanAttributesEditor = nullptr;

    /// @brief plan creator
    GNEPlanCreator* myPlanCreator = nullptr;

    /// @brief plan creator legend
    GNEPlanCreatorLegend* myPlanCreatorLegend = nullptr;

    /// @brief build person and return it (note: function includes a call to begin(...), but NOT a call to end(...))
    GNEDemandElement* buildPerson();
};
