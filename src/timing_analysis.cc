
#include "timing_analysis.hh"

#include "lemon/connectivity.h"

#include <exception>
#include <cassert>

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
  for (int i = 0; i < _graph.nodeNum(); ++i) {
     _arrivalTimes[_graph.nodeFromId(i)] = dirtyTime;
  }
  for (TimingGraph::NodeIt node(_graph); node != INVALID; ++node) {
    initArrivalTime(node);
  }
}

void TimingAnalysis::setDelay(int edgeId, Time delay) {
  TimingGraph::Arc edge = _graph.arcFromId(edgeId);
  Time oldDelay = _delays[edge];
  _delays[edge] = delay;

  Time sourceArrivalTime = _arrivalTimes[_graph.source(edge)];

  if (delay > oldDelay) {
    increaseArrivalTime(_graph.target(edge), sourceArrivalTime + delay);
  } else if (delay < oldDelay) {
    decreaseArrivalTime(_graph.target(edge), sourceArrivalTime + oldDelay, sourceArrivalTime + delay);
  }
}

void TimingAnalysis::decreaseArrivalTime(TimingGraph::Node node, Time oldAT, Time newAT) {
  Time nodeAT = _arrivalTimes[node];
  assert (newAT < oldAT);
  assert (nodeAT >= oldAT);
  if (nodeAT != oldAT) {
    // The arrival edge was not the critical one
    return;
  }

  // This was (one of) the critical edge for this node
  // We need to recompute the arrival time
  Time arrivalTime = 0;
  for (TimingGraph::InArcIt inArc(_graph, node); inArc != INVALID; ++inArc) {
    TimingGraph::Node parent = _graph.source(inArc);
    Time parentAT = _arrivalTimes[parent];
    arrivalTime = std::max(arrivalTime, parentAT + _delays[inArc]);
  }

  _arrivalTimes[node] = arrivalTime;
  for (TimingGraph::OutArcIt outArc(_graph, node); outArc != INVALID; ++outArc) {
    Time delay = _delays[outArc];
    decreaseArrivalTime(_graph.target(outArc), nodeAT + delay, arrivalTime + delay);
  }
}

void TimingAnalysis::increaseArrivalTime(TimingGraph::Node node, Time newAT) {
  Time oldAT = _arrivalTimes[node];
  if (newAT > oldAT) {
    // Arrival time increase in this node
    for (TimingGraph::OutArcIt outArc(_graph, node); outArc != INVALID; ++outArc) {
      increaseArrivalTime(_graph.target(outArc), newAT + _delays[outArc]);
    }
    _arrivalTimes[node] = newAT;
  }
}

void TimingAnalysis::checkConsistency() {
  std::vector<Time> oldATs(_graph.nodeNum());
  for (int i = 0; i < _graph.nodeNum(); ++i) {
    oldATs[i] = _arrivalTimes[_graph.nodeFromId(i)];
  }
  initArrivalTimes();
  for (int i = 0; i < _graph.nodeNum(); ++i) {
    assert(oldATs[i] == _arrivalTimes[_graph.nodeFromId(i)]);
  }
}

