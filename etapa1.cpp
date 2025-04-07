#include <iostream>
#include "Graph.hpp"

using namespace std;

int main() {
    int V, numArestas, numArcos;
    cout << "Digite o número de vértices: ";
    cin >> V;

    Graph g(V);

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

    g.printStats();

    return 0;
}
