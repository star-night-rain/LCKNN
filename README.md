# Querying Label-Constrained k-Nearest Neighbors on Road Networks

This repository contains the code for our paper "Querying Label-Constrained k-Nearest Neighbors on Road Networks".

## Overview

The most important files in this project are as follows:

-   `include/GraphIndex.h`: Defines the structure of LC-Index.
-   `include/utils.h`: Defines the structures of label set and edge profile.
-   `src/GraphQuerier.cpp`: Implements the LCKNN query algorithm.
-   `src/GraphDecomposition.cpp`: Implements the algorithms for LC-TreeDecomposition and LC-Index construction.
-   `src/GraphUpdate.cpp`: Implements our algorithms for handing candidate object updates, including the insertion of new objects and the deletion of existing objects.

## Environment

g++ version: 9.4.0

OS: Ubuntu

## Dataset

All datasets used in this paper are stored in the "dataset" folder. Insider, there is a separate folder for each dataset, and each dataset contains the following:

-   `USA-road.[dataset name].gr`: This file contains the road network data. The first line shows the number of vertices and edges. From the second line to the last line, each line represents an edge in the form of "[source vertex id] [destination vertex id] [edge distance] [edge label]".

-   `insert.txt`: This file stores all objects to be inserted into the candidate object set.

-   `delete.txt`: This file stores all objects to be deleted from the candidate object set.

-   `POI`: This folder contains five files, each representing a candidate object set with a specified density.

-   `query`: This folder contains 11 files, where each line of each file represent a query in the form of "[query vertex] [query labels]". The file "query.txt" stores 10,000 queries. Each file named "query[i].txt" contains 1,000 queries, each with a query label size of i.

This repository includes the COL and NY datasets, while the remaining datasets can be download from [DIMACS](http://www.diag.uniroma1.it/~challenge9/download.shtml).

## Usage

### Compile

Execute the following scripts to generate executable files:

```
make
```

### Run

To run the project:

```
exe [-n dataset] [-k k] [-d d] [-l label]
```

#### Arguments

-   `dataset`: The name of the dataset, with possible values: `COL`, `NY`.
-   `k`: The number of the nearest neighbors to consider.
-   `d`: POI density, with possible values: `001`,`005`,`010`,`050`,`100`.
-   `label`: The number of the labels.

#### Example

```
./main -n NY -k 20 -d 005 -l 10
```

### Test

To test the project:

-   Construct LC-Index and perform queries

```
./main -n NY
```

-   Insert objects into the candidate objects and perform queries

```
./insert -n NY
```

-   Delete objects from the candidate objects and perform queries

```
./delete -n NY
```
