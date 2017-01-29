
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

  void setDelays(const std::vector<Time> &delays);
  void setDelay(int edge, Time delay);

  Time getArrivalTime(int node) const {
    return _arrivalTimes[_graph.nodeFromId(node)];
  }

  void checkConsistency();

 private:
  // For full timing analysis
  void initArrivalTimes();
  Time initArrivalTime(TimingGraph::Node);

  // For incremental timing analysis
  void updateArrivalTime(TimingGraph::Node);
  void decreaseArrivalTime(TimingGraph::Node node, Time oldAT, Time newAT);
  void increaseArrivalTime(TimingGraph::Node node, Time newAT);

 private:
  TimingGraph &_graph;
  EdgeDelayMap _delays;

  NodeTimeMap _arrivalTimes;
};

#endif
