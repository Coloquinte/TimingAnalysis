
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

Time TimingAnalysis::computeArrivalTime(TimingGraph::Node node) {
  if (_arrivalTimes[node] != dirtyTime) {
    return _arrivalTimes[node];
  }

  Time arrivalTime = 0;
  for (TimingGraph::InArcIt inArc(_graph, node); inArc != INVALID; ++inArc) {
    TimingGraph::Node parent = _graph.source(inArc);
    Time parentAT = computeArrivalTime(parent);
    arrivalTime = std::max(arrivalTime, parentAT + _delays[inArc]);
  }
  _arrivalTimes[node] = arrivalTime;
  return arrivalTime;
}

void TimingAnalysis::computeArrivalTimes() {
  for (TimingGraph::NodeIt node(_graph); node != INVALID; ++node) {
    computeArrivalTime(node);
  }
}
