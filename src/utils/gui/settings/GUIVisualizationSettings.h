/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2001-2019 German Aerospace Center (DLR) and others.
// This program and the accompanying materials
// are made available under the terms of the Eclipse Public License v2.0
// which accompanies this distribution, and is available at
// http://www.eclipse.org/legal/epl-v20.html
// SPDX-License-Identifier: EPL-2.0
/****************************************************************************/
/// @file    GUIVisualizationSettings.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Sept 2002
/// @version $Id$
///
// Stores the information about how to visualize structures
/****************************************************************************/
#ifndef GUIVisualizationSettings_h
#define GUIVisualizationSettings_h


// ===========================================================================
// included modules
// ===========================================================================
#include <config.h>

#include <string>
#include <vector>
#include <map>
#include <utils/common/RGBColor.h>
#include <utils/common/ToString.h>
#include "GUIPropertySchemeStorage.h"


// ===========================================================================
// class declarations
// ===========================================================================
class BaseSchemeInfoSource;
class OutputDevice;
class GUIVisualizationSettings;
class GUIGlObject;


// ===========================================================================
// class definitions
// ===========================================================================

// cannot declare this as inner class because it needs to be used in forward
// declaration (@todo fix inclusion order by removing references to guisim!)
struct GUIVisualizationTextSettings {

    /// @brief constructor
    GUIVisualizationTextSettings(bool _show, double _size, RGBColor _color, RGBColor _bgColor = RGBColor(128, 0, 0, 0), bool _constSize = true);

    /// @brief equality comparator
    bool operator==(const GUIVisualizationTextSettings& other);

    /// @brief inequality comparator
    bool operator!=(const GUIVisualizationTextSettings& other);

    /// @brief print values in output device
    void print(OutputDevice& dev, const std::string& name) const;

    /// @brief get scale size
    double scaledSize(double scale, double constFactor = 0.1) const;

    /// @brief flag show
    bool show;

    /// @brief text size
    double size;

    /// @brief text color
    RGBColor color;

    /// @brief background text color
    RGBColor bgColor;

    /// @brif flag to avoid size changes
    bool constSize;
};


/// @brief struct for size settings
struct GUIVisualizationSizeSettings {

    /// @brief constructor
    GUIVisualizationSizeSettings(double _minSize, double _exaggeration = 1.0, bool _constantSize = false, bool _constantSizeSelected = false);

    /// @brief return the drawing size including exaggeration and constantSize values
    double getExaggeration(const GUIVisualizationSettings& s, const GUIGlObject* o, double factor = 20) const;

    /// @brief equality comparator
    bool operator==(const GUIVisualizationSizeSettings& other);

    /// @brief inequality comparator
    bool operator!=(const GUIVisualizationSizeSettings& other);

    /// @brief print values in output device
    void print(OutputDevice& dev, const std::string& name) const;

    /// @brief The minimum size to draw this object
    double minSize;

    /// @brief The size exaggeration (upscale)
    double exaggeration;

    /// @brief whether the object shall be drawn with constant size regardless of zoom
    bool constantSize;

    /// @brief whether only selected objects shall be drawn with constant
    bool constantSizeSelected;
};


/// @brief struct for color settings
struct GUIVisualizationColorSettings {

    /// @brief constructor
    GUIVisualizationColorSettings();

    /// @brief equality comparator
    bool operator==(const GUIVisualizationColorSettings& other);

    /// @brief inequality comparator
    bool operator!=(const GUIVisualizationColorSettings& other);

    /// @brief basic selection color
    RGBColor selectionColor;

    /// @brief edge selection color
    RGBColor selectedEdgeColor;

    /// @brief lane selection color
    RGBColor selectedLaneColor;

    /// @brief connection selection color
    RGBColor selectedConnectionColor;

    /// @brief prohibition selection color
    RGBColor selectedProhibitionColor;

    /// @brief crossings selection color
    RGBColor selectedCrossingColor;

    /// @brief additional selection color (busStops, Detectors...)
    RGBColor selectedAdditionalColor;

    /// @brief route selection color (used for routes and vehicle stops)
    RGBColor selectedRouteColor;

    /// @brief vehicle selection color
    RGBColor selectedVehicleColor;

    /// @brief person  selection color
    RGBColor selectedPersonColor;

    /// @brief person plan selection color (Rides, Walks, personStops...)
    RGBColor selectedPersonPlanColor;

    /// @brief color for child connections between parents and child elements
    static const RGBColor childConnections;

    /// @brief color for crossings
    static const RGBColor crossing;

    /// @brief color for priority crossing
    static const RGBColor crossingPriority;

    /// @brief color for invalid crossing
    static const RGBColor crossingInvalid;

    /// @brief color for busStops
    static const RGBColor busStop;

    /// @brief color for busStops signs
    static const RGBColor busStop_sign;

    /// @brief color for containerStops
    static const RGBColor containerStop;

    /// @brief color for containerStop signs
    static const RGBColor containerStop_sign;

    /// @brief color for chargingStations
    static const RGBColor chargingStation;

    /// @brief color for chargingStation sign
    static const RGBColor chargingStation_sign;

    /// @brief color for chargingStation during charging
    static const RGBColor chargingStation_charge;

    /// @brief color for parkingAreas
    static const RGBColor parkingArea;

    /// @brief color for parkingArea sign
    static const RGBColor parkingAreaSign;

    /// @brief color for parkingArea sign
    static const RGBColor parkingSpace;

    /// @brief color for parkingArea innen
    static const RGBColor parkingSpaceInnen;

    /// @brief color for E1 detectors
    static const RGBColor E1;

    /// @brief color for E1 Instant detectors
    static const RGBColor E1Instant;

    /// @brief color for E2 detectors
    static const RGBColor E2;

    /// @brief color for Entrys
    static const RGBColor E3Entry;

    /// @brief color for Exits
    static const RGBColor E3Exit;

    /// @brief color for Calibrators
    static const RGBColor calibrator;

    /// @brief color for route probes
    static const RGBColor routeProbe;

    /// @brief color for vaporizers
    static const RGBColor vaporizer;

    /// @brief color for Stops
    static const RGBColor stops;
        
    /// @brief color for vehicle trips
    static const RGBColor vehicleTrips;

    /// @brief color for personStops
    static const RGBColor personStops;

    /// @brief color for personStops
    static const RGBColor personTrip;

    /// @brief color for walks
    static const RGBColor walk;

    /// @brief color for rides
    static const RGBColor ride;
};


/// @brief struct for width settings
struct GUIVisualizationWidthSettings {

    /// @brief width of dotted contours (note: must be float)
    static const float dottedContour;

    /// @brief lenght of dotted contour segments
    static const double dottedContourSegmentLenght;

    /// @brief width for routes
    static const double route;

    /// @brief width for trips
    static const double trip;

    /// @brief width for person trips
    static const double personTrip;

    /// @brief width for walks
    static const double walk;

    /// @brief width for rides
    static const double ride;
};


/// @brief struct for detail settings
struct GUIVisualizationDetailSettings {

    /// @brief draw connections in demand mode
    static const double connectionsDemandMode;

    /// @brief details for lane textures
    static const double laneTextures;

    /// @brief lock icons
    static const double lockIcon;

    /// @brief details for additional textures
    static const double additionalTextures;

    /// @brief details for Geometry Points
    static const double geometryPointsDetails;

    /// @brief details for Geometry Points Texts
    static const double geometryPointsText;

    /// @brief details for stopping places
    static const double stoppingPlaceDetails;

    /// @brief details for stopping place texts
    static const double stoppingPlaceText;

    /// @brief details for detectors
    static const double detectorDetails;

    /// @brief details for detector texts
    static const double detectorText;

    /// @brief details for calibrator text
    static const double calibratorText;

    /// @brief details for stops
    static const double stopsDetails;

    /// @brief details for stop texts
    static const double stopsText;

    /// @brief details for draw vechicles as triangles
    static const double vehicleTriangles;

    /// @brief details for draw vechicles as boxes
    static const double vehicleBoxes;

    /// @brief details for draw vechicles as shapes
    static const double vehicleShapes;

    /// @brief details for draw vechicles as triangles
    static const double personTriangles;

    /// @brief details for draw vechicles as circles
    static const double personCircles;

    /// @brief details for draw vechicles as person shapes
    static const double personShapes;
};


/**
 * @class GUIVisualizationSettings
 * @brief Stores the information about how to visualize structures
 */
class GUIVisualizationSettings {

public:
    /// @brief constructor
    GUIVisualizationSettings(bool _netedit = false);

    /// @brief init default settings
    void initNeteditDefaults();
    void initSumoGuiDefaults();

    /** @brief Writes the settings into an output device
     * @param[in] dev The device to write the settings into
     */
    void save(OutputDevice& dev) const;

    /** @brief Returns the number of the active lane (edge) coloring schme
     * @return number of the active scheme
     */
    int getLaneEdgeMode() const;

    /** @brief Returns the number of the active lane (edge) scaling schme
     * @return number of the active scheme
     */
    int getLaneEdgeScaleMode() const;

    /** @brief Returns the current lane (edge) coloring schme
     * @return current scheme
     */
    GUIColorScheme& getLaneEdgeScheme();

    /** @brief Returns the current lane (edge) scaling schme
     * @return current scheme
     */
    GUIScaleScheme& getLaneEdgeScaleScheme();

    /// @brief Comparison operator
    bool operator==(const GUIVisualizationSettings& vs2);

    /// @brief map from LinkState to color constants
    static const RGBColor& getLinkColor(const LinkState& ls);

    /// @brief return an angle that is suitable for reading text aligned with the given angle (degrees)
    double getTextAngle(double objectAngle) const;

    /// @brief check if details can be drawn for the given GUIVisualizationDetailSettings and current scale and exxageration
    bool drawDetail(const double detail, const double exaggeration) const;

    /// @brief function to calculate circle resolution for all circles drawn in drawGL(...) functions
    const int getCircleResolution() const;

    /// @brief The name of this setting
    std::string name;

    /// @brief Whether the settings are for Netedit
    bool netedit;

    /// @brief The current view rotation angle
    double angle;

    /// @brief Information whether dithering shall be enabled
    bool dither;

    /// @brief Information whether frames-per-second should be drawn
    bool fps;

    /// @name Background visualization settings
    /// @{

    /// @brief The background color to use
    RGBColor backgroundColor;

    /// @brief Information whether a grid shall be shown
    bool showGrid;

    /// @brief Information about the grid spacings
    double gridXSize, gridYSize;
    /// @}


    /// @name lane visualization settings
    /// @{

    /// @brief The mesoscopic edge colorer
    GUIColorer edgeColorer;

    /// @brief The mesoscopic edge scaler
    GUIScaler edgeScaler;

    /// @brief this should be set at the same time as MSGlobals::gUseMesoSim
    static bool UseMesoSim;

    /// @brief The lane colorer
    GUIColorer laneColorer;

    /// @brief The lane scaler
    GUIScaler laneScaler;

    /// @brief Information whether lane borders shall be drawn
    bool laneShowBorders;

    /// @brief Information whether bicycle lane marking shall be drawn
    bool showBikeMarkings;

    /// @brief Information whether link textures (arrows) shall be drawn
    bool showLinkDecals;

    /// @brief Information whether link rules (colored bars) shall be drawn
    bool showLinkRules;

    /// @brief Information whether rails shall be drawn
    bool showRails;

    // Setting bundles for optional drawing names with size and color
    GUIVisualizationTextSettings edgeName, internalEdgeName, cwaEdgeName, streetName, edgeValue;

    /// @brief flag to show or hidde connectors
    bool hideConnectors;

    /// @brief The lane exaggeration (upscale thickness)
    double laneWidthExaggeration;

    /// @brief The minimum visual lane width for drawing
    double laneMinSize;

    /// @brief Whether to show direction indicators for lanes
    bool showLaneDirection;

    /// @brief Whether to show sublane boundaries
    bool showSublanes;

    /// @brief Whether to improve visualisation of superposed (rail) edges
    bool spreadSuperposed;

    /// @brief key for coloring by edge parameter
    std::string edgeParam, laneParam;

    /// @brief key for coloring by edgeData
    std::string edgeData;
    /// @}

    /// @name vehicle visualization settings
    /// @{

    /// @brief The vehicle colorer
    GUIColorer vehicleColorer;

    /// @brief The quality of vehicle drawing
    int vehicleQuality;

    /// @brief Information whether vehicle blinkers shall be drawn
    bool showBlinker;

    /// @brief Information whether the lane change preference shall be drawn
    bool drawLaneChangePreference;

    /// @brief Information whether the minimum gap shall be drawn
    bool drawMinGap;

    /// @brief Information whether the brake gap shall be drawn
    bool drawBrakeGap;

    /// @brief Information whether the communication range shall be drawn
    bool showBTRange;

    // Setting bundles for controling the size of the drawn vehicles
    GUIVisualizationSizeSettings vehicleSize;

    // Setting bundles for optional drawing vehicle names or color value
    GUIVisualizationTextSettings vehicleName, vehicleValue;

    /// @}


    /// @name person visualization settings
    /// @{

    /// @brief The person colorer
    GUIColorer personColorer;

    /// @brief The quality of person drawing
    int personQuality;

    // Setting bundles for controling the size of the drawn persons
    GUIVisualizationSizeSettings personSize;

    // Setting bundles for optional drawing person names
    GUIVisualizationTextSettings personName, personValue;
    /// @}


    /// @name container visualization settings
    /// @{

    /// @brief The container colorer
    GUIColorer containerColorer;

    /// @brief The quality of container drawing
    int containerQuality;

    // Setting bundles for controling the size of the drawn containers
    GUIVisualizationSizeSettings containerSize;

    // Setting bundles for optional drawing container names
    GUIVisualizationTextSettings containerName;
    /// @}


    /// @name junction visualization settings
    /// @{

    /// @brief The junction colorer
    GUIColorer junctionColorer;

    // Setting bundles for optional drawing junction names and indices
    GUIVisualizationTextSettings drawLinkTLIndex, drawLinkJunctionIndex, junctionName, internalJunctionName, tlsPhaseIndex;

    /// @brief Information whether lane-to-lane arrows shall be drawn
    bool showLane2Lane;
    /// @brief whether the shape of the junction should be drawn
    bool drawJunctionShape;
    /// @brief whether crosings and walkingareas shall be drawn
    bool drawCrossingsAndWalkingareas;
    // Setting bundles for controling the size of the drawn junction
    GUIVisualizationSizeSettings junctionSize;
    /// @}


    /// @name Additional structures visualization settings
    /// @{

    /// @brief The additional structures visualization scheme
    // @todo decouple addExageration for POIs, Polygons, Triggers etc
    int addMode;
    // Setting bundles for controling the size of additional items
    GUIVisualizationSizeSettings addSize;
    // Setting bundles for optional drawing additional names
    GUIVisualizationTextSettings addName;
    // Setting bundles for optional drawing additional full names
    GUIVisualizationTextSettings addFullName;
    /// @}


    /// @name shapes visualization settings
    /// @{

    /// @brief The POI colorer
    GUIColorer poiColorer;

    // Setting bundles for controling the size of the drawn POIs
    GUIVisualizationSizeSettings poiSize;

    // Setting bundles for optional drawing poi names
    GUIVisualizationTextSettings poiName;

    // Setting bundles for optional drawing poi types
    GUIVisualizationTextSettings poiType;

    /// @brief The polygon colorer
    GUIColorer polyColorer;

    // Setting bundles for controling the size of the drawn polygons
    GUIVisualizationSizeSettings polySize;

    // Setting bundles for optional drawing polygon names
    GUIVisualizationTextSettings polyName;

    // Setting bundles for optional drawing polygon types
    GUIVisualizationTextSettings polyType;
    /// @}

    /// @brief Information whether the size legend shall be drawn
    bool showSizeLegend;

    /// @brief Information whether the colo legend shall be drawn
    bool showColorLegend;

    /// @brief information about a lane's width (temporary, used for a single view)
    double scale;

    /// @brief whether the application is in gaming mode or not
    bool gaming;

    /// @brief enable or disable draw boundaries
    bool drawBoundaries;

    /// @brief the current selection scaling in NETEDIT (temporary)
    double selectionScale;

    /// @brief whether drawing is performed for the purpose of selecting objects
    bool drawForSelecting;
    
    /// @brief flag to force draw to selecting (see drawForSelecting)
    bool forceDrawForSelecting;

    /// @brief scheme names
    static const std::string SCHEME_NAME_EDGE_PARAM_NUMERICAL;
    static const std::string SCHEME_NAME_LANE_PARAM_NUMERICAL;
    static const std::string SCHEME_NAME_EDGEDATA_NUMERICAL;
    static const std::string SCHEME_NAME_SELECTION;
    static const std::string SCHEME_NAME_TYPE;

    /// @brief color settings
    GUIVisualizationColorSettings colorSettings;

    /// @brief width settings
    GUIVisualizationWidthSettings widthSettings;

    /// @brief detail settings
    GUIVisualizationDetailSettings detailSettings;
};


#endif

/****************************************************************************/

