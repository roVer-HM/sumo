/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.dev/sumo
// Copyright (C) 2004-2025 German Aerospace Center (DLR) and others.
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
/// @file    MFXRealSpinner.h
/// @author  Jakob Erdmann
/// @date    2022-02-25
///
// Thin wrapper around FXRealSpinner to modify key interception
/****************************************************************************/
#pragma once
#include <config.h>

#include "fxheader.h"

/**
 * @class MFXRealSpinner
 */
class MFXRealSpinner : public FXRealSpinner {
    /// @brief fox declaration
    FXDECLARE(MFXRealSpinner)

public:
    /// @brief constructor (Very similar to the FXButton constructor)
    MFXRealSpinner(FXComposite* p, FXint cols, FXObject* tgt = NULL, FXSelector sel = 0, FXuint opts = REALSPIN_NORMAL,
                   FXint x = 0, FXint y = 0, FXint w = 0, FXint h = 0, FXint pl = DEFAULT_PAD, FXint pr = DEFAULT_PAD, FXint pt = DEFAULT_PAD, FXint pb = DEFAULT_PAD);

    /// @brief destructor (Called automatically)
    ~MFXRealSpinner();

    /// @brief called when this MFXRealSpinner is updated
    long onKeyPress(FXObject* o, FXSelector sel, void* data);
    /// @}

protected:
    FOX_CONSTRUCTOR(MFXRealSpinner)

};
