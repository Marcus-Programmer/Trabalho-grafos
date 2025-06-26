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

// Constante para infinito, usada em algoritmos de caminho mais curto.
// Usar 'long long' para evitar overflow na soma de custos.
const long long INF = numeric_limits<long long>::max();

// Estrutura para representar uma aresta ou arco no grafo.
struct Edge {
    int to;       // Nó de destino
    int cost;     // Custo para atravessar
    bool required; // Se o serviço nesta aresta/arco é obrigatório
};

// Classe principal do Grafo
class Graph {
private:
    int V; // Número de vértices
    vector<list<Edge>> adj; // Lista de adjacência para representar as conexões

public:
    // Construtor: inicializa o grafo com um número específico de vértices.
    Graph(int vertices) : V(vertices) {
        if (V <= 0) {
            throw invalid_argument("O número de vértices deve ser positivo.");
        }
        adj.resize(V);
    }

    // Adiciona uma aresta (ou arco) ao grafo.
    void addEdge(int u, int v, int cost, bool isDirected = false, bool isRequired = false) {
        if (u < 0 || u >= V || v < 0 || v >= V) {
            cerr << "AVISO: Tentativa de adicionar aresta com nó inválido (" << u + 1 << ", " << v + 1 << "). Ignorando." << endl;
            return;
        }
        adj[u].push_back({v, cost, isRequired});
        if (!isDirected) {
            adj[v].push_back({u, cost, isRequired});
        }
    }
    
    // Retorna o número de vértices no grafo.
    int numNodes() const {
        return V;
    }

    // Algoritmo de Floyd-Warshall para calcular os caminhos mais curtos entre todos os pares de nós.
    // Retorna uma matriz de distâncias. Essencial para o Solver tomar decisões.
    vector<vector<long long>> floydWarshall() {
        vector<vector<long long>> dist(V, vector<long long>(V, INF));

        for (int u = 0; u < V; ++u) {
            dist[u][u] = 0;
            for (const auto& edge : adj[u]) {
                dist[u][edge.to] = min((long long)edge.cost, dist[u][edge.to]);
            }
        }

        for (int k = 0; k < V; ++k) {
            for (int i = 0; i < V; ++i) {
                for (int j = 0; j < V; ++j) {
                    if (dist[i][k] != INF && dist[k][j] != INF) {
                        dist[i][j] = min(dist[i][j], dist[i][k] + dist[k][j]);
                    }
                }
            }
        }
        
        cout << "LOG: Matriz de distâncias calculada via Floyd-Warshall." << endl;
        return dist;
    }

    // Gera um ficheiro .dot para visualização do grafo com Graphviz.
    void exportToDOT(const string& filename) {
        ofstream file(filename);
        if (!file.is_open()) {
            cerr << "Erro ao abrir o arquivo para escrita do DOT: " << filename << endl;
            return;
        }
    
        file << "digraph G {\n";
        file << "  node [shape=circle];\n";
    
        for (int u = 0; u < V; ++u) {
            for (const auto& edge : adj[u]) {
                 file << "  " << u + 1 << " -> " << edge.to + 1
                     << " [label=\"" << edge.cost << "\"" 
                     << (edge.required ? ", color=red, penwidth=2.0" : "") << "];\n";
            }
        }
    
        file << "}" << endl;
        file.close();
        cout << "LOG: Grafo exportado para " << filename << endl;
    }
    
    // Imprime estatísticas simples do grafo
    void printStatsToFile(const string& filename) {
        ofstream out(filename);
        if (!out.is_open()) {
            cerr << "Erro ao abrir o arquivo de estatísticas!" << endl;
            return;
        }
        out << "Estatísticas do Grafo" << endl;
        out << "---------------------" << endl;
        out << "Número de Vértices: " << V << endl;
        long long edgeCount = 0;
        for(int i=0; i<V; ++i) edgeCount += adj[i].size();
        out << "Número de Arestas/Arcos (entradas na lista de adj.): " << edgeCount << endl;
        out.close();
    }
};
