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

// Enum para controlar o estado do parser de forma robusta
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
        cerr << "ERRO: Não foi possível abrir o arquivo: " << filename << endl;
        return nullptr;
    }

    string line;
    int V = 0, capacity = 0, depot_node = 0;
    
    // Passada 1: Lê apenas as informações do cabeçalho de forma segura
    string file_content((istreambuf_iterator<char>(infile)), istreambuf_iterator<char>());
    stringstream header_stream(file_content);
    
    while(getline(header_stream, line)) {
        if (line.find("Capacity:") != string::npos) { stringstream ss(line); string tmp; ss >> tmp >> capacity; }
        else if (line.find("Depot Node:") != string::npos) { stringstream ss(line); string tmp1, tmp2; ss >> tmp1 >> tmp2 >> depot_node; }
        else if (line.find("#Nodes:") != string::npos) { stringstream ss(line); string tmp; ss >> tmp >> V; }
    }
    
    if (V <= 0 || capacity <= 0 || depot_node <= 0) {
        cerr << "ERRO CRÍTICO: Informações essenciais (Nós, Capacidade, Depósito) não encontradas ou inválidas." << endl;
        return nullptr;
    }

    cout << "LOG: Capacidade: " << capacity << ", Depósito: " << depot_node << ", Nós: " << V << endl;
    
    graph = new Graph(V);
    Solver* solver = new Solver(graph, depot_node - 1, capacity);
    cout << "LOG: Grafo e Solver inicializados com sucesso." << endl;

    int serviceId = 1;
    Section currentSection = Section::NONE;
    
    // Passada 2: Processa o conteúdo do ficheiro para ler os dados das secções
    stringstream data_stream(file_content);
    while (getline(data_stream, line)) {
        if (line.empty() || line.find_first_not_of(" \t\r\n") == string::npos) continue;

        // Determina a seção atual
        if (line.find("ReN.") != string::npos) { currentSection = Section::REQ_NODES; cout << "LOG: Lendo seção ReN..." << endl; continue; }
        if (line.find("ReE.") != string::npos) { currentSection = Section::REQ_EDGES; cout << "LOG: Lendo seção ReE..." << endl; continue; }
        if (line.find("ReA.") != string::npos) { currentSection = Section::REQ_ARCS; cout << "LOG: Lendo seção ReA..." << endl; continue; }
        if (line.find("EDGE") != string::npos && line.find("ReE.") == string::npos) { currentSection = Section::NON_REQ_EDGES; cout << "LOG: Lendo seção EDGE..." << endl; continue; }
        if (line.find("ARC") != string::npos && line.find("ReA.") == string::npos) { currentSection = Section::NON_REQ_ARCS; cout << "LOG: Lendo seção ARC..." << endl; continue; }
        
        // Ignora linhas que são cabeçalhos de tabela ou comentários
        if (line.find("DEMAND") != string::npos || line.find("From N.") != string::npos || line.find("Name:") != string::npos || line[0] == '#') continue;

        stringstream ss(line);
        string id_str;
        int u, v, cost, demand, s_cost;

        switch (currentSection) {
            case Section::REQ_NODES:
                if (ss >> id_str >> demand >> s_cost) {
                    u = stoi(id_str.substr(1));
                    solver->addService(serviceId++, 'N', u - 1, u - 1, demand, s_cost, 0);
                }
                break;
            case Section::REQ_EDGES:
                if (ss >> id_str >> u >> v >> cost >> demand >> s_cost) {
                    graph->addEdge(u - 1, v - 1, cost, false, true);
                    solver->addService(serviceId++, 'E', u - 1, v - 1, demand, s_cost, cost);
                }
                break;
            case Section::REQ_ARCS:
                if (ss >> id_str >> u >> v >> cost >> demand >> s_cost) {
                    graph->addEdge(u - 1, v - 1, cost, true, true);
                    solver->addService(serviceId++, 'A', u - 1, v - 1, demand, s_cost, cost);
                }
                break;
            case Section::NON_REQ_EDGES:
                 if (ss >> id_str >> u >> v >> cost) {
                    graph->addEdge(u - 1, v - 1, cost, false, false);
                }
                break;
            case Section::NON_REQ_ARCS:
                if (ss >> id_str >> u >> v >> cost) {
                    graph->addEdge(u - 1, v - 1, cost, true, false);
                }
                break;
            case Section::NONE:
                break;
        }
    }

    cout << "LOG: Parser concluído com sucesso!" << endl;
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
            cerr << "ERRO: Falha ao inicializar o problema a partir de " << filename << endl;
            if (graph) delete graph;
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
                break;
            case 2:
                solver->saveSolution(solution, filename);
                break;
            case 3:
                graph->printStatsToFile("estatisticas/estatisticas_" + baseFilename + ".txt");
                graph->exportToDOT("grafos/grafo_" + baseFilename + ".dot");
                solver->saveSolution(solution, filename);
                break;
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
        cout << "2 - Gerar solução (Etapa 2 e 3)" << endl;
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
        cout << "2 - Gerar solução (Etapa 2 e 3)" << endl;
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
