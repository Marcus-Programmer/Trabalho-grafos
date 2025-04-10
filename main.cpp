#include <iostream>
#include "Graph.hpp"

using namespace std;

// Função principal
int main() {
    string filename;
    cout << "Digite o caminho para o arquivo .dat: ";
    cin >> filename;

    ifstream infile(filename);
    if (!infile.is_open()) {
        cerr << "Erro ao abrir o arquivo: " << filename << endl;
        return 1;
    }

    string line;
    int V = 0;
    Graph* g = nullptr;

    while (getline(infile, line)) {
        if (line.empty() || line[0] == 'c') continue;

        if (line.find("#Nodes:") != string::npos) {
            stringstream ss(line);
            string tmp;
            ss >> tmp >> V;
            g = new Graph(V);
        }

        if (!g) continue; // Ignora tudo até inicializar o grafo

        if (line.find("ReN.") != string::npos) {
            while (getline(infile, line) && !line.empty() && line.find('.') == string::npos) {
                string id;
                int node, demand;
                stringstream ss(line);
                ss >> id >> node >> demand;
                g->setRequiredNode(node - 1); // Ajuste para índice começando em 0
            }
        }

        if (line.find("ReE.") != string::npos) {
            while (getline(infile, line) && !line.empty() && line.find('.') == string::npos) {
                string id;
                int u, v, cost, demand;
                stringstream ss(line);
                ss >> id >> u >> v >> cost >> demand;
                g->addEdge(u - 1, v - 1, cost, false, true);
            }
        }

        if (line.find("ReA.") != string::npos) {
            while (getline(infile, line) && !line.empty() && line.find('.') == string::npos) {
                string id;
                int u, v, cost, demand;
                stringstream ss(line);
                ss >> id >> u >> v >> cost >> demand;
                g->addEdge(u - 1, v - 1, cost, true, true);
            }
        }

        if (line.find("NRa.") != string::npos) {
            while (getline(infile, line) && !line.empty() && line.find('.') == string::npos) {
                string id;
                int u, v, cost, demand;
                stringstream ss(line);
                ss >> id >> u >> v >> cost >> demand;
                g->addEdge(u - 1, v - 1, cost, true, false);
            }
        }

        if (line.find("NRe.") != string::npos) {
            while (getline(infile, line) && !line.empty() && line.find('.') == string::npos) {
                string id;
                int u, v, cost, demand;
                stringstream ss(line);
                ss >> id >> u >> v >> cost >> demand;
                g->addEdge(u - 1, v - 1, cost, false, false);
            }
        }
    }

    infile.close();

    if (g) {
        g->printStats();
        g->exportToDOT("grafo.dot");
        cout << "\nArquivo grafo.dot gerado com sucesso. Você pode visualizá-lo com o Graphviz!" << endl;
        system("dot -Tpng grafo.dot -o grafo.png");
        cout << "Arquivo grafo.png gerado com sucesso!" << endl;
        delete g;
    } else {
        cerr << "Erro: grafo não inicializado!" << endl;
    }

    return 0;
}
