#pragma once

#include "utils.h"
#include "GraphIndex.h"

// define the basic structure of a graph and online search method for query
class Graph
{
public:
    // the number of vertices and edges
    int n, m;

    // mark whether each vertex is a POI
    vector<bool> isPOI;

    // the neighbors of each edge
    vector<vector<GraphEdge>> edges;

    // uniform label size
    int labelSize;

    int k;

    // vertex id -> order id
    vector<int> orderId;
    // order id -> vertex id
    vector<int> orderMap;

    // tree structure
    vector<TreeNode> trees;

    // int originalLabelCount = 0;
    // int compactLabelCount = 0;

    vector<map<int, SCAttr>> shortcuts;

    vector<vector<int>> descendants;

    // load graph file
    Graph(string graphPath, int labelSize, int k);

    // represent the original labels using letters
    uint resignLabel(string oldLabel);

    // refine labels to a unified number
    void reduceLabel();

    // show label information
    void showLabel();

    // load POI of graph
    void loadPOI(string poiPath);

    void loadPOI();

    // check vertex v is POI
    int checkPOI(int v);

    // obtain the node order in a tree decomposition
    void getOrder(string orderPath);

    // load vertex order of graph
    void loadOrder(string orderPath);

    void treeDecomposition();

    void treeDecompositionByOrder();

    void refine();

    // construct LC-Index in two phases
    void constructIndex();

    // build LC-Index in three steps
    void buildIndex();

    // prepare for object updates
    void prepareUpdate();

    void showShortcuts();

    // determine the parent-children relationships between tree nodes
    void buildTree();

    // calculate the attributes of tree
    void calculateStats();

    // report index size
    void reportIndexSize();

    // show the tree structure
    void showStructure();

    // check the correctness of all indices
    void checkIndex();

    // query label-constrained knn of s using dijkstra
    vector<PDI> dijkstra(int s, LabelSet labels);

    // compute the number of connected components in the graph
    void countComponent();

    // query label-constrained knn of s using indices
    vector<PDI> query(int u, LabelSet labels);

    vector<PDI> query(IndexList &indexList, int u, LabelSet &labels);

    void showIndex();

    void showKNN();

    // store index in a file in an easily viewable manner
    void storeIndex1(string indexPath);

    // store index in a file in a faster manner
    void storeIndex2(string indexPath);

    // load index from a file
    void loadIndex1(string indexPath);

    void loadIndex2(string indexPath);

    // process 10 groups queries
    void queryByGroup(string queryFolder);

    // process 10,000 queries
    void query(string queryFolder);

    // randomly generate 10,000 queries and process them
    void query();

    int generateRandomNumber(int left, int right);

    string generateRandomLabel(int nums);

    void clear();

    // reconstruct index from scratch to handle object updates
    void reconstructIndex(string poiFolder);

    // process object updates one by one
    void updatePOI(string poiFolder);

    // process batch object updates
    void batchUpdate(string poiFolder, string operation);

    /*
    functions about object insertion
    */

    // process object insertion one by one
    void insertPOI(string folder);

    // maintain index after insert an candidate vertex
    void insertPOI(int insertedVertex);

    vector<SCAttr> singleInsert(int insertedVertex);

    // process batch object insertions
    void batchInsert(set<int> &poi);

    vector<int> getAncestor(int u);

    // (id,distance,labels)
    tuple<int, double, LabelSet> getLastNeighbor(int v, LabelSet labels);

    bool checkInsertion(int v, SCAttr &path);

    // remove useless paths
    void removeInsertion(int &v, SCAttr &path);

    // update own index
    void updateInsertion(int &v);

    // update index of other vertices
    void updateInsertion(int insertedVertex, int &v, SCAttr &path);

    /*
    functions about object deletions
    */

    // process object deletion one by one
    void deletePOI(string folder);

    // only check partial vertices
    void deletePOI(int deletedVertex);

    unordered_map<int, bool> singleDelete(int deletedVertex);

    // process batch object deletions
    void batchDelete(set<int> &poi);
};