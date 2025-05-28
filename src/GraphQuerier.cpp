#include "../include/Graph.h"

vector<PDI> Graph::dijkstra(int s, LabelSet labels)
{
    vector<double> dist(n + 1, INT_MAX);
    dist[s] = 0;
    vector<bool> st(n + 1, false);
    priority_queue<PDI, vector<PDI>, greater<PDI>> heap;
    heap.push({0, s});
    vector<PDI> result;
    result.reserve(k);
    while (!heap.empty())
    {
        auto p = heap.top();
        heap.pop();
        auto &[d, v] = p;
        if (st[v])
            continue;
        st[v] = true;
        int poi = checkPOI(v);
        if (poi)
            result.emplace_back(d, v);
        // if (poi && v != s)
        //     result.emplace_back(d, v);
        if (result.size() == k)
            break;
        for (auto &edge : edges[v])
            if (labels.includes(edge.label) && d + edge.weight < dist[edge.target])
            {
                dist[edge.target] = d + edge.weight;
                heap.push({dist[edge.target], edge.target});
            }
    }
    return result;
}

vector<PDI> Graph::query(int u, LabelSet labels)
{
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
    for (int i = 1; i <= num; i++)
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
    return result;
}

vector<PDI> Graph::query(IndexList &indexList, int u, LabelSet &labels)
{
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
    for (auto [s, knn] : indexList.list)
        if (labels.includes(s))
            querySet.emplace_back(s, knn);
    vector<bool> isRemoved(querySet.size());
    for (int i = 1; i <= num; i++)
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
    return result;
}

int Graph::generateRandomNumber(int left, int right)
{
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(left, right);
    return dis(gen);
}

string Graph::generateRandomLabel(int nums)
{
    string labels;
    set<int> st;
    while (st.size() < nums)
    {
        int label = generateRandomNumber(1, labelSize);
        if (!st.count(label))
        {
            st.insert(label);
            labels += 'a' + label - 1;
        }
    }
    return labels;
}

void Graph::queryByGroup(string queryFolder)
{
    double queryTotalTime = 0;
    double dijkstraTotalTime = 0;
    for (int i = 1; i <= 10; i++)
    {
        string queryPath = queryFolder + "/query" + to_string(i) + ".txt";
        ifstream fin(queryPath);
        int v;
        string labels;
        int wrong = 0;
        cerr << fixed << setprecision(2);
        double queryTime = 0;
        double dijkstraTime = 0;
        while (fin >> v >> labels)
        {
            auto queryStart = chrono::high_resolution_clock::now();

            auto knn1 = query(v, labels);
            auto queryEnd = chrono::high_resolution_clock::now();
            chrono::duration<double> queryDuration = queryEnd - queryStart;
            queryTime += queryDuration.count();

            auto dijkstraStart = chrono::high_resolution_clock::now();
            auto knn2 = dijkstra(v, labels);
            auto dijkstraEnd = chrono::high_resolution_clock::now();
            chrono::duration<double> dijkstraDuration = dijkstraEnd - dijkstraStart;
            dijkstraTime += dijkstraDuration.count();

            bool flag = false;
            for (int i = 0; i < knn1.size(); i++)
            {
                auto &[d1, v1] = knn1[i];
                auto &[d2, v2] = knn2[i];
                if (abs(d1 - d2) > 1e-8)
                {
                    flag = true;
                    wrong++;
                    cerr << "wrong:" << wrong << "\n";
                    break;
                }
            }
            if (flag)
            {
                cerr << "v:" << v << "\n";
                cerr << "label: " << labels.c_str() << "\n";
                cerr << "query\n";
                cerr << "query size: " << knn1.size() << "\n";

                for (const auto &[d, v] : knn1)
                    cerr << "(" << d << ", " << v << ") ";
                cerr << "\n";

                cerr << "dijkstra\n";
                cerr << "dijkstra size: " << knn2.size() << "\n";

                for (const auto &[d, v] : knn2)
                    cerr << "(" << d << ", " << v << ") ";
                cerr << "\n\n";
            }
        }
        printf("the total time token for the %d-th group of queries using index is %.2lfms\n", i, queryTime * 1000);
        printf("the total time token for the %d-th group of queries using dijkstra algorithm is %.2lfms\n\n", i, dijkstraTime * 1000);
        fin.close();
        queryTotalTime += queryTime;
        dijkstraTotalTime += dijkstraTime;
    }
    printf("the average time token for the queries using index is %.2lfms\n", queryTotalTime * 100);
    printf("the average time token for the queries using dijkstra algorithm is %.2lfms\n", dijkstraTotalTime * 100);
}

void Graph::query()
{
    // the number of query vertices
    int sum = 1e4;

    double queryTime = 0;
    double dijkstraTime = 0;

    int wrong = 0;

    for (int i = 0; i < sum; i++)
    {
        int v = generateRandomNumber(1, n);

        int nums = generateRandomNumber(1, labelSize);
        string labels = generateRandomLabel(nums);

        auto queryStart = chrono::high_resolution_clock::now();

        auto knn1 = query(v, labels);

        auto queryEnd = chrono::high_resolution_clock::now();
        chrono::duration<double> queryDuration = queryEnd - queryStart;
        queryTime += queryDuration.count();

        auto dijkstraStart = chrono::high_resolution_clock::now();
        auto knn2 = dijkstra(v, labels);
        auto dijkstraEnd = chrono::high_resolution_clock::now();
        chrono::duration<double> dijkstraDuration = dijkstraEnd - dijkstraStart;
        dijkstraTime += dijkstraDuration.count();

        bool flag = false;
        for (int i = 0; i < knn1.size(); i++)
        {
            auto &[d1, v1] = knn1[i];
            auto &[d2, v2] = knn2[i];
            if (abs(d1 - d2) > 1e-8)
            {
                flag = true;
                wrong++;
                cerr << "wrong:" << wrong << "\n";
                break;
            }
        }
        if (flag)
        {
            cerr << "v:" << v << "\n";
            cerr << "label: " << labels.c_str() << "\n";
            cerr << "query\n";
            cerr << "query size: " << knn1.size() << "\n";

            for (const auto &[d, v] : knn1)
                cerr << "(" << d << ", " << v << ") ";
            cerr << "\n";

            cerr << "dijkstra\n";
            cerr << "dijkstra size: " << knn2.size() << "\n";

            for (const auto &[d, v] : knn2)
                cerr << "(" << d << ", " << v << ") ";
            cerr << "\n\n";
        }
    }

    printf("the total time token for each query using index is %.2lfms\n", queryTime * 1000);
    printf("the total time token for each query using dijkstra algorithm is %.2lfms\n", dijkstraTime * 1000);
}

void Graph::query(string queryFolder)
{
    printf("begin query...\n");
    string queryPath = queryFolder + "/query.txt";
    ifstream fin(queryPath);
    int v;
    string labels;
    int wrong = 0;
    cerr << fixed << setprecision(2);
    double queryTime = 0;
    double dijkstraTime = 0;
    while (fin >> v >> labels)
    {
        auto queryStart = chrono::high_resolution_clock::now();

        auto knn1 = query(v, labels);
        // auto knn1 = queryByLSDIndex(v, labels);

        auto queryEnd = chrono::high_resolution_clock::now();
        chrono::duration<double> queryDuration = queryEnd - queryStart;
        queryTime += queryDuration.count();

        auto dijkstraStart = chrono::high_resolution_clock::now();
        auto knn2 = dijkstra(v, labels);
        auto dijkstraEnd = chrono::high_resolution_clock::now();
        chrono::duration<double> dijkstraDuration = dijkstraEnd - dijkstraStart;
        dijkstraTime += dijkstraDuration.count();

        bool flag = false;
        for (int i = 0; i < knn1.size(); i++)
        {
            auto &[d1, v1] = knn1[i];
            auto &[d2, v2] = knn2[i];
            if (abs(d1 - d2) > 1)
            {
                flag = true;
                wrong++;
                cerr << "wrong:" << wrong << "\n";
                break;
            }
        }
        if (flag)
        {
            cerr << "v:" << v << "\n";
            cerr << "label: " << labels.c_str() << "\n";
            cerr << "query\n";
            cerr << "query size: " << knn1.size() << "\n";

            for (const auto &[d, v] : knn1)
                cerr << "(" << d << ", " << v << ") ";
            cerr << "\n";

            cerr << "dijkstra\n";
            cerr << "dijkstra size: " << knn2.size() << "\n";

            for (const auto &[d, v] : knn2)
                cerr << "(" << d << ", " << v << ") ";
            cerr << "\n\n";
        }
    }
    fin.close();
    printf("the total time token for the queries using index is %.6lfms\n", queryTime * 1000);
    printf("the total time token for the queries using dijkstra algorithm is %.6lfms\n", dijkstraTime * 1000);
}