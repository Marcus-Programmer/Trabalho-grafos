# Trabalho Prático - Grafos e suas aplicações
### Disciplina GCC262  
**Marcus Mendonça**  
**Gustavo Muniz**  

---

## 🧠 Descrição

Este projeto implementa um grafo com suporte a arestas e arcos (direcionados e não-direcionados), além de funcionalidades para análise de propriedades estruturais do grafo, como número de vértices, grau mínimo/máximo, densidade, componentes conectados, diâmetro, intermediação (betweenness), entre outras.

O objetivo é atender à **Etapa 1** do trabalho prático da disciplina de Grafos (UFLA), realizando a leitura dos dados do grafo e exibindo as principais estatísticas e métricas exigidas.

---

## 📁 Estrutura

- `Graph.hpp`: Implementação da classe `Graph`, contendo toda a lógica do grafo.
- `main.cpp`: Programa principal que interage com o usuário, lê os dados de entrada e exibe as estatísticas do grafo.

---

## 🧮 Funcionalidades Implementadas (Etapa 1)

1. Número de vértices  
2. Número de arestas (não direcionadas)  
3. Número de arcos (direcionados)  
4. Número de vértices obrigatórios  
5. Número de arestas obrigatórias  
6. Número de arcos obrigatórios  
7. Densidade do grafo  
8. Número de componentes conectados  
9. Grau mínimo dos vértices  
10. Grau máximo dos vértices  
11. Intermediação (Betweenness)  
12. Caminho médio  
13. Diâmetro do grafo  

---

## ⚙️ Como compilar e executar

### Pré-requisitos

- Compilador C++.

### Compilação

```bash
g++ main.cpp -o main
```

### Execução

```bash
./main
```

### Exemplo de entrada (interativa)

```
Digite o número de vértices: 4
Digite o número de arestas (não direcionadas): 2
Digite as arestas no formato: origem destino custo demanda
0 1 3 1
1 2 2 1
Digite o número de arcos (direcionadas): 1
Digite os arcos no formato: origem destino custo demanda
2 3 4 1
```

---

## 📚 Bibliotecas utilizadas

- `<vector>`, `<list>`, `<queue>`, `<limits>`, `<algorithm>`, `<iostream>`, `<iomanip>`  
  Todas são bibliotecas padrão da STL em C++.

---
