#include <iostream>
#include "Graph.hpp"

using namespace std;

// Função principal
int main() {
    // Inicialização o grafo
    int V, numArestas, numArcos;
    cout << "Digite o número de vértices: ";
    cin >> V;

    Graph g(V);
    // Fim da inicialização o grafo

    // Adição das arestas e arcos
    cout << "Digite o número de arestas (não direcionadas): ";
    cin >> numArestas;
    cout << "Digite as arestas no formato: origem destino custo demanda\n";
    for (int i = 0; i < numArestas; ++i) {
        int u, v, c, d;
        cin >> u >> v >> c >> d;
        g.addEdge(u, v, c, false, true);
    }

    cout << "Digite o número de arcos (direcionadas): ";
    cin >> numArcos;
    cout << "Digite os arcos no formato: origem destino custo demanda\n";
    for (int i = 0; i < numArcos; ++i) {
        int u, v, c, d;
        cin >> u >> v >> c >> d;
        g.addEdge(u, v, c, true, true);
    }
    // Fim da adição das arestas e arcos

    // Cálculo das estatísticas do grafo
    g.printStats();
    // Fim do cálculo das estatísticas do grafo

    return 0;
}
