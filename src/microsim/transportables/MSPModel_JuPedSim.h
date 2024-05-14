/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.dev/sumo
// Copyright (C) 2014-2024 German Aerospace Center (DLR) and others.
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
/// @file    MSPModel_JuPedSim.h
/// @author  Gregor Laemmel
/// @author  Benjamin Coueraud
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    Mon, 13 Jan 2014
///
// The pedestrian following model that can instantiate different pedestrian models
// that come with the JuPedSim third-party simulation framework.
/****************************************************************************/
#pragma once
#include <config.h>
#include <vector>
#include <map>
#include <geos_c.h>
#include <jupedsim/jupedsim.h>
#include "microsim/MSNet.h"
#include "MSPModel.h"


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSPModel_JuPedSim
 * @brief A pedestrian following model that acts as a proxy for pedestrian models
 * provided by the JuPedSim third-party simulation framework.
 */
class MSPModel_JuPedSim : public MSPModel {
public:
    MSPModel_JuPedSim(const OptionsCont& oc, MSNet* net);
    ~MSPModel_JuPedSim();

    MSTransportableStateAdapter* add(MSTransportable* person, MSStageMoving* stage, SUMOTime now) override;
    void remove(MSTransportableStateAdapter* state) override;
    SUMOTime execute(SUMOTime time);

    bool usingInternalLanes() override;
    bool usingShortcuts() override {
        return myHaveAdditionalWalkableAreas;
    }
    void registerArrived();
    int getActiveNumber() override;
    void clearState() override;

    class Event : public Command {
    public:
        explicit Event(MSPModel_JuPedSim* model)
            : myJPSModel(model) { }
        SUMOTime execute(SUMOTime currentTime) override {
            return myJPSModel->execute(currentTime);
        }

    private:
        MSPModel_JuPedSim* myJPSModel;
    };

private:
    /**
    * @class PState
    * @brief Holds pedestrian state and performs updates
    */
    class PState : public MSTransportableStateAdapter {
    public:
        PState(MSPerson* person, MSStageMoving* stage, JPS_JourneyId journeyId, JPS_StageId stageId, const std::vector<std::pair<Position, double> >& waypoints);
        ~PState() override;

        void reinit(MSStageMoving* stage, JPS_JourneyId journeyId, JPS_StageId stageId, const std::vector<std::pair<Position, double> >& waypoints);

        Position getPosition(const MSStageMoving& stage, SUMOTime now) const override;
        void setPosition(double x, double y);

        Position getPreviousPosition() const;
        void setPreviousPosition(Position previousPosition);

        double getAngle(const MSStageMoving& stage, SUMOTime now) const override;
        void setAngle(double angle);

        MSStageMoving* getStage() const;
        void setStage(MSStageMoving* const stage);

        MSPerson* getPerson() const;

        void setLanePosition(double lanePosition);
        double getEdgePos(const MSStageMoving& stage, SUMOTime now) const override;
        int getDirection(const MSStageMoving& stage, SUMOTime now) const override;
        SUMOTime getWaitingTime(const MSStageMoving& stage, SUMOTime now) const override;
        double getSpeed(const MSStageMoving& stage) const override;
        const MSEdge* getNextEdge(const MSStageMoving& stage) const override;
        const std::pair<Position, double>& getNextWaypoint() const;
        JPS_AgentId getAgentId() const;

        /// @brief whether the transportable has finished walking
        bool isFinished() const override {
            return myWaypoints.empty();
        }

        void setAgentId(JPS_AgentId id) {
            myAgentId = id;
            myWaitingToEnter = false;
        }

        JPS_JourneyId getJourneyId() const {
            return myJourneyId;
        }

        /// @brief first stage of the journey
        JPS_StageId getStageId() const {
            return myStageId;
        }

        bool isWaitingToEnter() const {
            return myWaitingToEnter;
        }

        bool advanceNextWaypoint() {
            myWaypoints.erase(myWaypoints.begin());
            return myWaypoints.empty();
        }

    private:
        MSPerson* myPerson;
        MSStageMoving* myStage;
        /// @brief id of the journey, needed for modifying it
        JPS_JourneyId myJourneyId;
        JPS_StageId myStageId;
        std::vector<std::pair<Position, double> > myWaypoints;
        JPS_AgentId myAgentId;
        Position myPosition;
        Position myPreviousPosition; // Will be initialized to zero automatically.
        double myAngle;
        double myLanePosition;
        /// @brief whether the pedestrian is waiting to start its walk
        bool myWaitingToEnter;
    };

    /// @brief The network on which the simulation runs.
    MSNet* const myNetwork;

    /// @brief The shape container used to add polygons to the rendering pipeline.
    ShapeContainer& myShapeContainer;

    /// @brief Timestep used in the JuPedSim simulator.
    const SUMOTime myJPSDeltaT;

    /// @brief Threshold to decide if a pedestrian has ended its journey or not.
    const double myExitTolerance;

    int myNumActivePedestrians = 0;
    std::vector<PState*> myPedestrianStates;

    /// @brief The GEOS polygon containing all computed connected components of the pedestrian network.
    GEOSGeometry* myGEOSPedestrianNetwork; // Kept because the largest component refers to it.

    /// @brief The GEOS polygon representing the largest (by area) connected component of the pedestrian network.
    const GEOSGeometry* myGEOSPedestrianNetworkLargestComponent;

    bool myHaveAdditionalWalkableAreas;

    /// @brief The JPS polygon representing the largest connected component of the pedestrian network.
    JPS_Geometry myJPSGeometry; // Kept because of dynamic geometry switching and JPS_Simulation object.

    /// @brief The JPS polygon representing the largest connected component plus carriages and ramps.
    JPS_Geometry myJPSGeometryWithTrainsAndRamps;
    JPS_OperationalModel myJPSModel;
    JPS_Simulation myJPSSimulation;

    /// @brief Structure that keeps data related to vanishing areas (and other types of areas).
    struct AreaData {
        const std::string id;
        const std::string areaType;
        const std::vector<JPS_Point> areaBoundary;
        const Parameterised::Map params;

        /// @brief The last time a pedestrian was removed in a vanishing area.
        SUMOTime lastRemovalTime;
    };

    /// @brief Array of special areas.
    std::vector<AreaData> myAreas;

    /// @brief Array of stopped trains, used to detect whether to add carriages and ramps to the geometry.
    std::vector<SUMOTrafficObject::NumericalID> myAllStoppedTrainIDs;

    static const int GEOS_QUADRANT_SEGMENTS;
    static const double GEOS_MITRE_LIMIT;
    static const double GEOS_MIN_AREA;
    static const double GEOS_BUFFERED_SEGMENT_WIDTH;
    static const double CARRIAGE_RAMP_LENGTH;
    static const RGBColor PEDESTRIAN_NETWORK_COLOR;
    static const RGBColor PEDESTRIAN_NETWORK_CARRIAGES_AND_RAMPS_COLOR;
    static const std::string PEDESTRIAN_NETWORK_ID;
    static const std::string PEDESTRIAN_NETWORK_CARRIAGES_AND_RAMPS_ID;

    void initialize(const OptionsCont& oc);
    void tryPedestrianInsertion(PState* state, const Position& p);
    bool addWaypoint(JPS_JourneyDescription journey, JPS_StageId& predecessor, const Position& point, const std::string& agentID, const double radius);
    static MSLane* getNextPedestrianLane(const MSLane* const currentLane);
    static const Position& getAnchor(const MSLane* const lane, const MSEdge* const edge, MSEdgeVector incoming);
    static const MSEdgeVector getAdjacentEdgesOfEdge(const MSEdge* const edge);
    static const MSEdge* getWalkingAreaInbetween(const MSEdge* const edge, const MSEdge* const otherEdge);
    static GEOSGeometry* createGeometryFromCenterLine(PositionVector centerLine, double width, int capStyle);
    static GEOSGeometry* createGeometryFromShape(PositionVector shape, std::string junctionID = std::string(""), std::string shapeID = std::string(""), bool isInternalShape = false);
    static GEOSGeometry* createGeometryFromAnchors(const Position& anchor, const MSLane* const lane, const Position& otherAnchor, const MSLane* const otherLane);
    GEOSGeometry* buildPedestrianNetwork(MSNet* network);
    static GEOSCoordSequence* convertToGEOSPoints(PositionVector shape);
    static std::vector<JPS_Point> convertToJPSPoints(const GEOSGeometry* geometry);
    static PositionVector convertToSUMOPoints(const GEOSGeometry* geometry);
    static double getLinearRingArea(const GEOSGeometry* linearRing);
    void removePolygonFromDrawing(const std::string& polygonId);
    void preparePolygonForDrawing(const GEOSGeometry* polygon, const std::string& polygonId, const RGBColor& color);
    static const GEOSGeometry* getLargestComponent(const GEOSGeometry* polygon, int& nbrComponents, double& maxArea, double& totalArea);
    static JPS_Geometry buildJPSGeometryFromGEOSGeometry(const GEOSGeometry* polygon);
    static void dumpGeometry(const GEOSGeometry* polygon, const std::string& filename, bool useGeoCoordinates = false);
    static double getRadius(const MSVehicleType& vehType);
};
