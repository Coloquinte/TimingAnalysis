// Copyright (c) 2017 Gabriel Gouvine - All Rights Reserved

#include "timing_analysis.hh"

#include "testing.hh"

#include <iostream>
#include <random>

using namespace lemon;

void genRandomGraph(TimingGraph &ret, int layers, int nodesPerLayer, double arcsPerNode) {
  int n = layers * nodesPerLayer;
  std::vector<std::pair<int, int> > arcs;

  std::uniform_int_distribution<int> dist(0, nodesPerLayer - 1);
  std::mt19937 rgen;

  for (int i = 0; i + 1 < layers; ++i) {
    for (int e = 0; e < arcsPerNode * nodesPerLayer; ++e) {
      int in = dist(rgen);
      int out = dist(rgen);
      arcs.emplace_back(i * nodesPerLayer + in, (i + 1) * nodesPerLayer + out);
    }
  }

  std::sort(arcs.begin(), arcs.end());  // Required for build

  ret.build(n, arcs.begin(), arcs.end());
}

BOOST_AUTO_TEST_SUITE(testTimingAnalysis)

BOOST_AUTO_TEST_CASE(testBasics) {
  int nNodes = 4;
  std::vector<std::pair<int, int> > arcs = {
      {0, 1}, {0, 2}, {1, 2}, {1, 3}, {2, 3}};
  std::vector<Time> delays = {5, 2, 6, 4, 3};

  TimingGraph graph;
  graph.build(nNodes, arcs.begin(), arcs.end());
  assert(graph.nodeNum() == nNodes);
  assert(graph.arcNum() == (int)arcs.size());

  TimingAnalysis ta(graph);
  ta.setDelays(delays);
  ta.checkConsistency();

  BOOST_CHECK_EQUAL(ta.getArrivalTime(0), 0ll);
  BOOST_CHECK_EQUAL(ta.getArrivalTime(1), 5ll);
  BOOST_CHECK_EQUAL(ta.getArrivalTime(2), 11ll);
  BOOST_CHECK_EQUAL(ta.getArrivalTime(3), 14ll);

  BOOST_CHECK_EQUAL(ta.getOutputTime(0), 14ll);
  BOOST_CHECK_EQUAL(ta.getOutputTime(1), 9ll);
  BOOST_CHECK_EQUAL(ta.getOutputTime(2), 3ll);
  BOOST_CHECK_EQUAL(ta.getOutputTime(3), 0ll);

  ta.setDelay(0, 2);
  ta.checkConsistency();
  BOOST_CHECK_EQUAL(ta.getArrivalTime(0), 0ll);
  BOOST_CHECK_EQUAL(ta.getArrivalTime(1), 2ll);
  BOOST_CHECK_EQUAL(ta.getArrivalTime(2), 8ll);
  BOOST_CHECK_EQUAL(ta.getArrivalTime(3), 11ll);

  ta.setDelay(2, 3);
  ta.checkConsistency();
  BOOST_CHECK_EQUAL(ta.getArrivalTime(0), 0ll);
  BOOST_CHECK_EQUAL(ta.getArrivalTime(1), 2ll);
  BOOST_CHECK_EQUAL(ta.getArrivalTime(2), 5ll);
  BOOST_CHECK_EQUAL(ta.getArrivalTime(3), 8ll);

  ta.setDelay(3, 8);
  ta.checkConsistency();
  BOOST_CHECK_EQUAL(ta.getArrivalTime(0), 0ll);
  BOOST_CHECK_EQUAL(ta.getArrivalTime(1), 2ll);
  BOOST_CHECK_EQUAL(ta.getArrivalTime(2), 5ll);
  BOOST_CHECK_EQUAL(ta.getArrivalTime(3), 10ll);

  ta.setDelay(3, 4);
  ta.checkConsistency();
  BOOST_CHECK_EQUAL(ta.getArrivalTime(0), 0ll);
  BOOST_CHECK_EQUAL(ta.getArrivalTime(1), 2ll);
  BOOST_CHECK_EQUAL(ta.getArrivalTime(2), 5ll);
  BOOST_CHECK_EQUAL(ta.getArrivalTime(3), 8ll);

  ta.setDelay(2, 6);
  ta.checkConsistency();
  BOOST_CHECK_EQUAL(ta.getArrivalTime(0), 0ll);
  BOOST_CHECK_EQUAL(ta.getArrivalTime(1), 2ll);
  BOOST_CHECK_EQUAL(ta.getArrivalTime(2), 8ll);
  BOOST_CHECK_EQUAL(ta.getArrivalTime(3), 11ll);

  ta.setDelay(0, 5);
  ta.checkConsistency();
  BOOST_CHECK_EQUAL(ta.getArrivalTime(0), 0ll);
  BOOST_CHECK_EQUAL(ta.getArrivalTime(1), 5ll);
  BOOST_CHECK_EQUAL(ta.getArrivalTime(2), 11ll);
  BOOST_CHECK_EQUAL(ta.getArrivalTime(3), 14ll);
}

BOOST_AUTO_TEST_CASE(testPerformance) {
  TimingGraph graph;

  const int nLayers = 10;
  const int nNodesPerLayer = 100;
  const int nNodes = nLayers * nNodesPerLayer;
  std::mt19937 rgen;
  std::uniform_int_distribution<Time> time_dist(0, 1000);

  genRandomGraph(graph, nLayers, nNodesPerLayer, 3.0);
  assert (nNodes == graph.nodeNum());

  TimingAnalysis ta (graph);

  std::vector<Time> delays(graph.arcNum());
  for (Time &d : delays) {
    d = time_dist(rgen);
  }
  ta.setDelays(delays);

  const int nChanges = 100000;
  std::uniform_int_distribution<int> arc_dist(0, graph.arcNum());
  for (int i = 0; i < nChanges; ++i) {
    int arc = arc_dist(rgen);
    Time d = time_dist(rgen);
    std::cout << i << ": arc " << arc << " from " << ta.getDelay(arc) << " to " << d << std::endl;
    ta.setDelay(arc, d);
    ta.checkConsistency();
  }
}

BOOST_AUTO_TEST_SUITE_END()
