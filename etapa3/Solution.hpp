#ifndef SOLUTION_HPP
#define SOLUTION_HPP

#include <vector>
#include <string>

using namespace std;

/**
 * @struct Service
 * @brief Representa um serviço obrigatório a ser atendido.
 * Pode ser um nó, uma aresta ou um arco requerido.
 */
struct Service {
    int id;
    char type;
    int u, v;
    int demand;
    int serviceCost;
    int travelCost; // Custo de travessia da aresta/arco (0 para nós)

    Service(int _id, char _type, int _u, int _v, int _demand, int _serviceCost, int _travelCost)
        : id(_id), type(_type), u(_u), v(_v), demand(_demand), serviceCost(_serviceCost), travelCost(_travelCost) {}
};

/**
 * @struct Route
 * @brief Representa a rota de um único veículo.
 * Contém uma sequência de serviços, a demanda total e o custo total da rota.
 */
struct Route { 
    int id; 
    vector<Service> services; 
    int totalDemand = 0; 
    long long totalCost = 0; 
};

/**
 * @struct Solution
 * @brief Representa a solução completa para o problema.
 * Contém o custo total, o conjunto de todas as rotas e o tempo de execução.
 */
struct Solution { 
    long long totalCost = 0; 
    vector<Route> routes; 
    long long executionTimeMicroseconds = 0; 
};

#endif