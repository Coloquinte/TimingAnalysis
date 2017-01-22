
#include "timing_analysis.hh"

#include "lemon/connectivity.h"

#include <exception>

using namespace lemon;

TimingAnalysis::TimingAnalysis(TimingGraph &graph)
    : _graph(graph), _delays(graph, 0), _arrivalTimes(graph, dirtyTime) {
  if (!lemon::dag(_graph)) {
    throw std::runtime_error("Given graph is not acyclic");
  }
}

void TimingAnalysis::setDelays(const std::vector<Time> &delays) {
  if (_graph.arcNum() != (int) delays.size()) {
    throw std::runtime_error("Vector of delays doesn't have the same size as the graph");
  }
  for (int i = 0; i < _graph.arcNum(); ++i) {
     _delays[_graph.arcFromId(i)] = delays[i];
  }
  initArrivalTimes();
}

Time TimingAnalysis::initArrivalTime(TimingGraph::Node node) {
  if (_arrivalTimes[node] != dirtyTime) {
    return _arrivalTimes[node];
  }

  Time arrivalTime = 0;
  for (TimingGraph::InArcIt inArc(_graph, node); inArc != INVALID; ++inArc) {
    TimingGraph::Node parent = _graph.source(inArc);
    Time parentAT = initArrivalTime(parent);
    arrivalTime = std::max(arrivalTime, parentAT + _delays[inArc]);
  }
  _arrivalTimes[node] = arrivalTime;
  return arrivalTime;
}

void TimingAnalysis::initArrivalTimes() {
  for (TimingGraph::NodeIt node(_graph); node != INVALID; ++node) {
    initArrivalTime(node);
  }
}

void TimingAnalysis::setDelay(int edgeId, Time delay) {
  TimingGraph::Arc edge = _graph.arcFromId(edgeId);
  _delays[edge] = delay;
  updateArrivalTime(_graph.target(edge));
}

void TimingAnalysis::updateArrivalTime(TimingGraph::Node node) {
  Time prevArrivalTime = _arrivalTimes[node];

  Time arrivalTime = 0;
  for (TimingGraph::InArcIt inArc(_graph, node); inArc != INVALID; ++inArc) {
    TimingGraph::Node parent = _graph.source(inArc);
    Time parentAT = _arrivalTimes[parent];
    arrivalTime = std::max(arrivalTime, parentAT + _delays[inArc]);
  }
  if (arrivalTime == prevArrivalTime) {
    return;
  }

  _arrivalTimes[node] = arrivalTime;
  for (TimingGraph::OutArcIt outArc(_graph, node); outArc != INVALID; ++outArc) {
    updateArrivalTime(_graph.target(outArc));
  }
}

