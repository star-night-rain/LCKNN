#include "../include/Graph.h"

Graph::Graph(string graphPath, int labelSize, int k)
{
    printf("load graph file...\n");
    this->labelSize = labelSize;
    this->k = k;

    ifstream fin(graphPath);

    fin >> n >> m;
    printf("n:%d,m:%d\n", n, m);

    edges.resize(n + 1);
    orderId.resize(n + 1, 0);
    orderMap.resize(n + 1, 0);
    trees.resize(n + 1);
    descendants.resize(n + 1);
    // shortcuts.resize(n + 1);

    uint u, v;
    double weight;
    string label;
    set<uint> labels;
    // int cnt = 0;
    while (fin >> u >> v >> weight >> label)
    {
        // printf("cnt:%d\n", cnt++);
        uint newLabel = resignLabel(label);
        edges[u].emplace_back(v, weight, newLabel);
        edges[v].emplace_back(u, weight, newLabel);
        labels.insert(newLabel);
    }
    fin.close();
    printf("original labels size:%ld\n", labels.size());

    reduceLabel();
    showLabel();
}

void Graph::reduceLabel()
{
    unordered_map<uint, int> labelFrequency;
    for (int i = 1; i <= n; ++i)
        for (auto &edge : edges[i])
        {
            if (!labelFrequency.count(edge.label))
                labelFrequency[edge.label] = 1;
            else
                labelFrequency[edge.label]++;
        }

    if (labelFrequency.size() < labelSize)
    {
        printf("error with label size larger than graph label size.\n");
        printf("graph label size:%ld\n", labelFrequency.size());
        printf("label size:%d\n", labelSize);
        labelSize = labelFrequency.size();

        for (int i = 1; i <= n; ++i)
            for (auto &edge : edges[i])
                edge.label = (1 << edge.label);
        return;
    }

    /*
    merge the two labels with similar frequency each time
    */

    vector<pair<int, uint>> labels;
    for (auto it : labelFrequency)
        labels.emplace_back(it.second, it.first);
    sort(labels.begin(), labels.end());
    unordered_map<uint, uint> newLabel;
    int x, y = 0;
    for (int i = 0; i < labelSize; ++i)
    {
        x = y;
        y = x + (labels.size() - i - 1) / labelSize + 1;
        for (int j = x; j < y; j++)
        {
            auto &p = labels[j];
            newLabel[p.second] = i;
        }
    }

    for (int i = 1; i <= n; ++i)
        for (auto &edge : edges[i])
            // use a single bit to represent a label
            edge.label = (1 << newLabel[edge.label]);
}

void Graph::showLabel()
{
    map<uint, int> labelCount;
    for (int i = 1; i <= n; ++i)
        for (auto &edge : edges[i])
        {
            if (!labelCount.count(edge.label))
                labelCount[edge.label] = 1;
            else
                labelCount[edge.label]++;
        }
    double sum = 0;
    printf("the number of labels:%ld\n", labelCount.size());
    printf("edge label summary is as follows:\n");
    int cnt = 1;
    for (auto it = labelCount.rbegin(); it != labelCount.rend(); ++it)
    {
        sum += it->second;
        printf("label:%-4u count:%-8d cumulative percentage:%5.4lf\n", it->first, it->second, sum / m);
        cnt++;
    }
}

uint Graph::resignLabel(string oldLabel)
{
    char type = oldLabel[0];
    int level = oldLabel[1] - '1';
    uint newLabel = (type - 'A') * 8 + level;
    return newLabel;
}

void Graph::loadPOI(string poiPath)
{
    printf("load poi file...\n");
    isPOI.resize(n + 1, false);

    ifstream fin(poiPath);
    int v;
    while (fin >> v)
        isPOI[v] = true;
    fin.close();
}

void Graph::loadPOI()
{
    printf("load poi file...\n");
    isPOI.resize(n + 1, false);

    int seed = 3307;
    vector<int> numbers(n);
    iota(numbers.begin(), numbers.end(), 1);
    mt19937 g(seed);
    vector<int> result;
    sample(numbers.begin(), numbers.end(), back_inserter(result), int(n * 5e-3), g);
    for (auto v : result)
        isPOI[v] = true;
}

int Graph::checkPOI(int v)
{
    return isPOI[v] ? v : 0;
}

void Graph::loadOrder(string orderPath)
{
    printf("load order file...\n");

    int order = 1;
    ifstream fin(orderPath);
    int v;
    while (fin >> v)
    {
        orderId[v] = order;
        orderMap[order] = v;
        order++;
    }
    fin.close();
}

void Graph::storeIndex1(string indexPath)
{
    printf("store index file...\n");
    ofstream fout(indexPath);
    for (int v = 1; v <= n; v++)
    {
        auto &list = trees[v].list.list;
        fout << "v:" << v << " " << list.size() << "\n";
        for (auto &[labels, knnList] : list)
        {
            auto &knn = knnList.list;
            fout << labels.c_str() << " " << knn.size();
            for (auto &[d, u] : knn)
                fout << " " << d << " " << u;
            fout << "\n";
        }
    }
    fout.close();
}

void Graph::storeIndex2(string indexPath)
{
    printf("store index file...\n");
    FILE *ofile = fopen(indexPath.c_str(), "wb");
    if (!ofile)
    {
        cerr << "Failed to open file: " << indexPath << "\n";
        return;
    }

    for (int v = 1; v <= n; ++v)
    {
        uint cnt = trees[v].list.list.size();
        fwrite(&cnt, sizeof(cnt), 1, ofile);

        for (auto &[s, knn] : trees[v].list.list)
        {
            int label = s.getLabels();
            uint knn_size = knn.list.size();
            fwrite(&label, sizeof(label), 1, ofile);
            fwrite(&knn_size, sizeof(knn_size), 1, ofile);

            for (auto &[d, v] : knn.list)
            {
                fwrite(&d, sizeof(d), 1, ofile);
                fwrite(&v, sizeof(v), 1, ofile);
            }
        }
    }

    fclose(ofile);
}

void Graph::loadIndex1(string indexPath)
{
    printf("load index file...\n");
    ifstream fin(indexPath);
    for (int v = 1; v <= n; v++)
    {
        auto &list = trees[v].list.list;
        int indexSum;
        string s;
        fin >> s >> indexSum;
        while (indexSum--)
        {
            string labels;
            int sum;
            fin >> labels >> sum;
            vector<PDI> knn;
            knn.reserve(sum);
            while (sum--)
            {
                double d;
                int u;
                fin >> d >> u;
                knn.emplace_back(d, u);
            }
            list.emplace_back(LabelSet(labels), KNNList(knn));
        }
    }
    fin.close();
}

void Graph::loadIndex2(string indexPath)
{
    printf("load index file...\n");
    FILE *ifile = fopen(indexPath.c_str(), "rb");
    if (!ifile)
    {
        cerr << "Failed to open file: " << indexPath << "\n";
        return;
    }

    for (int v = 1; v <= n; ++v)
    {
        uint p;
        fread(&p, sizeof(p), 1, ifile);

        auto &index = trees[v].list.list;
        index.reserve(p);

        for (int i = 0; i < p; i++)
        {
            uint labels, nums;
            fread(&labels, sizeof(labels), 1, ifile);
            fread(&nums, sizeof(nums), 1, ifile);

            vector<PDI> knn;
            knn.reserve(nums);

            for (int j = 0; j < nums; j++)
            {
                double dist;
                int u;
                fread(&dist, sizeof(dist), 1, ifile);
                fread(&u, sizeof(u), 1, ifile);
                knn.emplace_back(dist, u);
            }

            if (checkPOI(v) && knn.size() == k)
                knn.resize(k - 1);

            index.emplace_back(labels, move(knn));
        }
    }

    fclose(ifile);
}

void Graph::clear()
{
    for (int v = 1; v <= n; v++)
    {
        trees[v].list.list.clear();
        trees[v].neighbors.clear();
        trees[v].children.clear();
    }
    orderId.resize(n + 1, 0);
    orderMap.resize(n + 1, 0);
}