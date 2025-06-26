
# Trabalho Prático – Algoritmos em Grafos (GCC218/GCC262)

**Membros**  
- Marcus Mendonça  
- Gustavo Muniz  

---

## 🧠 Descrição

Este projeto implementa uma solução heurística para o **Problema de Roteamento de Arcos com Capacidade** (_Capacitated Arc Routing Problem - CARP_).  
O objetivo é determinar um conjunto de rotas de **custo mínimo** para uma frota de veículos, partindo de um depósito central, para atender a uma série de **serviços obrigatórios** (em nós, arestas ou arcos) em uma rede, respeitando a **capacidade de cada veículo**.

A solução é desenvolvida em **C++**, utilizando uma abordagem de **meta-heurística** composta por:

- Uma fase **construtiva**
- Uma fase de **melhoria com busca local (VNS)**

---

## 📁 Estrutura de Arquivos

O projeto está modularizado para promover **desacoplamento** e **clareza**:

- `main.cpp`: Ponto de entrada do programa. Responsável pela leitura das instâncias, execução do solver e salvamento das soluções.
- `Solution.hpp`: Define estruturas de dados como `Service`, `Route`, `Solution`.
- `Graph.hpp`: Implementa a estrutura de grafo e o algoritmo de Floyd-Warshall.
- `Solver.hpp`: Contém as heurísticas e algoritmos de busca local.
- `makefile`: Facilita a compilação do projeto.
- `entradas/`: Arquivos de instância.
- `solucoes/`: Soluções geradas.

---

## 🛠️ Metodologia Implementada

### ✅ Etapa 2: Construção da Solução Inicial

Foi implementada uma **Heurística Construtiva Simples** para criar uma solução viável inicial:

- **Lógica**: Para cada serviço obrigatório, cria-se uma rota:

  ```
  Depósito → Serviço → Depósito
  ```

- **Objetivo**: Criar um ponto de partida válido para otimização, ainda que com custo alto.

---

### 🔄 Etapa 3: Otimização com Busca Local (VNS)

Aplicação de **Busca Local com Vizinhança Variável (VNS)** para reduzir o custo:

- **Relocate (Mover)**: Move um serviço de uma rota para outra.
- **Swap (Trocar)**: Troca serviços entre duas rotas.
- **2-opt (Otimização Intra-Rota)**: Inverte uma subsequência de serviços dentro de uma mesma rota.

> Os movimentos são aplicados iterativamente até não haver mais melhorias — atingindo um **ótimo local**.

---

## ⚙️ Como Compilar e Executar

### 🔧 Pré-requisitos

- Compilador C++ (g++)

### 🧱 Compilação

No terminal, execute:

```bash
make
```

Isso gera o executável `solver`.

---

### 🚀 Execução

#### 🔹 Modo 1 – Rodar uma instância específica:

```bash
./solver
```

- Escolha a opção `1`
- Digite o nome do arquivo (ex: `BHW1.txt`)

#### 🔸 Modo 2 – Rodar todas as instâncias da pasta:

```bash
./solver
```

- Escolha a opção `2`
- O programa processará todos os arquivos `.dat` e `.txt` na pasta `entradas/`

---

## 📝 Saídas Geradas

- **Soluções**: Salvas como `sol-<nome_da_instancia>.dat` em `solucoes/`, no formato especificado pelo enunciado.
- **Logs**: O programa imprime no terminal informações sobre:
  - Leitura dos arquivos
  - Início e término da otimização
  - Custo final da solução

---

## ✅ Status

- [x] Fase Construtiva
- [x] Busca Local com VNS
- [x] Suporte a múltiplas instâncias
- [x] Geração de arquivos de solução formatados
- [x] Modularização completa do código

---

## 📌 Observação

Este projeto foi desenvolvido como parte da disciplina de **Algoritmos em Grafos (GCC218/GCC262)**, e tem finalidade **educacional**.

---
