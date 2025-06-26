#ifndef GRAPH_HPP
#define GRAPH_HPP

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
const long long INF = numeric_limits<long long>::max();

/**
 * @struct Edge
 * @brief Representa uma aresta ou arco no grafo.
 */
struct Edge {
    int to;       // Nó de destino
    int cost;     // Custo para atravessar
    bool required; // Se o serviço nesta aresta/arco é obrigatório
};

/**
 * @class Graph
 * @brief Representa o grafo do problema, contendo nós e suas conexões (arestas/arcos).
 */
class Graph {
private:
    int V; // Número de vértices
    vector<list<Edge>> adj; // Lista de adjacência para representar as conexões

public:
    /**
     * @brief Construtor da classe Graph.
     * @param vertices O número total de vértices no grafo.
     */
    Graph(int vertices) : V(vertices) {
        if (V <= 0) {
            throw invalid_argument("O número de vértices deve ser positivo.");
        }
        adj.resize(V);
    }

    /**
     * @brief Adiciona uma aresta ou arco ao grafo.
     * @param u Nó de origem.
     * @param v Nó de destino.
     * @param cost Custo de travessia.
     * @param isDirected Se a conexão é direcionada (arco) ou não (aresta).
     * @param isRequired Se a conexão corresponde a um serviço obrigatório.
     */
    void addEdge(int u, int v, int cost, bool isDirected = false, bool isRequired = false) {
        if (u < 0 || u >= V || v < 0 || v >= V) {
            return;
        }
        adj[u].push_back({v, cost, isRequired});
        if (!isDirected) {
            adj[v].push_back({u, cost, isRequired});
        }
    }
    
    /**
     * @brief Retorna o número de vértices no grafo.
     * @return O número de vértices.
     */
    int numNodes() const {
        return V;
    }

    /**
     * @brief Calcula os caminhos mais curtos entre todos os pares de nós usando o algoritmo de Floyd-Warshall.
     * @return Uma matriz de adjacência 2D contendo as distâncias mínimas entre cada par de nós.
     */
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
        return dist;
    }
};
#endif