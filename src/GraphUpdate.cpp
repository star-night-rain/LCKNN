#include "../include/Graph.h"

void Graph::insertPOI(int insertedVertex)
{
    // record whether the index of each vertex has changed
    vector<bool> indexChanged(n + 1, false);
    indexChanged[insertedVertex] = true;

    // record paths between self and w
    vector<SCAttr> paths(n + 1);

    // update index from down to top
    vector<int> ancestors = getAncestor(insertedVertex);
    for (auto &v : ancestors)
    {
        if (!indexChanged[v])
            continue;
        // update the distances of neighbors
        for (auto &u : trees[v].neighbors)
        {
            paths[u].combine(paths[v] + shortcuts[v][u]);
            // check
            if (!indexChanged[u] && checkInsertion(u, paths[u]))
                indexChanged[u] = true;
        }
    }

    priority_queue<int> que;
    vector<bool> status(n + 1, false);
    for (auto &v : ancestors)
        if (indexChanged[v])
        {
            que.push(orderId[v]);
            status[v] = true;
        }
    // update index from top to down
    while (!que.empty())
    {
        int id = que.top();
        que.pop();
        int v = orderMap[id];

        if (v == insertedVertex)
        {
            for (auto &u : descendants[v])
                if (!status[u])
                {
                    que.push(orderId[u]);
                    status[u] = true;
                }
            continue;
        }

        // compute complete distance
        for (auto &u : trees[v].neighbors)
            if (indexChanged[u])
                paths[v].combine(paths[u] + shortcuts[v][u]);

        // check
        if (!indexChanged[v] && checkInsertion(v, paths[v]))
            indexChanged[v] = true;

        // insert descendants to queue
        if (indexChanged[v])
        {
            for (auto &u : descendants[v])
                if (!status[u])
                {
                    que.push(orderId[u]);
                    status[u] = true;
                }
        }
    }
    // update index
    for (int v = 1; v <= n; v++)
        if (indexChanged[v])
        {
            if (v == insertedVertex)
                updateInsertion(v);
            else
            {
                removeInsertion(v, paths[v]);
                updateInsertion(insertedVertex, v, paths[v]);
            }
        }
    // insert poi
    isPOI[insertedVertex] = true;
}

vector<SCAttr> Graph::singleInsert(int insertedVertex)
{
    // record whether the index of each vertex has changed
    vector<bool> indexChanged(n + 1, false);
    indexChanged[insertedVertex] = true;

    // record paths between self and w
    vector<SCAttr> paths(n + 1);

    // update index from down to top
    vector<int> ancestors = getAncestor(insertedVertex);
    for (auto &v : ancestors)
    {
        if (!indexChanged[v])
            continue;
        // update the distances of neighbors
        for (auto &u : trees[v].neighbors)
        {
            paths[u].combine(paths[v] + shortcuts[v][u]);
            // check
            if (!indexChanged[u] && checkInsertion(u, paths[u]))
                indexChanged[u] = true;
        }
    }

    priority_queue<int> que;
    vector<bool> status(n + 1, false);
    for (auto &v : ancestors)
        if (indexChanged[v])
        {
            que.push(orderId[v]);
            status[v] = true;
        }
    // update index from top to down
    while (!que.empty())
    {
        int id = que.top();
        que.pop();
        int v = orderMap[id];

        if (v == insertedVertex)
        {
            for (auto &u : descendants[v])
                if (!status[u])
                {
                    que.push(orderId[u]);
                    status[u] = true;
                }
            continue;
        }

        // compute complete distance
        for (auto &u : trees[v].neighbors)
            if (indexChanged[u])
                paths[v].combine(paths[u] + shortcuts[v][u]);

        // check
        if (!indexChanged[v] && checkInsertion(v, paths[v]))
            indexChanged[v] = true;

        // insert descendants to queue
        if (indexChanged[v])
        {
            for (auto &u : descendants[v])
                if (!status[u])
                {
                    que.push(orderId[u]);
                    status[u] = true;
                }
        }
    }

    // for (int v = 1; v <= n; v++)
    //     if (indexChanged[v])
    //     {
    //         if (v == insertedVertex)
    //             updateInsertion(v);
    //         else
    //         {
    //             removeInsertion(v, paths[v]);
    //             updateInsertion(insertedVertex, v, paths[v]);
    //         }
    //     }

    return paths;
}

void Graph::batchInsert(set<int> &poi)
{
    vector<IndexList> receivedList(n + 1);

    for (auto &p : poi)
    {
        auto paths = singleInsert(p);
        for (int v = 1; v <= n; v++)
            if (!paths[v].empty())
            {
                auto &attrs = paths[v].attrs;
                set<uint> st;
                for (auto &[d, s] : attrs)
                {
                    if (st.count(s.getLabels()))
                        printf("redundancy\n");
                    st.insert(s.getLabels());
                }
                // sort(attrs.begin(), attrs.end());
                // paths[v].removeRedundancy();
                receivedList[v].combine(v, paths[v], p, n, k);
            }
    }

    // for (int v = 1; v <= n; v++)
    // {
    //     set<uint> st;
    //     for (auto &[s, knn] : receivedList[v].list)
    //     {
    //         if (st.count(s.getLabels()))
    //             printf("redundancy\n");
    //         st.insert(s.getLabels());
    //     }
    // }

    for (int v = 1; v <= n; v++)
        if (!receivedList[v].list.empty())
        {
            trees[v].list.combine(v, receivedList[v], n, k);
            int poiV = checkPOI(v);
            trees[v].list.compensate(v, poiV, n, k);
        }

    for (auto v : poi)
    {
        updateInsertion(v);
        isPOI[v] = true;
    }
}

vector<int> Graph::getAncestor(int u)
{
    vector<int> ancestors;
    ancestors.reserve(trees[u].height);
    while (u != -1)
    {
        ancestors.emplace_back(u);
        u = trees[u].parent;
    }
    return ancestors;
}

tuple<int, double, LabelSet> Graph::getLastNeighbor(int u, LabelSet labels)
{
    LabelSet originalLabels;

    vector<PDI> result;
    result.reserve(k);
    set<uint> st;
    int num = k;
    if (checkPOI(u))
    {
        result.emplace_back(0, u);
        st.insert(u);
        num--;
    }
    vector<pair<LabelSet, KNNList>> querySet;
    for (auto [s, knn] : trees[u].list.list)
        if (labels.includes(s))
            querySet.emplace_back(s, knn);
    vector<bool> isRemoved(querySet.size());
    while (num--)
    {
        int v = 0;
        double dist = inf;
        for (int j = 0; j < querySet.size(); j++)
        {
            auto &[s, knn] = querySet[j];
            bool flag = true;
            for (int p = 0; p < knn.size(); p++)
            {
                auto &[d, w] = knn.list[p];
                if (st.count(w) == 0)
                {
                    if (d < dist)
                    {
                        v = w;
                        dist = d;
                        originalLabels = s;
                    }
                    knn.list.erase(knn.list.begin(), knn.list.begin() + p);
                    flag = false;
                    break;
                }
            }
            isRemoved[j] = flag;
        }
        for (int j = querySet.size() - 1; j >= 0; j--)
            if (isRemoved[j])
                querySet.erase(querySet.begin() + j);
        if (v)
        {
            result.emplace_back(dist, v);
            st.insert(v);
        }
        else
            break;
    }
    return make_tuple(k - num - 1, result.back().first, originalLabels);
}

// NOTE improve: consider only the edges that will affect the index
bool Graph::checkInsertion(int v, SCAttr &path)
{
    int ptr = 0;
    auto &attrs = path.attrs;
    for (auto &[d1, s] : attrs)
    {
        auto [id, d2, _] = getLastNeighbor(v, s);
        if (d1 < d2 || id < k)
        {
            // remove useless path
            attrs.erase(attrs.begin(), attrs.begin() + ptr);
            return true;
        }
        ptr++;
    }
    return false;
}

void Graph::removeInsertion(int &v, SCAttr &path)
{
    auto &attrs = path.attrs;
    vector<bool> isRemoved(attrs.size(), false);
    for (int i = 0; i < attrs.size(); i++)
    {
        auto &[d1, s] = attrs[i];
        auto [id, d2, _] = getLastNeighbor(v, s);
        if (d1 >= d2 && id == k)
            isRemoved[i] = true;
    }
    int ptr = 0;
    for (int i = 0; i < attrs.size(); i++)
        if (!isRemoved[i])
        {
            if (ptr < i)
                attrs[ptr] = move(attrs[i]);
            ptr++;
        }
    attrs.erase(attrs.begin() + ptr, attrs.end());
}

// NOTE  the index size may be smaller that k
// label may decrease but not increase
void Graph::updateInsertion(int &v)
{
    auto &list = trees[v].list.list;

    // compute last nearest neighbor
    vector<int> ids(list.size());
    vector<LabelSet> indexLabels(list.size());
    for (int i = 0; i < list.size(); i++)
    {
        auto [lastId, _, lastLabels] = getLastNeighbor(v, list[i].first);
        ids[i] = lastId;
        indexLabels[i] = lastLabels;
    }
    // printf("show id and labels\n");
    // for (int i = 0; i < list.size(); i++)
    // {
    //     printf("id:%d,labels:%s\n", ids[i], indexLabels[i].c_str().c_str());
    // }

    // update index
    vector<bool> isRemoved(list.size(), false);
    for (int i = 0; i < list.size(); i++)
    {
        auto &[labels, knnList] = list[i];
        auto &knn = knnList.list;
        // auto [id, _, indexLabels] = getLastNeighbor(v, labels);

        int lastId = ids[i];
        LabelSet lastLabels = indexLabels[i];
        // auto [lastId, _, lastLabels] = getLastNeighbor(v, labels);

        if (lastId == k && lastLabels == labels)
        {
            knn.pop_back();
            if (knn.empty())
                isRemoved[i] = true;
        }
    }
    // remove empty index
    int ptr = 0;
    for (int i = 0; i < list.size(); i++)
        if (!isRemoved[i])
        {
            if (ptr < i)
                list[ptr] = move(list[i]);
            ptr++;
        }
    list.erase(list.begin() + ptr, list.end());
}

// NOTE improve: improve performance by modifying the order of paths
void Graph::updateInsertion(int insertedVertex, int &v, SCAttr &path)
{
    auto attrs = path.attrs;
    auto &list = trees[v].list.list;
    sort(attrs.begin(), attrs.end(), [](const pair<double, LabelSet> &p1, const pair<double, LabelSet> &p2)
         { return p1.second < p2.second; });

    vector<LabelSet> labels;
    labels.reserve(attrs.size() + list.size());

    // 0: only belong to index
    // 1: belong path and index
    // 2: only belong to path
    vector<int> status(attrs.size() + list.size(), 0);

    // combine labels
    int p1 = 0, p2 = 0;
    while (p1 < attrs.size() && p2 < list.size())
    {
        auto &[_, s1] = attrs[p1];
        auto &[s2, __] = list[p2];
        if (s1 < s2)
        {
            status[labels.size()] = 2;
            labels.emplace_back(s1);
            p1++;
        }
        else if (s2 < s1)
        {
            labels.emplace_back(s2);
            p2++;
        }
        else
        {
            status[labels.size()] = 1;
            labels.emplace_back(s1);
            p1++;
            p2++;
        }
    }
    while (p1 < attrs.size())
    {
        auto &[_, s] = attrs[p1];
        status[labels.size()] = 2;
        labels.emplace_back(s);
        p1++;
    }
    while (p2 < list.size())
    {
        auto &[s, _] = list[p2];
        labels.emplace_back(s);
        p2++;
    }

    // compute distance
    vector<double> dist(labels.size(), inf);
    int ptr = 0;
    for (int i = 0; i < labels.size(); i++)
    {
        if (!status[i])
            continue;
        auto &s1 = labels[i];
        auto &[pathDist, pathLabels] = attrs[ptr];
        ptr++;

        dist[i] = min(dist[i], pathDist);
        for (int j = i + 1; j < labels.size(); j++)
        {
            auto &s2 = labels[j];
            if (s2.includes(s1))
                dist[j] = min(dist[j], pathDist);
        }
    }

    // compute last nearest neighbor
    vector<int> ids(labels.size());
    vector<double> indexDist(labels.size());
    vector<LabelSet> indexLabels(labels.size());

    for (int i = 0; i < labels.size(); i++)
    {
        if (dist[i] == inf)
            continue;
        auto [lastId, lastDist, lastLabels] = getLastNeighbor(v, labels[i]);
        ids[i] = lastId;
        indexDist[i] = lastDist;
        indexLabels[i] = lastLabels;
    }

    vector<int> isRemoved(labels.size(), false);
    // update index
    for (int i = 0; i < labels.size(); i++)
    {
        if (dist[i] == inf)
            continue;
        int &lastId = ids[i];
        double &lastDist = indexDist[i];
        LabelSet &lastLabels = indexLabels[i];

        LabelSet &pathLabels = labels[i];
        double &pathDist = dist[i];
        if (status[i] == 0)
        {
            // KNN is affected, but the inserted vertex already exists in the index of the subset
            // remove last vertex
            if (lastId == k && pathDist < lastDist && pathLabels == lastLabels)
            {
                auto &[_, knnList] = list[i];
                auto &knn = knnList.list;
                knn.pop_back();
                if (knn.empty())
                    isRemoved[i] = true;
            }
        }
        else if (status[i] == 1)
        {
            auto &[_, knnList] = list[i];
            auto &knn = knnList.list;
            // add
            if (lastId < k)
            {
                auto it = upper_bound(knn.begin(), knn.end(), pathDist, [](const double &d, const PDI &p)
                                      { return d < p.first; });
                knn.insert(it, make_pair(pathDist, insertedVertex));
            }
            // add and remove
            else if (lastId == k && pathDist < lastDist)
            {
                // add
                auto it = upper_bound(knn.begin(), knn.end(), pathDist, [](const double &d, const PDI &p)
                                      { return d < p.first; });
                knn.insert(it, make_pair(pathDist, insertedVertex));
                // printf("site:%d\n", it - knn.begin());
                // remove
                if (pathLabels == lastLabels)
                    knn.pop_back();
            }
        }
        // removeInsertion assures pathDist < lastDist or id < k
        else if (status[i] == 2)
            list.insert(list.begin() + i, make_pair(pathLabels, KNNList(pathDist, insertedVertex)));
    }

    ptr = 0;
    for (int i = 0; i < labels.size(); i++)
        if (!isRemoved[i])
        {
            if (ptr < i)
                list[ptr] = move(list[i]);
            ptr++;
        }
    list.erase(list.begin() + ptr, list.end());
}

void Graph::deletePOI(int deletedVertex)
{
    isPOI[deletedVertex] = false;
    vector<int> ancestors = getAncestor(deletedVertex);

    // record the id of those vertices whose index require updating
    set<int, greater<int>> idSet;

    unordered_map<int, bool> deleteStatus;

    deleteStatus[deletedVertex] = true;
    for (int i = 1; i < ancestors.size(); i++)
    {
        int v = ancestors[i];
        if (trees[v].list.hasVertex(deletedVertex))
            deleteStatus[v] = true;
        else
            deleteStatus[v] = false;
    }

    priority_queue<int> que;
    vector<bool> status(n + 1, false);
    for (auto &v : ancestors)
        if (deleteStatus[v])
        {
            que.push(orderId[v]);
            idSet.insert(orderId[v]);
            status[v] = true;
        }

    while (!que.empty())
    {
        int id = que.top();
        que.pop();
        int v = orderMap[id];
        if (!deleteStatus[v] && trees[v].list.hasVertex(deletedVertex))
        {
            deleteStatus[v] = true;
            idSet.insert(orderId[v]);
        }
        if (deleteStatus[v])
            for (auto &u : descendants[v])
                if (!status[u])
                {
                    que.push(orderId[u]);
                    status[u] = true;
                }
    }

    // update index from bottom to top
    for (auto it = idSet.rbegin(); it != idSet.rend(); ++it)
    {
        int &v = orderMap[*it];

        for (auto &u : descendants[v])
        {
            int poiU = checkPOI(u);
            trees[v].list.combine(v, IndexList::join(v, shortcuts[v][u], trees[u].list, poiU, n, k), n, k);
        }
        int poiV = checkPOI(v);
        trees[v].list.compensate(v, poiV, n, k);
    }

    // update index from top to down
    for (int id : idSet)
    {
        int &v = orderMap[id];

        for (auto &u : trees[v].neighbors)
        {
            int poiU = checkPOI(u);
            trees[v].list.combine(v, IndexList::join(v, shortcuts[v][u], trees[u].list, poiU, n, k), n, k);
        }
        int poiV = checkPOI(v);
        trees[v].list.compensate(v, poiV, n, k);
    }
}

unordered_map<int, bool> Graph::singleDelete(int deletedVertex)
{
    isPOI[deletedVertex] = false;
    vector<int> ancestors = getAncestor(deletedVertex);

    unordered_map<int, bool> deleteStatus;

    deleteStatus[deletedVertex] = true;
    for (int i = 1; i < ancestors.size(); i++)
    {
        int v = ancestors[i];
        if (trees[v].list.hasVertex(deletedVertex))
            deleteStatus[v] = true;
        else
            deleteStatus[v] = false;
    }

    priority_queue<int> que;
    vector<bool> status(n + 1, false);
    for (auto &v : ancestors)
        if (deleteStatus[v])
        {
            que.push(orderId[v]);
            status[v] = true;
        }

    while (!que.empty())
    {
        int id = que.top();
        que.pop();
        int v = orderMap[id];

        if (!deleteStatus[v] && trees[v].list.hasVertex(deletedVertex))
            deleteStatus[v] = true;

        if (deleteStatus[v])
            for (auto &u : descendants[v])
                if (!status[u])
                {
                    que.push(orderId[u]);
                    status[u] = true;
                }
    }

    return deleteStatus;
}

void Graph::batchDelete(set<int> &poi)
{
    set<int, greater<int>> idSet;
    vector<bool> deleteStatus(n + 1, false);

    for (auto &p : poi)
    {
        auto status = singleDelete(p);
        for (int v = 1; v <= n; v++)
            if (status[v])
                deleteStatus[v] = true;
    }

    for (int v = 1; v <= n; v++)
        if (deleteStatus[v])
            idSet.insert(orderId[v]);

    // update index from bottom to top
    for (auto it = idSet.rbegin(); it != idSet.rend(); ++it)
    {
        int &v = orderMap[*it];
        for (auto &u : descendants[v])
        {
            int poiU = checkPOI(u);
            trees[v].list.combine(v, IndexList::join(v, shortcuts[v][u], trees[u].list, poiU, n, k), n, k);
        }
        int poiV = checkPOI(v);
        trees[v].list.compensate(v, poiV, n, k);
    }

    // update index from top to down
    for (int id : idSet)
    {
        int &v = orderMap[id];
        for (auto &u : trees[v].neighbors)
        {
            int poiU = checkPOI(u);
            trees[v].list.combine(v, IndexList::join(v, shortcuts[v][u], trees[u].list, poiU, n, k), n, k);
        }
        int poiV = checkPOI(v);
        trees[v].list.compensate(v, poiV, n, k);
    }

    printf("affected vertex:%ld\n", idSet.size());
}

void Graph::insertPOI(string folder)
{
    printf("begin insert poi...\n");

    set<int> poi;
    ifstream fin(folder);
    int v;
    while (fin >> v)
        poi.insert(v);
    fin.close();

    auto start = chrono::high_resolution_clock::now();
    for (auto &v : poi)
        insertPOI(v);
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;
    printf("update time after insertion of POIs:%.2lfs\n", duration.count());
}

void Graph::deletePOI(string folder)
{
    printf("begin delete poi...\n");

    set<int> poi;
    ifstream fin(folder);
    int v;
    while (fin >> v)
        poi.insert(v);
    fin.close();

    auto start = chrono::high_resolution_clock::now();
    for (auto &v : poi)
        deletePOI(v);
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;
    printf("update time after deletion of POIs:%.2lfs\n", duration.count());
}

void Graph::updatePOI(string poiFolder)
{
    printf("begin update poi...\n");
    vector<pair<char, int>> updatedPOI;
    char c;
    int v;
    ifstream fin(poiFolder);
    while (fin >> c >> v)
        updatedPOI.emplace_back(c, v);
    fin.close();

    int insertSum = 0;
    int deleteSum = 0;

    double insertTime = 0;
    double deleteTime = 0;
    for (auto &[c, v] : updatedPOI)
    {
        auto start = high_resolution_clock::now();
        if (c == 'i')
            insertPOI(v);
        else if (c == 'd')
            deletePOI(v);
        auto end = high_resolution_clock::now();
        duration<double> duration = end - start;

        if (c == 'i')
        {
            insertTime += duration.count();
            insertSum++;
        }
        else if (c == 'd')
        {
            deleteTime += duration.count();
            deleteSum++;
        }
    }
    int sum = insertSum + deleteSum;
    insertTime *= 1000;
    deleteTime *= 1000;
    printf("average update time of updating %d POIs:%.2lfms\n", sum, (insertTime + deleteTime) / sum);
    if (insertSum)
        printf("average update time of inserting %d POIs:%.2lfms\n", insertSum, insertTime / insertSum);
    if (deleteSum)
        printf("average update time of deleting %d POIs:%.2lfms\n", deleteSum, deleteTime / deleteSum);
}

void Graph::batchUpdate(string poiFolder, string operation)
{
    printf("begin update poi...\n");
    set<int> poi;
    char c;
    int v;
    ifstream fin(poiFolder);
    while (fin >> c >> v)
        poi.insert(v);
    fin.close();

    auto start = high_resolution_clock::now();
    if (operation == "insert")
        batchInsert(poi);
    else if (operation == "delete")
        batchDelete(poi);
    auto end = high_resolution_clock::now();
    duration<double> duration = end - start;

    duration *= 1000;
    if (operation == "insert")
        printf("total update time of inserting %d POIs:%.2lfms\n", poi.size(), duration);
    else if (operation == "delete")
        printf("total update time of deleting %d POIs:%.2lfms\n", poi.size(), duration);
}

void Graph::reconstructIndex(string poiFolder)
{
    printf("begin update poi...\n");
    vector<pair<char, int>> updatedPOI;
    char c;
    int v;
    ifstream fin(poiFolder);
    while (fin >> c >> v)
        updatedPOI.emplace_back(c, v);
    fin.close();

    int insertSum = 0;
    int deleteSum = 0;

    double insertTime = 0;
    double deleteTime = 0;
    for (auto &[c, v] : updatedPOI)
    {
        auto start = high_resolution_clock::now();
        if (c == 'i')
        {
            isPOI[v] = true;
            clear();
            buildIndex();
        }
        else if (c == 'd')
        {
            isPOI[v] = false;
            clear();
            buildIndex();
        }
        auto end = high_resolution_clock::now();
        duration<double> duration = end - start;
        if (c == 'i')
        {
            insertTime += duration.count();
            insertSum++;
        }
        else if (c == 'd')
        {
            deleteTime += duration.count();
            deleteSum++;
        }
    }
    int sum = insertSum + deleteSum;
    insertTime *= 1000;
    deleteTime *= 1000;
    printf("average update time of updating %d POIs:%.2lfms\n", sum, (insertTime + deleteTime) / sum);
    printf("average update time of inserting %d POIs:%.2lfms\n", insertSum, insertTime / insertSum);
    printf("average update time of deleting %d POIs:%.2lfms\n", deleteSum, deleteTime / deleteSum);
}