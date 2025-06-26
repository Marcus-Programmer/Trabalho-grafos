#pragma once
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

// Constantes
inline constexpr int INF = numeric_limits<int>::max();

// Estrutura de aresta
struct Edge {
    int to;
    int cost;
    bool required;
};

// Classe do grafo
class Graph {
private:
    int V;
    vector<list<Edge>> adj;
    vector<bool> required_nodes;
    vector<vector<bool>> required;
    bool directed;
    vector<vector<int>> predecessor;

public:
    // Construtor
    Graph(int vertices, bool isDirected = false) {
        V = vertices;
        adj.resize(V);
        required_nodes.resize(V, false);
        required.resize(V, vector<bool>(V, false));
        directed = isDirected;
    }

    void addEdge(int u, int v, int cost, bool isDirected = false, bool isRequired = false) {
        if (u >= V || v >= V || u < 0 || v < 0) return; // Proteção
        directed = directed || isDirected;
        adj[u].push_back({v, cost, isRequired});
        required[u][v] = isRequired;
        if (!isDirected) {
            adj[v].push_back({u, cost, isRequired});
            required[v][u] = isRequired;
        }
    }

    void setRequiredNode(int u) {
        if (u >= V || u < 0) return; // Proteção
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

    // ====================================================================
    // CORREÇÃO CRÍTICA FINAL NO ALGORITMO DE FLOYD-WARSHALL
    // ====================================================================
    pair<vector<vector<int>>, vector<vector<int>>> floydWarshall() {
        vector<vector<int>> dist(V, vector<int>(V, INF));
        vector<vector<int>> pred(V, vector<int>(V, -1));
    
        // 1. Inicializar a matriz de distâncias
        for (int u = 0; u < V; ++u) {
            dist[u][u] = 0; // Custo para ir de um nó a ele mesmo é 0
            pred[u][u] = u;
            for (auto& edge : adj[u]) {
                // Considera apenas a aresta de menor custo se houver múltiplas
                if (edge.cost < dist[u][edge.to]) {
                    dist[u][edge.to] = edge.cost;
                    pred[u][edge.to] = u;
                }
            }
        }
    
        // 2. Executar o algoritmo de Floyd-Warshall
        for (int k = 0; k < V; ++k) {
            for (int i = 0; i < V; ++i) {
                for (int j = 0; j < V; ++j) {
                    // Verifica se o caminho via k é mais curto
                    if (dist[i][k] != INF && dist[k][j] != INF && dist[i][k] + dist[k][j] < dist[i][j]) {
                        dist[i][j] = dist[i][k] + dist[k][j];
                        pred[i][j] = pred[k][j];
                    }
                }
            }
        }
    
        predecessor = pred; 
        return {dist, pred};
    }

    vector<int> reconstructPath(int u, int v) {
        vector<int> path;
        if (predecessor.empty() || u >= (int)predecessor.size() || v >= (int)predecessor.size() || predecessor[u][v] == -1) return path;
        
        for (int at = v; at != u; at = predecessor[u][at]) {
            if (at == -1) return {};
            path.push_back(at);
        }
        path.push_back(u);
        reverse(path.begin(), path.end());
        return path;
    }

    void exportToDOT(const string& filename) {
        ofstream file(filename);
        if (!file.is_open()) {
            cerr << "Erro ao abrir o arquivo para escrita do DOT." << endl;
            return;
        }
    
        file << (directed ? "digraph" : "graph") << " G {\n";
    
        for (int u = 0; u < V; ++u) {
            for (const auto& edge : adj[u]) {
                if (!directed && u > edge.to) continue;
    
                file << "  " << u + 1 << (directed ? " -> " : " -- ") << edge.to + 1
                     << " [label=\"" << edge.cost << "\"" 
                     << (edge.required ? ", color=red" : "") << "];\n";
            }
        }
    
        file << "}" << endl;
        file.close();
        cout << "Arquivo DOT gerado em: " << filename << endl;
    }

    int numNodes() { return V; }

    int numEdges() {
        int count = 0;
        for (int i = 0; i < V; ++i) {
            for (auto& edge : adj[i]) {
                if (!directed && i < edge.to) { // Corrigido para contar arestas não-dirigidas corretamente
                    count++;
                } else if(directed) {
                    count++;
                }
            }
        }
        // Se o grafo for misto, a definição de "numEdges" vs "numArcs" fica ambígua.
        // Esta contagem assume que arestas são não-dirigidas e arcos são dirigidos.
        return count;
    }
    
    int numRequiredNodes() {
        return count(required_nodes.begin(), required_nodes.end(), true);
    }

    void printStatsToFile(const string& filename = "estatisticas.txt") {
        auto [dist, pred] = floydWarshall();
        ofstream out(filename);

        if (!out.is_open()) {
            cerr << "Erro ao abrir o arquivo de estatísticas!" << endl;
            return;
        }

        out << "+-------------------------------+" << endl;
        out << "|       Estatísticas do Grafo  |" << endl;
        out << "+-------------------------------+" << endl;

        out << left << setw(30) << "Número de Vértices:" << V << endl;
        out << left << setw(30) << "Número de Arestas/Arcos:" << numEdges() << endl;
        out << left << setw(30) << "Número de Nós Obrigatórios:" << numRequiredNodes() << endl;

        out.close();
        cout << "Estatísticas salvas em: " << filename << endl;
    }
};
