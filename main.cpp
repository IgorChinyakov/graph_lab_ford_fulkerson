#include <iostream>
#include <fstream>
#include <climits>
#include <cstring>
#include <queue>

struct Edge {
    int16_t to;           // Конечная вершина ребра
    int16_t capacity;     // Пропускная способность
    int16_t flow;         // Текущий поток
    Edge* reverse;        // Обратное ребро
    Edge* next;           // Следующее ребро в списке смежности
};

// Функция для добавления ребра в граф
void addEdge(Edge** adjList, int16_t from, int16_t to, int16_t capacity) {
    Edge* direct = new Edge{to, capacity, 0, nullptr, nullptr};

    // Добавляем ребро в список смежности для вершины from
    if (adjList[from] == nullptr) {
        adjList[from] = direct;
    } else {
        Edge* temp = adjList[from];
        while (temp->next != nullptr) {
            temp = temp->next;
        }
        temp->next = direct;
    }

    // Добавляем обратное ребро с нулевой пропускной способностью
    Edge* reverse = new Edge{from, 0, 0, direct, nullptr};

    if (adjList[to] == nullptr) {
        adjList[to] = reverse; 
    } else {
        Edge* temp = adjList[to];
        while (temp->next != nullptr) {
            temp = temp->next;
        }
        temp->next = reverse;
    }

    // Связываем прямое и обратное ребра
    direct->reverse = reverse;
}

// Определение истока и стока в графе
void findSourceSink(Edge** adjList, int16_t vertexCount, int16_t& src, int16_t& sink) {
    bool* potentialSource = new bool[vertexCount];
    bool* potentialSink = new bool[vertexCount];

    std::memset(potentialSource, false, vertexCount * sizeof(bool));
    std::memset(potentialSink, false, vertexCount * sizeof(bool));

    // Определяем потенциальные истоки и стоки
    for (int u = 0; u < vertexCount; ++u) {
        for (Edge* edge = adjList[u]; edge != nullptr; edge = edge->next) {
            if (edge->capacity > 0) { 
                potentialSource[u] = true;  
                potentialSink[edge->to] = true;
            }
        }
    }

    // Находим исток
    src = -1;
    for (int i = 0; i < vertexCount; ++i) {
        if (potentialSource[i] && !potentialSink[i]) {
            src = i;
            break;
        }
    }

    // Находим сток
    sink = -1;
    for (int i = 0; i < vertexCount; ++i) {
        if (!potentialSource[i] && potentialSink[i]) {
            sink = i;
            break;
        }
    }

    delete[] potentialSource;
    delete[] potentialSink;
}

// Функция BFS для поиска пути в остаточной сети
bool bfs(int src, int sink, Edge** path, bool* visited, Edge** adjList) {
    std::queue<int> q;
    q.push(src);
    visited[src] = true;
    path[src] = nullptr;

    while (!q.empty()) {
        int u = q.front();
        q.pop();

        for (Edge* edge = adjList[u]; edge != nullptr; edge = edge->next) {
            if (!visited[edge->to] && edge->capacity > edge->flow) {
                q.push(edge->to);
                visited[edge->to] = true;
                path[edge->to] = edge;

                if (edge->to == sink) {
                    return true;
                }
            }
        }
    }
    return false;
}

// Алгоритм Форда-Фалкерсона для нахождения максимального потока
int fordFulkerson(Edge** adjList, int vertexCount, int src, int sink) {
    int maxFlow = 0;
    Edge** path = new Edge*[vertexCount];
    bool* visited = new bool[vertexCount];

    while (true) {
        std::memset(visited, 0, vertexCount * sizeof(bool));
        if (!bfs(src, sink, path, visited, adjList)) break;

        int flow = INT_MAX;
        for (int v = sink; v != src; v = path[v]->reverse->to) {
            flow = std::min(flow, path[v]->capacity - path[v]->flow);
        }

        for (int v = sink; v != src; v = path[v]->reverse->to) {
            path[v]->flow += flow;
            path[v]->reverse->flow -= flow;
        }

        maxFlow += flow;
    }

    delete[] path;
    delete[] visited;
    return maxFlow;
}

// Вывод информации о потоке
void outputFlow(std::ostream& out, Edge** adjList, int vertexCount, int maxFlow, int16_t src, int16_t sink) {
    out << "Max Flow from " << src << " to " << sink << ": " << maxFlow << '\n';

    for (int u = 0; u < vertexCount; ++u) {
        for (Edge* edge = adjList[u]; edge != nullptr; edge = edge->next) {
            if (edge->flow > 0) {
                out << u << " -> " << edge->to << " | Flow: " << edge->flow << '\n';
            }
        }
    }
}

// Освобождение памяти, занятой списком смежности
void freeAdjList(Edge** adjList, int16_t vertexCount) {
    for (int i = 0; i < vertexCount; ++i) {
        Edge* edge = adjList[i];
        while (edge) {
            Edge* temp = edge;
            edge = edge->next;
            delete temp;
        }
    }
    delete[] adjList;
}

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "Russian");
    if (argc < 2) {
        std::cerr << "Укажите имя входного файла!" << std::endl;
        return 1;
    }

    std::string inputFile = argv[1];
    std::string outputFile = "output.txt";

    std::ofstream out(outputFile);
    if (!out.is_open()) {
        std::cerr << "Ошибка открытия выходного файла!" << std::endl;
        return 1;
    }

    std::ifstream input(inputFile, std::ios::binary);
    if (!input) {
        std::cerr << "Ошибка! Не удалось открыть файл." << std::endl;
        return 1;
    }

    int16_t vertexCount;
    input.read(reinterpret_cast<char*>(&vertexCount), sizeof(int16_t));

    Edge** adjList = new Edge*[vertexCount];
    std::memset(adjList, 0, vertexCount * sizeof(Edge*));

    int16_t from, to, capacity;
    while (input.read(reinterpret_cast<char*>(&from), sizeof(int16_t)) &&
           input.read(reinterpret_cast<char*>(&to), sizeof(int16_t)) &&
           input.read(reinterpret_cast<char*>(&capacity), sizeof(int16_t))) {
        addEdge(adjList, from, to, capacity);
    }
    input.close();

    int16_t src, sink;
    findSourceSink(adjList, vertexCount, src, sink);

    if (src == -1 || sink == -1) {
        std::cerr << "Ошибка: Не удалось определить исток или сток." << std::endl;
        freeAdjList(adjList, vertexCount);
        return 1;
    }

    int maxFlow = fordFulkerson(adjList, vertexCount, src, sink);
    outputFlow(out, adjList, vertexCount, maxFlow, src, sink);

    out.close();
    freeAdjList(adjList, vertexCount);

    return 0;
}
