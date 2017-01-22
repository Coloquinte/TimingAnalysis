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
  std::vector<Time> delays = {5, 2, 6, 4, 3};

  TimingGraph graph;
  graph.build(nNodes, arcs.begin(), arcs.end());
  assert(graph.nodeNum() == nNodes);
  assert(graph.arcNum() == (int)arcs.size());

  TimingAnalysis ta(graph);
  ta.setDelays(delays);

  BOOST_CHECK_EQUAL(ta.getArrivalTime(0), 0ll);
  BOOST_CHECK_EQUAL(ta.getArrivalTime(1), 5ll);
  BOOST_CHECK_EQUAL(ta.getArrivalTime(2), 11ll);
  BOOST_CHECK_EQUAL(ta.getArrivalTime(3), 14ll);

  ta.setDelay(0, 2);
  BOOST_CHECK_EQUAL(ta.getArrivalTime(0), 0ll);
  BOOST_CHECK_EQUAL(ta.getArrivalTime(1), 2ll);
  BOOST_CHECK_EQUAL(ta.getArrivalTime(2), 8ll);
  BOOST_CHECK_EQUAL(ta.getArrivalTime(3), 11ll);

  ta.setDelay(2, 3);
  BOOST_CHECK_EQUAL(ta.getArrivalTime(0), 0ll);
  BOOST_CHECK_EQUAL(ta.getArrivalTime(1), 2ll);
  BOOST_CHECK_EQUAL(ta.getArrivalTime(2), 5ll);
  BOOST_CHECK_EQUAL(ta.getArrivalTime(3), 8ll);

  ta.setDelay(3, 8);
  BOOST_CHECK_EQUAL(ta.getArrivalTime(0), 0ll);
  BOOST_CHECK_EQUAL(ta.getArrivalTime(1), 2ll);
  BOOST_CHECK_EQUAL(ta.getArrivalTime(2), 5ll);
  BOOST_CHECK_EQUAL(ta.getArrivalTime(3), 10ll);
}

BOOST_AUTO_TEST_SUITE_END()
