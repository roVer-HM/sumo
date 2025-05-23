#ifdef SWIGPYTHON
// avoid warnings about keyword arguments with overloaded functions
#pragma SWIG nowarn=511
// avoid warnings about unknown base class std::runtime_error
#pragma SWIG nowarn=401

%naturalvar;
%rename(edge) Edge;
%rename(gui) GUI;
%rename(inductionloop) InductionLoop;
%rename(junction) Junction;
%rename(lane) Lane;
%rename(lanearea) LaneArea;
%rename(multientryexit) MultiEntryExit;
%rename(person) Person;
%rename(poi) POI;
%rename(polygon) Polygon;
%rename(route) Route;
%rename(simulation) Simulation;
%rename(trafficlight) TrafficLight;
%rename(vehicle) Vehicle;
%rename(vehicletype) VehicleType;
%rename(calibrator) Calibrator;
%rename(busstop) BusStop;
%rename(parkingarea) ParkingArea;
%rename(chargingstation) ChargingStation;
%rename(overheadwire) OverheadWire;
%rename(rerouter) Rerouter;
%rename(meandata) MeanData;
%rename(variablespeedsign) VariableSpeedSign;
%rename(routeprobe) RouteProbe;

/* There is currently no TraCIPosition used as input so this is only for future usage
%typemap(in) const libsumo::TraCIPosition& (libsumo::TraCIPosition pos) {
    const Py_ssize_t size = PySequence_Size($input);
    if (size == 2 || size == 3) {
        pos.x = PyFloat_AsDouble(PySequence_GetItem($input, 0));
        pos.y = PyFloat_AsDouble(PySequence_GetItem($input, 1));
        pos.z = (size == 3 ? PyFloat_AsDouble(PySequence_GetItem($input, 2)) : 0.);
    } else {
    // TODO error handling
    }
    $1 = &pos;
}
*/

%typemap(in) const libsumo::TraCIPositionVector& (libsumo::TraCIPositionVector shape) {
    const Py_ssize_t size = PySequence_Size($input);
    for (Py_ssize_t i = 0; i < size; i++) {
        PyObject* posTuple = PySequence_GetItem($input, i);
        const Py_ssize_t posSize = PySequence_Size(posTuple);
        libsumo::TraCIPosition pos;
        if (posSize == 2 || posSize == 3) {
            PyObject* item = PySequence_GetItem(posTuple, 0);
            pos.x = PyFloat_Check(item) ? PyFloat_AsDouble(item) : PyLong_AsDouble(item);
            item = PySequence_GetItem(posTuple, 1);
            pos.y = PyFloat_Check(item) ? PyFloat_AsDouble(item) : PyLong_AsDouble(item);
            pos.z = 0.;
            if (posSize == 3) {
                item = PySequence_GetItem(posTuple, 2);
                pos.z = PyFloat_Check(item) ? PyFloat_AsDouble(item) : PyLong_AsDouble(item);
            }
        } else {
        // TODO error handling
        }
        shape.value.push_back(pos);
    }
    $1 = &shape;
}

%typemap(in) const libsumo::TraCIColor& (libsumo::TraCIColor col) {
    const Py_ssize_t size = PySequence_Size($input);
    if (size == 3 || size == 4) {
        col.r = (unsigned char)PyLong_AsLong(PySequence_GetItem($input, 0));
        col.g = (unsigned char)PyLong_AsLong(PySequence_GetItem($input, 1));
        col.b = (unsigned char)PyLong_AsLong(PySequence_GetItem($input, 2));
        col.a = (unsigned char)(size == 4 ? PyLong_AsLong(PySequence_GetItem($input, 3)) : 255);
    } else {
    // TODO error handling
    }
    $1 = &col;
}

%typemap(in) const std::vector<int>& (std::vector<int> vars) {
    const Py_ssize_t size = PySequence_Size($input);
    for (Py_ssize_t i = 0; i < size; i++) {
        vars.push_back(PyLong_AsLong(PySequence_GetItem($input, i)));
    }
    $1 = &vars;
}

%{
#if PY_MAJOR_VERSION < 3
#define PyUnicodeCheck PyStringCheck
#define PyUnicode_AsUTF8 PyString_AsString
#endif
%}

%typemap(in) const libsumo::TraCIResults& (libsumo::TraCIResults parameters) {
    if (!PyDict_Check($input)) {
        PyErr_SetString(PyExc_TypeError, "Expecting a dict");
        SWIG_fail;
    }
    PyObject *key, *value;
    Py_ssize_t pos = 0;

    while (PyDict_Next($input, &pos, &key, &value)) {
        const int key_int = PyLong_AsLong(key);
        if (PyInt_Check(value)) {
            parameters[key_int] = std::make_shared<libsumo::TraCIInt>(PyInt_AsLong(value));
        } else if (PyFloat_Check(value)) {
            parameters[key_int] = std::make_shared<libsumo::TraCIDouble>(PyFloat_AsDouble(value));
        } else if (PyUnicode_Check(value)) {
            parameters[key_int] = std::make_shared<libsumo::TraCIString>(PyUnicode_AsUTF8(value));
        } else if (PySequence_Check(value)) {
            const Py_ssize_t size = PySequence_Size(value);
            if (size < 2 || !PyUnicode_Check(PySequence_GetItem(value, 0))) {
                PyErr_SetString(PyExc_TypeError, "Wrong parameter format");
                SWIG_fail;
            }
            const std::string format = PyUnicode_AsUTF8(PySequence_GetItem(value, 0));
            if (format == "b") {
                parameters[key_int] = std::make_shared<libsumo::TraCIInt>(PyInt_AsLong(PySequence_GetItem(value, 1)), libsumo::TYPE_BYTE);
            } else if (format == "B") {
                parameters[key_int] = std::make_shared<libsumo::TraCIInt>(PyInt_AsLong(PySequence_GetItem(value, 1)), libsumo::TYPE_UBYTE);
            } else if (format == "tru") {
                PyObject* pyRoadPos = PySequence_GetItem(value, 2);
                const std::string edge = PyUnicode_AsUTF8(PySequence_GetItem(pyRoadPos, 0));
                const double pos = PyFloat_AsDouble(PySequence_GetItem(pyRoadPos, 1));
                const int laneIndex = PyInt_AsLong(PySequence_GetItem(pyRoadPos, 2));
                parameters[key_int] = std::make_shared<libsumo::TraCIRoadPosition>(edge, pos, laneIndex);
            } else if (format == "tou") {
                PyObject* pyPos = PySequence_GetItem(value, 2);
                libsumo::TraCIPosition pos;
                pos.x = PyFloat_AsDouble(PySequence_GetItem(pyPos, 0));
                pos.y = PyFloat_AsDouble(PySequence_GetItem(pyPos, 1));
                parameters[key_int] = std::make_shared<libsumo::TraCIPosition>(pos);
            } else if (format == "tisb") {
                libsumo::TraCIStringDoublePairList wrap;
                wrap.value.emplace_back(std::pair<std::string, double>(PyUnicode_AsUTF8(PySequence_GetItem(value, 3)), (double)PyInt_AsLong(PySequence_GetItem(value, 2))));
                wrap.value.emplace_back(std::pair<std::string, double>("", (double)PyInt_AsLong(PySequence_GetItem(value, 4))));
                parameters[key_int] = std::make_shared<libsumo::TraCIStringDoublePairList>(wrap);
            } else if (format == "tds") {
                libsumo::TraCIStringDoublePairList wrap;
                wrap.value.emplace_back(std::pair<std::string, double>(PyUnicode_AsUTF8(PySequence_GetItem(value, 3)), PyFloat_AsDouble(PySequence_GetItem(value, 2))));
                parameters[key_int] = std::make_shared<libsumo::TraCIStringDoublePairList>(wrap);
            } else if (format == "tdddds") {
                libsumo::TraCIStringDoublePairList wrap;
                wrap.value.emplace_back(std::pair<std::string, double>("", PyFloat_AsDouble(PySequence_GetItem(value, 2))));
                wrap.value.emplace_back(std::pair<std::string, double>("", PyFloat_AsDouble(PySequence_GetItem(value, 3))));
                wrap.value.emplace_back(std::pair<std::string, double>("", PyFloat_AsDouble(PySequence_GetItem(value, 4))));
                wrap.value.emplace_back(std::pair<std::string, double>(PyUnicode_AsUTF8(PySequence_GetItem(value, 6)), PyFloat_AsDouble(PySequence_GetItem(value, 5))));
                parameters[key_int] = std::make_shared<libsumo::TraCIStringDoublePairList>(wrap);
            } else if (format == "tddds") {
                libsumo::TraCIStringDoublePairList wrap;
                wrap.value.emplace_back(std::pair<std::string, double>("", PyFloat_AsDouble(PySequence_GetItem(value, 2))));
                wrap.value.emplace_back(std::pair<std::string, double>("", PyFloat_AsDouble(PySequence_GetItem(value, 3))));
                wrap.value.emplace_back(std::pair<std::string, double>(PyUnicode_AsUTF8(PySequence_GetItem(value, 5)), PyFloat_AsDouble(PySequence_GetItem(value, 4))));
                parameters[key_int] = std::make_shared<libsumo::TraCIStringDoublePairList>(wrap);
            } else if (format == "tdd") {
                libsumo::TraCIStringDoublePairList wrap;
                wrap.value.emplace_back(std::pair<std::string, double>("", PyFloat_AsDouble(PySequence_GetItem(value, 2))));
                wrap.value.emplace_back(std::pair<std::string, double>("", PyFloat_AsDouble(PySequence_GetItem(value, 3))));
                parameters[key_int] = std::make_shared<libsumo::TraCIStringDoublePairList>(wrap);
            }
        } else {
            PyErr_SetString(PyExc_TypeError, "Unknown parameter format");
            SWIG_fail;
        }
    }
    $1 = &parameters;
}

%typemap(typecheck, precedence=SWIG_TYPECHECK_INTEGER) const std::vector<int>& {
    $1 = PySequence_Check($input) ? 1 : 0;
}

// this is just a workaround to ignore the Simulation::start _stdout argument
%typemap(in) void* {
    $1 = nullptr;
}


%{
#include <libsumo/TraCIDefs.h>

static PyObject* parseConnectionList(const std::vector<libsumo::TraCIConnection>& connList) {
    PyObject* result = PyTuple_New(connList.size());
    int index = 0;
    for (auto iter = connList.begin(); iter != connList.end(); ++iter) {
        PyTuple_SetItem(result, index++, Py_BuildValue("(sNNNsssd)",
                                                        iter->approachedLane.c_str(),
                                                        PyBool_FromLong(iter->hasPrio),
                                                        PyBool_FromLong(iter->isOpen),
                                                        PyBool_FromLong(iter->hasFoe),
                                                        iter->approachedInternal.c_str(),
                                                        iter->state.c_str(),
                                                        iter->direction.c_str(),
                                                        iter->length));
    }
    return result;
}

static PyObject* parseSubscriptionMap(const std::map<int, std::shared_ptr<libsumo::TraCIResult> >& subMap) {
    PyObject* result = PyDict_New();
    for (auto iter = subMap.begin(); iter != subMap.end(); ++iter) {
        const libsumo::TraCIResult* const traciVal = iter->second.get();
        PyObject* pyVal = nullptr;
        const libsumo::TraCIDouble* const theDouble = dynamic_cast<const libsumo::TraCIDouble*>(traciVal);
        if (theDouble != nullptr) {
            pyVal = PyFloat_FromDouble(theDouble->value);
        }
        if (pyVal == nullptr) {
            const libsumo::TraCIInt* const theInt = dynamic_cast<const libsumo::TraCIInt*>(traciVal);
            if (theInt != nullptr) {
                pyVal = PyInt_FromLong(theInt->value);
            }
        }
        if (pyVal == nullptr) {
            const libsumo::TraCIString* const theString = dynamic_cast<const libsumo::TraCIString*>(traciVal);
            if (theString != nullptr) {
                pyVal = PyUnicode_FromString(theString->value.c_str());
            }
        }
        if (pyVal == nullptr) {
            const libsumo::TraCIStringList* const theStringList = dynamic_cast<const libsumo::TraCIStringList*>(traciVal);
            if (theStringList != nullptr) {
                const Py_ssize_t size = theStringList->value.size();
                pyVal = PyTuple_New(size);
                for (Py_ssize_t i = 0; i < size; i++) {
                    PyTuple_SetItem(pyVal, i, PyUnicode_FromString(theStringList->value[i].c_str()));
                }
            }
        }
        if (pyVal == nullptr) {
            const libsumo::TraCIDoubleList* const theDoubleList = dynamic_cast<const libsumo::TraCIDoubleList*>(traciVal);
            if (theDoubleList != nullptr) {
                const Py_ssize_t size = theDoubleList->value.size();
                pyVal = PyTuple_New(size);
                for (Py_ssize_t i = 0; i < size; i++) {
                    PyTuple_SetItem(pyVal, i, PyFloat_FromDouble(theDoubleList->value[i]));
                }
            }
        }
        if (pyVal == nullptr) {
            const libsumo::TraCIPosition* const thePosition = dynamic_cast<const libsumo::TraCIPosition*>(traciVal);
            if (thePosition != nullptr) {
                if (thePosition->z != libsumo::INVALID_DOUBLE_VALUE) {
                    pyVal = Py_BuildValue("(ddd)", thePosition->x, thePosition->y, thePosition->z);
                } else {
                    pyVal = Py_BuildValue("(dd)", thePosition->x, thePosition->y);
                }
            }
        }
        if (pyVal == nullptr) {
            const libsumo::TraCIRoadPosition* const theRoadPosition = dynamic_cast<const libsumo::TraCIRoadPosition*>(traciVal);
            if (theRoadPosition != nullptr) {
                if (theRoadPosition->laneIndex != libsumo::INVALID_INT_VALUE) {
                    pyVal = Py_BuildValue("(sdi)", theRoadPosition->edgeID.c_str(), theRoadPosition->pos, theRoadPosition->laneIndex);
                } else {
                    pyVal = Py_BuildValue("(sd)", theRoadPosition->edgeID.c_str(), theRoadPosition->pos);
                }
            }
        }
        if (pyVal == nullptr) {
            const libsumo::TraCIConnectionVectorWrapped* const theConnectionList = dynamic_cast<const libsumo::TraCIConnectionVectorWrapped*>(traciVal);
            if (theConnectionList != nullptr) {
                pyVal = parseConnectionList(theConnectionList->value);
            }
        }
        if (pyVal == nullptr) {
            pyVal = SWIG_NewPointerObj(SWIG_as_voidptr(traciVal), SWIGTYPE_p_libsumo__TraCIResult, 0);
        }
        PyObject* const pyKey = PyInt_FromLong(iter->first);
        PyDict_SetItem(result, pyKey, pyVal);
        Py_DECREF(pyKey);
        Py_DECREF(pyVal);
    }
    return result;
}
%}

%typemap(out) std::map<int, std::shared_ptr<libsumo::TraCIResult> > {
    $result = parseSubscriptionMap($1);
};

%typemap(out) std::map<std::string, std::map<int, std::shared_ptr<libsumo::TraCIResult> > > {
    $result = PyDict_New();
    for (auto iter = $1.begin(); iter != $1.end(); ++iter) {
        PyObject* const pyKey = PyUnicode_FromString(iter->first.c_str());
        PyObject* const pyVal = parseSubscriptionMap(iter->second);
        PyDict_SetItem($result, pyKey, pyVal);
        Py_DECREF(pyKey);
        Py_DECREF(pyVal);
    }
};

%typemap(out) std::map<std::string, std::map<std::string, std::map<int, std::shared_ptr<libsumo::TraCIResult> > > > {
    $result = PyDict_New();
    for (auto iter = $1.begin(); iter != $1.end(); ++iter) {
        PyObject* const pyKey = PyUnicode_FromString(iter->first.c_str());
        PyObject* const innerDict = PyDict_New();
        for (auto inner = iter->second.begin(); inner != iter->second.end(); ++inner) {
            PyObject* const innerKey = PyUnicode_FromString(inner->first.c_str());
            PyObject* const innerVal = parseSubscriptionMap(inner->second);
            PyDict_SetItem(innerDict, innerKey, innerVal);
            Py_DECREF(innerKey);
            Py_DECREF(innerVal);
        }
        PyDict_SetItem($result, pyKey, innerDict);
        Py_DECREF(pyKey);
        Py_DECREF(innerDict);
    }
};

%typemap(out) libsumo::TraCIPosition {
    if ($1.z != libsumo::INVALID_DOUBLE_VALUE) {
        $result = Py_BuildValue("(ddd)", $1.x, $1.y, $1.z);
    } else {
        $result = Py_BuildValue("(dd)", $1.x, $1.y);
    }
};

%typemap(out) libsumo::TraCIPositionVector {
    $result = PyTuple_New($1.value.size());
    int index = 0;
    for (auto iter = $1.value.begin(); iter != $1.value.end(); ++iter) {
        PyTuple_SetItem($result, index++, Py_BuildValue("(dd)", iter->x, iter->y));
    }
};

%typemap(out) libsumo::TraCIColor {
    $result = Py_BuildValue("(iiii)", $1.r, $1.g, $1.b, $1.a);
};

%typemap(out) libsumo::TraCIRoadPosition {
    $result = Py_BuildValue("(sdi)", $1.edgeID.c_str(), $1.pos, $1.laneIndex);
};

%typemap(out) std::vector<libsumo::TraCIConnection> {
    $result = parseConnectionList($1);
};

%typemap(out) std::vector<libsumo::TraCIVehicleData> {
    $result = PyTuple_New($1.size());
    int index = 0;
    for (auto iter = $1.begin(); iter != $1.end(); ++iter) {
        PyTuple_SetItem($result, index++, Py_BuildValue("(sddds)",
                                                        iter->id.c_str(),
                                                        iter->length,
                                                        iter->entryTime,
                                                        iter->leaveTime,
                                                        iter->typeID.c_str()));
    }
};

%typemap(out) std::vector<libsumo::TraCIBestLanesData> {
    $result = PyTuple_New($1.size());
    int index = 0;
    for (auto iter = $1.begin(); iter != $1.end(); ++iter) {
        const int size = (int)iter->continuationLanes.size();
        auto nextLanes = PyTuple_New(size);
        for (int i = 0; i < size; i++) {
            PyTuple_SetItem(nextLanes, i, PyUnicode_FromString(iter->continuationLanes[i].c_str()));
        }
        PyTuple_SetItem($result, index++, Py_BuildValue("(sddiNN)",
                                                        iter->laneID.c_str(),
                                                        iter->length,
                                                        iter->occupation,
                                                        iter->bestLaneOffset,
                                                        PyBool_FromLong(iter->allowsContinuation),
                                                        nextLanes));
    }
};

%typemap(out) std::vector<libsumo::TraCINextTLSData> {
    $result = PyTuple_New($1.size());
    int index = 0;
    for (auto iter = $1.begin(); iter != $1.end(); ++iter) {
        PyTuple_SetItem($result, index++, Py_BuildValue("(sidN)",
                                                        iter->id.c_str(),
                                                        iter->tlIndex,
                                                        iter->dist,
                                                        PyUnicode_FromStringAndSize(&iter->state, 1)));
    }
};

%typemap(out) std::vector<std::vector<libsumo::TraCILink> > {
    $result = PyTuple_New($1.size());
    int index = 0;
    for (auto iter = $1.begin(); iter != $1.end(); ++iter) {
        PyObject* innerList = PyTuple_New(iter->size());
        int innerIndex = 0;
        for (auto inner = iter->begin(); inner != iter->end(); ++inner) {
            PyTuple_SetItem(innerList, innerIndex++, Py_BuildValue("(sss)",
                                                                   inner->fromLane.c_str(),
                                                                   inner->toLane.c_str(),
                                                                   inner->viaLane.c_str()));
        }
        PyTuple_SetItem($result, index++, innerList);
    }
};

%typemap(out) std::vector<libsumo::TraCIJunctionFoe> {
    $result = PyTuple_New($1.size());
    int index = 0;
    for (auto iter = $1.begin(); iter != $1.end(); ++iter) {
        PyTuple_SetItem($result, index++, Py_BuildValue("(sddddssNN)",
                                                        iter->foeId.c_str(),
                                                        iter->egoDist,
                                                        iter->foeDist,
                                                        iter->egoExitDist,
                                                        iter->foeExitDist,
                                                        iter->egoLane.c_str(),
                                                        iter->foeLane.c_str(),
                                                        PyBool_FromLong(iter->egoResponse),
                                                        PyBool_FromLong(iter->foeResponse)));
    }
};

%typemap(out) std::vector<std::pair<std::string, double> > {
    $result = PyTuple_New($1.size());
    int index = 0;
    for (auto iter = $1.begin(); iter != $1.end(); ++iter) {
        PyTuple_SetItem($result, index++, Py_BuildValue("(sd)", iter->first.c_str(), iter->second));
    }
};

%exceptionclass libsumo::TraCIException;
%exceptionclass libsumo::FatalTraCIError;

%pythonprepend SWIG_MODULE::Vehicle::add(const std::string&, const std::string&, const std::string&, const std::string&, const std::string&,
                                     const std::string&, const std::string&, const std::string&, const std::string&, const std::string&,
                                     const std::string&, const std::string&, const std::string&, int, int) %{
    args = [str(a) for a in args[:13]] + list(args[13:])
    for key, val in kwargs.items():
        if key not in ("personCapacity", "personNumber"):
            kwargs[key] = str(val)
%}

%pythonappend SWIG_MODULE::Vehicle::getLeader(const std::string&, double) %{
    if val[0] == "" and vehicle._legacyGetLeader:
        return None
%}

%define SUBSCRIBE_HELPER(domain)
%pythonprepend SWIG_MODULE::domain::subscribe(const std::string&, const std::vector<int>&, double, double, const libsumo::TraCIResults&) %{
    if len(args) > 1 and args[1] is None:
        args = (args[0], [-1]) + args[2:]
    if len(args) > 4 and args[4] is None:
        args = args[:4] + ({},)
    if "varIDs" in kwargs and kwargs["varIDs"] is None:
        kwargs["varIDs"] = [-1]
    if "parameters" in kwargs and kwargs["parameters"] is None:
        kwargs["parameters"] = {}
%}

%pythonprepend SWIG_MODULE::domain::subscribeContext(const std::string&, int, double, const std::vector<int>&, double, double, const libsumo::TraCIResults&) %{
    if len(args) > 3 and args[3] is None:
        args = (args[0], args[1], args[2], [-1]) + args[4:]
    if len(args) > 6 and args[6] is None:
        args = args[:6] + ({},)
    if "varIDs" in kwargs and kwargs["varIDs"] is None:
        kwargs["varIDs"] = [-1]
    if "parameters" in kwargs and kwargs["parameters"] is None:
        kwargs["parameters"] = {}
%}
%enddef

SUBSCRIBE_HELPER(Edge)
SUBSCRIBE_HELPER(GUI)
SUBSCRIBE_HELPER(InductionLoop)
SUBSCRIBE_HELPER(Junction)
SUBSCRIBE_HELPER(Lane)
SUBSCRIBE_HELPER(LaneArea)
SUBSCRIBE_HELPER(MultiEntryExit)
SUBSCRIBE_HELPER(Person)
SUBSCRIBE_HELPER(POI)
SUBSCRIBE_HELPER(Polygon)
SUBSCRIBE_HELPER(Route)
SUBSCRIBE_HELPER(TrafficLight)
SUBSCRIBE_HELPER(Vehicle)
SUBSCRIBE_HELPER(VehicleType)
SUBSCRIBE_HELPER(Calibrator)
SUBSCRIBE_HELPER(BusStop)
SUBSCRIBE_HELPER(ParkingArea)
SUBSCRIBE_HELPER(ChargingStation)
SUBSCRIBE_HELPER(OverheadWire)
SUBSCRIBE_HELPER(Rerouter)
SUBSCRIBE_HELPER(MeanData)
SUBSCRIBE_HELPER(VariableSpeedSign)
SUBSCRIBE_HELPER(RouteProbe)

%pythonprepend SWIG_MODULE::Simulation::subscribe(const std::vector<int>&, double, double, const libsumo::TraCIResults&) %{
    if len(args) > 0 and args[0] is None:
        args = ([-1],) + args[1:]
    if len(args) > 3 and args[3] is None:
        args = args[:3] + ({},)
    if "varIDs" in kwargs and kwargs["varIDs"] is None:
        kwargs["varIDs"] = [-1]
    if "parameters" in kwargs and kwargs["parameters"] is None:
        kwargs["parameters"] = {}
%}

%pythonprepend SWIG_MODULE::Simulation::subscribeContext(const std::string&, int, double, const std::vector<int>&, double, double, const libsumo::TraCIResults&) %{
    if len(args) > 3 and args[3] is None:
        args = (args[0], args[1], args[2], [-1]) + args[4:]
    if len(args) > 6 and args[6] is None:
        args = args[:6] + ({},)
    if "varIDs" in kwargs and kwargs["varIDs"] is None:
        kwargs["varIDs"] = [-1]
    if "parameters" in kwargs and kwargs["parameters"] is None:
        kwargs["parameters"] = {}
%}

%ignore SWIG_MODULE::Simulation::subscribe(const std::string&, const std::vector<int>&, double begin, double, const libsumo::TraCIResults&);

#endif // SWIGPYTHON

%begin %{
#ifdef _MSC_VER
// ignore constant conditional expression (C4127) and unreachable/unsafe code warnings
// and hidden local declaration (C4456), uninitialized variable (C4701), assignment in conditional expression (C4706)
// also see config.h.cmake
#pragma warning(disable:4127 4456 4701 4702 4706 4996 4365 4820 4514 5045 4191 4710 4668)
#else
// ignore unused parameter warnings for vector template code
#pragma GCC diagnostic ignored "-Wunused-parameter"
// ignore uninitialized fields for typeobject::tp_vectorcall and typeobject::tp_print
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
#define SWIG_PYTHON_2_UNICODE

#include <iostream>
%}


%include "std_shared_ptr.i"
%shared_ptr(libsumo::TraCIPhase)
#ifndef SWIGPYTHON
%shared_ptr(libsumo::TraCIResult)
%shared_ptr(libsumo::TraCIPosition)
%shared_ptr(libsumo::TraCIRoadPosition)
%shared_ptr(libsumo::TraCIColor)
%shared_ptr(libsumo::TraCIPositionVector)
%shared_ptr(libsumo::TraCIInt)
%shared_ptr(libsumo::TraCIDouble)
%shared_ptr(libsumo::TraCIString)
%shared_ptr(libsumo::TraCIStringList)
%shared_ptr(libsumo::TraCIDoubleList)
%shared_ptr(libsumo::TraCIIntList)
%shared_ptr(libsumo::TraCIStringDoublePairList)
%shared_ptr(libsumo::TraCIPhase)
%shared_ptr(libsumo::TraCILogic)
%shared_ptr(libsumo::TraCILogicVectorWrapped)
%shared_ptr(libsumo::TraCILink)
%shared_ptr(libsumo::TraCILinkVectorVectorWrapped)
%shared_ptr(libsumo::TraCIConnection)
%shared_ptr(libsumo::TraCIConnectionVectorWrapped)
%shared_ptr(libsumo::TraCIVehicleData)
%shared_ptr(libsumo::TraCIVehicleDataVectorWrapped)
%shared_ptr(libsumo::TraCINextTLSData)
%shared_ptr(libsumo::TraCINextTLSDataVectorWrapped)
%shared_ptr(libsumo::TraCINextStopData)
%shared_ptr(libsumo::TraCINextStopDataVectorWrapped)
%shared_ptr(libsumo::TraCIBestLanesData)
%shared_ptr(libsumo::TraCIBestLanesDataVectorWrapped)
%shared_ptr(libsumo::TraCIStage)
%shared_ptr(libsumo::TraCIReservation)
%shared_ptr(libsumo::TraCIReservationVectorWrapped)
%shared_ptr(libsumo::TraCICollision)
%shared_ptr(libsumo::TraCICollisionVectorWrapped)
%shared_ptr(libsumo::TraCISignalConstraint)
%shared_ptr(libsumo::TraCISignalConstraintVectorWrapped)
%shared_ptr(libsumo::TraCIJunctionFoe)
%shared_ptr(libsumo::TraCIJunctionFoeVectorWrapped)
#endif

// replacing vector instances of standard types, see https://stackoverflow.com/questions/8469138
%include "std_string.i"
%include "std_vector.i"
%include "std_map.i"
%template(StringVector) std::vector<std::string>;
%template(IntVector) std::vector<int>;
%template(DoubleVector) std::vector<double>;
#ifdef SWIGPYTHON
%template() std::map<std::string, std::string>;
#else
%template(StringStringMap) std::map<std::string, std::string>;
#endif

// replacing pair instances of standard types, see https://stackoverflow.com/questions/54733078
%include "std_pair.i"
%template(StringStringPair) std::pair<std::string, std::string>;
%template(IntStringPair) std::pair<int, std::string>;
%template(IntIntPair) std::pair<int, int>;
%template(StringDoublePair) std::pair<std::string, double>;
%template(StringDoublePairVector) std::vector<std::pair<std::string, double> >;

// exception handling
%include "exception.i"

// taken from here https://stackoverflow.com/questions/1394484/how-do-i-propagate-c-exceptions-to-python-in-a-swig-wrapper-library
%exception {
    try {
        $action
    } catch (const libsumo::TraCIException& e) {
        const std::string s = e.what();
        std::string printError;
        if (std::getenv("TRACI_PRINT_ERROR") != nullptr) {
            printError = std::getenv("TRACI_PRINT_ERROR");
        }
#ifdef LIBTRACI
        if (printError == "all" || printError == "client") {
#else
        if (printError == "all" || printError == "libsumo") {
#endif
            std::cerr << "Error: " << s << std::endl;
        }
#ifdef SWIGPYTHON
        PyErr_SetString(SWIG_Python_ExceptionType(SWIGTYPE_p_libsumo__TraCIException), s.c_str());
        SWIG_fail;
#else
        SWIG_exception(SWIG_ValueError, s.c_str());
#endif
    } catch (const std::exception& e) {
        const std::string s = e.what();
        std::string printError;
        if (std::getenv("TRACI_PRINT_ERROR") != nullptr) {
            printError = std::getenv("TRACI_PRINT_ERROR");
        }
#ifdef LIBTRACI
        if (printError == "all" || printError == "client") {
#else
        if (printError == "all" || printError == "libsumo") {
#endif
            std::cerr << "Error: " << s << std::endl;
        }
#ifdef SWIGPYTHON
        PyErr_SetString(SWIG_Python_ExceptionType(SWIGTYPE_p_libsumo__FatalTraCIError), s.c_str());
        SWIG_fail;
#else
        SWIG_exception(SWIG_UnknownError, s.c_str());
#endif
    } catch (...) {
        SWIG_exception(SWIG_UnknownError, "unknown exception");
    }
}

#if SWIG_VERSION < 0x040100 && defined(SWIGJAVA)
// see https://github.com/supranational/blst/issues/53
/* SWIG versions prior 4.1 were crossing the MinGW's ways on the path
 * to JNI 'jlong' type */
%begin %{
#if defined(__MINGW32__) && defined(__int64)
# undef __int64
#endif
%}
#endif // SWIGJAVA

// this checking happens just to make static code analysis happy, see https://github.com/swig/swig/issues/2865
#ifdef SWIGJAVA
%define SELF_NULL_CHECKER(traci_class)
%typemap(check) libsumo::traci_class *self %{
  if (!$1) {
    SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "NULL self");
    return $null;
  }
%}
%enddef
#endif
#ifdef SWIGPYTHON
%define SELF_NULL_CHECKER(traci_class)
%typemap(check) libsumo::traci_class *self %{
  if (!$1) {
    PyErr_SetString(PyExc_ValueError, "NULL self");
    SWIG_fail;
  }
%}
%enddef
#endif
#if defined(SWIGJAVA) || defined(SWIGPYTHON)
SELF_NULL_CHECKER(TraCIResult)
SELF_NULL_CHECKER(TraCIPosition)
SELF_NULL_CHECKER(TraCIPositionVector)
SELF_NULL_CHECKER(TraCIRoadPosition)
SELF_NULL_CHECKER(TraCIColor)
SELF_NULL_CHECKER(TraCIInt)
SELF_NULL_CHECKER(TraCIDouble)
SELF_NULL_CHECKER(TraCIDoubleList)
SELF_NULL_CHECKER(TraCIString)
SELF_NULL_CHECKER(TraCIStringList)
SELF_NULL_CHECKER(TraCIPhase)
SELF_NULL_CHECKER(TraCILogic)
SELF_NULL_CHECKER(TraCILink)
SELF_NULL_CHECKER(TraCIConnection)
SELF_NULL_CHECKER(TraCIVehicleData)
SELF_NULL_CHECKER(TraCINextTLSData)
SELF_NULL_CHECKER(TraCINextStopData)
SELF_NULL_CHECKER(TraCINextStopDataVectorWrapped)
SELF_NULL_CHECKER(TraCIBestLanesData)
SELF_NULL_CHECKER(TraCIStage)
SELF_NULL_CHECKER(TraCIReservation)
SELF_NULL_CHECKER(TraCICollision)
SELF_NULL_CHECKER(TraCISignalConstraint)
SELF_NULL_CHECKER(TraCIJunctionFoe)
#endif

// %feature("compactdefaultargs") libsumo::Simulation::findRoute;
