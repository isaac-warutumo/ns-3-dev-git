/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef Cluster_h
#define Cluster_h

#pragma once
// C++ program to find the Number of Subsets that have sum between A and B
#include <vector>
#include <algorithm>
using namespace std;
class Cluster
{
  int m_nNodes;
  int *m_nodeCapacities; //a pointer to array to hold cluster node capacities
  int m_upperNodeCapacity;

public:
  Cluster (int myClusterNodes, int arrayWithNodeCapacities[], int myUpperNodeCapacity);

  int findAndPrintSubsets (int mySet[], size_t nElements, int lowerLimit, int upperLimit);
  void generateSubsets (int start, int setSize, int myFullSet[],
                        vector<vector<int>> &subsetSumVector2D, vector<int> &subsetSumVector1D);
  void printHalfSetSumArrays (int myHalfSetSize, vector<vector<int>> &myHalfSetArray2D,
                              vector<int> &myHalfSetArray1D);
};

#endif // Cluster_h