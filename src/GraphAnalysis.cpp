#include "../include/Graph.h"

void Graph::calculateStats()
{
    printf("begin calculate status ...\n");
    queue<int> que;
    for (int v = 1; v <= n; ++v)
        if (trees[v].parent == -1)
        {
            que.push(v);
            trees[v].height = 0;
        }
    while (!que.empty())
    {
        int v = que.front();
        que.pop();
        for (int child : trees[v].children)
        {
            que.push(child);
            trees[child].height = trees[v].height + 1;
        }
    }

    int maxHeight = 0;
    double heightSum = 0;
    int maxWidth = 0;
    double widthSum = 0;
    int maxIndexCount = 0;
    double indexCountSum = 0;
    for (int v = 1; v <= n; ++v)
    {
        heightSum += trees[v].height;
        maxHeight = max(maxHeight, int(trees[v].height));
        widthSum += trees[v].width;
        maxWidth = max(maxWidth, int(trees[v].width));
        int indexCount = trees[v].list.list.size();
        maxIndexCount = max(maxIndexCount, indexCount);
        indexCountSum += indexCount;
    }
    printf("max height:%d,average height:%.2lf\n", maxHeight, heightSum / n);
    printf("max width:%d,average width:%.2lf\n", maxWidth, widthSum / n);
    printf("max index count:%d, average index count:%.2lf\n", maxIndexCount, indexCountSum / n);
}

void Graph::reportIndexSize()
{
    printf("begin report ...\n");

    LL labelCount = 0;
    LL indexCount = 0;

    for (int i = 1; i <= n; i++)
        for (auto &p : trees[i].list.list)
        {
            labelCount++;
            indexCount += p.second.size();
        }

    double indexSize = (n * 4 + labelCount * 4 + indexCount * 8) / 1024 / 1024.0;
    printf("index size:%.2lfMB\n", indexSize);
}

void Graph::countComponent()
{
    int cnt = 0;
    vector<bool> st(n + 1, false);
    for (int i = 1; i <= n; i++)
        if (!st[i])
        {
            cnt++;
            st[i] = true;
            queue<int> que;
            que.push(i);
            while (!que.empty())
            {
                auto v = que.front();
                que.pop();
                for (auto &edge : edges[v])
                {
                    int u = edge.target;
                    if (!st[u])
                    {
                        st[u] = true;
                        que.push(u);
                    }
                }
            }
        }
    printf("the number of connected components:%d\n", cnt);
}

void Graph::showShortcuts()
{
    printf("show shortcuts\n");
    for (int v = 1; v <= n; v++)
    {
        printf("v:%d\n", v);
        for (auto &u : trees[v].neighbors)
        {
            printf("u:%d\n", u);
            shortcuts[v][u].report();
        }
        printf("\n");
    }
}

void Graph::showStructure()
{
    printf("\nrelationship\n");
    for (int id = 1; id <= n; id++)
    {
        int v = orderMap[id];
        printf("v:%d\n", v);
        printf("parent:%d\n", trees[v].parent);
        printf("children:");
        for (auto &child : trees[v].children)
            printf("%d ", child);
        printf("\n");
        printf("neighbors:");
        for (auto &neighbor : trees[v].neighbors)
            printf("%d ", neighbor);
        printf("\n\n");
    }
}

void generateLabels(vector<LabelSet> &labels, LabelSet label, int idx, int size)
{
    if (idx >= size)
    {
        labels.emplace_back(label);
        return;
    }
    generateLabels(labels, label, idx + 1, size);
    generateLabels(labels, label + (1 << idx), idx + 1, size);
}

void Graph::checkIndex()
{
    printf("begin check the correctness of index...\n");
    vector<LabelSet> labels;
    generateLabels(labels, 0, 0, labelSize);

    int wrong = 0;
    cerr << fixed << setprecision(2);
    for (int v = 1; v <= n; v++)
    {
        for (auto &label : labels)
        {
            auto knn1 = query(v, label);
            auto knn2 = dijkstra(v, label);
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
                cerr << "label: " << label.c_str() << "\n";
                cerr << "query\n";
                cerr << "query size: " << knn1.size() << "\n";

                for (auto &[d, v] : knn1)
                    cerr << "(" << d << ", " << v << ") ";
                cerr << "\n";

                cerr << "dijkstra\n";
                cerr << "dijkstra size: " << knn2.size() << "\n";

                for (auto &[d, v] : knn2)
                    cerr << "(" << d << ", " << v << ") ";
                cerr << "\n\n";
            }
        }
    }
    printf("finish check\n");
}

void Graph::showIndex()
{
    printf("\nshow index\n");
    for (int v = 1; v <= n; v++)
    {
        printf("v:%d\n", v);
        trees[v].list.report();
        printf("\n");
    }
    int sum = 0;
    int sumPOI = 0;
    for (int v = 1; v <= n; v++)
    {
        sum += trees[v].list.list.size();
        if (checkPOI(v))
            sumPOI += trees[v].list.list.size();
    }
    printf("sum:%d\n", sum);
    printf("sum of POIs:%d\n", sumPOI);
}

void Graph::showKNN()
{
    vector<LabelSet> labels;
    generateLabels(labels, 0, 0, labelSize);
    // printf("size:%ld\n", labels.size());

    int wrong = 0;
    for (int v = 1; v <= n; v++)
    {
        printf("v:%d\n", v);
        for (auto &label : labels)
        {
            printf("query\n");
            auto knn1 = query(v, label);
            printf("label:%s\n", label.c_str().c_str());
            for (auto &[d, v] : knn1)
                printf("(%.2lf,%d) ", d, v);
            printf("\n\n");

            printf("dijkstra\n");
            auto knn2 = dijkstra(v, label);
            for (auto &[d, v] : knn2)
                printf("(%.2lf,%d) ", d, v);
            printf("\n\n");
        }
    }
}
