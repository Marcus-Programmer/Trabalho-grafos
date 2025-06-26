#ifndef SOLVER_HPP
#define SOLVER_HPP

#include "Graph.hpp"
#include <chrono>
#include <random>
#include <algorithm>
#include <filesystem>
#include <vector>

// Estruturas de dados (não precisam de alteração)
struct Service {
    int id; char type; int u, v; int demand; int serviceCost; int travelCost;
    Service(int _id, char _type, int _u, int _v, int _demand, int _serviceCost, int _travelCost)
        : id(_id), type(_type), u(_u), v(_v), demand(_demand), serviceCost(_serviceCost), travelCost(_travelCost) {}
};
struct Route { int id; vector<Service> services; int totalDemand = 0; long long totalCost = 0; };
struct Solution { long long totalCost = 0; vector<Route> routes; long long executionTimeMicroseconds = 0; };

// ====================================================================
// A NOVA ABORDAGEM: UMA HEURÍSTICA DE CONSTRUÇÃO E MELHORIA SEPARADAS
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
            throw invalid_argument("Invalid solver parameters.");
        }
        auto [dist, pred] = graph->floydWarshall();
        distances = dist;
    }

    void addService(int id, char type, int u, int v, int demand, int serviceCost, int travelCost) {
        allServices.emplace_back(id, type, u, v, demand, serviceCost, travelCost);
    }

    int getDistance(int from, int to) {
        if (from < 0 || to < 0 || static_cast<size_t>(from) >= distances.size() || static_cast<size_t>(to) >= distances.size()) return INF;
        return distances[from][to];
    }
    
    // Calcula o custo exato de uma ÚNICA rota
    long long calculateRouteCost(const vector<Service>& services) {
        if (services.empty()) return 0;
        long long currentCost = 0;
        int lastNode = depot;
        for (const auto& service : services) {
            int travelToServiceCost = getDistance(lastNode, service.u);
            if (travelToServiceCost >= INF) return INF;
            currentCost += travelToServiceCost;
            currentCost += service.serviceCost;
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
    
    // Estrutura para encontrar o melhor local para inserir um serviço numa ROTA
    struct BestPositionInfo {
        bool possible = false;
        size_t route_idx = -1;
        size_t position_idx = -1;
        long long costDelta = INF; // Variação de custo
        Service orientedService = Service(0,' ',0,0,0,0,0);
    };

    // ====================================================================
    // ETAPA 2: HEURÍSTICA CONSTRUTIVA (Inserção Sequencial Robusta)
    // Garante que todos os serviços são inseridos numa rota válida.
    // ====================================================================
    Solution constructInitialSolution() {
        Solution solution;
        vector<Service> unserved = allServices;
        auto rng = std::default_random_engine(std::chrono::system_clock::now().time_since_epoch().count());
        std::shuffle(unserved.begin(), unserved.end(), rng);

        for(const auto& service : unserved) {
            BestPositionInfo best_insertion;
            best_insertion.costDelta = INF;

            // 1. Tenta inserir numa rota existente
            for (size_t i = 0; i < solution.routes.size(); ++i) {
                if (solution.routes[i].totalDemand + service.demand > capacity) continue;

                long long original_route_cost = solution.routes[i].totalCost;

                for (size_t j = 0; j <= solution.routes[i].services.size(); ++j) {
                    for (int orientation = 0; orientation < (service.type == 'E' ? 2 : 1); ++orientation) {
                        Service oriented_service = service;
                        if(orientation == 1) swap(oriented_service.u, oriented_service.v);
                        
                        vector<Service> test_services = solution.routes[i].services;
                        test_services.insert(test_services.begin() + j, oriented_service);
                        long long new_cost = calculateRouteCost(test_services);

                        if (new_cost < INF) {
                            if (new_cost - original_route_cost < best_insertion.costDelta) {
                                best_insertion.costDelta = new_cost - original_route_cost;
                                best_insertion.route_idx = i;
                                best_insertion.position_idx = j;
                                best_insertion.orientedService = oriented_service;
                            }
                        }
                    }
                }
            }

            // 2. Considera criar uma nova rota
            long long new_route_cost = calculateRouteCost({service});
            if (new_route_cost < INF && new_route_cost < best_insertion.costDelta) {
                 best_insertion.costDelta = new_route_cost;
                 best_insertion.route_idx = solution.routes.size(); // Sinaliza nova rota
                 best_insertion.position_idx = 0;
                 best_insertion.orientedService = service;
            }

            // 3. Executa a melhor inserção encontrada
            if (best_insertion.route_idx != (size_t)-1) {
                if (best_insertion.route_idx == solution.routes.size()) { // Cria nova rota
                    Route new_route;
                    new_route.id = solution.routes.size() + 1;
                    solution.routes.push_back(new_route);
                }
                solution.routes[best_insertion.route_idx].services.insert(solution.routes[best_insertion.route_idx].services.begin() + best_insertion.position_idx, best_insertion.orientedService);
                recalculateSolutionMetrics(solution);
            } else {
                 // Se não for possível inserir, a solução é inviável
                 solution.totalCost = INF;
                 return solution;
            }
        }
        return solution;
    }

    // ====================================================================
    // ETAPA 3: ALGORITMO DE MELHORIA (Busca Local com "Relocate")
    // Pega a solução inicial e tenta melhorá-la.
    // ====================================================================
    void localSearch(Solution& solution) {
        bool improvement = true;
        while (improvement) {
            improvement = false;
            long long bestCost = solution.totalCost;
            if(bestCost >= INF) return;

            // Tenta mover cada serviço para uma nova posição
            for (size_t i = 0; i < solution.routes.size(); ++i) {
                for (size_t j = 0; j < solution.routes[i].services.size(); ++j) {
                    
                    Service serviceToMove = solution.routes[i].services[j];
                    
                    // Guarda o estado original para restaurar se não houver melhoria
                    vector<Service> original_route_i_services = solution.routes[i].services;

                    // Remove o serviço para testar
                    solution.routes[i].services.erase(solution.routes[i].services.begin() + j);

                    BestPositionInfo best_relocation;
                    
                    // Tenta inserir em todas as outras rotas
                    for (size_t k = 0; k < solution.routes.size(); ++k) {
                        if (solution.routes[k].totalDemand + serviceToMove.demand > capacity) continue;
                        
                        long long original_target_cost = calculateRouteCost(solution.routes[k].services);
                        
                        for (size_t l = 0; l <= solution.routes[k].services.size(); ++l) {
                             for (int orientation = 0; orientation < (serviceToMove.type == 'E' ? 2 : 1); ++orientation) {
                                Service orientedService = serviceToMove;
                                if (orientation == 1) swap(orientedService.u, orientedService.v);
                                
                                vector<Service> test_target_services = solution.routes[k].services;
                                test_target_services.insert(test_target_services.begin() + l, orientedService);
                                long long new_target_cost = calculateRouteCost(test_target_services);

                                if(new_target_cost < INF){
                                    long long cost_change = (calculateRouteCost(solution.routes[i].services) - original_route_i_services[j].serviceCost) + (new_target_cost - original_target_cost);
                                    if(cost_change < best_relocation.costDelta){
                                        best_relocation.costDelta = cost_change;
                                        best_relocation.route_idx = k;
                                        best_relocation.position_idx = l;
                                        best_relocation.orientedService = orientedService;
                                    }
                                }
                             }
                        }
                    }

                    // Se encontrou uma melhoria, aplica
                    if(best_relocation.costDelta < 0){
                        solution.routes[best_relocation.route_idx].services.insert(solution.routes[best_relocation.route_idx].services.begin() + best_relocation.position_idx, best_relocation.orientedService);
                        recalculateSolutionMetrics(solution);
                        improvement = true;
                        goto next_iteration;
                    } else {
                        // Restaura
                        solution.routes[i].services = original_route_i_services;
                    }
                }
            }
            next_iteration:;
        }
        // Remove rotas que possam ter ficado vazias
        solution.routes.erase(remove_if(solution.routes.begin(), solution.routes.end(), 
            [](const Route& r){ return r.services.empty(); }), solution.routes.end());
        recalculateSolutionMetrics(solution);
    }

    // Função principal que orquestra tudo
    Solution solve() {
        auto start = chrono::high_resolution_clock::now();
        
        // FASE 1: Constrói
        Solution solution = constructInitialSolution();
        
        // FASE 2: Melhora
        if (solution.totalCost < INF) {
            cout << "Solucao inicial construida com custo " << solution.totalCost << ". Iniciando busca local..." << endl;
            localSearch(solution);
            cout << "Busca local concluida. Custo final: " << solution.totalCost << endl;
        }

        auto end = chrono::high_resolution_clock::now();
        solution.executionTimeMicroseconds = chrono::duration_cast<chrono::microseconds>(end - start).count();
        return solution;
    }

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
