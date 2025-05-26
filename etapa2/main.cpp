#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <vector>
#include <string>
#include "Graph.hpp"
#include "Solver.hpp"

using namespace std;
namespace fs = std::filesystem;

// Função para ler arquivo de entrada e configurar solver
Solver* parseInputFile(const string& filename, Graph*& graph) {
    ifstream infile(filename);
    if (!infile.is_open()) {
        cerr << "Erro ao abrir o arquivo: " << filename << endl;
        return nullptr;
    }

    string line;
    int V = 0, capacity = 0, depot = 0; // Inicializar depot como 0
    Solver* solver = nullptr;
    
    // Contadores para IDs de serviços
    int serviceId = 1;

    cout << "Iniciando leitura do arquivo: " << filename << endl;

    while (getline(infile, line)) {
        if (line.empty() || line[0] == 'c') continue;

        // Lê número de nós
        if (line.find("#Nodes:") != string::npos) {
            stringstream ss(line);
            string tmp;
            ss >> tmp >> V;
            
            if (V <= 0 || V > 10000) {
                cerr << "Erro: Número de nós inválido: " << V << endl;
                infile.close();
                return nullptr;
            }
            
            try {
                graph = new Graph(V);
                cout << "Número de nós: " << V << endl;
            } catch (const exception& e) {
                cerr << "Erro ao criar grafo: " << e.what() << endl;
                infile.close();
                return nullptr;
            }
        }
        
        // Lê capacidade
        if (line.find("Capacity:") != string::npos) {
            stringstream ss(line);
            string tmp;
            ss >> tmp >> capacity;
            if (capacity <= 0) {
                cerr << "Aviso: Capacidade inválida (" << capacity << "), usando capacidade 100" << endl;
                capacity = 100;
            }
            cout << "Capacidade: " << capacity << endl;
        }
        
        // Lê nó depósito
        if (line.find("Depot Node:") != string::npos) {
            stringstream ss(line);
            string tmp1, tmp2;
            int depotInput;
            ss >> tmp1 >> tmp2 >> depotInput;
            depot = depotInput - 1; // Ajusta para índice base 0
            
            if (depot < 0) {
                cerr << "Aviso: Depot inválido (" << depotInput << "), usando depot 1" << endl;
                depot = 0;
            } else if (V > 0 && depot >= V) {
                cerr << "Aviso: Depot fora dos limites (" << depotInput << "), usando depot 1" << endl;
                depot = 0;
            }
            
            cout << "Depósito: " << depot + 1 << " (índice " << depot << ")" << endl;
        }

        if (!graph) continue;

        // Inicializa solver quando tiver todas as informações
        if (!solver && V > 0 && capacity > 0) {
            try {
                solver = new Solver(graph, depot, capacity);
                cout << "Solver inicializado com sucesso" << endl;
            } catch (const exception& e) {
                cerr << "Erro ao inicializar solver: " << e.what() << endl;
                continue;
            }
        }

        // Lê nós requeridos
        if (line.find("ReN.") != string::npos && line.find("DEMAND") != string::npos) {
            cout << "Lendo nós requeridos..." << endl;
            while (getline(infile, line) && !line.empty() && line[0] != '#' && 
                   line.find("ReE.") == string::npos && line.find("EDGE") == string::npos && 
                   line.find("ReA.") == string::npos && line.find("ARC") == string::npos) {
                if (line.empty() || line.find("From N.") != string::npos) continue;
                
                stringstream ss(line);
                string nodeId;
                int demand, serviceCost;
                
                if (!(ss >> nodeId >> demand >> serviceCost)) {
                    cerr << "Erro ao ler linha de nó requerido: " << line << endl;
                    continue;
                }
                
                try {
                    // Extrai número do nó do ID (ex: "N4" -> 4)
                    if (nodeId.length() < 2 || nodeId[0] != 'N') {
                        cerr << "Formato de ID de nó inválido: " << nodeId << endl;
                        continue;
                    }
                    
                    int nodeNum = stoi(nodeId.substr(1)) - 1; // Ajusta para base 0
                    
                    if (nodeNum < 0 || nodeNum >= V) {
                        cerr << "Aviso: Nó " << nodeNum + 1 << " fora dos limites, ignorando" << endl;
                        continue;
                    }
                    
                    if (demand < 0 || serviceCost < 0) {
                        cerr << "Aviso: Valores negativos para nó " << nodeNum + 1 << ", ignorando" << endl;
                        continue;
                    }
                    
                    graph->setRequiredNode(nodeNum);
                    if (solver) {
                        solver->addService(serviceId++, 'N', nodeNum, nodeNum, demand, serviceCost, 0);
                        cout << "Nó requerido: " << nodeNum + 1 << " (demanda: " << demand << ", custo: " << serviceCost << ")" << endl;
                    }
                } catch (const exception& e) {
                    cerr << "Erro ao processar nó requerido: " << line << " - " << e.what() << endl;
                }
            }
        }

        // Lê arestas requeridas
        if (line.find("ReE.") != string::npos && line.find("From N.") != string::npos) {
            cout << "Lendo arestas requeridas..." << endl;
            while (getline(infile, line) && !line.empty() && line[0] != '#' && 
                   line.find("EDGE") == string::npos && line.find("ReA.") == string::npos && 
                   line.find("ARC") == string::npos) {
                if (line.empty()) continue;
                
                stringstream ss(line);
                string edgeId;
                int u, v, travelCost, demand, serviceCost;
                
                if (!(ss >> edgeId >> u >> v >> travelCost >> demand >> serviceCost)) {
                    cerr << "Erro ao ler linha de aresta requerida: " << line << endl;
                    continue;
                }
                
                u--; v--; // Ajusta para base 0
                
                if (u < 0 || u >= V || v < 0 || v >= V) {
                    cerr << "Aviso: Aresta com nós inválidos (" << u+1 << "," << v+1 << "), ignorando" << endl;
                    continue;
                }
                
                if (travelCost < 0 || demand < 0 || serviceCost < 0) {
                    cerr << "Aviso: Valores negativos para aresta " << u+1 << "-" << v+1 << ", ignorando" << endl;
                    continue;
                }
                
                try {
                    graph->addEdge(u, v, travelCost, false, true);
                    if (solver) {
                        solver->addService(serviceId++, 'E', u, v, demand, serviceCost, travelCost);
                        cout << "Aresta requerida: " << u + 1 << "-" << v + 1 << " (custo viagem: " << travelCost 
                             << ", demanda: " << demand << ", custo serviço: " << serviceCost << ")" << endl;
                    }
                } catch (const exception& e) {
                    cerr << "Erro ao adicionar aresta requerida: " << e.what() << endl;
                }
            }
        }

        // Lê arcos requeridos
        if (line.find("ReA.") != string::npos && line.find("FROM N.") != string::npos) {
            cout << "Lendo arcos requeridos..." << endl;
            while (getline(infile, line) && !line.empty() && line[0] != '#' && 
                   line.find("EDGE") == string::npos && line.find("ARC") == string::npos) {
                if (line.empty()) continue;
                
                stringstream ss(line);
                string arcId;
                int u, v, travelCost, demand, serviceCost;
                
                if (!(ss >> arcId >> u >> v >> travelCost >> demand >> serviceCost)) {
                    cerr << "Erro ao ler linha de arco requerido: " << line << endl;
                    continue;
                }
                
                u--; v--; // Ajusta para base 0
                
                if (u < 0 || u >= V || v < 0 || v >= V) {
                    cerr << "Aviso: Arco com nós inválidos (" << u+1 << "," << v+1 << "), ignorando" << endl;
                    continue;
                }
                
                if (travelCost < 0 || demand < 0 || serviceCost < 0) {
                    cerr << "Aviso: Valores negativos para arco " << u+1 << "->" << v+1 << ", ignorando" << endl;
                    continue;
                }
                
                try {
                    graph->addEdge(u, v, travelCost, true, true);
                    if (solver) {
                        solver->addService(serviceId++, 'A', u, v, demand, serviceCost, travelCost);
                        cout << "Arco requerido: " << u + 1 << "->" << v + 1 << " (custo viagem: " << travelCost 
                             << ", demanda: " << demand << ", custo serviço: " << serviceCost << ")" << endl;
                    }
                } catch (const exception& e) {
                    cerr << "Erro ao adicionar arco requerido: " << e.what() << endl;
                }
            }
        }

        // Lê arestas não requeridas
        if (line.find("EDGE") != string::npos && line.find("ReE.") == string::npos && 
            line.find("FROM N.") != string::npos) {
            cout << "Lendo arestas não requeridas..." << endl;
            while (getline(infile, line) && !line.empty() && line[0] != '#' && 
                   line.find("ARC") == string::npos) {
                if (line.empty()) continue;
                
                stringstream ss(line);
                string edgeId;
                int u, v, cost;
                
                if (!(ss >> edgeId >> u >> v >> cost)) {
                    cerr << "Erro ao ler linha de aresta: " << line << endl;
                    continue;
                }
                
                u--; v--; // Ajusta para base 0
                
                if (u < 0 || u >= V || v < 0 || v >= V) {
                    cerr << "Aviso: Aresta com nós inválidos (" << u+1 << "," << v+1 << "), ignorando" << endl;
                    continue;
                }
                
                if (cost < 0) {
                    cerr << "Aviso: Custo negativo para aresta " << u+1 << "-" << v+1 << ", ignorando" << endl;
                    continue;
                }
                
                try {
                    graph->addEdge(u, v, cost, false, false);
                    cout << "Aresta: " << u + 1 << "-" << v + 1 << " (custo: " << cost << ")" << endl;
                } catch (const exception& e) {
                    cerr << "Erro ao adicionar aresta: " << e.what() << endl;
                }
            }
        }

        // Lê arcos não requeridos
        if (line.find("ARC") != string::npos && line.find("ReA.") == string::npos && 
            line.find("FROM N.") != string::npos) {
            cout << "Lendo arcos não requeridos..." << endl;
            while (getline(infile, line) && !line.empty()) {
                if (line.empty()) continue;
                
                stringstream ss(line);
                string arcId;
                int u, v, cost;
                
                if (!(ss >> arcId >> u >> v >> cost)) {
                    cerr << "Erro ao ler linha de arco: " << line << endl;
                    continue;
                }
                
                u--; v--; // Ajusta para base 0
                
                if (u < 0 || u >= V || v < 0 || v >= V) {
                    cerr << "Aviso: Arco com nós inválidos (" << u+1 << "," << v+1 << "), ignorando" << endl;
                    continue;
                }
                
                if (cost < 0) {
                    cerr << "Aviso: Custo negativo para arco " << u+1 << "->" << v+1 << ", ignorando" << endl;
                    continue;
                }
                
                try {
                    graph->addEdge(u, v, cost, true, false);
                    cout << "Arco: " << u + 1 << "->" << v + 1 << " (custo: " << cost << ")" << endl;
                } catch (const exception& e) {
                    cerr << "Erro ao adicionar arco: " << e.what() << endl;
                }
            }
        }
    }

    infile.close();
    
    if (!solver) {
        cerr << "Erro: Não foi possível criar o solver. Verifique se o arquivo contém todas as informações necessárias." << endl;
        if (V == 0) cerr << "- Número de nós não foi especificado" << endl;
        if (capacity == 0) cerr << "- Capacidade não foi especificada" << endl;
    } else {
        cout << "Parser concluído com sucesso!" << endl;
    }
    
    return solver;
}

// Função para processar um único arquivo
bool processFile(const string& filename, int opcao) {
    cout << "\n" << string(60, '=') << endl;
    cout << "PROCESSANDO: " << filename << endl;
    cout << string(60, '=') << endl;
    
    string inputPath = "entradas/" + filename;
    Graph* graph = nullptr;
    Solver* solver = nullptr;
    
    try {
        solver = parseInputFile(inputPath, graph);
        
        if (!graph || !solver) {
            cerr << "Erro: Falha ao processar " << filename << endl;
            if (graph) delete graph;
            if (solver) delete solver;
            return false;
        }

        string baseFilename = filename.substr(0, filename.find('.'));
        
        switch (opcao) {
            case 1:
                // Apenas estatísticas
                try {
                    graph->printStatsToFile("estatisticas_" + baseFilename + ".txt");
                    graph->exportToDOT("grafo_" + baseFilename + ".dot");
                    cout << "✓ Estatísticas geradas para " << filename << endl;
                } catch (const exception& e) {
                    cerr << "✗ Erro ao gerar estatísticas para " << filename << ": " << e.what() << endl;
                }
                break;
                
            case 2: {
                // Apenas solução
                try {
                    string outputFile = "sol-" + filename;
                    solver->saveSolution(outputFile);
                    cout << "✓ Solução gerada para " << filename << endl;
                    cout << "  Custo total: " << solver->getTotalCost() << endl;
                    cout << "  Número de rotas: " << solver->getNumRoutes() << endl;
                } catch (const exception& e) {
                    cerr << "✗ Erro ao gerar solução para " << filename << ": " << e.what() << endl;
                }
                break;
            }
            
            case 3:
                // Estatísticas e solução
                try {
                    graph->printStatsToFile("estatisticas_" + baseFilename + ".txt");
                    graph->exportToDOT("grafo_" + baseFilename + ".dot");
                    
                    string outputFile = "sol-" + filename;
                    solver->saveSolution(outputFile);
                    
                    cout << "✓ Processamento completo para " << filename << endl;
                    cout << "  Custo total: " << solver->getTotalCost() << endl;
                    cout << "  Número de rotas: " << solver->getNumRoutes() << endl;
                    
                } catch (const exception& e) {
                    cerr << "✗ Erro ao processar " << filename << ": " << e.what() << endl;
                }
                break;
        }

        // Cleanup
        delete graph;
        delete solver;
        return true;
        
    } catch (const exception& e) {
        cerr << "✗ Erro geral ao processar " << filename << ": " << e.what() << endl;
        if (graph) delete graph;
        if (solver) delete solver;
        return false;
    }
}

// Função para obter todos os arquivos .dat da pasta
vector<string> getDatFiles(const string& folderPath) {
    vector<string> datFiles;
    
    try {
        if (!fs::exists(folderPath)) {
            cerr << "Pasta não encontrada: " << folderPath << endl;
            return datFiles;
        }

        for (const auto& entry : fs::directory_iterator(folderPath)) {
            if (entry.is_regular_file()) {
                string filename = entry.path().filename().string();
                if (filename.size() >= 4 && 
                    filename.substr(filename.size() - 4) == ".dat") {
                    datFiles.push_back(filename);
                }
            }
        }
        
        // Ordena os arquivos alfabeticamente
        sort(datFiles.begin(), datFiles.end());
        
    } catch (const exception& e) {
        cerr << "Erro ao ler pasta " << folderPath << ": " << e.what() << endl;
    }
    
    return datFiles;
}

// Função principal
int main() {
    cout << "=== PROCESSADOR DE ARQUIVOS CARP ===" << endl;
    cout << "\nEscolha uma opção:" << endl;
    cout << "1 - Processar arquivo específico" << endl;
    cout << "2 - Processar todos os arquivos .dat da pasta 'entradas'" << endl;
    
    int modoProcessamento;
    cin >> modoProcessamento;
    
    if (modoProcessamento == 1) {
        // Modo original - arquivo específico
        string filename;
        cout << "Digite o nome do arquivo (com extensão .dat): ";
        cin >> filename;
        
        cout << "\nEscolha o tipo de processamento:" << endl;
        cout << "1 - Gerar apenas estatísticas do grafo" << endl;
        cout << "2 - Gerar solução inicial" << endl;
        cout << "3 - Gerar estatísticas e solução" << endl;
        
        int opcao;
        cin >> opcao;
        
        if (opcao >= 1 && opcao <= 3) {
            processFile(filename, opcao);
        } else {
            cout << "Opção inválida!" << endl;
        }
        
    } else if (modoProcessamento == 2) {
        // Modo lote - todos os arquivos
        cout << "\nEscolha o tipo de processamento para todos os arquivos:" << endl;
        cout << "1 - Gerar apenas estatísticas do grafo" << endl;
        cout << "2 - Gerar solução inicial" << endl;
        cout << "3 - Gerar estatísticas e solução" << endl;
        
        int opcao;
        cin >> opcao;
        
        if (opcao < 1 || opcao > 3) {
            cout << "Opção inválida!" << endl;
            return 1;
        }
        
        // Cria pastas de saída se não existirem
        try {
            fs::create_directories("solucoes");
            fs::create_directories("estatisticas");
            fs::create_directories("grafos");
        } catch (const exception& e) {
            cerr << "Aviso: Erro ao criar pastas de saída: " << e.what() << endl;
        }
        
        vector<string> datFiles = getDatFiles("entradas");
        
        if (datFiles.empty()) {
            cout << "Nenhum arquivo .dat encontrado na pasta 'entradas'" << endl;
            return 1;
        }
        
        cout << "\nEncontrados " << datFiles.size() << " arquivo(s) .dat:" << endl;
        for (size_t i = 0; i < datFiles.size(); i++) {
            cout << "  " << (i+1) << ". " << datFiles[i] << endl;
        }
        
        cout << "\nIniciando processamento..." << endl;
        
        int sucessos = 0;
        int falhas = 0;
        
        for (const string& filename : datFiles) {
            if (processFile(filename, opcao)) {
                sucessos++;
            } else {
                falhas++;
            }
        }
        
        cout << "\n" << string(60, '=') << endl;
        cout << "RESUMO DO PROCESSAMENTO EM LOTE" << endl;
        cout << string(60, '=') << endl;
        cout << "Total de arquivos: " << datFiles.size() << endl;
        cout << "Sucessos: " << sucessos << endl;
        cout << "Falhas: " << falhas << endl;
        cout << "Taxa de sucesso: " << fixed << setprecision(1) 
             << (100.0 * sucessos / datFiles.size()) << "%" << endl;
        
    } else {
        cout << "Opção inválida!" << endl;
        return 1;
    }
    
    return 0;
}