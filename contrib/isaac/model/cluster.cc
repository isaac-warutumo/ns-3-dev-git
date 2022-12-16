#include "cluster.h"
#include <vector>
#include <algorithm>
#include <iostream>
#include <cmath> //required to use pow

Cluster::Cluster (int myClusterNodes, int arrayWithNodeCapacities[], int myRequiredCapacity)
    : m_nNodes (myClusterNodes),
      m_nodeCapacities (arrayWithNodeCapacities),
      m_RequiredCapacity (myRequiredCapacity)
{
}

//function to sort the 2D vector on basis of a particular column (0 in this case)
bool
sortcol (const vector<int> &v1, const vector<int> &v2)
{
  return v1[0] < v2[0];
}

/* Function to Generate all subsets of a set
start --> Starting Index of the Set for the	first/second half Set
setSize --> Number of element in half Set
mySet --> Original Complete Set
subsetSumVector --> Store the subsets sums */

void
Cluster::generateSubsets (int start, int setSize, int myFullSet[],
                          vector<vector<int>> &subsetSumVector2D, vector<int> &subsetSumVector1D)
{
  // setSize of power set of a set with setSize N is (2^n - 1)
  unsigned int pow_setSize = pow (2, setSize);

  // Store the sum of particular subset of set
  int sum;

  // Run from counter (i) 000..0 to 111..1
  for (int i = 0; i < static_cast<int> (pow_setSize); i++)
    {
      //a vector to hold all the elements that make a sum eg if we get 7 from 3 and 4 then our
      vector<int> sumElements;
      //vector is 7 2 3 4 where 2 is the number of elements making the sum
      // set the sum initially to zero
      sum = 0;

      for (int j = 0; j < setSize; j++)
        {
          // Check if jth bit in the counter (i) is set If set then print jth element from set
          //cout << "i: " << i << " j: " << j << " (1 << j): " << (1 << j) << " (i & (1 << j)) " << (i & (1 << j)) << endl;
          if (i & (1 << j))
            {
              /*cout << "if (i & (1 << j)): " << (i & (1 << j)) << endl;
				cout << "sum before: sum += myFullSet[j + start]: " << sum << endl;*/

              sum += myFullSet[j + start];

              sumElements.push_back (myFullSet[j + start]); //keep the element that make the sum
              /*cout << "j: " << j << " start: " << start << endl;
				cout << "myFullSet[j + start]: " << myFullSet[j + start] << endl;
				cout << "sum after: sum += myFullSet[j + start]: " << sum << endl;*/
            }
        }
      //insert the sum at the beggining of the elements that make up the sum
      sumElements.insert (sumElements.begin (), sum);

      //keep the number of elements in the second element
      sumElements.insert (sumElements.begin () + 1, sumElements.size () - 1);

      //keep the sum elements in the sum vector
      subsetSumVector2D.push_back (sumElements);
      subsetSumVector1D.push_back (sum);
      //cout << endl;
    }

  // cout << "Half Subset Sums is: " << endl;
  // for (size_t i = 0; i < subsetSumVector2D.size (); i++)
  //   {
  //     for (size_t j = 0; j < subsetSumVector2D[i].size (); j++)
  //       {
  //         cout << subsetSumVector2D[i][j] << "\t";
  //       }
  //     cout << endl;
  //   }
}
void
Cluster::printHalfSetSumArrays (int myHalfSetSize, vector<vector<int>> &myHalfSetArray2D,
                                vector<int> &myHalfSetArray1D)
{
  // //printHalfSubset1 Arrays  -- to change it to become a function
  // cout << "myHalfSetArray1D" << endl;
  // for (size_t i = 0; i < pow (2, myHalfSetSize); i++)
  //   {
  //     cout << myHalfSetArray1D[i] << "\t";
  //   }
  // cout << endl;

  // cout << "myHalfSetArray2D" << endl;
  // for (size_t i = 0; i < pow (2, myHalfSetSize); i++)
  //   {
  //     for (size_t j = 0; j < myHalfSetArray2D[i].size (); j++)
  //       {
  //         cout << myHalfSetArray2D[i][j] << "\t";
  //       }
  //     cout << endl;
  //   }
}
int
Cluster::findAndPrintSubsets (int myFullSet[], size_t nElements, int lowerLimit, int upperLimit)
{
  if (lowerLimit > upperLimit)
    {
      int temp = lowerLimit;
      lowerLimit = upperLimit;
      upperLimit = temp;
    }
  cout << "myFullSet has nElements:" << nElements << " and target lowerLimit: " << lowerLimit
       << " and upperlimit: " << upperLimit << endl;
  for (size_t i = 0; i < nElements; i++)
    {
      cout << myFullSet[i] << "\t";
    }
  cout << endl;

  //Vectors to store the subsets sums of two half sets individually
  //2d vector to store elements forming the sum
  vector<vector<int>> halfSet1Sum2D, halfSet2Sum2D, finalSet2D;
  //1D vector to store sums
  vector<int> halfSet1Sum1D, halfSet2Sum1D, finalSet1D;

  //Generate subset sums for the first half set
  int nHalfSet1 = nElements / 2;
  generateSubsets (0, nHalfSet1, myFullSet, halfSet1Sum2D, halfSet1Sum1D);
  // cout << "Generated subset sums for halfSet1" << endl;

  //Generate subset sums for the second half set
  int nHalfSet2 = 0;
  if (nElements % 2 != 0)
    nHalfSet2 = nElements / 2 + 1;
  else
    nHalfSet2 = nElements / 2;

  generateSubsets (nElements / 2, nHalfSet2, myFullSet, halfSet2Sum2D, halfSet2Sum1D);

  // cout << "Generated subset sums for halfSet1" << endl;
  printHalfSetSumArrays (nHalfSet1, halfSet1Sum2D, halfSet1Sum1D);

  // Sort the second half set
  sort (halfSet2Sum2D.begin (), halfSet2Sum2D.end (), sortcol);
  sort (halfSet2Sum1D.begin (), halfSet2Sum1D.end ());

  //cout << "My ordered halfSet2Sum \n";
  printHalfSetSumArrays (nHalfSet2, halfSet2Sum2D, halfSet2Sum1D);

  // Vector Iterator for S1 and S2;
  vector<int>::iterator low, high;

  // number of required subsets with desired Sum
  int nSubsetsFound = 0;

  for (int i = 0; i < static_cast<int> (halfSet1Sum2D.size ()); i++)
    {
      // search for lower bound
      low =
          lower_bound (halfSet2Sum1D.begin (), halfSet2Sum1D.end (), lowerLimit - halfSet1Sum1D[i]);
      /*cout << "low = lower_bound(" << *halfSet2Sum1D.begin() << ", " << *(halfSet2Sum1D.end() - 1) << ", " << lowerLimit - halfSet1Sum1D[i] << ") : " << *low << " at position: " << low - halfSet2Sum1D.begin() << endl;
		cout << "i :" << i << " lowerLimit - halfSet1[i]): " << lowerLimit << " - " << halfSet1Sum1D[i] << " = " << lowerLimit - halfSet1Sum1D[i] << endl;*/

      // search for upper bound
      high =
          upper_bound (halfSet2Sum1D.begin (), halfSet2Sum1D.end (), upperLimit - halfSet1Sum1D[i]);
      /*cout << "high = upper_bound(" << *halfSet2Sum1D.begin() << ", " << *(halfSet2Sum1D.end() - 1) << ", " << upperLimit - halfSet1Sum1D[i] << "): " << *high << " at position: " << high - halfSet2Sum1D.begin() << endl;*/
      // Add up to get the desired answer
      size_t nSumElements = (high - low);

      nSubsetsFound += (high - low);

      for (size_t k = 0; k < nSumElements; k++)
        {
          finalSet1D = halfSet1Sum2D[i]; //finalset1D is made up of two parts, half 1 and half 2
          int myLow = distance (halfSet2Sum1D.begin (), low);

          for (size_t m = 2; m < halfSet2Sum2D[myLow + k].size (); m++)
            {
              finalSet1D.push_back (halfSet2Sum2D[myLow + k][m]);
            }
          finalSet1D[0] += halfSet2Sum2D[myLow + k][0]; //sum halfset1sum[0] and halfset2sum[0]
          finalSet1D[1] +=
              halfSet2Sum2D[myLow + k][1]; //sum number of elements in halfset1 and halfset2
          finalSet2D.push_back (finalSet1D);
        }
      sort (finalSet2D.begin (), finalSet2D.end (), sortcol);
    }
  cout << "finalSet2D printout" << endl;
  for (size_t i = 0; i < finalSet2D.size (); i++)
    {
      for (size_t j = 0; j < finalSet2D[i].size (); j++)
        {
          cout << finalSet2D[i][j] << "\t";
        }
      cout << endl;
    }
  cout << "nSubsetsFound: " << nSubsetsFound << endl;

  m_relaysCounter = finalSet2D.size ();

  if (nSubsetsFound > 0)
    {
      m_minSubSumRelays = finalSet2D[0][1];
      cout << "m_minSubSumRelays: " << m_minSubSumRelays << endl;

      m_maxSubSumRelays = finalSet2D[m_relaysCounter - 1][1];
      cout << "m_maxSubSumRelays: " << m_maxSubSumRelays << endl;
    }

  return nSubsetsFound;
}

int
Cluster::getMinSubSumRelays ()
{
  return m_minSubSumRelays;
}
int
Cluster::getMaxSubSumRelays ()
{
  return m_maxSubSumRelays;
}