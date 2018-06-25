#include <cstdio>
#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <ctime>
#include <list>
#include <string>
#include <chrono>

#define INF 2 << 22

using namespace std;

vector<vector<int>> graph;

const int G = 3; // кол-во подграфов в графе
const int N_I = 10; // кол-во вершин в I подграфе
const int N = G * N_I; // кол-во вершин в графе
unsigned long* graphDeg = new unsigned long[N]; // массив степеней вершин
unsigned long* counterDistances = new unsigned long[N - 1]; // массив кол-в путей между вершиными (i - длина пути)
unsigned long infDistance = 0; // бесконечные пути


int findShortestPaths(int s) {
	/*
	* поиск (в ширину) кратчайшего пути от вершины с номером s до остальных вершин
	*
	* s номер вершины, от которой производится поиск
	*/

	auto* dist = new int[N]; // массив длин путей до i-ой вершины
	auto* parent = new int[N];

	auto v = 0;
#pragma omp parallel
	{
#pragma omp  for private(v) // изначально расстояния от вершины с номером s до остальных = INF
		for (v = 0; v < N; ++v) {
			dist[v] = INF;
			parent[v] = -1;
		}
	}

	dist[s] = 0;
	queue<int> Q;
	Q.push(s);

	while (!Q.empty()) // обход в ширину
	{
		int u = Q.front();
		Q.pop();

		for (auto it = graph[u].begin(); it != graph[u].end(); ++it) {
			if (dist[*it] == INF) {
				Q.push(*it);
				dist[*it] = dist[u] + 1;
				parent[*it] = u;
			}
		}
	}

#pragma omp parallel shared(counterDistances, infPath) 
	{
#pragma omp  for private(v) // перенос дистанций от вершины с номером s в общий массив или перем. infDistance (нету пути до вершины)
		for (v = 0; v < N; ++v) {
			if (v > s) // фиксируем путь только от меньшей вершины к большей, чтобы не посчитать один путь дважды (например от 1 к 0 и от 0 к 1)
			{
				if (dist[v] != INF) {
#pragma omp atomic	
					counterDistances[dist[v]]++;
					//printf("%d -> %d: %d\n", s, v, dist[v]);
				} else {
#pragma omp atomic	
					infDistance++;
				}
			}
		}
	}

	delete[] dist;
	delete[] parent;

	return 0;
}

void barAl(int subgrNum, int minConnectionCount, int maxConnectionCount) {
	/*
	 * modified model Barabási–Albert model
	 *
	 * subgrNum номер подграфа
	 * minConnectionCount минимальное кол-во связей у каждой новой вершины
	 * maxConnectionCount максимальная степень вершины
	*/

	auto offset = subgrNum * N_I; // номер вершины, с которой начинается текущий подграф 

	/* создаём и соединяем первые 2 вершины (0 и 1) */
	vector<int> line0;
	line0.push_back(1 + offset);
	graph.push_back(line0);
	line0.pop_back();
	line0.push_back(0 + offset);
	graph.push_back(line0);

	graphDeg[0 + offset] = 1; // степень 0 вершины
	graphDeg[1 + offset] = 1; // степень 1 вершины
	unsigned long sumDeg = 2; // суммарная степень в подграфе

	// create the adjacency list
	for (auto i = 2 + offset; i < N_I + offset; i++) // цикл добавления новой (i-ой) вершины (-2, т.к. 2 вершины уже есть)
	{
		vector<int> line;
		line.reserve(N_I);

		auto minCC = min(static_cast<int>(graph.size() - offset), minConnectionCount); // т.к. изначально нельзя присоединиться к больше вершинам, чем их всего в графе

		while (graphDeg[i] < minCC) // цикл перебора существующих вершин для дальнейшего не/присоединения
		{
			auto j = offset + rand() % (graph.size() - offset); // выбираем случайную вершину

			bool alreadyInList = false;
			for (auto k = 0; k < line.size(); k++) // проверка на отсутствие связи с этой вершиной
			{
				if (line[k] == j) {
					alreadyInList = true;
					break;
				}
			}

			if (alreadyInList || graphDeg[j] == maxConnectionCount) // уже соединены с этой вершиной или у нее достигнут предел степени
			{
				continue;
			}

			auto p = graphDeg[j] / static_cast<float>(sumDeg); // вероятность присоединения к j-ой вершине
			auto r = rand() / (static_cast<float>(RAND_MAX) + 1.0);

			if (p > r) // добавляем вершину
			{
				// добавляем i к j и j к i вершинам в матрицу смежности
				line.push_back(j);
				graph[j].push_back(i);
				graphDeg[j]++;
				graphDeg[i]++;
				sumDeg += 2;
			}
		}
		graph.push_back(line);
	}
}

void erdosRenyi(int subgrNum, float p) {
	/*
	* classic model Erdős–Rényi model
	*
	* subgrNum номер подграфа
	* p вероятность соединения
	*/

	auto offset = subgrNum * N_I; // номер вершины, с которой начинается текущий подграф 

	p = p * 100;
	for (auto i = 0; i < N_I; i++) // создаем вершины
	{
		vector<int> line;
		line.reserve(N_I);
		graph.push_back(line);
	}

	// create the adjacency list
	for (auto i = 0 + offset; i < N_I + offset; i++) {
		for (auto j = i + 1; j < N_I + offset; j++) // соединяем вершины случайным образом (с учетом вероятности)
		{
			float r = rand() % 100;

			if (p > r) {
				graph[j].push_back(i);
				graph[i].push_back(j);
				graphDeg[j]++;
				graphDeg[i]++;
			}
		}
	}
}

int main(int argc, char* argv[]) {
	/*if(argc < 3)
		exit(0);

	auto N_I = atoi(argv[1]);
	auto type = atoi(argv[2]);*/

	srand(time(nullptr));

	/* создание G подграфов */
	graph.reserve(N);
	fill_n(counterDistances, N, 0); // заполняем 0
	fill_n(graphDeg, N, 0); // заполняем 0

	for (auto i = 0; i < G; i++) {
		auto type = 1; // 1 - barAl, 2 - erdosRenyi

		/* рост подграфа */
		if (type == 1) { // TODO для каждого подграфа своя модель
			//auto minConnectionCount = atoi(argv[1]);

			auto minConnectionCount = 2;
			auto maxConnectionCount = 32;
			barAl(i, minConnectionCount, maxConnectionCount);
		} else if (type == 2) {
			// auto p = atof(argv[1]);

			auto p = 0.1;
			erdosRenyi(i, p);
		} else {
			exit(0);
		}

		cout << "The " << i << " subgraph has been grown." << endl;
	}

	cout << "The model has been grown." << endl;

	/* запись графа в файл */
	ofstream out("graph.txt");
	auto k = 0;
	for (auto j = graph.begin(); j != graph.end(); ++j , k++) {
		out << k << ") ";
		for (auto e = (*j).begin(); e != (*j).end(); ++e)
			out << *e << " ";
		out << endl;
	}
	out.close();
	cout << "The model has been writen." << endl;


	/* чтение графа из файла */
	string line;
	ifstream in("graph.txt");
	while (getline(in, line)) {
		auto pos = line.find_first_of(" ");
		line = line.substr(pos + 1);
		vector<int> subLine;
		while ((pos = line.find_first_of(" ")) != string::npos) {
			auto p = stoi(line.substr(0, pos));
			subLine.push_back(p);
			line = line.substr(pos + 1);
		}
		graph.push_back(subLine);
	}
	in.close();
	cout << "The model has been read." << endl;

	/* обход графа */
	auto start = chrono::steady_clock::now();
	auto i = 0; // создание для omp
#pragma omp parallel shared(counterDistances, infPath)
	{
#pragma omp  for private(i)
		for (i = 0; i < N_I - 1; i++) // -1, с. м. функцию обхода графа (например от 1 к 0 и от 0 к 1)
			findShortestPaths(i);
		auto duration = chrono::duration_cast<chrono::minutes>(chrono::steady_clock::now() - start);
		cout << duration.count() << endl;
	}

	/* запись распределения путей от кол-ва в файл */
	out.open("graph_ch.txt");
	for (i = 1; i < N_I; i++) {
		if (counterDistances[i] > 0)
			out << i << " " << counterDistances[i] << endl;
	}
	out << "infPath " << infDistance;
	out.close();
	cout << "The paths have been found." << endl;

	delete[] counterDistances;
	delete[] graphDeg;

	return 0;
}
