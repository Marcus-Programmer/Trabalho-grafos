#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <vector>
#include <string>
#include <algorithm>
#include <cctype>
#include "Graph.hpp"
#include "Solver.hpp"

using namespace std;
namespace fs = std::filesystem;

// Enum para controlar o estado do parser de forma robusta
enum class Section {
    HEADER,
    REQ_NODES,
    REQ_EDGES,
    REQ_ARCS,
    NON_REQ_EDGES,
    NON_REQ_ARCS
};

// Função para converter uma string para maiúsculas para comparação insensível
string toUpper(string s) {
    transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return toupper(c); });
    return s;
}

// Abordagem de parser em dois passos para máxima robustez.
Solver* parseInputFile(const string& filename, Graph*& graph) {
    ifstream infile(filename);
    if (!infile.is_open()) {
        cerr << "ERRO: Não foi possível abrir o arquivo: " << filename << endl;
        return nullptr;
    }

    // Lê o arquivo inteiro para uma stringstream para permitir múltiplos passos
    stringstream filestream;
    filestream << infile.rdbuf();
    infile.close();

    string line;
    int V = 0, capacity = 0, depot_node = 0;

    // --- PASSO 1: LER APENAS O CABEÇALHO ---
    cout << "LOG: Passo 1: Lendo o cabeçalho..." << endl;
    while(getline(filestream, line)) {
        string upper_line = toUpper(line);
        size_t pos = line.find(":");
        if (pos == string::npos) continue; // Ignora linhas sem ':' no cabeçalho

        try {
            string value_str = line.substr(pos + 1);
            if (upper_line.find("CAPACITY") != string::npos) {
                capacity = stoi(value_str);
            } else if (upper_line.find("#NODES") != string::npos) {
                V = stoi(value_str);
            } else if (upper_line.find("DEPOT NODE") != string::npos) {
                depot_node = stoi(value_str);
            }
        } catch (const std::exception& e) {
            // Ignora erros de conversão, pode ser uma linha de texto qualquer
        }
    }
    
    // --- VALIDAÇÃO E INICIALIZAÇÃO ---
    cout << "LOG: Fim do Passo 1. Valores lidos -> Nós: " << V << ", Capacidade: " << capacity << ", Depósito: " << depot_node << endl;
    if (V <= 0 || capacity <= 0 || depot_node <= 0) {
        cerr << "ERRO CRÍTICO: Falha ao ler informações essenciais do cabeçalho. Verifique se '#Nodes', 'Capacity' e 'Depot Node' existem e são válidos." << endl;
        return nullptr;
    }

    try {
        graph = new Graph(V);
        Solver* solver = new Solver(graph, depot_node - 1, capacity);
        cout << "LOG: Grafo e Solver inicializados com sucesso." << endl;

        // --- PASSO 2: LER OS DADOS DO GRAFO ---
        filestream.clear(); // Limpa os flags de fim de arquivo
        filestream.seekg(0, ios::beg); // "Rebobina" a stream para o início

        Section currentSection = Section::HEADER;
        int serviceId = 1;

        cout << "LOG: Passo 2: Lendo os dados do grafo..." << endl;
        while(getline(filestream, line)) {
            if (line.find_first_not_of(" \t\r\n") == string::npos || line[0] == '#') continue;
            
            string upper_line = toUpper(line);

            bool is_section_marker = false;
            if (upper_line.find("REN.") != string::npos)      { currentSection = Section::REQ_NODES; is_section_marker = true; }
            else if (upper_line.find("REE.") != string::npos) { currentSection = Section::REQ_EDGES; is_section_marker = true; }
            else if (upper_line.find("REA.") != string::npos) { currentSection = Section::REQ_ARCS;  is_section_marker = true; }
            else if (upper_line.find("EDGE") != string::npos && upper_line.find("REE.") == string::npos) { currentSection = Section::NON_REQ_EDGES; is_section_marker = true; }
            else if (upper_line.find("ARC") != string::npos && upper_line.find("REA.") == string::npos)  { currentSection = Section::NON_REQ_ARCS;  is_section_marker = true; }

            if (is_section_marker || (currentSection != Section::HEADER && upper_line.find("FROM N.") != string::npos) ) {
                continue;
            }

            if (currentSection != Section::HEADER) {
                stringstream ss(line);
                int u, v, cost, demand, s_cost;
                string id_str;
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
                    default:
                        break;
                }
            }
        }
        cout << "LOG: Passo 2 concluído. Total de serviços adicionados: " << serviceId - 1 << endl;
        return solver;

    } catch (const exception& e) {
        cerr << "ERRO CRÍTICO durante a inicialização: " << e.what() << endl;
        if(graph) delete graph;
        return nullptr;
    }
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
        if (!solver) {
            cerr << "ERRO: Falha ao inicializar o problema a partir de " << filename << ". Parser retornou nulo." << endl;
            if (graph) delete graph;
            return false;
        }

        string baseFilename = fs::path(filename).stem().string();
        string ext = fs::path(filename).extension().string();
        
        Solution solution;
        if (opcao == 2 || opcao == 3) {
            solution = solver->solve(); 
        }

        switch (opcao) {
            case 1:
                if(graph) {
                    graph->printStatsToFile("estatisticas/estatisticas_" + baseFilename + ".txt");
                    graph->exportToDOT("grafos/grafo_" + baseFilename + ".dot");
                }
                break;
            case 2:
                solver->saveSolution(solution, baseFilename + ext);
                break;
            case 3:
                 if(graph) {
                    graph->printStatsToFile("estatisticas/estatisticas_" + baseFilename + ".txt");
                    graph->exportToDOT("grafos/grafo_" + baseFilename + ".dot");
                }
                solver->saveSolution(solution, baseFilename + ext);
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
            string ext = entry.path().extension().string();
            if (ext == ".dat" || ext == ".txt") {
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
        cout << "2 - Gerar solução (Etapa 3)" << endl;
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
        cout << "2 - Gerar solução (Etapa 3)" << endl;
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
