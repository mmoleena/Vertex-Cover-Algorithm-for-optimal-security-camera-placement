#include <pthread.h>
#include <time.h>
#include <algorithm>
#include <iostream>
#include <map>
#include <random>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>
#include <atomic>


#include "minisat/minisat/core/Solver.h"
#include "minisat/minisat/core/SolverTypes.h"

using namespace Minisat;
using namespace std;

// Stores info. of the min vertex cover, including size and covered vertices in
// a graph.
class VertexCoverInfo {
 public:
  size_t VxCv;
  vector<int> minVertexCover;
  size_t minVertexCoverSize;

  VertexCoverInfo(size_t VxCv, const vector<int>& minVertexCover,
                  size_t minVertexCoverSize)
      : VxCv(VxCv),
        minVertexCover(minVertexCover),
        minVertexCoverSize(minVertexCoverSize) {}
};

// Describes a graph and its constructor, which initializes the graph with the
// specified number of vertices and vertex cover information.
class Graph {
 public:
  size_t vertices;
  vector<int> edgesInPlot;
  vector<vector<int>> graphOutput;
  VertexCoverInfo vertexCoverInfo;
  std::vector<bool> vertexCoverA1;
  std::set<int> vertexCoverA2;
  std::vector<int> vertexCoverListA2;

  Graph() : vertices(0), graphOutput(0), vertexCoverInfo(0, {}, 0) {}
  Graph(int vertices)
      : vertices(vertices),
        graphOutput(vertices),
        vertexCoverInfo(0, {}, vertices) {}

  void GenerateAdjMatrix();
  void initVertexCoverA1();
  void StoreEdges(const std::vector<int>& edges);
  vector<int> CheckVertexCover(size_t VxCv);
  void printOutput(Graph& street, bool timeout);
  void cnfSatVC();
  void atLeastOneVertexEdge(Solver& solver, size_t vertices, size_t VxCv, vector<vector<Lit>>& Vertices);
  std::vector<bool> approxVC1(std::vector<std::pair<int, int>>& edgeList,
                              Graph& street);
  std::vector<int> approxVC2(std::vector<std::pair<int, int>>& edgeList);
};

void Graph::initVertexCoverA1() {
  if (vertices == 0) {
    return;  // No vertices, nothing to do
  }
  vertexCoverA1 = std::vector<bool>(vertices, false);
}

// This method is in charge of creating the adjacency matrix (graphOutput) using
// the graph's provided edges.
void Graph::GenerateAdjMatrix() {
  if (vertices == 0) {
    return;  // No vertices, nothing to do
  }

  graphOutput = vector<vector<int>>(vertices, vector<int>(vertices, 0));

  for (size_t i = 0; i < edgesInPlot.size(); i += 2) {
    int vertex1 = edgesInPlot[i];
    int vertex2 = edgesInPlot[i + 1];

    graphOutput[vertex1][vertex2] = 1;
    graphOutput[vertex2][vertex1] = 1;
  }
}

// Based on the provided input, this method stores the graph's edges.
// The new set of edges is then added to the edgesInPlot vector after any
// previous edge data has been cleared.
void Graph::StoreEdges(const std::vector<int>& edges) {
  edgesInPlot.clear();
  edgesInPlot.reserve(edges.size());
  for (std::vector<int>::const_iterator it = edges.begin(); it != edges.end();
       ++it) {
    edgesInPlot.push_back(*it);
  }
}

// Set up the vertices' initial values in a 2D vector so that the vertex cover can be calculated.
void setupVertices(Solver& solver, size_t vertices, size_t VxCv, vector<vector<Lit>>& Vertices) {
    for (size_t i = 0; i < vertices; i++) {
      Vertices.push_back(vector<Lit>());
      for (size_t j = 0; j < VxCv; j++) {
        Vertices.back().push_back(mkLit(solver.newVar()));
      }
    }
}

// Cond. 1: To ensure each vertex is covered at least once in the vertex cover.
void coveredAtLeastOnce(Solver& solver, size_t vertices, size_t VxCv, vector<vector<Lit>>& Vertices){
    for (size_t i = 0; i < VxCv; i++) {
    vec<Lit> clauses;
    for (size_t j = 0; j < vertices; j++) {
      clauses.push(Vertices[j][i]);
    }
    solver.addClause(clauses);
  }
}

// Cond. 2: To ensure no more than one literal is true for each vertex in the vertex cover.
void noMoreThanOneLiteral(Solver& solver, size_t vertices, size_t VxCv, vector<vector<Lit>>& Vertices){
    for (size_t i = 0; i < vertices; i++) {
    vec<Lit> clauses;

    // Check if the vertex is adjacent to any covered vertex in the cover.
    for (size_t j = 0; j < VxCv; j++) {
        clauses.push(Vertices[i][j]);
        for (size_t k = 0; k < vertices; k++) {
            if (k != i) {
                clauses.push(~Vertices[k][j]);
            }
        }
    }

    solver.addClause(clauses);
  }
}

// Cond. 3: To ensure that at least one vertex in the vertex cover is true for each position in the cover.
void atLeastOneVertexPosition(Solver& solver, size_t vertices, size_t VxCv, vector<vector<Lit>>& Vertices){
    for (size_t i = 0; i < VxCv; i++) {
        for (size_t j = 0; j < vertices - 1; j++) {
          for (size_t k = j + 1; k < vertices; k++) {
            solver.addClause(~Vertices[j][i], ~Vertices[k][i]);
          }
        }
      }
}

// Cond. 4: // To ensure that at least one vertex in the vertex cover is true for each edge in the graph.
void Graph::atLeastOneVertexEdge(Solver& solver, size_t vertices, size_t VxCv, vector<vector<Lit>>& Vertices){
  for (size_t i = 0; i < edgesInPlot.size(); i += 2) {
        vec<Lit> clauses;
        for (size_t k = 0; k < VxCv; k++) {
          clauses.push(Vertices[edgesInPlot[i]][k]);
          clauses.push(Vertices[edgesInPlot[i + 1]][k]);
        }
        solver.addClause(clauses);
      }
}

// This method is responsible for determining a vertex cover of the graph given
// a specified size VxCv Have taken multiple online references for this section
// of code
vector<int> Graph::CheckVertexCover(size_t VxCv) {
  Solver solver;
  vector<vector<Lit>> Vertices;
  setupVertices(solver, vertices, VxCv, Vertices);
  coveredAtLeastOnce(solver, vertices, VxCv, Vertices);
  atLeastOneVertexPosition(solver, vertices, VxCv, Vertices);
  atLeastOneVertexEdge(solver, vertices, VxCv, Vertices);
  // This part of the code checks the satisfiability of the MiniSat solver and
  // retrieves the minimum vertex cover if a solution exists.
  vector<int> cover;
  auto isSatisfiable = solver.solve();
  cover.clear(); 
  if (isSatisfiable) {
      vector<bool> modelValues(VxCv, false);
      for (size_t j = 0; j < VxCv; j++) {
        for (size_t i = 0; i < vertices; i++) {
          if (solver.modelValue(Vertices[i][j]) == l_True) {
            cover.push_back(i);
            modelValues[j] = true;
            break;
          }
        }
        if (!modelValues[j]) {
          cover.push_back(-1);
        }
      }
  } else {
    cover = {-1};
  }
  return cover;
}

std::vector<int> Graph::approxVC2(std::vector<std::pair<int, int>>& edgeList) {
  std::set<int> vertexCoverA2;

  std::shuffle(edgeList.begin(), edgeList.end(), std::default_random_engine());

  while (!edgeList.empty()) {
    // Pick the last edge and add both vertices to the vertex cover
    int u = edgeList.back().first;
    int v = edgeList.back().second;

    vertexCoverA2.insert(u);
    vertexCoverA2.insert(v);

    // Remove vertices u and v from the edge list
    edgeList.erase(std::remove_if(edgeList.begin(), edgeList.end(),
                                  [u, v](const std::pair<int, int>& edge) {
                                    return edge.first == u ||
                                           edge.second == u ||
                                           edge.first == v || edge.second == v;
                                  }),
                   edgeList.end());
  }

  return std::vector<int>(vertexCoverA2.begin(), vertexCoverA2.end());
}

std::vector<bool> Graph::approxVC1(std::vector<std::pair<int, int>>& edgeList,
                                   Graph& street) {
  street.initVertexCoverA1();

  while (!edgeList.empty()) {
    std::map<int, int> degreeCount;

    for (const auto& edge : edgeList) {
      degreeCount[edge.first]++;
      degreeCount[edge.second]++;
    }

    int maxDegreeVertex = -1;
    int maxDegree = -1;

    for (const auto& entry : degreeCount) {
      if (!vertexCoverA1[entry.first] && entry.second > maxDegree) {
        maxDegree = entry.second;
        maxDegreeVertex = entry.first;
      }
    }

    if (maxDegreeVertex == -1) {
      break;
    }

    vertexCoverA1[maxDegreeVertex] = true;

    auto it =
        std::remove_if(edgeList.begin(), edgeList.end(),
                       [maxDegreeVertex](const std::pair<int, int>& edge) {
                         return edge.first == maxDegreeVertex ||
                                edge.second == maxDegreeVertex;
                       });

    edgeList.erase(it, edgeList.end());

    for (const auto& edge : edgeList) {
      if (edge.first == maxDegreeVertex) {
        degreeCount[edge.second]--;
      } else if (edge.second == maxDegreeVertex) {
        degreeCount[edge.first]--;
      }
    }
  }
  return vertexCoverA1;
}
// This method iteratively checks vertex covers of increasing sizes to determine
// the graph's minimal vertex cover.
void Graph::cnfSatVC() {  
  size_t i = 1;
  while (i < vertices && vertexCoverInfo.minVertexCoverSize == vertices) {
      vertexCoverInfo.minVertexCover = CheckVertexCover(i);

      if (vertexCoverInfo.minVertexCover[0] != -1 &&
          vertexCoverInfo.minVertexCover.size() < vertexCoverInfo.minVertexCoverSize) {
          vertexCoverInfo.minVertexCoverSize = vertexCoverInfo.minVertexCover.size();
      }

      ++i;
  }
}


// This method parses the input string representing graph edges and extracts
// vertex pairs using regular expressions. This section is the modified version
// of the code from Assignment 2
vector<pair<int, int>> extractEdges(const string& edgesStr) {
  vector<pair<int, int>> returnData;
  regex regex_pattern("<(\\d+),(\\d+)>|\\{(\\d+),(\\d+)\\}");
  smatch match;

  string edgesStrCopy = edgesStr;

  while (regex_search(edgesStrCopy, match, regex_pattern)) {
    try {
      int vertex1 = match[1].length() ? stoi(match[1].str()) : stoi(match[3].str());
      int vertex2 = match[2].length() ? stoi(match[2].str()) : stoi(match[4].str());
      if(vertex1 > 0 && vertex2 > 0){
        returnData.push_back(make_pair(vertex1, vertex2));
      }
    } catch (const invalid_argument& e) {
      std::cerr << "Error: Invalid argument in extractEdges" << e.what()
                << endl;
    } catch (const out_of_range& e) {
      std::cerr << "Error: Out of range in extractEdges" << e.what() << endl;
    }

    edgesStrCopy = match.suffix().str();
  }

  return returnData;
}

void Graph::printOutput(Graph& street, bool timeout) {
  cout << "CNF-SAT-VC: ";
  if(timeout || street.vertexCoverInfo.minVertexCover[0] == -1){
    std::cout << "timeout";
  } else {
    if (!street.vertexCoverInfo.minVertexCover.empty()) {
    sort(street.vertexCoverInfo.minVertexCover.begin(),
         street.vertexCoverInfo.minVertexCover.end());
    for (size_t i = 0; i < street.vertexCoverInfo.minVertexCover.size(); i++) {
      if (i > 0) {
        std::cout << ",";
      }
      std::cout << street.vertexCoverInfo.minVertexCover[i];
    }
  } else {
    throw runtime_error("No valid vertex cover found.");
  }
} 
  std::cout << std::endl;
  std::cout << "APPROX-VC-1: ";
  int n = vertices;
  bool first = true;
  for (int i = 1; i < n; i++) {
    if (vertexCoverA1[i]) {
      if (!first) {
        std::cout << ",";
      }
      std::cout << i;
      first = false;
    }
  }
  std::cout << std::endl;
  std::cout << "APPROX-VC-2: ";
  for (vector<int>::size_type i = 0; i < street.vertexCoverListA2.size(); i++) {
    if (i > 0) {
      std::cout << ",";
    }
    std::cout << vertexCoverListA2[i];
  }
  std::cout << std::endl;
}

// This method represents the entry point for the thread that executes 
// CNF SAT for the Minimum Vertex Cover problem.
void* threadCNF(void* arg) {
  Graph& street = *reinterpret_cast<Graph*>(arg);
  clockid_t cpuClockId;
  pthread_getcpuclockid(pthread_self(), &cpuClockId);
  struct timespec startTime;
  clock_gettime(cpuClockId, &startTime);
  street.cnfSatVC();
  struct timespec endTime;
  clock_gettime(cpuClockId, &endTime);
  // double duration = endTime.tv_sec - startTime.tv_sec + (endTime.tv_nsec - startTime.tv_nsec) / 1e9;
  // << "the duration time for CNF is: " << duration << endl;
  return 0;
}

// This method represents the entry point for the thread that executes 
// Approximation Algorithm 1 for the Minimum Vertex Cover problem.
void* threadApproxAppVC1(void* arg) {
  auto args = static_cast<std::tuple<Graph*, vector<pair<int, int>>>*>(arg);
  Graph& street = *std::get<0>(*args);
  vector<pair<int, int>> edgePairs = std::get<1>(*args);
  clockid_t cpuClockId;
  pthread_getcpuclockid(pthread_self(), &cpuClockId);
  struct timespec startTime;
  clock_gettime(cpuClockId, &startTime);
  street.approxVC1(edgePairs, street);
  struct timespec endTime;
  clock_gettime(cpuClockId, &endTime);
  // double duration = endTime.tv_sec - startTime.tv_sec + (endTime.tv_nsec - startTime.tv_nsec) / 1e9;
  //cout << "the duration time for Approx VC 1 is: " << duration << endl;
  return 0;
}

// This method represents the entry point for the thread that executes 
// Approximation Algorithm 2 for the Minimum Vertex Cover problem.
void* threadAppVC2(void* arg) {
  auto args = static_cast<std::tuple<Graph*, vector<pair<int, int>>>*>(arg);
  Graph& street = *std::get<0>(*args);
  vector<pair<int, int>> edgePairs = std::get<1>(*args);
  clockid_t cpuClockId;
  pthread_getcpuclockid(pthread_self(), &cpuClockId);
  struct timespec startTime;
  clock_gettime(cpuClockId, &startTime);
  street.vertexCoverListA2 = street.approxVC2(edgePairs);
  struct timespec endTime;
  clock_gettime(cpuClockId, &endTime);
  // double duration = endTime.tv_sec - startTime.tv_sec + (endTime.tv_nsec - startTime.tv_nsec) / 1e9;
  //cout << "the duration time for Approx VC 2 is: " << duration << endl;
  return 0;
}
// This method creates a graph, stores edges, and uses the included Minisat
// solver to determine the minimal vertex cover by parsing the input line and
// interpreting instructions ('V' for indicating the number of vertices and 'E'
// for adding edges).
int parseInput(const string& line, Graph& street) {
  istringstream iss(line);
  char command;
  iss >> command;

  try {
    if (command == 'V') {
      int numVertices;
      if (iss >> numVertices) {
        if (numVertices <= 0) {
          // Clear the graph-related information and continue reading inputs
          street.edgesInPlot.clear();
          street.vertexCoverInfo.minVertexCover.clear();
          street = Graph();  // This constructor should create an empty graph
          return 0;
        } else {
          street = Graph(numVertices + 1);
        }
      } else {
        throw invalid_argument("Invalid number of vertices");
      }
    } else if (command == 'E') {
      string edgesStr;
      if (iss >> edgesStr) {
        vector<pair<int, int>> edgePairs = extractEdges(edgesStr);
        for (const auto& edge : edgePairs) {
          street.edgesInPlot.push_back(edge.first);
          street.edgesInPlot.push_back(edge.second);
        }

        street.GenerateAdjMatrix();
        street.vertexCoverInfo.VxCv = street.vertices;
        pthread_t thread0, thread1, thread2;
        pthread_create(&thread0, nullptr, threadCNF, &street);
        bool timeout = false;
        if(street.vertices > 10){
          int timeout_duration = 15;
          sleep(timeout_duration);
          int joinResult = pthread_tryjoin_np(thread0, nullptr);
          if (joinResult != 0) {
            timeout = true; 
          }
        } else {
          pthread_join(thread0, nullptr);
        }
        std::tuple<Graph*, std::vector<std::pair<int, int>>> args(&street,
                                                                  edgePairs);
        pthread_create(&thread1, nullptr, threadApproxAppVC1, &args);
        pthread_create(&thread2, nullptr, threadAppVC2, &args);
        pthread_join(thread1, nullptr);
        pthread_join(thread2, nullptr);
        street.printOutput(street, timeout);
        return 0;
      } else {
        throw invalid_argument("Invalid edge format");
      }
    } else {
      throw invalid_argument("Unknown command");
    }
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
  return 0;
}

void* threadIO(void* arg) {
    std::string line;
    Graph& street = *reinterpret_cast<Graph*>(arg);
    while (std::getline(std::cin, line)) {
        if (line.empty() || std::all_of(line.begin(), line.end(), ::isspace)) {
            continue;
        }
        // Parse and process the input line
        parseInput(line, street);
    }
    if (!std::cin.eof()) {
        std::cerr << "Error: Input terminated unexpectedly." << std::endl;
    }
    return nullptr;
}

// This main method creates IO thread for processing of input and output
int main() {
  Graph street(0);
  pthread_t threadio;
  pthread_create(&threadio, nullptr, threadIO, &street);
  pthread_join(threadio, nullptr);
  return 0;
}