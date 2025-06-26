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

// ====================================================================
// ABORDAGEM FINAL: CONSTRUÇÃO GARANTIDA + OTIMIZAÇÃO PODEROSA
// Esta é a arquitetura definitiva que separa a construção de uma solução
// inicial (Etapa 2) da sua melhoria (Etapa 3).
// ====================================================================
class Solver {
private:
    Graph* graph;
    vector<Service> allServices;
    int depot;
    int capacity;
    vector<vector<int>> distances;

public:
    Solver(Graph* g, int depotNode, int vehicleCapacity) 
        : graph(g), depot(depotNode), capacity(vehicleCapacity) {
        if (!graph || depotNode < 0 || depotNode >= g->numNodes() || vehicleCapacity <= 0) {
            throw invalid_argument("Parâmetros do Solver inválidos.");
        }
        distances = graph->floydWarshall();
    }

    void addService(int id, char type, int u, int v, int demand, int serviceCost, int travelCost) {
        allServices.emplace_back(id, type, u, v, demand, serviceCost, travelCost);
    }

    int getDistance(int from, int to) {
        if (from < 0 || to < 0 || static_cast<size_t>(from) >= distances.size() || static_cast<size_t>(to) >= distances.size()) return INF;
        return distances[from][to];
    }
    
    // Calcula o custo exato de uma ÚNICA rota. Função auxiliar crucial.
    long long calculateRouteCost(const vector<Service>& services) {
        if (services.empty()) return 0;
        long long currentCost = 0;
        int lastNode = depot;
        for (const auto& service : services) {
            int travelToServiceCost = getDistance(lastNode, service.u);
            if (travelToServiceCost >= INF) return INF;
            currentCost += travelToServiceCost + service.serviceCost;
            if (service.type != 'N') currentCost += service.travelCost;
            lastNode = service.v;
        }
        int travelToDepotCost = getDistance(lastNode, depot);
        if (travelToDepotCost >= INF) return INF;
        currentCost += travelToDepotCost;
        return currentCost;
    }
    
    // Recalcula todas as informações de uma solução (custos e demandas)
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

    // ====================================================================
    // ETAPA 2: HEURÍSTICA CONSTRUTIVA (Uma Rota Por Serviço)
    // A heurística mais simples e robusta para GARANTIR uma solução inicial completa.
    // ====================================================================
    Solution constructInitialSolution() {
        Solution solution;
        int routeIdCounter = 1;

        for (const auto& service : allServices) {
            // Verifica se o serviço é sequer possível de ser atendido
            if (service.demand > capacity) {
                cerr << "ERRO CRÍTICO: Serviço " << service.id << " tem demanda (" << service.demand
                     << ") maior que a capacidade do veículo (" << capacity << "). Solução inviável." << endl;
                solution.totalCost = INF;
                return solution;
            }
            // Cria uma rota trivial: Depósito -> Serviço -> Depósito
            Route newRoute;
            newRoute.id = routeIdCounter++;
            newRoute.services.push_back(service);
            
            if (calculateRouteCost(newRoute.services) >= INF) {
                cerr << "ERRO CRÍTICO: Serviço " << service.id << " é inalcançável a partir do depósito." << endl;
                solution.totalCost = INF;
                return solution;
            }
            solution.routes.push_back(newRoute);
        }
        
        recalculateSolutionMetrics(solution);
        cout << "LOG: Solução inicial (1 rota/serviço) construída com custo " << solution.totalCost << " e " << solution.routes.size() << " rotas." << endl;
        return solution;
    }

    // ====================================================================
    // ETAPA 3: ALGORITMO DE MELHORIA (Busca Local)
    // Otimiza a solução inicial movendo e fundindo rotas.
    // ====================================================================
    void localSearch(Solution& solution) {
        bool improvement = true;
        while (improvement) {
            improvement = false;
            long long bestCost = solution.totalCost;

            // Vizinhança 1: Tenta juntar rotas
            for (size_t i = 0; i < solution.routes.size(); ++i) {
                for (size_t j = i + 1; j < solution.routes.size(); ++j) {
                    if (solution.routes[i].totalDemand + solution.routes[j].totalDemand <= capacity) {
                        
                        // Testa fundir j no fim de i
                        vector<Service> merged_fwd = solution.routes[i].services;
                        merged_fwd.insert(merged_fwd.end(), solution.routes[j].services.begin(), solution.routes[j].services.end());
                        
                        // Testa fundir i no fim de j
                        vector<Service> merged_rev = solution.routes[j].services;
                        merged_rev.insert(merged_rev.end(), solution.routes[i].services.begin(), solution.routes[i].services.end());

                        long long cost_fwd = calculateRouteCost(merged_fwd);
                        long long cost_rev = calculateRouteCost(merged_rev);
                        
                        long long original_sum_cost = solution.routes[i].totalCost + solution.routes[j].totalCost;

                        if (cost_fwd < original_sum_cost || cost_rev < original_sum_cost) {
                             if(cost_fwd < cost_rev) {
                                solution.routes[i].services = merged_fwd;
                             } else {
                                solution.routes[i].services = merged_rev;
                             }
                             solution.routes.erase(solution.routes.begin() + j);
                             recalculateSolutionMetrics(solution);
                             improvement = true;
                             cout << "LOG: Rotas fundidas! Novo custo: " << solution.totalCost << endl;
                             goto next_iteration;
                        }
                    }
                }
            }
            next_iteration:;
        }
        // Remove rotas que possam ter ficado vazias (não deve acontecer com esta lógica, mas é uma segurança)
        solution.routes.erase(remove_if(solution.routes.begin(), solution.routes.end(), 
            [](const Route& r){ return r.services.empty(); }), solution.routes.end());
        recalculateSolutionMetrics(solution);
    }

    // Função principal que orquestra todo o processo
    Solution solve() {
        auto start = chrono::high_resolution_clock::now();
        
        cout << "LOG: Total de serviços a serem atendidos: " << allServices.size() << endl;
        
        // FASE 1: Constrói uma solução inicial garantidamente viável
        Solution solution = constructInitialSolution();
        
        // FASE 2: Otimiza a solução se ela for viável
        if (solution.totalCost < INF) {
            localSearch(solution);
        } else {
             cout << "ERRO: Não foi possível construir uma solução inicial viável." << endl;
        }

        auto end = chrono::high_resolution_clock::now();
        solution.executionTimeMicroseconds = chrono::duration_cast<chrono::microseconds>(end - start).count();
        return solution;
    }

    // Guarda a solução final no formato especificado
    void saveSolution(const Solution& solution, const string& instanceName) {
        string dirPath = "solucoes";
        try { if (!filesystem::exists(dirPath)) filesystem::create_directories(dirPath); }
        catch (const exception& e) { cerr << "Error creating directory " << dirPath << ": " << e.what() << endl; return; }
        
        string solutionPath = dirPath + "/sol-" + instanceName;
        ofstream out(solutionPath);
        if (!out.is_open()) { cerr << "Error creating solution file at: " << solutionPath << endl; return; }

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
        
        out.close();
        cout << "Solution saved to: " << solutionPath << endl;
        cout << "  - Total Cost: " << solution.totalCost << endl;
        cout << "  - Num Routes: " << solution.routes.size() << endl;
    }
};

#endif
