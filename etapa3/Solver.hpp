#ifndef SOLVER_HPP
#define SOLVER_HPP

#include "Graph.hpp"
#include "Solution.hpp"
#include <chrono>
#include <random>
#include <algorithm>
#include <filesystem>
#include <vector>
#include <cmath>

/**
 * @class Solver
 * @brief Classe principal que encapsula a lógica para resolver o problema de roteamento.
 * Contém métodos para construção de solução inicial e otimização através de busca local.
 */
class Solver {
private:
    Graph* graph;
    vector<Service> allServices;
    int depot;
    int capacity;
    string instanceName;
    vector<vector<long long>> distances;
    bool areDistancesCalculated = false;

    /**
     * @brief Garante que a matriz de distâncias seja calculada (via Floyd-Warshall) apenas uma vez e quando necessário.
     */
    void ensureDistancesCalculated() {
        if (!areDistancesCalculated) {
            distances = graph->floydWarshall();
            areDistancesCalculated = true;
        }
    }

public:
    /**
     * @brief Construtor da classe Solver.
     * @param g Ponteiro para o objeto Graph.
     * @param depotNode O nó de início e fim de todas as rotas.
     * @param vehicleCapacity A capacidade máxima de cada veículo.
     * @param name O nome da instância, usado para logs.
     */
    Solver(Graph* g, int depotNode, int vehicleCapacity, string name) 
        : graph(g), depot(depotNode), capacity(vehicleCapacity), instanceName(name) {
        if (!graph || depotNode < 0 || depotNode >= g->numNodes() || vehicleCapacity <= 0) {
            throw invalid_argument("Parâmetros do Solver inválidos.");
        }
    }

    /**
     * @brief Adiciona um novo serviço à lista de serviços a serem atendidos.
     */
    void addService(int id, char type, int u, int v, int demand, int serviceCost, int travelCost) {
        allServices.emplace_back(id, type, u, v, demand, serviceCost, travelCost);
    }

    /**
     * @brief Retorna a distância mínima entre dois nós.
     * @return A distância, ou INF se não houver caminho.
     */
    long long getDistance(int from, int to) {
        ensureDistancesCalculated();
        if (from < 0 || to < 0 || static_cast<size_t>(from) >= distances.size() || static_cast<size_t>(to) >= distances.size()) return INF;
        return distances[from][to];
    }
    
    /**
     * @brief Calcula o custo total exato de uma rota, somando custos de deslocamento e de serviço.
     * @param services Vetor de serviços que compõem a rota.
     * @return O custo total da rota, ou INF se a rota for inviável.
     */
    long long calculateRouteCost(const vector<Service>& services) {
        if (services.empty()) return 0;
        long long currentCost = 0;
        int lastNode = depot;
        for (const auto& service : services) {
            long long travelToServiceCost = getDistance(lastNode, service.u);
            if (travelToServiceCost >= INF) return INF;
            currentCost += travelToServiceCost + service.serviceCost;
            if (service.type != 'N') currentCost += service.travelCost;
            lastNode = service.v;
        }
        long long travelToDepotCost = getDistance(lastNode, depot);
        if (travelToDepotCost >= INF) return INF;
        currentCost += travelToDepotCost;
        return currentCost;
    }
    
    /**
     * @brief Recalcula todas as métricas (custo, demanda) de uma solução.
     * Também remove rotas que possam ter ficado vazias após movimentos de busca local.
     * @param solution A solução a ser recalculada.
     */
    void recalculateSolutionMetrics(Solution& solution) {
        solution.totalCost = 0;
        for (auto& route : solution.routes) {
            route.totalDemand = 0;
            for (const auto& service : route.services) {
                route.totalDemand += service.demand;
            }
            route.totalCost = calculateRouteCost(route.services);
            if (route.totalCost >= INF) {
                solution.totalCost = INF;
                return;
            }
            solution.totalCost += route.totalCost;
        }
        solution.routes.erase(remove_if(solution.routes.begin(), solution.routes.end(), 
            [](const Route& r){ return r.services.empty(); }), solution.routes.end());
    }

    /**
     * @brief (Etapa 2) Constrói uma solução inicial viável, mas ingênua.
     * Cria uma rota separada para cada serviço obrigatório (Depósito -> Serviço -> Depósito).
     * @return Uma struct Solution inicial.
     */
    Solution constructInitialSolution() {
        Solution solution;
        int routeIdCounter = 1;
        for (const auto& service : allServices) {
            if (service.demand > capacity) {
                cerr << "ERRO CRÍTICO [" << instanceName << "]: Serviço " << service.id << " tem demanda maior que a capacidade." << endl;
                solution.totalCost = INF;
                return solution;
            }
            Route newRoute;
            newRoute.id = routeIdCounter++;
            newRoute.services.push_back(service);
            if (calculateRouteCost(newRoute.services) >= INF) {
                cerr << "ERRO CRÍTICO [" << instanceName << "]: Serviço " << service.id << " é inalcançável." << endl;
                solution.totalCost = INF;
                return solution;
            }
            solution.routes.push_back(newRoute);
        }
        recalculateSolutionMetrics(solution);
        return solution;
    }

    /**
     * @brief Tenta mover um serviço de uma rota para outra (inter-rota).
     * @return True se uma melhoria foi encontrada e aplicada, false caso contrário.
     */
    bool tryRelocate(Solution& solution) {
        for (size_t r1_idx = 0; r1_idx < solution.routes.size(); ++r1_idx) {
            for (size_t s_idx = 0; s_idx < solution.routes[r1_idx].services.size(); ++s_idx) {
                Service service_to_move = solution.routes[r1_idx].services[s_idx];
                for (size_t r2_idx = 0; r2_idx < solution.routes.size(); ++r2_idx) {
                    if (r1_idx == r2_idx) continue;
                    if (solution.routes[r2_idx].totalDemand + service_to_move.demand > capacity) continue;
                    vector<Service> r1_after_removal = solution.routes[r1_idx].services;
                    r1_after_removal.erase(r1_after_removal.begin() + s_idx);
                    long long cost_r1_after = calculateRouteCost(r1_after_removal);
                    
                    for (size_t pos = 0; pos <= solution.routes[r2_idx].services.size(); ++pos) {
                        vector<Service> r2_after_insertion = solution.routes[r2_idx].services;
                        r2_after_insertion.insert(r2_after_insertion.begin() + pos, service_to_move);
                        long long cost_r2_after = calculateRouteCost(r2_after_insertion);
                        if (cost_r1_after != INF && cost_r2_after != INF && cost_r1_after + cost_r2_after < solution.routes[r1_idx].totalCost + solution.routes[r2_idx].totalCost) {
                            solution.routes[r1_idx].services = r1_after_removal;
                            solution.routes[r2_idx].services = r2_after_insertion;
                            recalculateSolutionMetrics(solution);
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }

    /**
     * @brief Tenta trocar (swap) um serviço de uma rota com um serviço de outra.
     * @return True se uma melhoria foi encontrada e aplicada, false caso contrário.
     */
    bool trySwap(Solution& solution) {
        for (size_t r1_idx = 0; r1_idx < solution.routes.size(); ++r1_idx) {
            for (size_t r2_idx = r1_idx + 1; r2_idx < solution.routes.size(); ++r2_idx) {
                for (size_t s1_idx = 0; s1_idx < solution.routes[r1_idx].services.size(); ++s1_idx) {
                    for (size_t s2_idx = 0; s2_idx < solution.routes[r2_idx].services.size(); ++s2_idx) {
                        Service s1 = solution.routes[r1_idx].services[s1_idx];
                        Service s2 = solution.routes[r2_idx].services[s2_idx];
                        if (solution.routes[r1_idx].totalDemand - s1.demand + s2.demand <= capacity &&
                            solution.routes[r2_idx].totalDemand - s2.demand + s1.demand <= capacity) {
                            vector<Service> new_r1_services = solution.routes[r1_idx].services;
                            vector<Service> new_r2_services = solution.routes[r2_idx].services;
                            new_r1_services[s1_idx] = s2;
                            new_r2_services[s2_idx] = s1;
                            long long cost_new_r1 = calculateRouteCost(new_r1_services);
                            long long cost_new_r2 = calculateRouteCost(new_r2_services);
                            if (cost_new_r1 != INF && cost_new_r2 != INF && cost_new_r1 + cost_new_r2 < solution.routes[r1_idx].totalCost + solution.routes[r2_idx].totalCost) {
                                solution.routes[r1_idx].services = new_r1_services;
                                solution.routes[r2_idx].services = new_r2_services;
                                recalculateSolutionMetrics(solution);
                                return true;
                            }
                        }
                    }
                }
            }
        }
        return false;
    }
    
    /**
     * @brief Aplica a heurística 2-opt para otimizar o caminho DENTRO de cada rota.
     * Tenta remover cruzamentos no percurso.
     * @return True se uma melhoria foi encontrada e aplicada, false caso contrário.
     */
    bool try2Opt(Solution& solution) {
        for(size_t r_idx = 0; r_idx < solution.routes.size(); ++r_idx) {
            if (solution.routes[r_idx].services.size() < 2) continue;
            bool local_improvement = true;
            while(local_improvement) {
                local_improvement = false;
                for (size_t i = 0; i < solution.routes[r_idx].services.size() - 1; ++i) {
                    for (size_t j = i + 1; j < solution.routes[r_idx].services.size(); ++j) {
                        vector<Service> new_services = solution.routes[r_idx].services;
                        reverse(new_services.begin() + i, new_services.begin() + j + 1);
                        long long new_cost = calculateRouteCost(new_services);
                        if (new_cost < solution.routes[r_idx].totalCost) {
                            solution.routes[r_idx].services = new_services;
                            recalculateSolutionMetrics(solution);
                            local_improvement = true;
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }

    /**
     * @brief (Etapa 3) Executa uma busca local baseada em vizinhanças variáveis (VNS).
     * Aplica repetidamente os movimentos Relocate, Swap e 2-opt até que nenhuma melhoria seja possível.
     * @param solution A solução a ser otimizada.
     */
    void localSearch(Solution& solution) {
        bool improvement = true;
        while(improvement){
            improvement = false;
            if (tryRelocate(solution)) {
                improvement = true;
                continue;
            }
            if (trySwap(solution)) {
                improvement = true;
                continue;
            }
            if (try2Opt(solution)) {
                improvement = true;
                continue;
            }
        }
    }

    /**
     * @brief Orquestra todo o processo: construção da solução inicial e otimização.
     * @return A solução final (otimizada).
     */
    Solution solve() {
        auto start = chrono::high_resolution_clock::now();
        cout << "LOG [" << instanceName << "]: Total de serviços a serem atendidos: " << allServices.size() << endl;
        ensureDistancesCalculated();
        Solution solution = constructInitialSolution();
        if (solution.totalCost < INF) {
            cout << "LOG [" << instanceName << "]: Iniciando busca local..." << endl;
            localSearch(solution);
            cout << "LOG [" << instanceName << "]: Busca local concluida. Custo final: " << solution.totalCost << endl;
        } else {
             cout << "ERRO [" << instanceName << "]: Não foi possível construir uma solução inicial viável. Otimizacao abortada." << endl;
        }
        auto end = chrono::high_resolution_clock::now();
        solution.executionTimeMicroseconds = chrono::duration_cast<chrono::microseconds>(end - start).count();
        for(size_t i = 0; i < solution.routes.size(); ++i) {
            solution.routes[i].id = i + 1;
        }
        return solution;
    }

    /**
     * @brief Salva a solução final em um arquivo, seguindo o formato especificado.
     * @param solution A solução a ser salva.
     * @param instanceName O nome do arquivo de saída.
     */
    void saveSolution(const Solution& solution, const string& instanceName) {
        string dirPath = "solucoes";
        try { if (!filesystem::exists(dirPath)) filesystem::create_directories(dirPath); }
        catch (const exception& e) { cerr << "Erro ao criar diretório " << dirPath << ": " << e.what() << endl; return; }
        string solutionPath = dirPath + "/sol-" + instanceName;
        ofstream out(solutionPath);
        if (!out.is_open()) { cerr << "Erro ao criar arquivo de solucao em: " << solutionPath << endl; return; }
        if (solution.totalCost >= INF || (solution.routes.empty() && !allServices.empty()) ) {
            out << "inviavel" << endl;
        } else {
            out << solution.totalCost << endl;
            out << solution.routes.size() << endl;
            out << solution.executionTimeMicroseconds << endl;
            out << solution.executionTimeMicroseconds << endl;
            for (const auto& route : solution.routes) {
                out << " 0 1 " << route.id << " " << route.totalDemand << " " << route.totalCost 
                    << " " << (route.services.size() + 2);
                out << " (D 0," << depot + 1 << "," << depot + 1 << ")";
                for (const auto& service : route.services) {
                    out << " (S " << service.id << "," << service.u + 1 << "," << service.v + 1 << ")";
                }
                out << " (D 0," << depot + 1 << "," << depot + 1 << ")";
                out << endl;
            }
        }
        out.close();
        if(solution.totalCost < INF) {
            cout << "Solucao salva em: " << solutionPath << endl;
            cout << "  - Custo Total: " << solution.totalCost << endl;
            cout << "  - N. de Rotas: " << solution.routes.size() << endl;
        }
    }
};
#endif