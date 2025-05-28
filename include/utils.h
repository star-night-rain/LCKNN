#pragma once

#include <iostream>
#include <vector>
#include <unordered_map>
#include <random>
#include <set>
#include <fstream>
#include <array>
#include <queue>
#include <cstring>
#include <filesystem>
#include <algorithm>
#include <getopt.h>
#include <climits>
#include <map>
#include <chrono>
#include <omp.h>
#include <tuple>
#include <stack>
#include <unordered_set>

using namespace std;
using namespace chrono;

#define inf INT_MAX

typedef long long LL;
typedef unsigned int uint;
typedef pair<double, int> PDI;
typedef pair<uint, uint> PUU;
typedef pair<uint, double> PUD;
typedef pair<int, int> PII;

struct GraphEdge
{
    uint target;
    double weight;
    uint label;

    GraphEdge() : target(0), weight(0), label(0) {}

    GraphEdge(uint target, double weight, uint label) : target(target), weight(weight), label(label) {}
};

struct Index
{
    int vertex;
    double distance;
    uint labels;
    Index(int vertex, double distance, uint labels) : vertex(vertex), distance(distance), labels(labels) {}
    Index() {}
};

// label set
struct LabelSet
{
    uint labels;

    LabelSet() : labels(0) {}

    LabelSet(uint labels) : labels(labels) {}

    LabelSet(string labels)
    {
        this->labels = 0;
        for (char label : labels)
        {
            int index = label - 'a';
            this->labels |= 1 << index;
        }
    }

    LabelSet operator+(const LabelSet &other) const
    {
        return LabelSet(this->labels | other.labels);
    }

    bool operator==(const LabelSet &other) const
    {
        return this->labels == other.labels;
    }

    bool operator!=(const LabelSet &other)
    {
        return !operator==(other);
    }

    bool operator<(const LabelSet &other) const
    {
        return getLabels() < other.getLabels();
        // return size() < other.size();
    }

    bool operator>(const LabelSet &other) const
    {
        return getLabels() > other.getLabels();
    }

    // DONE check whether two label sets have a contain relationship or not
    bool includes(const LabelSet &other)
    {
        return !(~labels & other.labels);
    }

    // bool strictlyIncludes(const LabelSet &other)
    // {
    //     return includes(other) && (labels != other.labels);
    // }

    size_t size() const
    {
        uint x = labels - ((labels >> 1u) & 0x55555555u);
        x = (x & 0x33333333u) + ((x >> 2u) & 0x33333333u);
        x = (x + (x >> 4u)) & 0x0f0f0f0fu;
        x = x + (x >> 8u);
        return (x + (x >> 16u)) & 0xffu;
    }

    uint getLabels() const
    {
        return this->labels;
    }

    string c_str()
    {
        string s = "";
        for (int i = 0; i < sizeof(uint) * 8; i++)
            if (this->labels & (1 << i))
            {
                char c = 'a' + i;
                s += c;
            }
        return s;
    }

    uint lowBit()
    {
        return labels & -labels;
    }

    uint higBit()
    {
        return 1 << (31 - __builtin_clz(labels));
    }
};

// short cut attributes
// vector<pair<distance,labels>>
struct SCAttr
{
    // distance, labels
    vector<pair<double, LabelSet>> attrs;

    SCAttr() : attrs() {}

    void report()
    {
        for (auto &p : attrs)
            printf("(%.2lf,%s) ", p.first, p.second.c_str().c_str());
        printf("\n");
    }

    int size()
    {
        return attrs.size();
    }

    void emplace_back_attr(double distance, uint label)
    {
        attrs.emplace_back(distance, LabelSet(label));
    }

    void combine(SCAttr &&other)
    {
        // printf("combine...\n");
        if (this->attrs.empty())
        {
            this->attrs = move(other.attrs);
            return;
        }

        auto &attrs1 = attrs;
        auto &attrs2 = other.attrs;
        vector<bool> isRemoved1(attrs1.size(), false);
        vector<bool> isRemoved2(attrs2.size(), false);

        // remove redundant shortcut
        for (int i = 0; i < attrs1.size(); ++i)
            for (int j = 0; j < attrs2.size(); ++j)
            {
                if (isRemoved2[j])
                    continue;

                auto &[d1, s1] = attrs1[i];
                auto &[d2, s2] = attrs2[j];

                if (d1 < d2)
                    isRemoved2[j] = s2.includes(s1);
                else if (d1 > d2)
                    isRemoved1[i] = s1.includes(s2);
                else
                {
                    isRemoved2[j] = s2.includes(s1);
                    if (!isRemoved2[j])
                        isRemoved1[i] = s1.includes(s2);
                }

                if (isRemoved1[i])
                    break;
            }

        // merge two shortcuts
        vector<pair<double, LabelSet>> temp;
        temp.reserve(attrs1.size() + attrs2.size());
        int i = 0, j = 0;
        while (i < attrs1.size() && j < attrs2.size())
        {
            double &d1 = attrs1[i].first;
            double &d2 = attrs2[j].first;

            if (d1 <= d2)
            {
                if (!isRemoved1[i])
                    temp.emplace_back(move(attrs1[i]));
                ++i;
            }
            else
            {
                if (!isRemoved2[j])
                    temp.emplace_back(move(attrs2[j]));
                ++j;
            }
        }

        while (i < attrs1.size())
        {
            if (!isRemoved1[i])
                temp.emplace_back(move(attrs1[i]));
            ++i;
        }

        while (j < attrs2.size())
        {
            if (!isRemoved2[j])
                temp.emplace_back(move(attrs2[j]));
            ++j;
        }

        this->attrs = move(temp);
    }

    // assume attrs is ordered by distance
    void removeRedundancy()
    {
        vector<bool> isRemoved(attrs.size(), false);
        for (uint i = 0; i < attrs.size(); ++i)
        {
            if (isRemoved[i])
                continue;
            LabelSet &s1 = attrs[i].second;
            for (uint j = i + 1; j < attrs.size(); j++)
            {
                if (isRemoved[j])
                    continue;
                LabelSet &s2 = attrs[j].second;
                if (s2.includes(s1))
                    isRemoved[j] = true;
            }
        }
        uint ptr = 0;
        for (uint i = 0; i < attrs.size(); ++i)
            if (!isRemoved[i])
            {
                if (ptr < i)
                    attrs[ptr] = move(attrs[i]);
                ptr++;
            }
        attrs.erase(attrs.begin() + ptr, attrs.end());
    }

    SCAttr operator+(const SCAttr &other) const
    {
        // printf("add...\n");
        SCAttr attr;
        auto &o1 = this->attrs;
        auto &o2 = other.attrs;
        auto &o = attr.attrs;
        if (o1.empty())
            for (auto &p2 : o2)
                o.emplace_back(p2.first, p2.second);
        else
            // path from u to v
            for (auto &p1 : o1)
                // path from v to w
                for (auto &p2 : o2)
                    o.emplace_back(p1.first + p2.first, p1.second + p2.second);
        sort(o.begin(), o.end());

        attr.removeRedundancy();
        return attr;
    }

    void clear()
    {
        attrs.clear();
    }

    bool empty()
    {
        return attrs.empty();
    }
};

struct utils
{
    // merge two KNNs from down to top(owner==0) or top to down(owner!=0)
    // vector<pair<double,int>>
    template <class C1, class C2>
    static vector<PDI> mergeKNN(int &owner, C1 &ownKNN, C2 &receivedKNN, int &n, int &k)
    {
        vector<PDI> result;
        result.reserve(k);
        vector<bool> hasPOI(n + 1, false);
        int ptr1 = 0, ptr2 = 0;
        // knn:(distance,vertex)
        while (ptr1 < ownKNN.size() && ptr2 < receivedKNN.size())
        {
            if (result.size() == k)
                return result;
            auto &[d1, v1] = ownKNN[ptr1];
            auto &[d2, v2] = receivedKNN[ptr2];
            if (hasPOI[v1] || v1 == owner)
            {
                ptr1++;
                continue;
            }
            if (hasPOI[v2] || v2 == owner)
            {
                ptr2++;
                continue;
            }

            if (d1 < d2)
            {
                result.emplace_back(move(ownKNN[ptr1]));
                hasPOI[v1] = true;
                ptr1++;
            }
            else if (d2 < d1)
            {
                result.emplace_back(move(receivedKNN[ptr2]));
                hasPOI[v2] = true;
                ptr2++;
            }
            else
            {
                result.emplace_back(move(ownKNN[ptr1]));
                hasPOI[v1] = true;
                if (result.size() == k)
                    return result;
                if (!hasPOI[v2])
                {
                    result.emplace_back(move(receivedKNN[ptr2]));
                    hasPOI[v2] = true;
                }
                ptr1++;
                ptr2++;
            }
        }

        while (ptr1 < ownKNN.size())
        {
            if (result.size() == k)
                return result;
            int &v = ownKNN[ptr1].second;
            if (!hasPOI[v] && v != owner)
            {
                result.emplace_back(move(ownKNN[ptr1]));
                hasPOI[v] = true;
            }
            ptr1++;
        }

        while (ptr2 < receivedKNN.size())
        {
            if (result.size() == k)
                return result;
            int &v = receivedKNN[ptr2].second;
            if (!hasPOI[v] && v != owner)
            {
                result.emplace_back(move(receivedKNN[ptr2]));
                hasPOI[v] = true;
            }
            ptr2++;
        }

        return result;
    }

    // compensate two KNNs
    // vector<Index> = vector<vertex,distance,labels>
    template <class C1, class C2>
    static vector<Index> compensateKNN(int owner, C1 &ownKNN, const C2 &receivedKNN, int &n, int &k)
    {
        vector<Index> result;
        vector<bool> hasPOI(n + 1, false);
        int ptr1 = 0, ptr2 = 0;
        while (ptr1 < ownKNN.size() && ptr2 < receivedKNN.size())
        {
            if (result.size() == k)
                return result;
            auto &[v1, d1, s1] = ownKNN[ptr1];
            const auto &[v2, d2, s2] = receivedKNN[ptr2];
            if (hasPOI[v1] || v1 == owner)
            {
                ptr1++;
                continue;
            }
            if (hasPOI[v2] || v2 == owner)
            {
                ptr2++;
                continue;
            }
            if (d1 < d2)
            {
                result.emplace_back(move(ownKNN[ptr1]));
                // result.emplace_back(v1, d1, s1);
                hasPOI[v1] = true;
                ptr1++;
            }
            else if (d2 < d1)
            {
                result.emplace_back(v2, d2, s2);
                hasPOI[v2] = true;
                ptr2++;
            }
            else
            {
                result.emplace_back(v2, d2, s2);
                hasPOI[v2] = true;
                // if (result.size() == k)
                //     return result;
                // if (!hasPOI[v1])
                // {
                //     result.emplace_back(move(ownKNN[ptr1]));
                //     // result.emplace_back(v1, d1, s1);
                //     hasPOI[v1] = true;
                // }
                // ptr1++;
                ptr2++;
            }
        }

        while (ptr1 < ownKNN.size())
        {
            if (result.size() == k)
                return result;
            auto &[v1, d1, s1] = ownKNN[ptr1];
            if (!hasPOI[v1] && v1 != owner)
            {
                result.emplace_back(move(ownKNN[ptr1]));
                // result.emplace_back(v1, d1, s1);
                hasPOI[v1] = true;
            }
            ptr1++;
        }

        while (ptr2 < receivedKNN.size())
        {
            if (result.size() == k)
                return result;
            auto &[v2, d2, s2] = receivedKNN[ptr2];
            if (!hasPOI[v2] && v2 != owner)
            {
                result.emplace_back(v2, d2, s2);
                hasPOI[v2] = true;
            }
            ptr2++;
        }

        return result;
    }
};