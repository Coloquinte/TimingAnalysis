// Copyright (c) 2017 Gabriel Gouvine - All Rights Reserved

#include "timing_analysis.hh"

#include "testing.hh"

#include <iostream>
#include <random>

using namespace lemon;

TimingGraph genRandomGraph(int layers, int nodesPerLayer, double arcsPerNode) {
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

  TimingGraph ret;
  ret.build(n, arcs.begin(), arcs.end());
  return ret;
}

BOOST_AUTO_TEST_SUITE(testTimingAnalysis)

BOOST_AUTO_TEST_CASE(testBasics) {
  int nNodes = 4;
  std::vector<std::pair<int, int> > arcs = {
      {0, 1}, {0, 2}, {1, 2}, {1, 3}, {2, 3}};

  TimingGraph graph;
  graph.build(nNodes, arcs.begin(), arcs.end());
  assert(graph.nodeNum() == nNodes);
  assert(graph.arcNum() == (int)arcs.size());

  TimingAnalysis ta(graph);
  ta.setDelay(0, 5);
  ta.setDelay(1, 2);
  ta.setDelay(2, 6);
  ta.setDelay(3, 4);
  ta.setDelay(4, 3);

  ta.computeArrivalTimes();
  BOOST_CHECK_EQUAL(ta.getArrivalTime(0), 0ll);
  BOOST_CHECK_EQUAL(ta.getArrivalTime(1), 5ll);
  BOOST_CHECK_EQUAL(ta.getArrivalTime(2), 11ll);
  BOOST_CHECK_EQUAL(ta.getArrivalTime(3), 14ll);
}

BOOST_AUTO_TEST_SUITE_END()
