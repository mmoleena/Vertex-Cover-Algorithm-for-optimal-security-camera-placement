#include <iostream>
#include <list>
#include <queue>
#include <vector>
#include <sstream>
#include <regex>
#include <unordered_map>
#include <set>

using namespace std;

//Below Graph class gives a basic illustration of a graph data structure.
class Graph 
{
public:
    int vert;
    unordered_map<int, list<int>> adjlist;
    set<int> vertices;
    Graph();
    void addEdge(int x, int y);
    vector<int> shortestPath(int src, int dest);
    void clearGraph();
};

//The initial state of a Graph object is created by this constructor by setting the number of vertices to 0.
Graph::Graph() 
{
    vert = 0;
}

void Graph::addEdge(int x, int y) 
{
    adjlist[x].push_back(y);
    adjlist[y].push_back(x);
    vertices.insert(x);
    vertices.insert(y);
}

//This section executes BFS to identify the shortest path between any two vertices, and returns a vector of vertices that represents the shortest path. 
vector<int> Graph::shortestPath(int src, int dest) 
{
    vector<int> vertex_dist(vert + 1, -1);
    vector<int> parent(vert + 1, -1);
    queue<int> q;

    q.push(src);
    vertex_dist[src] = 0;

    while (!q.empty()) 
    {
        int u = q.front();
        q.pop();

        for (int v : adjlist[u])
        {
            if (vertex_dist[v] == -1) 
            {
                vertex_dist[v] = vertex_dist[u] + 1;
                parent[v] = u;
                q.push(v);
            }
        }
    }

    vector<int> path;
    int current = dest;

    while (current != -1) 
    {
        path.push_back(current);
        current = parent[current];
    }

    reverse(path.begin(), path.end());

    return path;
}

//The below clearGraph method resets the graph to an empty state.
void Graph::clearGraph() 
{
    vert = 0;
    adjlist.clear();
    vertices.clear();
}

//The below function extracts edge pairs from a string in the format <x,y> 
list<pair<int, int>> extractEdgesPairs(string complete_edges) 
{
    list<pair<int, int>> return_data;

    string subject = complete_edges;
    regex regex_pattern("<(\\d+),(\\d+)>");
    smatch match;

    while (regex_search(subject, match, regex_pattern)) 
    {
        return_data.push_back(
            make_pair(
                atoi(match[1].str().c_str()), 
                atoi(match[2].str().c_str())
            )
        );
        subject = match.suffix().str();
    }

    return return_data;
}


//This function has been taken from the git repository. 
//It takes an input line, finds the command character, trims the argument, and if successful returns true. It returns false if the input line or parsing was problematic.

bool parse_line(const string &line, char &cmd, string &arg, string &err_msg) 
{
    istringstream input(line);
    string temp;
    input >> ws;

    if (input.eof()) 
    {
        return false;
    }

    if (input.fail()) 
    {
        return false;
    }

    input.get(cmd);
    input.ignore(numeric_limits<streamsize>::max(), ' ');
    getline(input, temp);

    temp = temp.substr(temp.find_first_not_of(" \t"), temp.find_last_not_of(" \t") + 1);
    arg = temp;

    return true;
}

//The main function is shown below. It initializes a Graph object, reads and analyzes input lines, and takes various actions depending on the kind of input command ('V', 'E', or's'). 
int main(int argc, char** argv) 
{
    Graph v;
    string line;
    list<pair<int, int>> edgeList;
    
    while (getline(cin, line)) 
    {
        if(line.empty()) 
        {
            cerr<<"Error: Empty Line"<<endl;
            continue;
        }
        char cmd;
        string arg;
        string err_msg;

        if (!parse_line(line, cmd, arg, err_msg)) 
        {
            cerr << "Error: " << err_msg << endl;
            continue; 
        }

        if (cmd == 'V') 
        {
            v.clearGraph();
            edgeList.clear();
            v.vert = stoi(arg);
            if(v.vert==0)
            {
               std::cerr<<"Error: 0 is not correct vertex"<<std::endl;
               continue;
            }
        } 
        else if (cmd == 'E') 
        {
            
            if(v.vert != -1 && edgeList.empty())
            {
                edgeList = extractEdgesPairs(arg);
            }
            if(v.vert == -1)
            {
                break;
            }
            for (const auto& edge : edgeList) 
            {
                if (edge.first > v.vert || edge.second > v.vert) 
                {
                    cerr << "Error: Incorrect Edge passed" << endl;
                    v.vert = -1;
                    break; 
                }
                v.addEdge(edge.first, edge.second);
            }
        } else if (cmd == 's') 
        {
            int src, dest;
            istringstream(arg) >> src >> dest;
            
            if(v.vert == -1)
            {
                 continue;
            }
            
            if(src == 0 || dest == 0) 
            {
                cerr << "Error: 0 is not correct vertex" <<std::endl;
                continue;
            }            
            if (src > v.vert || dest > v.vert) 
            {
                cerr << "Error: Vertex does not exist" << endl;
                continue; 
            }
            
            vector<int> shortestPath;
            
            if(v.vert != -1)
            {
                 shortestPath = v.shortestPath(src, dest);
            }
               
            if (shortestPath.empty() || shortestPath[0] != src) 
            {
                cerr << "Error: No path exists between " << src << " and " << dest << endl;
            } else {
                for (std::vector<int>::size_type i = 0; i < shortestPath.size() - 1; i++) 
                {
                    cout << shortestPath[i] << "-";
                }
                cout << shortestPath[shortestPath.size() - 1] << endl;
            }
        }
    }

    return 0;
}