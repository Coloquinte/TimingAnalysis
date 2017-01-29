
#include "timing_analysis.hh"

#include "lemon/connectivity.h"

#include <exception>
#include <cassert>

using namespace lemon;

const Time dirtyTime = -1;

/*
 * The helper class to perform the propagation for both arrival and output time
 * Using the graph and the reversed timing graph
 */

template <typename GR>
class TimingHelper {
 public:
  TimingHelper(const GR &graph, const EdgeDelayMap &delays, NodeTimeMap &arrivalTimes);

  // For full timing analysis
  void initArrivalTimes();
  Time initArrivalTime(typename GR::Node);

  // For incremental timing analysis
  void changeDelay(typename GR::Arc edge, Time oldDelay, Time delay);
  void decreaseArrivalTime(typename GR::Node node, Time oldAT, Time newAT);
  void increaseArrivalTime(typename GR::Node node, Time newAT);

  void checkConsistency();
  Time computeArrivalTime(typename GR::Node node) const;

 private:
  const GR &_graph;
  const EdgeDelayMap &_delays;
  NodeTimeMap &_arrivalTimes;
};

typedef TimingHelper<TimingGraph>  FTimingHelper;
typedef TimingHelper<RTimingGraph> RTimingHelper;

TimingAnalysis::TimingAnalysis(const TimingGraph &graph)
: _graph(graph), _rgraph(graph), _delays(graph, 0)
, _arrivalTimes(graph, dirtyTime)
, _outputTimes(graph, dirtyTime) {
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

  FTimingHelper helper(_graph, _delays, _arrivalTimes);
  helper.initArrivalTimes();
  RTimingHelper rhelper(_rgraph, _delays, _outputTimes);
  rhelper.initArrivalTimes();
}

void TimingAnalysis::setDelay(int edgeId, Time delay) {
  assert (edgeId < _graph.arcNum());

  TimingGraph::Arc edge = _graph.arcFromId(edgeId);
  Time oldDelay = _delays[edge];
  _delays[edge] = delay;

  FTimingHelper helper(_graph, _delays, _arrivalTimes);
  helper.changeDelay(edge, oldDelay, delay);
  RTimingHelper rhelper(_rgraph, _delays, _outputTimes);
  rhelper.changeDelay(edge, oldDelay, delay);
}

Time TimingAnalysis::getDelay(int edgeId) const {
  assert (edgeId < _graph.arcNum());
  return _delays[_graph.arcFromId(edgeId)];
}

void TimingAnalysis::checkConsistency() {
  FTimingHelper helper(_graph, _delays, _arrivalTimes);
  helper.checkConsistency();
  RTimingHelper rhelper(_rgraph, _delays, _outputTimes);
  rhelper.checkConsistency();
}

template <typename GR>
TimingHelper<GR>::TimingHelper(const GR &graph, const EdgeDelayMap &delays, NodeTimeMap &arrivalTimes)
: _graph(graph)
, _delays(delays)
, _arrivalTimes(arrivalTimes) {
}

template <typename GR>
void TimingHelper<GR>::changeDelay(typename GR::Arc edge, Time oldDelay, Time delay) {
  Time sourceArrivalTime = _arrivalTimes[_graph.source(edge)];

  if (delay > oldDelay) {
    increaseArrivalTime(_graph.target(edge), sourceArrivalTime + delay);
  } else if (delay < oldDelay) {
    decreaseArrivalTime(_graph.target(edge), sourceArrivalTime + oldDelay, sourceArrivalTime + delay);
  }
}

template <typename GR>
Time TimingHelper<GR>::initArrivalTime(typename GR::Node node) {
  if (_arrivalTimes[node] != dirtyTime) {
    return _arrivalTimes[node];
  }

  Time arrivalTime = 0;
  for (typename GR::InArcIt inArc(_graph, node); inArc != INVALID; ++inArc) {
    typename GR::Node parent = _graph.source(inArc);
    Time parentAT = initArrivalTime(parent);
    arrivalTime = std::max(arrivalTime, parentAT + _delays[inArc]);
  }
  _arrivalTimes[node] = arrivalTime;
  return arrivalTime;
}

template <typename GR>
void TimingHelper<GR>::initArrivalTimes() {
  for (int i = 0; i < _graph.nodeNum(); ++i) {
     _arrivalTimes[_graph.nodeFromId(i)] = dirtyTime;
  }
  for (typename GR::NodeIt node(_graph); node != INVALID; ++node) {
    initArrivalTime(node);
  }
}

template <typename GR>
void TimingHelper<GR>::decreaseArrivalTime(typename GR::Node node, Time oldAT, Time newAT) {
  Time nodeAT = _arrivalTimes[node];
  assert (newAT < oldAT);
  //assert (nodeAT >= oldAT /* May not be true if parallel edges mean the decrease happened already */ );
  if (nodeAT != oldAT) {
    // The arrival edge was not the critical one
    return;
  }

  // This was (one of) the critical edge for this node
  // We need to recompute the arrival time
  Time arrivalTime = computeArrivalTime(node);

  assert (arrivalTime <= nodeAT);
  if (arrivalTime == nodeAT) {
    // No decrease
    return;
  }

  // Propagate the new arrival time forward
  _arrivalTimes[node] = arrivalTime;
  for (typename GR::OutArcIt outArc(_graph, node); outArc != INVALID; ++outArc) {
    typename GR::Node child = _graph.target(outArc);
    Time delay = _delays[outArc];
    decreaseArrivalTime(child, nodeAT + delay, arrivalTime + delay);
  }
}

template <typename GR>
void TimingHelper<GR>::increaseArrivalTime(typename GR::Node node, Time newAT) {
  Time oldAT = _arrivalTimes[node];
  if (newAT > oldAT) {
    // Arrival time increase in this node
    for (typename GR::OutArcIt outArc(_graph, node); outArc != INVALID; ++outArc) {
      increaseArrivalTime(_graph.target(outArc), newAT + _delays[outArc]);
    }
    _arrivalTimes[node] = newAT;
  }
}

template <typename GR>
void TimingHelper<GR>::checkConsistency() {
  for (typename GR::NodeIt node(_graph); node != INVALID; ++node) {
    assert(computeArrivalTime(node) == _arrivalTimes[node]);
  }
}

template <typename GR>
Time TimingHelper<GR>::computeArrivalTime(typename GR::Node node) const {
  Time arrivalTime = 0;
  for (typename GR::InArcIt inArc(_graph, node); inArc != INVALID; ++inArc) {
    typename GR::Node parent = _graph.source(inArc);
    Time parentAT = _arrivalTimes[parent];
    arrivalTime = std::max(arrivalTime, parentAT + _delays[inArc]);
  }
  return arrivalTime;
}
