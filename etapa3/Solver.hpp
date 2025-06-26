#ifndef SOLVER_HPP
#define SOLVER_HPP

#include "Graph.hpp"
#include <chrono>
#include <random>
#include <algorithm>
#include <filesystem>
#include <vector>
#include <cmath>

// Estruturas de dados para organizar a solução
struct Service {
    int id; char type; int u, v; int demand; int serviceCost; int travelCost;
    Service(int _id, char _type, int _u, int _v, int _demand, int _serviceCost, int _travelCost)
        : id(_id), type(_type), u(_u), v(_v), demand(_demand), serviceCost(_serviceCost), travelCost(_travelCost) {}
};
struct Route { int id; vector<Service> services; int totalDemand = 0; long long totalCost = 0; };
struct Solution { long long totalCost = 0; vector<Route> routes; long long executionTimeMicroseconds = 0; };

class Solver {
private:
    Graph* graph;
    vector<Service> allServices;
    int depot;
    int capacity;
    vector<vector<long long>> distances; // Matriz de distâncias
    bool areDistancesCalculated = false; // Flag para controlar o cálculo

    // Método privado para garantir que as distâncias sejam calculadas antes do uso
    void ensureDistancesCalculated() {
        if (!areDistancesCalculated) {
            cout << "LOG: Acessando distâncias pela primeira vez. Calculando a matriz (Floyd-Warshall)..." << endl;
            distances = graph->floydWarshall();
            areDistancesCalculated = true;
        }
    }

public:
    Solver(Graph* g, int depotNode, int vehicleCapacity) 
        : graph(g), depot(depotNode), capacity(vehicleCapacity) {
        if (!graph || depotNode < 0 || depotNode >= g->numNodes() || vehicleCapacity <= 0) {
            throw invalid_argument("Parâmetros do Solver inválidos.");
        }
        // AVISO: O cálculo de 'distances' foi REMOVIDO daqui.
    }

    void addService(int id, char type, int u, int v, int demand, int serviceCost, int travelCost) {
        allServices.emplace_back(id, type, u, v, demand, serviceCost, travelCost);
    }

    long long getDistance(int from, int to) {
        ensureDistancesCalculated(); // Garante que a matriz de distâncias existe
        if (from < 0 || to < 0 || static_cast<size_t>(from) >= distances.size() || static_cast<size_t>(to) >= distances.size()) return INF;
        return distances[from][to];
    }
    
    // Calcula o custo exato de uma ÚNICA rota.
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
    }

    // ETAPA 2: Heurística Construtiva Simples
    Solution constructInitialSolution() {
        Solution solution;
        int routeIdCounter = 1;

        for (const auto& service : allServices) {
            if (service.demand > capacity) {
                cerr << "ERRO CRÍTICO: Serviço " << service.id << " tem demanda (" << service.demand
                     << ") maior que a capacidade do veículo (" << capacity << ")." << endl;
                solution.totalCost = INF;
                return solution;
            }
            Route newRoute;
            newRoute.id = routeIdCounter++;
            newRoute.services.push_back(service);
            
            if (calculateRouteCost(newRoute.services) >= INF) {
                cerr << "ERRO CRÍTICO: Serviço " << service.id << " (de " << service.u + 1 << " para " << service.v + 1 << ") é inalcançável a partir do depósito." << endl;
                solution.totalCost = INF;
                return solution;
            }
            solution.routes.push_back(newRoute);
        }
        
        recalculateSolutionMetrics(solution);
        cout << "LOG: Solução inicial (1 rota/serviço) construída com custo " << solution.totalCost << " e " << solution.routes.size() << " rotas." << endl;
        return solution;
    }

    // ETAPA 3: Busca Local para otimização
    void localSearch(Solution& solution) {
        bool improvement_found;
        do {
            improvement_found = false;
            long long best_cost_reduction = 0;
            int best_i = -1, best_j = -1;
            vector<Service> best_merged_services;

            for (size_t i = 0; i < solution.routes.size(); ++i) {
                for (size_t j = i + 1; j < solution.routes.size(); ++j) {
                    if (solution.routes[i].totalDemand + solution.routes[j].totalDemand <= capacity) {
                        long long original_sum_cost = solution.routes[i].totalCost + solution.routes[j].totalCost;
                        
                        vector<Service> merged_fwd = solution.routes[i].services;
                        merged_fwd.insert(merged_fwd.end(), solution.routes[j].services.begin(), solution.routes[j].services.end());
                        long long cost_fwd = calculateRouteCost(merged_fwd);

                        if (cost_fwd < original_sum_cost) {
                            long long reduction = original_sum_cost - cost_fwd;
                            if (reduction > best_cost_reduction) {
                                best_cost_reduction = reduction;
                                best_i = i;
                                best_j = j;
                                best_merged_services = merged_fwd;
                            }
                        }

                        vector<Service> merged_rev = solution.routes[j].services;
                        merged_rev.insert(merged_rev.end(), solution.routes[i].services.begin(), solution.routes[i].services.end());
                        long long cost_rev = calculateRouteCost(merged_rev);

                        if (cost_rev < original_sum_cost) {
                            long long reduction = original_sum_cost - cost_rev;
                            if (reduction > best_cost_reduction) {
                                best_cost_reduction = reduction;
                                best_i = j;
                                best_j = i;
                                best_merged_services = merged_rev;
                            }
                        }
                    }
                }
            }

            if (best_cost_reduction > 0) {
                solution.routes[best_i].services = best_merged_services;
                solution.routes.erase(solution.routes.begin() + best_j);
                recalculateSolutionMetrics(solution);
                cout << "LOG: Rotas fundidas! Novo custo total: " << solution.totalCost << ". Numero de rotas: " << solution.routes.size() << endl;
                improvement_found = true;
            }

        } while (improvement_found);

        for(size_t i = 0; i < solution.routes.size(); ++i) {
            solution.routes[i].id = i + 1;
        }
    }


    // Orquestra todo o processo de resolução.
    Solution solve() {
        auto start = chrono::high_resolution_clock::now();
        
        cout << "LOG: Total de serviços a serem atendidos: " << allServices.size() << endl;
        
        // Garante que o cálculo de distância seja feito antes de qualquer coisa
        ensureDistancesCalculated();

        Solution solution = constructInitialSolution();
        
        if (solution.totalCost < INF) {
            cout << "LOG: Iniciando busca local para otimizar a solucao..." << endl;
            localSearch(solution);
            cout << "LOG: Busca local concluida. Custo final: " << solution.totalCost << endl;
        } else {
             cout << "ERRO: Não foi possível construir uma solução inicial viável. Otimizacao abortada." << endl;
        }

        auto end = chrono::high_resolution_clock::now();
        solution.executionTimeMicroseconds = chrono::duration_cast<chrono::microseconds>(end - start).count();
        return solution;
    }

    // Guarda a solução final no formato especificado.
    void saveSolution(const Solution& solution, const string& instanceName) {
        string dirPath = "solucoes";
        try { if (!filesystem::exists(dirPath)) filesystem::create_directories(dirPath); }
        catch (const exception& e) { cerr << "Erro ao criar diretório " << dirPath << ": " << e.what() << endl; return; }
        
        string solutionPath = dirPath + "/sol-" + instanceName;
        ofstream out(solutionPath);
        if (!out.is_open()) { cerr << "Erro ao criar arquivo de solucao em: " << solutionPath << endl; return; }

        if (solution.totalCost >= INF || solution.routes.empty() && !allServices.empty()) {
            out << "inviavel" << endl;
            cout << "AVISO: Solução é inviável, salva como 'inviavel'." << endl;
        } else {
            out << solution.totalCost << endl;
            out << solution.routes.size() << endl;
            out << solution.executionTimeMicroseconds << endl;
            out << solution.executionTimeMicroseconds << endl;

            for (const auto& route : solution.routes) {
                out << " " << depot + 1 << " 1 " << route.id << " " << route.totalDemand << " " << route.totalCost 
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
