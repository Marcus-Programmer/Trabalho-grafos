# Trabalho Prático - Grafos e suas aplicações
### Disciplina GCC262  
**Marcus Mendonça**  
**Gustavo Muniz**  

---

## 🧠 Descrição

Este projeto implementa um grafo com suporte a arestas e arcos (direcionados e não-direcionados), além de funcionalidades para análise de propriedades estruturais do grafo, como número de vértices, grau mínimo/máximo, densidade, componentes conectados, diâmetro, intermediação (betweenness), entre outras.

O objetivo é atender à **Etapa 1** do trabalho prático da disciplina de Grafos (UFLA), realizando a leitura automatizada de dados a partir de arquivos `.dat` e exibindo as principais estatísticas e métricas exigidas, com suporte visual via Graphviz.

---

## 📁 Estrutura

- `Graph.hpp`: Implementação da classe `Graph`, contendo toda a lógica do grafo, algoritmos e métricas.
- `main.cpp`: Programa principal que lê um arquivo `.dat`, constrói o grafo, gera métricas e exporta o grafo em formato visual.
- `README.md`: Instruções do projeto.

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
14. Exportação para arquivo `.dot` e `.png`
15. Visualização da matriz de distâncias

---

## ⚙️ Como compilar e executar

### Pré-requisitos

- Compilador C++
- Graphviz instalado (para gerar o grafo como imagem)

### Como instalar o Graphviz

```bash
sudo apt update
sudo apt install graphviz
```

### Compilação

```bash
g++ main.cpp -o main
```

### Execução

```bash
./main
```

O programa pedirá o nome do arquivo `.dat` (sem o caminho), que estão na pasta `entradas/`.

### Saídas Geradas

- Estatísticas do grafo no terminal.
- Arquivo `grafo.dot` (formato Graphviz).
- Arquivo `grafo.png` (imagem gerada automaticamente).

---

## 📚 Bibliotecas utilizadas

- STL: `<vector>`, `<list>`, `<queue>`, `<limits>`, `<algorithm>`, `<iostream>`, `<iomanip>`, `<fstream>`, `<sstream>`, `<string>`

---