#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <vector>
#include <string>
#include <algorithm>
#include "Graph.hpp"
#include "Solver.hpp"

using namespace std;
namespace fs = std::filesystem;

// Enum para controlar o estado do parser
enum class Section {
    NONE,
    REQ_NODES,
    REQ_EDGES,
    REQ_ARCS,
    NON_REQ_EDGES,
    NON_REQ_ARCS
};

// Função para ler arquivo de entrada e configurar solver
Solver* parseInputFile(const string& filename, Graph*& graph) {
    ifstream infile(filename);
    if (!infile.is_open()) {
        cerr << "Erro ao abrir o arquivo: " << filename << endl;
        return nullptr;
    }

    string line;
    int V = 0, capacity = 0, depot_node = 0;
    Solver* solver = nullptr;
    int serviceId = 1;

    cout << "Iniciando leitura do arquivo: " << filename << endl;

    // Lê as informações do cabeçalho primeiro
    while (getline(infile, line)) {
        if (line.find("Capacity:") != string::npos) {
            stringstream ss(line);
            string tmp;
            ss >> tmp >> capacity;
            cout << "Capacidade: " << capacity << endl;
        } else if (line.find("Depot Node:") != string::npos) {
            stringstream ss(line);
            string tmp1, tmp2;
            ss >> tmp1 >> tmp2 >> depot_node;
            cout << "Depósito: " << depot_node << endl;
        } else if (line.find("#Nodes:") != string::npos) {
            stringstream ss(line);
            string tmp;
            ss >> tmp >> V;
            cout << "Número de nós: " << V << endl;
        }
        // Para quando encontrar a primeira seção de dados para não ler o arquivo todo aqui
        if (line.find("ReN.") != string::npos || line.find("#Required") != string::npos) {
            break;
        }
    }
    
    if (V <= 0 || capacity <= 0 || depot_node <= 0) {
        cerr << "Erro: Informações essenciais (Nós, Capacidade, Depósito) não encontradas no cabeçalho." << endl;
        return nullptr;
    }
    
    graph = new Graph(V);
    solver = new Solver(graph, depot_node - 1, capacity); // Ajusta depot para base 0 aqui
    cout << "Solver inicializado com sucesso" << endl;

    // ====================================================================
    // MUDANÇA CRÍTICA: Lógica de parsing robusta com máquina de estados
    // ====================================================================
    Section currentSection = Section::NONE;
    
    // Continua lendo do ponto onde parou
    do {
        // Ignora linhas de comentário ou vazias
        if (line.empty() || line[0] == 'c' || line.find_first_not_of(" \t\r\n") == string::npos) {
            continue;
        }

        // Detecta o início de uma nova seção
        bool is_header = false;
        if (line.find("ReN.") != string::npos || line.find("#Required N:") != string::npos) {
            currentSection = Section::REQ_NODES;
            is_header = true;
        } else if (line.find("ReE.") != string::npos || line.find("#Required E:") != string::npos) {
            currentSection = Section::REQ_EDGES;
            is_header = true;
        } else if (line.find("ReA.") != string::npos || line.find("#Required A:") != string::npos) {
            currentSection = Section::REQ_ARCS;
            is_header = true;
        } else if (line.find("EDGE") != string::npos && line.find("ReE.") == string::npos) {
            currentSection = Section::NON_REQ_EDGES;
            is_header = true;
        } else if (line.find("ARC") != string::npos && line.find("ReA.") == string::npos) {
            currentSection = Section::NON_REQ_ARCS;
            is_header = true;
        }

        // Se for um cabeçalho de seção ou de coluna, pula para a próxima linha
        if (is_header || line.find("DEMAND") != string::npos || line.find("From N.") != string::npos || line.find("FROM N.") != string::npos) {
             continue;
        }

        stringstream ss(line);
        switch (currentSection) {
            case Section::REQ_NODES: {
                string nodeIdStr;
                int demand, serviceCost;
                if (ss >> nodeIdStr >> demand >> serviceCost) {
                    int u_node = stoi(nodeIdStr.substr(1)) - 1;
                    graph->setRequiredNode(u_node);
                    solver->addService(serviceId++, 'N', u_node, u_node, demand, serviceCost, 0);
                }
                break;
            }
            case Section::REQ_EDGES: {
                string edgeIdStr;
                int u_edge, v_edge, travelCost, demand, serviceCost;
                if (ss >> edgeIdStr >> u_edge >> v_edge >> travelCost >> demand >> serviceCost) {
                    u_edge--; v_edge--;
                    graph->addEdge(u_edge, v_edge, travelCost, false, true);
                    solver->addService(serviceId++, 'E', u_edge, v_edge, demand, serviceCost, travelCost);
                }
                break;
            }
            case Section::REQ_ARCS: {
                string arcIdStr;
                int u_arc, v_arc, travelCost, demand, serviceCost;
                if (ss >> arcIdStr >> u_arc >> v_arc >> travelCost >> demand >> serviceCost) {
                    u_arc--; v_arc--;
                    graph->addEdge(u_arc, v_arc, travelCost, true, true);
                    solver->addService(serviceId++, 'A', u_arc, v_arc, demand, serviceCost, travelCost);
                }
                break;
            }
            case Section::NON_REQ_EDGES: {
                string edgeIdStr;
                int u_nre, v_nre, cost;
                if (ss >> edgeIdStr >> u_nre >> v_nre >> cost) {
                    u_nre--; v_nre--;
                    graph->addEdge(u_nre, v_nre, cost, false, false);
                }
                break;
            }
            case Section::NON_REQ_ARCS: {
                 string arcIdStr;
                int u_nra, v_nra, cost;
                 if (ss >> arcIdStr >> u_nra >> v_nra >> cost) {
                    u_nra--; v_nra--;
                    graph->addEdge(u_nra, v_nra, cost, true, false);
                }
                break;
            }
            case Section::NONE:
                break;
        }
    } while (getline(infile, line));

    infile.close();
    cout << "Parser concluído com sucesso!" << endl;
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
        
        Solution solution;
        if (opcao == 2 || opcao == 3) {
            solution = solver->solve(); 
        }

        switch (opcao) {
            case 1:
                graph->printStatsToFile("estatisticas/estatisticas_" + baseFilename + ".txt");
                graph->exportToDOT("grafos/grafo_" + baseFilename + ".dot");
                cout << "✓ Estatísticas geradas para " << filename << endl;
                break;
                
            case 2: {
                solver->saveSolution(solution, filename);
                cout << "✓ Solução gerada para " << filename << endl;
                break;
            }
            
            case 3: {
                graph->printStatsToFile("estatisticas/estatisticas_" + baseFilename + ".txt");
                graph->exportToDOT("grafos/grafo_" + baseFilename + ".dot");
                solver->saveSolution(solution, filename);
                cout << "✓ Processamento completo para " << filename << endl;
                break;
            }
        }

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

vector<string> getDatFiles(const string& folderPath) {
    vector<string> datFiles;
    if (!fs::exists(folderPath)) {
        cerr << "Pasta não encontrada: " << folderPath << endl;
        return datFiles;
    }
    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.is_regular_file()) {
            string filename = entry.path().filename().string();
            if (filename.size() >= 4 && (filename.substr(filename.size() - 4) == ".dat" || filename.substr(filename.size() - 4) == ".txt")) {
                datFiles.push_back(filename);
            }
        }
    }
    sort(datFiles.begin(), datFiles.end());
    return datFiles;
}

int main() {
    cout << "=== PROCESSADOR DE ARQUIVOS CARP ===" << endl;
    
    try {
        fs::create_directories("solucoes");
        fs::create_directories("estatisticas");
        fs::create_directories("grafos");
    } catch (const exception& e) {
        cerr << "Aviso: Erro ao criar pastas de saída: " << e.what() << endl;
    }
    
    cout << "\nEscolha uma opção:" << endl;
    cout << "1 - Processar arquivo específico" << endl;
    cout << "2 - Processar todos os arquivos da pasta 'entradas'" << endl;
    
    int modoProcessamento;
    cin >> modoProcessamento;
    
    if (modoProcessamento == 1) {
        string filename;
        cout << "Digite o nome do arquivo (com extensão): ";
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
        
        vector<string> datFiles = getDatFiles("entradas");
        
        if (datFiles.empty()) {
            cout << "Nenhum arquivo .dat ou .txt encontrado na pasta 'entradas'" << endl;
            return 1;
        }
        
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
             << (datFiles.empty() ? 0 : (100.0 * sucessos / datFiles.size())) << "%" << endl;
        
    } else {
        cout << "Opção inválida!" << endl;
        return 1;
    }
    
    return 0;
}
