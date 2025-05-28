#include "../include/Graph.h"

int main(int argc, char *argv[])
{
    string graphName = "COL";
    int labelSize = 10;
    int k = 20;
    string poiDensity = "005";
    set<string> densityList = {"001", "005", "010", "050", "100"};
    string subgraphId = "0";

    int option = -1;
    while (-1 != (option = getopt(argc, argv, "n:k:d:l:s:")))
    {
        if (option == 'n')
            graphName = optarg;
        else if (option == 'k')
            k = stoi(optarg);
        else if (option == 'd')
        {
            if (!densityList.count(optarg))
            {
                printf("please specify the POI density -d in [\"001\", \"005\", \"010\", \"050\", \"100\"]\n");
                return 0;
            }
            poiDensity = optarg;
        }
        else if (option == 'l')
            labelSize = stoi(optarg);
        else if (option == 's')
            subgraphId = optarg;
    }
    string folder = "datasets/" + graphName + "/";

    string graphPath = folder + "USA-road." + graphName + ".gr";
    if (subgraphId != "0")
        graphPath = folder + "/subgraph/USA-road." + graphName + "." + subgraphId + ".gr";
    string poiPath = folder + "/POI/POI" + poiDensity + ".txt";
    string orderPath = folder + "/order.txt";
    // string indexPath = folder + "/index/index" + indexFile + ".txt";
    string indexPath = folder + "/index/index.k" + to_string(k) + ".density" + poiDensity + ".txt";
    if (subgraphId != "0")
        indexPath = folder + "/index/index.k" + to_string(k) + ".density" + poiDensity + "." + subgraphId + ".txt";
    string queryFolder = folder + "/query";
    string indexFolder = folder + "/index";
    if (!filesystem::exists(indexFolder))
        filesystem::create_directory(indexFolder);

    string poiFolder = folder + "insert.txt";

    Graph graph(graphPath, labelSize, k);
    printf("graph path: %s\n", graphPath.c_str());

    graph.loadPOI(poiPath);

    if (!filesystem::exists(indexPath))
    {
        graph.buildIndex();
        graph.storeIndex2(indexPath);
    }
    else
    {
        graph.prepareUpdate();
        graph.loadIndex2(indexPath);
    }

    graph.updatePOI(poiFolder);

    graph.query(queryFolder);

    return 0;
}