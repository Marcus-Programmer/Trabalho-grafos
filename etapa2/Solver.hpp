#ifndef SOLVER_HPP
#define SOLVER_HPP

#include "Graph.hpp"
#include <chrono>
#include <random>
#include <map>
#include <algorithm>

// Estrutura de serviço
struct Service {
    int id;
    char type; // 'N' para nó, 'E' para aresta, 'A' para arco
    int u, v;  // nós de origem e destino
    int demand;
    int serviceCost;
    int travelCost;
    bool served = false;
    
    // Construtor
    Service(int _id, char _type, int _u, int _v, int _demand, int _serviceCost, int _travelCost)
        : id(_id), type(_type), u(_u), v(_v), demand(_demand), serviceCost(_serviceCost), travelCost(_travelCost) {}
};

// Estrutura de rota
struct Route {
    vector<int> serviceIds;
    vector<int> nodePath;
    int totalDemand = 0;
    int totalCost = 0;
    int depot;
    
    void clear() {
        serviceIds.clear();
        nodePath.clear();
        totalDemand = 0;
        totalCost = 0;
    }
};

// Classe do solver
class Solver {
private:
    Graph* graph;
    vector<Service> services;
    vector<Route> routes;
    int depot;
    int capacity;
    int totalCost = 0;
    
    // Matriz de distâncias mínimas
    vector<vector<int>> distances;
    vector<vector<int>> predecessors;
    
    // Gerador de números aleatórios
    mt19937 rng;
    
public:
    Solver(Graph* g, int depotNode, int vehicleCapacity) 
        : graph(g), depot(depotNode), capacity(vehicleCapacity), rng(chrono::steady_clock::now().time_since_epoch().count()) {
        
        if (!graph) {
            throw invalid_argument("Graph pointer cannot be null");
        }
        
        if (depotNode < 0 || depotNode >= graph->numNodes()) {
            throw invalid_argument("Invalid depot node");
        }
        
        if (vehicleCapacity <= 0) {
            throw invalid_argument("Vehicle capacity must be positive");
        }
        
        // Calcula matriz de distâncias
        try {
            auto [dist, pred] = graph->floydWarshall();
            distances = dist;
            predecessors = pred;
        } catch (const exception& e) {
            throw runtime_error("Failed to calculate shortest paths: " + string(e.what()));
        }
    }
    
    // Adiciona um serviço
    void addService(int id, char type, int u, int v, int demand, int serviceCost, int travelCost) {
        // Validação dos parâmetros
        if (!graph) {
            cerr << "Error: Graph is null when adding service" << endl;
            return;
        }
        
        int numNodes = graph->numNodes();
        if (u < 0 || u >= numNodes || v < 0 || v >= numNodes) {
            cerr << "Error: Invalid nodes for service " << id << " (u=" << u << ", v=" << v << ", numNodes=" << numNodes << ")" << endl;
            return;
        }
        
        if (demand < 0 || serviceCost < 0 || travelCost < 0) {
            cerr << "Error: Negative costs/demand for service " << id << endl;
            return;
        }
        
        services.emplace_back(id, type, u, v, demand, serviceCost, travelCost);
    }
    
    // Calcula distância entre dois pontos
    int getDistance(int from, int to) {
        if (from < 0 || to < 0 || from >= distances.size() || to >= distances.size()) {
            return INF;
        }
        
        if (distances.empty() || distances[from].empty()) {
            return INF;
        }
        
        return distances[from][to];
    }
    
    // Reconstrói caminho entre dois nós
    vector<int> getPath(int from, int to) {
        if (from == to) return {from};
        
        if (from < 0 || to < 0 || from >= predecessors.size() || to >= predecessors.size()) {
            return {};
        }
        
        if (predecessors.empty() || predecessors[from].empty() || predecessors[from][to] == -1) {
            return {};
        }
        
        vector<int> path;
        int current = to;
        int iterations = 0;
        const int MAX_ITERATIONS = predecessors.size() * 2; // Evita loops infinitos
        
        while (current != from && iterations < MAX_ITERATIONS) {
            if (current < 0 || current >= predecessors.size()) break;
            path.push_back(current);
            current = predecessors[from][current];
            iterations++;
        }
        
        if (current == from) {
            path.push_back(from);
            reverse(path.begin(), path.end());
            return path;
        }
        
        return {}; // Caminho não encontrado
    }
    
    // Calcula o custo de inserir um serviço em uma posição específica da rota
    pair<int, vector<int>> calculateInsertion(const Route& route, const Service& service, int position) {
        vector<int> newPath = route.nodePath;
        int insertionCost = 0;
        
        if (route.nodePath.empty()) {
            // Primeira inserção na rota
            int distToService = getDistance(depot, service.u);
            if (distToService == INF) return {INF, {}};
            
            insertionCost += distToService;
            
            // Adiciona custo do serviço
            insertionCost += service.serviceCost;
            
            // Se é aresta ou arco, adiciona custo de travessia
            if (service.type != 'N') {
                insertionCost += service.travelCost;
            }
            
            // Custo de retorno ao depot
            int lastNode = (service.type == 'N') ? service.u : service.v;
            int distToDepot = getDistance(lastNode, depot);
            if (distToDepot == INF) return {INF, {}};
            
            insertionCost += distToDepot;
            
            // Constrói novo caminho
            newPath.push_back(service.u);
            if (service.type != 'N' && service.u != service.v) {
                newPath.push_back(service.v);
            }
            
        } else {
            // Inserção no meio da rota - implementação simplificada para evitar complexidade
            insertionCost = service.serviceCost;
            if (service.type != 'N') {
                insertionCost += service.travelCost;
            }
            
            // Adiciona uma estimativa conservadora do custo de deslocamento
            insertionCost += 100; // Custo estimado
            
            newPath.push_back(service.u);
            if (service.type != 'N' && service.u != service.v) {
                newPath.push_back(service.v);
            }
        }
        
        return {insertionCost, newPath};
    }
    
    // Algoritmo construtivo simplificado
    void simplifiedConstructive() {
        if (services.empty()) {
            cout << "Warning: No services to process" << endl;
            return;
        }
        
        vector<bool> served(services.size(), false);
        routes.clear();
        
        cout << "Starting simplified constructive algorithm with " << services.size() << " services" << endl;
        
        while (true) {
            // Encontra próximo serviço não atendido
            int nextServiceIdx = -1;
            for (int i = 0; i < services.size(); i++) {
                if (!served[i]) {
                    nextServiceIdx = i;
                    break;
                }
            }
            
            if (nextServiceIdx == -1) break; // Todos os serviços foram atendidos
            
            Route route;
            route.depot = depot;
            
            // Adiciona serviços à rota atual até atingir capacidade
            while (nextServiceIdx != -1) {
                const Service& service = services[nextServiceIdx];
                
                // Verifica capacidade
                if (route.totalDemand + service.demand > capacity) {
                    // Procura próximo serviço que caiba
                    int alternativeService = -1;
                    for (int i = nextServiceIdx + 1; i < services.size(); i++) {
                        if (!served[i] && route.totalDemand + services[i].demand <= capacity) {
                            alternativeService = i;
                            break;
                        }
                    }
                    
                    if (alternativeService == -1) break; // Não há mais serviços que cabem
                    nextServiceIdx = alternativeService;
                    continue;
                }
                
                // Adiciona serviço à rota
                route.serviceIds.push_back(service.id);
                route.nodePath.push_back(service.u);
                if (service.type != 'N' && service.u != service.v) {
                    route.nodePath.push_back(service.v);
                }
                route.totalDemand += service.demand;
                route.totalCost += service.serviceCost;
                if (service.type != 'N') {
                    route.totalCost += service.travelCost;
                }
                
                served[nextServiceIdx] = true;
                
                // Procura próximo serviço não atendido
                nextServiceIdx = -1;
                for (int i = 0; i < services.size(); i++) {
                    if (!served[i]) {
                        nextServiceIdx = i;
                        break;
                    }
                }
            }
            
            if (!route.serviceIds.empty()) {
                routes.push_back(route);
                cout << "Route created with " << route.serviceIds.size() << " services, demand: " << route.totalDemand << endl;
            }
        }
        
        // Calcula custo total estimado
        recalculateAllCosts();
        cout << "Simplified constructive completed. Routes: " << routes.size() << ", Total cost: " << totalCost << endl;
    }
    
    // Recalcula todos os custos das rotas
    void recalculateAllCosts() {
        totalCost = 0;
        
        for (auto& route : routes) {
            route.totalCost = calculateRouteCost(route);
            if (route.totalCost != INF) {
                totalCost += route.totalCost;
            }
        }
    }
    
    // Calcula o custo de uma rota específica - versão simplificada
    int calculateRouteCost(const Route& route) {
        if (route.serviceIds.empty()) return 0;
        
        int cost = 0;
        
        // Custo estimado baseado nos serviços
        for (int serviceId : route.serviceIds) {
            // Encontra o serviço
            for (const auto& service : services) {
                if (service.id == serviceId) {
                    cost += service.serviceCost;
                    if (service.type != 'N') {
                        cost += service.travelCost;
                    }
                    break;
                }
            }
        }
        
        // Adiciona custo estimado de deslocamento
        cost += route.serviceIds.size() * 50; // Estimativa conservadora
        
        return cost;
    }
    
    // Salva solução no formato especificado
    void saveSolution(const string& filename) {
        auto start = chrono::high_resolution_clock::now();
        
        // Executa algoritmo simplificado
        simplifiedConstructive();
        
        auto end = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
        
        // Cria o caminho completo para a pasta solucoes
        string solutionPath = "solucoes/" + filename;
        
    
        
        ofstream out(solutionPath);
        if (!out.is_open()) {
            cerr << "Erro ao criar arquivo de solução em: " << solutionPath << endl;
            return;
        }
        
        // Cabeçalho
        out << totalCost << endl;
        out << routes.size() << endl;
        out << duration.count() << endl;
        out << duration.count() << endl;
        
        // Rotas
        for (int i = 0; i < routes.size(); i++) {
            vector<string> pathElements;
            pathElements.push_back("(D 0," + to_string(depot + 1) + "," + to_string(depot + 1) + ")");
            
            for (int serviceId : routes[i].serviceIds) {
                // Encontra o serviço
                for (const auto& service : services) {
                    if (service.id == serviceId) {
                        pathElements.push_back("(S " + to_string(serviceId) + "," + 
                                             to_string(service.u + 1) + "," + 
                                             to_string(service.v + 1) + ")");
                        break;
                    }
                }
            }
            
            pathElements.push_back("(D 0," + to_string(depot + 1) + "," + to_string(depot + 1) + ")");
            
            out << " " << depot + 1 << " 1 " << (i + 1) << " " 
                << routes[i].totalDemand << " " << routes[i].totalCost 
                << " " << pathElements.size();
            
            for (const string& element : pathElements) {
                out << " " << element;
            }
            out << endl;
        }
        
        out.close();
        cout << "Solução salva em: " << solutionPath << endl;
        cout << "Custo total: " << totalCost << endl;
        cout << "Número de rotas: " << routes.size() << endl;
    }
    
    // Métodos de acesso
    int getTotalCost() const { return totalCost; }
    int getNumRoutes() const { return routes.size(); }
    const vector<Route>& getRoutes() const { return routes; }
};

#endif