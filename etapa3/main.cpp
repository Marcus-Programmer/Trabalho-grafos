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
#include "Solution.hpp"

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

/**
 * @brief Converte uma string para maiúsculas.
 * @param s A string a ser convertida.
 * @return A string em maiúsculas.
 */
string toUpper(string s) {
    transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return toupper(c); });
    return s;
}

/**
 * @brief Lê e interpreta um arquivo de instância do problema.
 * Utiliza uma abordagem de dois passos para garantir robustez.
 * @param filename O caminho para o arquivo de instância.
 * @param graph Referência a um ponteiro de Graph que será alocado.
 * @return Um ponteiro para um objeto Solver inicializado, ou nullptr em caso de erro.
 */
Solver* parseInputFile(const string& filename, Graph*& graph) {
    ifstream infile(filename);
    if (!infile.is_open()) {
        cerr << "ERRO: Não foi possível abrir o arquivo: " << filename << endl;
        return nullptr;
    }

    stringstream filestream;
    filestream << infile.rdbuf();
    infile.close();

    string line;
    int V = 0, capacity = 0, depot_node = 0;

    // PASSO 1: LER O CABEÇALHO
    while(getline(filestream, line)) {
        string upper_line = toUpper(line);
        size_t pos = line.find(":");
        if (pos == string::npos) continue;
        try {
            string value_str = line.substr(pos + 1);
            if (upper_line.find("CAPACITY") != string::npos) capacity = stoi(value_str);
            else if (upper_line.find("#NODES") != string::npos) V = stoi(value_str);
            else if (upper_line.find("DEPOT NODE") != string::npos) depot_node = stoi(value_str);
        } catch (const std::exception& e) {}
    }
    
    if (V <= 0 || capacity <= 0 || depot_node <= 0) {
        cerr << "ERRO CRÍTICO: Falha ao ler informações essenciais do cabeçalho." << endl;
        return nullptr;
    }

    try {
        string instance_name = fs::path(filename).filename().string();
        graph = new Graph(V);
        Solver* solver = new Solver(graph, depot_node - 1, capacity, instance_name);
        
        // PASSO 2: LER OS DADOS DO GRAFO
        filestream.clear();
        filestream.seekg(0, ios::beg);
        Section currentSection = Section::HEADER;
        int serviceId = 1;

        while(getline(filestream, line)) {
            if (line.find_first_not_of(" \t\r\n") == string::npos || line[0] == '#') continue;
            string upper_line = toUpper(line);

            bool is_section_marker = false;
            if (upper_line.find("REN.") != string::npos) { currentSection = Section::REQ_NODES; is_section_marker = true; }
            else if (upper_line.find("REE.") != string::npos) { currentSection = Section::REQ_EDGES; is_section_marker = true; }
            else if (upper_line.find("REA.") != string::npos) { currentSection = Section::REQ_ARCS; is_section_marker = true; }
            else if (upper_line.find("EDGE") != string::npos && upper_line.find("REE.") == string::npos) { currentSection = Section::NON_REQ_EDGES; is_section_marker = true; }
            else if (upper_line.find("ARC") != string::npos && upper_line.find("REA.") == string::npos) { currentSection = Section::NON_REQ_ARCS; is_section_marker = true; }

            if (is_section_marker || (currentSection != Section::HEADER && (upper_line.find("FROM N.") != string::npos || upper_line.find("DEMAND") != string::npos))) {
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
                    default: break;
                }
            }
        }
        return solver;
    } catch (const exception& e) {
        cerr << "ERRO CRÍTICO durante a inicialização: " << e.what() << endl;
        if(graph) delete graph;
        return nullptr;
    }
}

/**
 * @brief Orquestra o processamento de um único arquivo de instância.
 * @param filename O nome do arquivo a ser processado.
 * @param opcao O tipo de operação (1: Estatísticas, 2: Solução, 3: Ambos).
 * @return True se o processamento foi bem-sucedido, false caso contrário.
 */
bool processFile(const string& filename, int opcao) {
    string inputPath = "entradas/" + filename;
    Graph* graph = nullptr;
    Solver* solver = nullptr;
    
    try {
        solver = parseInputFile(inputPath, graph);
        if (!solver) {
            cerr << "ERRO: Falha ao inicializar o problema a partir de " << filename << "." << endl;
            if (graph) delete graph;
            return false;
        }

        string baseFilename = fs::path(filename).stem().string();
        string ext = fs::path(filename).extension().string();
        
        Solution solution;
        if (opcao == 2 || opcao == 3) {
            solution = solver->solve(); 
        }

        // Simplificado: Sempre salva a solução se a opção for 2 ou 3
        if (opcao == 2 || opcao == 3) {
            solver->saveSolution(solution, baseFilename + ext);
        }
        
        // As estatísticas podem ser geradas independentemente (opção 1 ou 3)
        // mas não foram implementadas nesta versão final.

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

/**
 * @brief Obtém uma lista de todos os arquivos .dat e .txt de uma pasta.
 * @param folderPath O caminho da pasta.
 * @return Um vetor de strings com os nomes dos arquivos.
 */
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

/**
 * @brief Função principal do programa.
 */
int main() {
    cout << "=== PROCESSADOR DE ARQUIVOS CARP (ETAPA 3) ===" << endl;
    
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
            cout << "\n" << string(60, '=') << endl;
            cout << "PROCESSANDO: " << filename << endl;
            cout << string(60, '=') << endl;
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
             cout << "\n" << string(60, '=') << endl;
            cout << "PROCESSANDO: " << filename << endl;
            cout << string(60, '=') << endl;
            if (processFile(filename, opcao)) {
                sucessos++;
            } else {
                falhas++;
            }
        }
        
        cout << "\n" << string(60, '=') << endl;
        cout << "RESUMO DO PROCESSAMENTO EM LOTE" << endl;
        cout << string(60, '=') << endl;
        cout << "Total de arquivos processados: " << datFiles.size() << endl;
        cout << "  - Sucessos: " << sucessos << endl;
        cout << "  - Falhas: " << falhas << endl;
    } else {
        cout << "Opção inválida!" << endl;
        return 1;
    }
    
    return 0;
}
