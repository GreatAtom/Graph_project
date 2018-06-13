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
const int N = 10000; // кол-во вершин в графе
unsigned long sumDeg = 0; // суммарная степень в графе
unsigned long* graphDeg = new unsigned long[N]; // массив степеней вершин
unsigned long* counterDistances = new unsigned long[N - 1]; // массив кол-в путей между вершиными (i - длина пути)
unsigned long infDistance = 0;


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
			if (v > s) // фиксируем путь только от большей вершины к меньшей, чтобы не посчитать один путь дважды (например от 1 к 0 и от 0 к 1)
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

void barAl(int minConnectionCount, int maxConnectionCount) // 
{
	/*
	 * modified model Barabási–Albert model
	 *
	 * minConnectionCount минимальное кол-во связей у каждой новой вершины
	 * maxConnectionCount максимальная степень вершины
	*/

	// create the adjacency list
	for (auto i = 2; i < N; i++) // цикл добавления новой (i-ой) вершины (-2, т.к. 2 вершины уже есть)
	{
		vector<int> line;
		line.reserve(N);

		auto minCC = min(static_cast<int>(graph.size()), minConnectionCount); // т.к. изначально нельзя присоединиться к больше вершинам, чем их всего в графе

		while (graphDeg[i] < minCC) // цикл перебора существующих вершин для дальнейшего не/присоединения
		{
			auto j = rand() % graph.size(); // выбираем случайную вершину

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

void erdosRenyi(float p) {
	/*
	* classic model Erdős–Rényi model
	*
	* p вероятность соединения
	*/

	p = p * 100;
	for (auto i = 0; i < N; i++) // создаем вершины
	{
		vector<int> line;
		line.reserve(N);
		graph.push_back(line);
	}

	// create the adjacency list
	for (auto i = 0; i < N; i++) {
		for (auto j = i + 1; j < N; j++) // соединяем вершины случайным образом (с учетом вероятности)
		{
			float r = rand() % 100;

			if (p > r) {
				graph[j].push_back(i);
				graph[i].push_back(j);
				graphDeg[j]++;
				graphDeg[i]++;
				sumDeg += 2;
			}
		}
	}
}

int main(int argc, char* argv[]) {
	/*if(argc < 3)
		exit(0);

	auto N = atoi(argv[1]);
	auto type = atoi(argv[2]);*/

	srand(time(nullptr));
	graph.reserve(N);

	fill_n(counterDistances, N, 0); // заполняем 0
	fill_n(graphDeg, N, 0); // заполняем 0

	auto type = 1; // 1 - barAl, 2 - erdosRenyi

	// grow graph
	if (type == 1) {
		//auto minConnectionCount = atoi(argv[1]);

		auto minConnectionCount = 2;
		auto maxConnectionCount = 32;

		// создаём и соединяем первые 2 вершины (0 и 1)
		vector<int> line;
		line.push_back(1);
		graph.push_back(line);
		line.pop_back();
		line.push_back(0);
		graph.push_back(line);

		graphDeg[0] = 1; // степень 0 вершины
		graphDeg[1] = 1; // степень 1 вершины
		sumDeg = 2; // суммарная степень всех вершин

		barAl(minConnectionCount, maxConnectionCount);
	} else if (type == 2) {
		// auto p = atof(argv[1]);

		auto p = 0.01;
		erdosRenyi(p);
	} else {
		exit(0);
	}

	cout << "The model has been grown." << endl;

	/* запись графа в файл */
	ofstream out("graph.txt");
	auto k = 0;
	for (auto i = graph.begin(); i != graph.end(); ++i , k++) {
		out << k << ") ";
		for (auto j = (*i).begin(); j != (*i).end(); ++j)
			out << *j << " ";
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
		for (i = 0; i < N - 1; i++) // -1, с. м. функцию обхода графа (например от 1 к 0 и от 0 к 1)
			findShortestPaths(i);
		auto duration = chrono::duration_cast<chrono::minutes>(chrono::steady_clock::now() - start);
		cout << duration.count() << endl;
	}

	/* запись распределения путей от кол-ва в файл */
	out.open("graph_ch.txt");
	for (i = 1; i < N; i++) {
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
