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
const int INF = numeric_limits<int>::max();

// Estrutura de aresta
struct Edge {
    int to;
    int cost;
    bool required;
};

// Classe do grafo
class Graph {
private:
    // Número de vértices
    int V;

    // Lista de adjacência
    vector<list<Edge>> adj;

    // Vetor de nós obrigatórios
    vector<bool> required_nodes;

    // Matriz de arestas obrigatórias
    vector<vector<bool>> required;

    // Lista de arestas obrigatórias
    bool directed;
    
    //Matriz de predecessores
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

    // Adiciona uma aresta
    void addEdge(int u, int v, int cost, bool isDirected = false, bool isRequired = false) {
        directed = directed || isDirected;
        adj[u].push_back({v, cost, isRequired});
        required[u][v] = isRequired;
        if (!isDirected) {
            adj[v].push_back({u, cost, isRequired});
            required[v][u] = isRequired;
        }
    }

    // Seta um nó como obrigatório
    void setRequiredNode(int u) {
        required_nodes[u] = true;
    }

    // Algoritmo de busca em profundidade
    void dfs(int u, vector<bool>& visited) {
        visited[u] = true;
        for (auto& edge : adj[u]) {
            if (!visited[edge.to]) {
                dfs(edge.to, visited);
            }
        }
    }

    // Algoritmo de Floyd-Warshall
    pair<vector<vector<int>>, vector<vector<int>>> floydWarshall() {
        vector<vector<int>> dist(V, vector<int>(V, INF));
        vector<vector<int>> pred(V, vector<int>(V, -1));
    
        for (int u = 0; u < V; ++u) {
            dist[u][u] = 0;
            pred[u][u] = u;
            for (auto& edge : adj[u]) {
                dist[u][edge.to] = edge.cost;
                pred[u][edge.to] = u;
            }
        }
    
        for (int k = 0; k < V; ++k) {
            for (int i = 0; i < V; ++i) {
                for (int j = 0; j < V; ++j) {
                    if (dist[i][k] < INF && dist[k][j] < INF && dist[i][k] + dist[k][j] < dist[i][j]) {
                        dist[i][j] = dist[i][k] + dist[k][j];
                        pred[i][j] = pred[k][j];
                    }
                }
            }
        }
    
        predecessor = pred; 
        return {dist, pred};
    }

    // Função para reconstruir caminhos matriz de predecessores
    vector<int> reconstructPath(int u, int v) {
        vector<int> path;
        if (predecessor.empty() || predecessor[u][v] == -1) return path;
        
        for (int at = v; at != u; at = predecessor[u][at]) {
            if (at == -1) return {};
            path.push_back(at);
        }
        path.push_back(u);
        reverse(path.begin(), path.end());
        return path;
    }

    // Função para exportar grafo como imagem
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
    
                file << "  " << u << (directed ? " -> " : " -- ") << edge.to
                     << " [label=\"" << edge.cost << "\"" 
                     << (edge.required ? ", color=red" : "") << "];\n";
            }
        }
    
        file << "}" << endl;
        file.close();
        cout << "Arquivo DOT gerado em: " << filename << endl;
    }

    // (1 - Quantidade de vértices)
    int numNodes() {
        return V;
    }

    // (2 - Quantidade de arestas)
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

    // (3 - Quantidade de arcos)
    int numArcs() {
        int count = 0;
        for (int i = 0; i < V; ++i)
            count += adj[i].size();
        return count;
    }

    // (4 - Quantidade de vértices requeridos)
    int numRequiredNodes() {
        return count(required_nodes.begin(), required_nodes.end(), true);
    }

    // (5 - Quantidade de arestas requeridas)
    int numRequiredEdges() {
        int count = 0;
        for (int i = 0; i < V; ++i)
            for (int j = i + 1; j < V; ++j)
                if (required[i][j] || required[j][i])
                    count++;
        return count;
    }

    // (6 - Quantidade de arcos requeridos)
    int numRequiredArcs() {
        int count = 0;
        for (int i = 0; i < V; ++i)
            for (int j = 0; j < V; ++j)
                if (required[i][j])
                    count++;
        return count;
    }

    // (7 - Densidade do grafo (order strength))
    double density() {
        int e = numEdges();
        return directed ? (double)e / (V * (V - 1)) : (double)e / (V * (V - 1) / 2);
    }

    // (8 - Componentes conectados)
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

    // (9 - Grau mínimo dos vértices)
    int minDegree() {
        int minDeg = INF;
        for (auto& edges : adj)
            minDeg = min(minDeg, (int)edges.size());
        return minDeg;
    }

    // (10 - Grau máximo dos vértices)
    int maxDegree() {
        int maxDeg = 0;
        for (auto& edges : adj)
            maxDeg = max(maxDeg, (int)edges.size());
        return maxDeg;
    }

    // (11 - Intermediação)
    // (A intermediação de um nó mede a frequência com que ele
    // aparece nos caminhos mais curtos entre outros nós.
    // Não é necessário calcular outros caminhos mais curtos alternativos)
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

    // (12 - Caminho médio)
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

    // (13 - Diâmetro)
    int diameter(const vector<vector<int>>& dist) {
        int dia = 0;
        for (int i = 0; i < V; ++i)
            for (int j = 0; j < V; ++j)
                if (dist[i][j] < INF)
                    dia = max(dia, dist[i][j]);
        return dia;
    }

    // (Impressão dos dados)
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
        out << left << setw(30) << "Número de Arestas:" << numEdges() << endl;
        out << left << setw(30) << "Número de Arcos:" << numArcs() << endl;
        out << left << setw(30) << "Número de Nós Obrigatórios:" << numRequiredNodes() << endl;
        out << left << setw(30) << "Arestas Obrigatórias:" << numRequiredEdges() << endl;
        out << left << setw(30) << "Arcos Obrigatórios:" << numRequiredArcs() << endl;
        out << left << setw(30) << "Densidade:" << fixed << setprecision(4) << density() << endl;
        out << left << setw(30) << "Componentes Conectados:" << connectedComponents() << endl;
        out << left << setw(30) << "Grau Mínimo:" << minDegree() << endl;
        out << left << setw(30) << "Grau Máximo:" << maxDegree() << endl;
        out << left << setw(30) << "Caminho Médio:" << fixed << setprecision(2) << averagePathLength(dist) << endl;
        out << left << setw(30) << "Diâmetro:" << diameter(dist) << endl;

        out << "\n+-------------------------------+" << endl;
        out << "|   Intermediação (Betweenness) |" << endl;
        out << "+-------------------------------+" << endl;
        auto btwn = betweenness(dist);
        out << left << setw(10) << "Nó" << "Valor" << endl;
        out << "-------------------------------" << endl;
        for (int i = 0; i < V; ++i)
            out << left << setw(10) << i << fixed << setprecision(2) << btwn[i] << endl;

        out << "\n+-------------------------------+" << endl;
        out << "|      Matriz de Distâncias     |" << endl;
        out << "+-------------------------------+" << endl;

        out << setw(6) << " ";
        for (int j = 0; j < V; ++j)
            out << setw(6) << j;
        out << endl;

        for (int i = 0; i < V; ++i) {
            out << setw(6) << i;
            for (int j = 0; j < V; ++j) {
                if (dist[i][j] == INF)
                    out << setw(6) << "INF";
                else
                    out << setw(6) << dist[i][j];
            }
            out << endl;
        }

        out.close();
        cout << "Estatísticas salvas em: " << filename << endl;
    }
};