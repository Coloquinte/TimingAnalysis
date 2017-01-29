
#ifndef SQUASH_TIMING_ANALYSIS
#define SQUASH_TIMING_ANALYSIS

#include "lemon/static_graph.h"
#include "lemon/adaptors.h"

typedef std::int64_t Time;

typedef lemon::StaticDigraph TimingGraph;
typedef lemon::ReverseDigraph<const TimingGraph> RTimingGraph;
typedef TimingGraph::NodeMap<Time> NodeTimeMap;
typedef TimingGraph::ArcMap<Time> EdgeDelayMap;

class TimingAnalysis {
 public:
  TimingAnalysis(const TimingGraph &graph);

  void setDelays(const std::vector<Time> &delays);
  void setDelay(int edge, Time delay);

  Time getArrivalTime(int node) const {
    return _arrivalTimes[_graph.nodeFromId(node)];
  }
  Time getOutputTime(int node) const {
    return _outputTimes[_graph.nodeFromId(node)];
  }

  void checkConsistency();

 private:
  const TimingGraph &_graph;
  RTimingGraph       _rgraph;
  EdgeDelayMap       _delays;

  NodeTimeMap _arrivalTimes;
  NodeTimeMap _outputTimes;
};

#endif
