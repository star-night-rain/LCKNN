#pragma once

#include "utils.h"
typedef pair<LabelSet, double> PLD;
// vector<pair<distance,vertex>> list
struct KNNList
{
    // (distance,vertex)
    vector<PDI> list;

    KNNList() {}

    KNNList(vector<PDI> list) : list(list) {}

    KNNList(double weight, int vertex)
    {
        list.emplace_back(make_pair(weight, vertex));
    }

    KNNList operator+(const double &weight)
    {
        vector<PDI> list;
        list.reserve(this->list.size());
        for (const auto &t : this->list)
            list.emplace_back(t.first + weight, t.second);
        return KNNList(list);
    }

    // position,distance,vertex
    void emplace(int pos, double distance, int &vertex)
    {
        list.emplace(list.begin() + pos, distance, vertex);
    }

    void resize(int &k)
    {
        if (list.size() > k)
            list.resize(k);
    }

    size_t size()
    {
        return list.size();
    }

    bool operator==(const KNNList &other) const
    {
        return this->list == other.list;
    }
};

// vector<pair<labels,knn>> list1
struct IndexList
{
    // kNN is sorted by distance from smallest to largest
    vector<pair<LabelSet, KNNList>> list;

    IndexList() {}

    IndexList(vector<pair<LabelSet, KNNList>> list) : list(list) {}

    void report() const
    {
        for (auto t : list)
        {
            printf("labels:%s\n", t.first.c_str().c_str());
            printf("knn: ");
            for (auto t1 : t.second.list)
                printf("(%d,%.2f) ", t1.second, t1.first);

            printf("\n");
        }
    }

    // combine two KNNs
    void combine(int owner, IndexList &&other, int &n, int k)
    {
        if (other.list.empty())
            return;
        if (this->list.empty())
        {
            this->list = other.list;
            return;
        }

        auto &list1 = this->list;
        auto &list2 = other.list;
        vector<pair<LabelSet, KNNList>> result;
        result.reserve(list1.size() + list2.size());
        int ptr1 = 0, ptr2 = 0;
        while (ptr1 < list1.size() && ptr2 < list2.size())
        {
            auto &[s1, knn1] = list1[ptr1];
            auto &[s2, knn2] = list2[ptr2];
            if (s1 < s2)
            {
                result.emplace_back(move(list1[ptr1]));
                ptr1++;
            }
            else if (s2 < s1)
            {
                result.emplace_back(move(list2[ptr2]));
                ptr2++;
            }
            else
            {
                auto knn = utils::mergeKNN(owner, knn1.list, knn2.list, n, k);
                result.emplace_back(s1, move(knn));
                ptr1++;
                ptr2++;
            }
        }
        while (ptr1 < list1.size())
        {
            result.emplace_back(move(list1[ptr1]));
            ptr1++;
        }
        while (ptr2 < list2.size())
        {
            result.emplace_back(move(list2[ptr2]));
            ptr2++;
        }
        this->list = result;
    }

    // combine two KNNs
    void combine(int owner, IndexList &other, int &n, int k)
    {
        if (this->list.empty())
        {
            this->list = other.list;
            return;
        }

        auto &list1 = this->list;
        auto &list2 = other.list;
        vector<pair<LabelSet, KNNList>> result;
        result.reserve(list1.size() + list2.size());
        int ptr1 = 0, ptr2 = 0;
        while (ptr1 < list1.size() && ptr2 < list2.size())
        {
            auto &[s1, knn1] = list1[ptr1];
            auto &[s2, knn2] = list2[ptr2];
            if (s1 < s2)
            {
                result.emplace_back(move(list1[ptr1]));
                ptr1++;
            }
            else if (s2 < s1)
            {
                result.emplace_back(move(list2[ptr2]));
                ptr2++;
            }
            else
            {
                auto knn = utils::mergeKNN(owner, knn1.list, knn2.list, n, k);
                result.emplace_back(s1, move(knn));
                ptr1++;
                ptr2++;
            }
        }
        while (ptr1 < list1.size())
        {
            result.emplace_back(move(list1[ptr1]));
            ptr1++;
        }
        while (ptr2 < list2.size())
        {
            result.emplace_back(move(list2[ptr2]));
            ptr2++;
        }
        this->list = result;
    }

    void combine(int owner, SCAttr &path, int insertedVertex, int &n, int k)
    {
        IndexList other;
        sort(path.attrs.begin(), path.attrs.end(), [](const pair<double, LabelSet> &p1, const pair<double, LabelSet> &p2)
             { return p1.second < p2.second; });
        for (auto &[d, s] : path.attrs)
            other.list.emplace_back(LabelSet(s), KNNList(d, insertedVertex));
        // if (owner == 319916)
        //     printf("size:%d\n", other.list.size());
        if (other.list.empty())
            return;
        if (this->list.empty())
        {
            this->list = other.list;
            return;
        }

        auto &list1 = this->list;
        auto &list2 = other.list;
        vector<pair<LabelSet, KNNList>> result;
        result.reserve(list1.size() + list2.size());
        int ptr1 = 0, ptr2 = 0;
        while (ptr1 < list1.size() && ptr2 < list2.size())
        {
            auto &[s1, knn1] = list1[ptr1];
            auto &[s2, knn2] = list2[ptr2];
            if (s1 < s2)
            {
                result.emplace_back(move(list1[ptr1]));
                ptr1++;
            }
            else if (s2 < s1)
            {
                result.emplace_back(move(list2[ptr2]));
                ptr2++;
            }
            else
            {
                auto knn = utils::mergeKNN(owner, knn1.list, knn2.list, n, k);
                result.emplace_back(s1, move(knn));
                ptr1++;
                ptr2++;
            }
        }
        while (ptr1 < list1.size())
        {
            result.emplace_back(move(list1[ptr1]));
            ptr1++;
        }
        while (ptr2 < list2.size())
        {
            result.emplace_back(move(list2[ptr2]));
            ptr2++;
        }
        this->list = result;
    }

    // down-top: self -> neighbor
    // top-down: neighbor -> self
    // merge shortcuts and neighbor's knn
    static IndexList join(int owner, SCAttr &scAttr, const IndexList &kNN, int &poi, int &n, int k)
    {
        map<uint, vector<pair<int, int>>> unionSet;
        for (int i = scAttr.size() - 1; i >= 0; i--)
        {
            auto &[weight, s1] = scAttr.attrs[i];
            for (int j = kNN.list.size() - 1; j >= 0; j--)
            {
                auto &[s2, knn] = kNN.list[j];
                uint labels = s1.labels | s2.labels;
                if (!unionSet.count(labels))
                    unionSet[labels].reserve((i + 1) * (j + 1));
                unionSet[labels].emplace_back(i, j);
            }

            // the label set s2 is empty
            if (poi)
            {
                auto &labels = s1.labels;
                if (!unionSet.count(labels))
                    unionSet[labels].reserve((i + 1) * kNN.list.size());
                unionSet[labels].emplace_back(i, -1);
            }
        }

        // combine knn from different combinations
        vector<pair<LabelSet, KNNList>> result(unionSet.size());
        int idx = 0;
        for (auto &[s, index] : unionSet)
        {
            result[idx].first = move(LabelSet(s));
            vector<PDI> ownKNN;
            for (auto &[i, j] : index)
            {
                auto &[weight, s1] = scAttr.attrs[i];
                if (j == -1)
                {
                    KNNList receivedKNN(weight, poi);
                    ownKNN = utils::mergeKNN(owner, ownKNN, receivedKNN.list, n, k);
                }
                else
                {
                    auto [s2, receivedKNN] = kNN.list[j];
                    if (poi)
                        receivedKNN.emplace(0, 0, poi);
                    receivedKNN.resize(k);
                    receivedKNN = receivedKNN + weight;
                    ownKNN = utils::mergeKNN(owner, ownKNN, receivedKNN.list, n, k);
                }
            }
            result[idx].second = move(ownKNN);
            idx++;
        }
        return IndexList(result);
    }

    void compensate(int owner, int poi, int n, int k)
    {
        vector<vector<int>> parents(list.size());

        for (int i = list.size() - 2; i >= 0; i--)
        {
            parents[i].reserve(list.size() - i);
            auto &[s1, _] = list[i];
            set<int> st;
            for (int j = i + 1; j < list.size(); j++)
            {
                auto &[s2, __] = list[j];
                if (s2.includes(s1) && !st.count(j))
                {
                    parents[i].emplace_back(j);
                    st.insert(j);
                    for (auto &p : parents[j])
                        st.insert(p);
                }
            }
        }

        vector<pair<uint, vector<Index>>> result(list.size());
        for (int i = 0; i < list.size(); i++)
        {
            auto &[s, knn] = list[i];
            result[i].first = s.getLabels();
            result[i].second.reserve(k);
            for (auto &[d, v] : knn.list)
                result[i].second.emplace_back(v, d, s.getLabels());
        }

        for (int i = 0; i < result.size(); i++)
        {
            auto &[s1, knn1] = result[i];
            // #pragma omp parallel for num_threads(1)
            for (auto &j : parents[i])
            {
                auto &[s2, knn2] = result[j];
                // s2 includes s1
                if ((s2 | s1) == s2)
                    knn2 = utils::compensateKNN(owner, knn2, knn1, n, k);
            }
        }

        // for (int i = 0; i < result.size(); i++)
        // {
        //     auto &[s1, knn1] = result[i];
        //     // #pragma omp parallel for num_threads(1)
        //     for (int j = i + 1; j < result.size(); j++)
        //     {
        //         auto &[s2, knn2] = result[j];
        //         // s2 includes s1
        //         if ((s2 | s1) == s2)
        //             knn2 = utils::compensateKNN(owner, knn2, knn1, n, k);
        //     }
        // }

        vector<pair<LabelSet, KNNList>> newList;
        newList.reserve(list.size());
        for (auto &[s1, index] : result)
        {
            vector<PDI> knn;
            knn.reserve(k);
            int cnt = 0;
            if (poi && index.size() == k)
                index.resize(k - 1);
            for (auto &[v, d, s2] : index)
            {
                if (s2 == s1)
                {
                    cnt++;
                    knn.emplace_back(move(d), move(v));
                }
            }
            if (cnt)
            {
                // if (poi && knn.size() == k)
                //     knn.resize(k - 1);
                newList.emplace_back(move(s1), move(knn));
            }
        }
        list = newList;
    }

    void removeVertex(int u)
    {
        for (auto &[label, knnList] : list)
        {
            auto &knn = knnList.list;
            for (auto it = knn.begin(); it != knn.end(); ++it)
            {
                auto &[d, v] = *it;
                if (v == u)
                {
                    knn.erase(it);
                    break;
                }
            }
        }
    }

    // label status:
    // 0: index of label not contain specified vertex
    // 1: index of label contains specified vertex
    // add new features: remove deleted vertex
    bool hasVertex(int deletedVertex)
    {
        bool isContaining = false;
        vector<bool> isRemoved(list.size(), false);
        for (int i = 0; i < list.size(); i++)
        {
            auto &[label, knnList] = list[i];
            auto &knn = knnList.list;
            for (int j = 0; j < knn.size(); j++)
            {
                auto &[d, v] = knn[j];
                if (v == deletedVertex)
                {
                    knn.erase(knn.begin() + j);
                    isContaining = true;
                    if (knn.empty())
                        isRemoved[i] = true;
                    break;
                }
            }
        }
        int ptr = 0;
        for (int i = 0; i < list.size(); i++)
            if (!isRemoved[i])
            {
                if (ptr < i)
                    list[ptr] = move(list[i]);
                ptr++;
            }
        list.erase(list.begin() + ptr, list.end());
        return isContaining;
    }
};

struct TreeNode
{
    int height;
    int width;
    int level;

    int parent;
    vector<int> children;

    vector<int> neighbors;

    // unordered_map<uint, SCAttr> shortcuts;
    IndexList list;
};