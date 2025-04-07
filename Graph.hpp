#include <iostream>
#include <iomanip>
#include <vector>
#include <list>
#include <queue>
#include <limits>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

const int INF = numeric_limits<int>::max();

struct Edge {
    int to;
    int cost;
    bool required;
};

class Graph {
private:
    int V;
    vector<list<Edge>> adj;
    vector<bool> required_nodes;
    vector<vector<bool>> required;
    bool directed;

public:
    Graph(int vertices, bool isDirected = false) {
        V = vertices;
        adj.resize(V);
        required_nodes.resize(V, false);
        required.resize(V, vector<bool>(V, false));
        directed = isDirected;
    }

    void addEdge(int u, int v, int cost, bool isDirected = false, bool isRequired = false) {
        directed = directed || isDirected;
        adj[u].push_back({v, cost, isRequired});
        required[u][v] = isRequired;
        if (!isDirected) {
            adj[v].push_back({u, cost, isRequired});
            required[v][u] = isRequired;
        }
    }

    void setRequiredNode(int u) {
        required_nodes[u] = true;
    }

    void dfs(int u, vector<bool>& visited) {
        visited[u] = true;
        for (auto& edge : adj[u]) {
            if (!visited[edge.to]) {
                dfs(edge.to, visited);
            }
        }
    }

    vector<vector<int>> floydWarshall() {
        vector<vector<int>> dist(V, vector<int>(V, INF));
        for (int u = 0; u < V; ++u) {
            dist[u][u] = 0;
            for (auto& edge : adj[u]) {
                dist[u][edge.to] = min(dist[u][edge.to], edge.cost);
            }
        }

        for (int k = 0; k < V; ++k)
            for (int i = 0; i < V; ++i)
                for (int j = 0; j < V; ++j)
                    if (dist[i][k] < INF && dist[k][j] < INF)
                        dist[i][j] = min(dist[i][j], dist[i][k] + dist[k][j]);

        return dist;
    }

    int numVertices() {
        return V;
    }

    int numEdges() {
        int count = 0;
        for (int i = 0; i < V; ++i) {
            for (auto& edge : adj[i]) {
                if (directed || i < edge.to) {
                    count++;
                }
            }
        }
        return count;
    }

    int numArcs() {
        int count = 0;
        for (int i = 0; i < V; ++i)
            count += adj[i].size();
        return count;
    }

    int numRequiredNodes() {
        return count(required_nodes.begin(), required_nodes.end(), true);
    }

    int numRequiredEdges() {
        int count = 0;
        for (int i = 0; i < V; ++i)
            for (int j = i + 1; j < V; ++j)
                if (required[i][j] || required[j][i])
                    count++;
        return count;
    }

    int numRequiredArcs() {
        int count = 0;
        for (int i = 0; i < V; ++i)
            for (int j = 0; j < V; ++j)
                if (required[i][j])
                    count++;
        return count;
    }

    double density() {
        int e = numEdges();
        return directed ? (double)e / (V * (V - 1)) : (double)e / (V * (V - 1) / 2);
    }

    int connectedComponents() {
        vector<bool> visited(V, false);
        int count = 0;
        for (int i = 0; i < V; ++i) {
            if (!visited[i]) {
                dfs(i, visited);
                count++;
            }
        }
        return count;
    }

    int minDegree() {
        int minDeg = INF;
        for (auto& edges : adj)
            minDeg = min(minDeg, (int)edges.size());
        return minDeg;
    }

    int maxDegree() {
        int maxDeg = 0;
        for (auto& edges : adj)
            maxDeg = max(maxDeg, (int)edges.size());
        return maxDeg;
    }

    vector<double> betweenness(const vector<vector<int>>& dist) {
        vector<double> result(V, 0.0);
        for (int s = 0; s < V; ++s) {
            for (int t = 0; t < V; ++t) {
                if (s == t || dist[s][t] == INF) continue;
                for (int v = 0; v < V; ++v) {
                    if (v != s && v != t && dist[s][t] == dist[s][v] + dist[v][t])
                        result[v] += 1;
                }
            }
        }
        return result;
    }

    double averagePathLength(const vector<vector<int>>& dist) {
        double total = 0;
        int count = 0;
        for (int i = 0; i < V; ++i)
            for (int j = 0; j < V; ++j)
                if (i != j && dist[i][j] < INF) {
                    total += dist[i][j];
                    count++;
                }
        return (count == 0) ? 0 : total / count;
    }

    int diameter(const vector<vector<int>>& dist) {
        int dia = 0;
        for (int i = 0; i < V; ++i)
            for (int j = 0; j < V; ++j)
                if (dist[i][j] < INF)
                    dia = max(dia, dist[i][j]);
        return dia;
    }

    void printStats() {
        cout << "Vértices: " << numVertices() << endl;
        cout << "Arestas (arcos direcionados): " << numEdges() << endl;
        cout << "Densidade: " << fixed << setprecision(4) << density() << endl;
        cout << "Componentes Conectados: " << connectedComponents() << endl;
        cout << "Grau Mínimo: " << minDegree() << endl;
        cout << "Grau Máximo: " << maxDegree() << endl;

        auto dist = floydWarshall();
        cout << "Caminho Médio: " << fixed << setprecision(2) << averagePathLength(dist) << endl;
        cout << "Diâmetro: " << diameter(dist) << endl;

        auto btwn = betweenness(dist);
        cout << "Intermediação (Betweenness):" << endl;
        for (int i = 0; i < V; ++i)
            cout << "  Nó " << i << ": " << fixed << setprecision(2) << btwn[i] << endl;
    }
};