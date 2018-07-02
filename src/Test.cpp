﻿#include <cstdio>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <queue>
#include <ctime>
#include <list>
#include <string>
#include <chrono>
#include <algorithm>

#define INF 2 << 22
#define uint  unsigned int
#define ulong unsigned long

using namespace std;

vector<vector<uint>> graph;
vector<vector<uint>> subGraph;

const uint N = 1000; // кол-во вершин в графе
const uint G = 30; // кол-во подграфов в графе
ulong* sumDeg = new ulong[G]; // суммарная степень вершин в подграфах
ulong* graphDeg = new ulong[N]; // массив степеней вершин
ulong* counterDistances = new ulong[N - 1]; // массив кол-в путей между вершиными (i - длина пути)
ulong* externConSubgr = new ulong[G]; // массив кол-ва внешних связий у подграфов
ulong sumConSubgr = 0; // сумма кол-ва внешних связий у подграфов 
ulong infDistance = 0; // бесконечные пути


uint findShortestPaths(uint s) {
	/*
	* поиск (в ширину) кратчайшего пути от вершины с номером s до остальных вершин
	*
	* s номер вершины, от которой производится поиск
	*/

	auto n = graph.size(); // кол-во вершин в графе
	auto dist = new uint[n]; // массив длин путей до i-ой вершины
	auto parent = new uint[n];

	auto v = 0; // создание для omp
#pragma omp parallel
	{
#pragma omp  for private(v) // изначально расстояния от вершины с номером s до остальных = INF
		for (v = 0; v < n; ++v) {
			dist[v] = INF;
			parent[v] = -1;
		}
	}

	dist[s] = 0;
	queue<uint> Q;
	Q.push(s);

	while (!Q.empty()) // обход в ширину
	{
		uint u = Q.front();
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
		for (v = 0; v < n; ++v) {
			if (v > s) // фиксируем путь только от меньшей вершины к большей, чтобы не посчитать один путь дважды (например от 1 к 0 и от 0 к 1)
			{
				if (dist[v] != INF) {
#pragma omp atomic	
					counterDistances[dist[v]]++;
					//pruint f("%d -> %d: %d\n", s, v, dist[v]);
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

void barAlTest(uint minConnectionCount, uint maxConnectionCount) {
	/*
	* modified model Barabási–Albert model
	* выбирается подграф для присоединения новой вершины, а затем
	* внутри подграфа она присоединяется в соответствии с моделью Barabasi-Albert
	*
	* minConnectionCount минимальное кол-во связей у каждой новой вершины
	* maxConnectionCount максимальная степень вершины
	*/

	/* create the adjacency list */
	for (auto i = G * 2; i < N; i++) // цикл добавления новой (i-ой) вершины (- G * 2 -> уже созданы)
	{
		while (true) // цикл перебора существующих подгафов для дальнейшего не/присоединения
		{
			auto s = rand() % G; // выбираем случайный номер подграфа
			auto p = rand() / (static_cast<float>(RAND_MAX) + 1.0); // вероятность присоединения к s-му подгафу 
			auto r = rand() / (static_cast<float>(RAND_MAX) + 1.0);

			if (p > r) // бросаем текущую вершину в s подграф
			{
				vector<uint> line;
				line.reserve(maxConnectionCount);

				// *(т.к. изначально нельзя присоединиться к больше вершинам, чем их всего в подграфе)
				auto minCC = min(subGraph[s].size(), minConnectionCount);

				while (graphDeg[i] < minCC) // цикл перебора существующих вершин для дальнейшего не/присоединения
				{
					auto pos = rand() % subGraph[s].size(); // выбираем случайную вершину в s подграфе
					auto j = subGraph[s][pos]; // выбираем случайную вершину в s подграфе

					auto alreadyInList = false;
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

					p = graphDeg[j] / static_cast<float>(sumDeg[s]); // вероятность присоединения к j-ой вершине
					r = rand() / (static_cast<float>(RAND_MAX) + 1.0);

					if (p > r) // добавляем вершину
					{
						// добавляем i к j и j к i вершинам в матрицу смежности
						line.push_back(j);
						graph[j].push_back(i);
						graphDeg[j]++;
						graphDeg[i]++;
						sumDeg[s] += 2;
					}
				}
				graph.push_back(line);
				subGraph[s].push_back(i); // бросаем текущую вершину в s подграф
				break;
			}
		}
	}
}

void subgraphConnectBarAl(uint subgrNum, uint connectionCount) {
	/*
	* modified model Barabási–Albert model
	*
	* subgrNum номер текущего подграфа
	* connectionCount кол-во связей у текущего подграфа
	*/

	while (externConSubgr[subgrNum] < connectionCount) // цикл перебора существующих подгафов для дальнейшего не/присоединения
	{
		auto j = rand() % subgrNum; // выбираем случайный номер подграфа

		// *(если sumConSubgr == 0 или externConSubgr[j] == 0  subgrNum подграф соединится с j подграфом с вероятностью 1)
		auto p = sumConSubgr > 0 || externConSubgr[j] == 0 ? externConSubgr[j] / static_cast<float>(sumConSubgr) : 1.; // вероятность присоединения к j-му подгафу 
		auto r = rand() / (static_cast<float>(RAND_MAX) + 1.0);

		if (p > r) // соединяем текущий погграф с j подграфом
		{
			// новая связь соединяет вершину (?с максимальной степенью? ← это затем может быть изменено) G_i со случайной вершиной G_j;
			// *(при равных степенях выбирается первая найденная вершина)
			auto pos = rand() % subGraph[j].size(); // выбираем случайную вершину в G_j подграфе
			auto v = subGraph[j][pos]; // выбираем случайную вершину в s подграфе
			// выбираем  вершину с максимальной степенью из подграфа G_i
			auto maxDeg = 0, posMaxDeg = 0;
			for (auto i = 0; i < subGraph[subgrNum].size(); i++) {
				auto deg = graph[subgrNum][subGraph[subgrNum][i]];
				if (deg > maxDeg) {
					maxDeg = deg;
					posMaxDeg = subGraph[subgrNum][i];
				}
			}

			// добавляем v к numMaxhDegInSubrg[subgrNum] и numMaxhDegInSubrg[subgrNum] к v вершинам в матрицу смежности
			graph[posMaxDeg].push_back(v);
			graph[v].push_back(posMaxDeg);
			graphDeg[posMaxDeg]++;
			graphDeg[v]++;
			externConSubgr[j]++;
			externConSubgr[subgrNum]++;
			sumConSubgr += 1;
		}
	}
}

int main(int argc, char* argv[]) {
	srand(time(nullptr));

	/* инициализация переменных */
	graph.reserve(N);

	fill_n(sumDeg, G, 0); // заполняем 0
	fill_n(graphDeg, N, 0); // заполняем 0
	fill_n(counterDistances, N - 1, 0); // заполняем 0	
	//fill_n(externConSubgr, G, 0); // заполняем 0	

	/* создание G подграфов */
	subGraph.reserve(G);
	for (auto i = 0; i < G * 2; i += 2) {
		vector<uint> line;
		line.push_back(i + 1);
		graph.push_back(line);
		line.pop_back();
		line.push_back(i);
		graph.push_back(line);

		graphDeg[i] = 1; // степень i вершины
		graphDeg[i + 1] = 1; // степень i+1 вершины
		sumDeg[i / 2] = 2; // суммарная степень в подграфе

		line.pop_back();
		line.push_back(i); // по 2 вершины в подграфах (i)
		line.push_back(i + 1); // по 2 вершины в подграфах (i+1)
		subGraph.push_back(line);
	}

	/* рост графа */
	auto minConnectionCount = 2;
	auto maxConnectionCount = 32;
	barAlTest(minConnectionCount, maxConnectionCount);
	for (auto i = 1; i < G; i++) {
		subgraphConnectBarAl(i, 1); // соединяем подграфы, если это ненулевой подграф
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
	cout << "The model has been writen." << endl << endl;

	/* чтение графа из файла */
	/*string line;
	ifstream in("graph.txt");
	while (getline(in, line)) {
	auto pos = line.find_first_of(" ");
	line = line.substr(pos + 1);
	vector<uint> subLine;
	while ((pos = line.find_first_of(" ")) != string::npos) {
	auto p = stoi(line.substr(0, pos));
	subLine.push_back(p);
	line = line.substr(pos + 1);
	}
	graph.push_back(subLine);
	}
	in.close();
	cout << "The model has been read." << endl << endl;*/

	/* обход графа */
	auto start = chrono::steady_clock::now();
	auto i = 0; // создание для omp
#pragma omp parallel shared(counterDistances, infPath)
	{
#pragma omp  for private(i)
		for (i = 0; i < graph.size() - 1; i++) // -1, с. м. функцию обхода графа (например от 1 к 0 и от 0 к 1)
			findShortestPaths(i);
		auto duration = chrono::duration_cast<chrono::minutes>(chrono::steady_clock::now() - start);
		cout << duration.count() << endl;
	}

#define CALC_MOMENTS        
#ifdef CALC_MOMENTS        
	// Расчёт характеристик распределения

	// Максимальная длина
	int max_exists_len = 0;
	for (i = 1; i < N - 1; i++) {
		if (counterDistances[i] > 0)
			max_exists_len = i;
	}
	//cout << "max_exists_len = " << max_exists_len << endl;

	// Среднее
	int lencount = 0;
	double sumlen = 0;
	for (i = 1; i <= max_exists_len; i++) {
		lencount += counterDistances[i];
		sumlen += counterDistances[i] * i;
	}
	double zmean = sumlen / lencount;

	// Центральные моменты
	double centersum2 = 0;
	double centersum3 = 0;
	double centersum4 = 0;
	for (i = 1; i <= max_exists_len; i++) {
		double i_centered = i - zmean;

		centersum2 += counterDistances[i] * i_centered * i_centered;
		centersum3 += counterDistances[i] * i_centered * i_centered * i_centered;
		centersum4 += counterDistances[i] * i_centered * i_centered * i_centered * i_centered;
	}
	double zstd = sqrt(centersum2 / (lencount - 1));
	double zskewness = centersum3 / (lencount * zstd * zstd * zstd);
	double zkurtosis = centersum4 / (lencount * zstd * zstd * zstd * zstd) - 3;
#endif        

	/* запись распределения путей от кол-ва в файл */
	out.open("graph_ch.txt");
	for (i = 1; i < N - 1; i++) {
		if (counterDistances[i] > 0)
			out << i << " " << counterDistances[i] << endl;
	}
	out << "infPath " << infDistance;
#ifdef CALC_MOMENTS        
	out << setprecision(2) << endl << endl << "mean = " << zmean << "\tstd = " << zstd << "\tskewness = " << zskewness << "\tkurtosis-3 = " << zkurtosis << endl;
#endif        
	out.close();
	cout << "The paths have been found." << endl;

#ifdef CALC_MOMENTS        
	cout << setprecision(2) << "mean = " << zmean << "\tstd = " << zstd << "\tskewness = " << zskewness << "\tkurtosis-3 = " << zkurtosis << endl;
#endif   

	delete[] sumDeg;
	delete[] graphDeg;
	delete[] counterDistances;
	delete[] externConSubgr;

	return 0;
}