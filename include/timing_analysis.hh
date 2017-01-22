
#ifndef SQUASH_TIMING_ANALYSIS
#define SQUASH_TIMING_ANALYSIS

#include "lemon/static_graph.h"

typedef std::int64_t Time;
const Time dirtyTime = -1;

typedef lemon::StaticDigraph TimingGraph;
typedef TimingGraph::NodeMap<Time> NodeTimeMap;
typedef TimingGraph::ArcMap<Time> EdgeDelayMap;

class TimingAnalysis {
 public:
  TimingAnalysis(TimingGraph &graph);

  void setDelay(int edge, Time delay) {
    _delays[_graph.arcFromId(edge)] = delay;
  }
  Time getArrivalTime(int node) const {
    return _arrivalTimes[_graph.nodeFromId(node)];
  }

  void computeArrivalTimes();
  Time computeArrivalTime(TimingGraph::Node);

 private:
  TimingGraph &_graph;
  EdgeDelayMap _delays;

  NodeTimeMap _arrivalTimes;
};

#endif
