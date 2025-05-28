#include "../include/Graph.h"

void Graph::treeDecomposition()
{
    printf("begin tree decomposition...\n");
    // degreeBucket[degree] = a series of vertices with the same degree
    vector<vector<int>> degreeBucket;
    // vertex id -> (degree,index)
    vector<pair<int, int>> vertexPosition(n + 1);
    for (int i = 1; i <= n; ++i)
    {
        int degree = edges[i].size();
        if (degree >= degreeBucket.size())
            degreeBucket.resize(degree + 1);
        vertexPosition[i] = make_pair(degree, degreeBucket[degree].size());
        degreeBucket[degree].emplace_back(i);
    }

    // init shortcuts with thr graph
    // vertex id -> neighbor id -> vector(distance,labels)
    vector<map<int, SCAttr>> shortcuts(n + 1);
    for (int i = 1; i <= n; ++i)
        for (auto &edge : edges[i])
            shortcuts[i][edge.target].emplace_back_attr(edge.weight, edge.label);

    // the minimum degree of the remaining graph
    int minDegree = 0;

    // current degree threshold
    int currentThreshold = 0;

    // the number of vertices reduced for the current threshold
    int reducedVertices = 0;

    // down-top
    for (int id = 1; id <= n; ++id)
    {
        while (minDegree < degreeBucket.size() && degreeBucket[minDegree].empty())
        {
            if (minDegree == currentThreshold)
            {
                if (reducedVertices)
                {
                    printf("reducing %d ...\n", minDegree);
                    reducedVertices = 0;
                }
                currentThreshold++;
            }
            minDegree++;
        }

        // no more vertices can be reduced
        if (minDegree == degreeBucket.size())
            break;

        // vertex with minimum degree
        int v = degreeBucket[minDegree].back();
        degreeBucket[minDegree].pop_back();

        reducedVertices++;
        orderId[v] = id;
        orderMap[id] = v;

        // record the valid neighbors that is not deleted
        auto &shortcut = shortcuts[v];

        // the index of valid neighbor
        // NOTE index rather than id
        vector<int> validNeighborIndex;
        for (auto it = shortcut.begin(); it != shortcut.end(); ++it)
            if (!orderId[it->first])
                validNeighborIndex.emplace_back(it->first);

        // record the degree change of each neighbor
        vector<int> degreeIncreaseCount(validNeighborIndex.size(), -1);

        // add shortcuts
        for (int i = 0; i < validNeighborIndex.size(); ++i)
        {
            int &u = validNeighborIndex[i];
            for (int j = i + 1; j < validNeighborIndex.size(); ++j)
            {
                int &w = validNeighborIndex[j];
                // u and w are not neighbors
                if (!shortcuts[u].count(w))
                {
                    degreeIncreaseCount[i]++;
                    degreeIncreaseCount[j]++;
                }

                // add shortcuts
                // NOTE combine shortcuts
                shortcuts[u][w].combine(shortcuts[v][u] + shortcuts[v][w]);
                shortcuts[w][u] = shortcuts[u][w];
            }
        }
        // update the degreeBucket and the associated data structure position
        for (int i = 0; i < validNeighborIndex.size(); ++i)
            if (degreeIncreaseCount[i])
            {
                int &u = validNeighborIndex[i];
                auto [oldDegree, oldIndex] = vertexPosition[u];
                int newDegree = oldDegree + degreeIncreaseCount[i];

                // swap and delete
                degreeBucket[oldDegree][oldIndex] = degreeBucket[oldDegree].back();
                vertexPosition[degreeBucket[oldDegree].back()].second = oldIndex;
                degreeBucket[oldDegree].pop_back();

                // place in a new position
                if (newDegree >= degreeBucket.size())
                    degreeBucket.resize(newDegree + 1);
                minDegree = min(minDegree, newDegree);
                vertexPosition[u] = make_pair(newDegree, degreeBucket[newDegree].size());
                degreeBucket[newDegree].emplace_back(u);
            }
        // build tree node
        // DONE add neighbors into tree node X(v)
        trees[v].neighbors.reserve(validNeighborIndex.size());
        for (int i = 0; i < validNeighborIndex.size(); ++i)
        {
            int &neighbor = validNeighborIndex[i];
            trees[v].neighbors.emplace_back(neighbor);
        }
        trees[v].width = trees[v].neighbors.size();
    }

    buildTree();

    this->shortcuts = move(shortcuts);
}

void Graph::refine()
{
    printf("begin refining tree structure...\n");
    // record an edge whether need to be removed
    vector<unordered_map<int, unordered_map<int, int>>> flags(n + 1);

    // refine
    for (int order = n; order; order--)
    {
        int &v = orderMap[order];
        for (auto u : trees[v].neighbors)
        {
            for (auto w : trees[v].neighbors)
            {
                if (u == w)
                    continue;
                auto &shortcut = shortcuts[v][u];
                auto &attrs = (orderId[u] < orderId[w]) ? shortcuts[u][w].attrs : shortcuts[w][u].attrs;
                for (int i = 0; i < shortcut.attrs.size(); i++)
                {
                    auto &[d1, s1] = shortcut.attrs[i];
                    int p = 0;
                    auto &attrs = shortcuts[u][w].attrs;
                    for (; p < attrs.size(); p++)
                        if (s1.includes(attrs[p].second))
                            break;

                    int j = 0;
                    auto &attrs2 = shortcuts[v][w].attrs;
                    for (; j < attrs2.size(); j++)
                        if (s1.includes(attrs2[j].second))
                            break;

                    if (p < attrs.size() && j < attrs2.size())
                    {
                        auto &[d, s] = attrs[p];
                        auto &[d2, s2] = attrs2[j];

                        if (d1 >= d2 + d && s1.includes(s2 + s))
                        {
                            d1 = d2 + d;
                            s1 = s2 + s;
                            flags[v][u][i] = 1;
                        }
                    }
                }
            }
        }
    }

    // remove all marked edges
    for (int v = 1; v <= n; v++)
    {
        for (auto &u : trees[v].neighbors)
        {
            auto &attrs = shortcuts[v][u].attrs;
            int ptr = 0;
            for (int i = 0; i < attrs.size(); i++)
            {
                if (!flags[v][u][i])
                {
                    if (ptr < i)
                        attrs[ptr] = move(attrs[i]);
                    ptr++;
                }
            }
            attrs.erase(attrs.begin() + ptr, attrs.end());
        }
    }

    // remove useless neighbors
    for (int v = 1; v <= n; v++)
    {
        vector<int> neighbors;
        neighbors.reserve(trees[v].neighbors.size());
        for (auto &u : trees[v].neighbors)
        {
            if (!shortcuts[v][u].empty())
            {
                neighbors.emplace_back(u);
                // descendants[u].emplace_back(v);
            }
        }
        trees[v].neighbors = neighbors;
        trees[v].width = neighbors.size();
    }
}

void Graph::constructIndex()
{
    // down-top(knn)
    printf("start building the index from down to top...\n");
    for (int id = 1; id <= n; id++)
    {
        int v = orderMap[id];
        int poiV = checkPOI(v);
        trees[v].list.compensate(v, poiV, n, k);
        for (auto &u : trees[v].neighbors)
            trees[u]
                .list.combine(u, IndexList::join(u, shortcuts[v][u], trees[v].list, poiV, n, k), n, k);
    }

    // top-down(knn)
    printf("start building the index from top to down...\n");
    for (int id = n; id; id--)
    {
        int v = orderMap[id];
        for (auto &u : trees[v].neighbors)
        {
            int poiU = checkPOI(u);
            trees[v]
                .list.combine(v,
                              IndexList::join(v, shortcuts[v][u], trees[u].list, poiU, n, k), n, k);
        }
        int poiV = checkPOI(v);
        trees[v].list.compensate(v, poiV, n, k);
    }
}

void Graph::buildIndex()
{
    auto start = chrono::high_resolution_clock::now();

    treeDecomposition();
    refine();
    constructIndex();

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;
    printf("indexing time:%.2lfs\n", duration.count());

    calculateStats();
    reportIndexSize();
}

void Graph::prepareUpdate()
{
    treeDecomposition();
    refine();

    for (int v = 1; v <= n; v++)
        for (auto &u : trees[v].neighbors)
            descendants[u].emplace_back(v);
}

void Graph::buildTree()
{
    printf("begin merge tree nodes...\n");
    // merge tree node
    int cnt = 0;
    for (int v = 1; v <= n; ++v)
    {
        // trees[v].width = trees[v].neighbors.size();
        auto &neighbors = trees[v].neighbors;
        if (!neighbors.empty())
        {
            int minOrder = INT_MAX;
            int parentId = -1;
            for (auto &u : neighbors)
                if (orderId[u] < minOrder)
                {
                    minOrder = orderId[u];
                    parentId = u;
                }
            trees[v].parent = parentId;
            trees[parentId].children.emplace_back(v);
        }
        else
        {
            trees[v].parent = -1;
            cnt++;
        }
    }
    printf("the number of root nodes:%d\n", cnt);
}

void Graph::treeDecompositionByOrder()
{
    printf("begin ordered tree decomposition...\n");
    // init shortcuts with thr graph
    // vertex id -> neighbor id -> vector(distance,labels)
    vector<map<int, SCAttr>> shortcuts(n + 1);
    for (int i = 1; i <= n; ++i)
        for (auto &edge : edges[i])
            shortcuts[i][edge.target].emplace_back_attr(edge.weight, edge.label);

    // down-top
    for (int id = 1; id <= n; ++id)
    {
        int v = orderMap[id];

        // record the valid neighbors that is not deleted
        auto &shortcut = shortcuts[v];

        // the index of valid neighbor
        // NOTE index rather than id
        vector<int> validNeighborIndex;
        for (auto it = shortcut.begin(); it != shortcut.end(); ++it)
            if (orderId[it->first] > id)
                validNeighborIndex.emplace_back(it->first);

        // add shortcuts
        for (int i = 0; i < validNeighborIndex.size(); ++i)
        {
            int &u = validNeighborIndex[i];

            for (int j = i + 1; j < validNeighborIndex.size(); ++j)
            {
                int &w = validNeighborIndex[j];
                // add shortcuts
                shortcuts[u][w].combine(shortcuts[v][u] + shortcuts[v][w]);
                shortcuts[w][u] = shortcuts[u][w];
            }
        }

        // build tree node
        // DONE add neighbors into tree node X(v)
        trees[v].neighbors.reserve(validNeighborIndex.size());
        for (int i = 0; i < validNeighborIndex.size(); ++i)
        {
            int &neighbor = validNeighborIndex[i];
            trees[v].neighbors.emplace_back(neighbor);
        }
        trees[v].width = trees[v].neighbors.size();
    }
    buildTree();

    this->shortcuts = shortcuts;
}

void Graph::getOrder(string orderPath)
{
    treeDecomposition();

    // restore order
    ofstream fout(orderPath);
    for (int v = 1; v <= n; v++)
        fout << orderId[v] << "\n";
    fout.close();
}
