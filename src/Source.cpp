#include <cstdio>
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

const uint G = 3; // кол-во подграфов в графе
uint* N_I = new uint [G]; // кол-во вершин в подграфах
uint* offset = new uint [G]; // номера вершин, с которой начинаются подграфы
ulong* graphDeg = nullptr; // массив степеней вершин
ulong* counterDistances = nullptr; // массив кол-в путей между вершиными (i - длина пути)
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
	auto dist = new uint [n]; // массив длин путей до i-ой вершины
	auto parent = new uint [n];

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

void barAl(uint subgrNum, uint minConnectionCount, uint maxConnectionCount) {
	/*
	 * modified model Barabási–Albert model
	 *
	 * subgrNum номер текущего подграфа
	 * minConnectionCount минимальное кол-во связей у каждой новой вершины
	 * maxConnectionCount максимальная степень вершины
	*/

	auto n_i = N_I[subgrNum]; // кол-во вершин в I подграфе
	auto offset_i = offset[subgrNum]; // номер вершины, с которой начинается текущий подграф 

	/* создаём и соединяем первые 2 вершины (0 и 1) */
	vector<uint> line0;
	line0.push_back(1 + offset_i);
	graph.push_back(line0);
	line0.pop_back();
	line0.push_back(0 + offset_i);
	graph.push_back(line0);

	graphDeg[0 + offset_i] = 1; // степень 0 вершины
	graphDeg[1 + offset_i] = 1; // степень 1 вершины
	ulong sumDeg = 2; // суммарная степень в подграфе

	// create the adjacency list
	for (auto i = 2 + offset_i; i < n_i + offset_i; i++) // цикл добавления новой (i-ой) вершины (-2, т.к. 2 вершины уже есть)
	{
		vector<uint> line;
		line.reserve(n_i);

		// *(т.к. изначально нельзя присоединиться к больше вершинам, чем их всего в графе)
		auto minCC = min(graph.size() - offset_i, minConnectionCount);

		while (graphDeg[i] < minCC) // цикл перебора существующих вершин для дальнейшего не/присоединения
		{
			auto j = offset_i + rand() % (graph.size() - offset_i); // выбираем случайную вершину

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

void erdosRenyi(uint subgrNum, float p) {
	/*
	* classic model Erdős–Rényi model
	*
	* subgrNum номер текущего подграфа
	* p вероятность соединения
	*/

	auto n_i = N_I[subgrNum]; // кол-во вершин в I подграфе
	auto offset_i = offset[subgrNum]; // номер вершины, с которой начинается текущий подграф 

	p = p * 100;
	for (auto i = 0; i < n_i; i++) // создаем вершины
	{
		vector<uint> line;
		line.reserve(n_i);
		graph.push_back(line);
	}

	// create the adjacency list
	for (auto i = 0 + offset_i; i < n_i + offset_i; i++) {
		for (auto j = i + 1; j < n_i + offset_i; j++) // соединяем вершины случайным образом (с учетом вероятности)
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

		// *(если sumConSubgr == 0 - это первый подграф, он соединится с 0 подграфом с вероятностью 1)
		auto p = sumConSubgr > 0 ? externConSubgr[j] / static_cast<float>(sumConSubgr) : 1; // вероятность присоединения к j-му подгафу 
		auto r = rand() / (static_cast<float>(RAND_MAX) + 1.0);

		if (p > r) // соединяем текущий погграф с j подграфом
		{
			// новая связь соединяет вершину (?с максимальной степенью? ← это затем может быть изменено) G_i со случайной вершиной G_j;
			// *(при равных степенях выбирается первая найденная вершина)
			auto v = offset[j] + rand() % N_I[j]; // выбираем случайную вершину из подграфа G_j
			// выбираем  вершину с максимальной степенью из подграфа G_i
			auto numMaxDegI = distance(graphDeg, max_element(graphDeg + offset[subgrNum], graphDeg + offset[subgrNum] + N_I[subgrNum]));

			// добавляем v к numMaxhDegInSubrg[subgrNum] и numMaxhDegInSubrg[subgrNum] к v вершинам в матрицу смежности
			graph[numMaxDegI].push_back(v);
			graph[v].push_back(numMaxDegI);
			graphDeg[numMaxDegI]++;
			graphDeg[v]++;
			externConSubgr[j]++;
			externConSubgr[subgrNum]++;
			sumConSubgr += 1;
		}
	}
}

int main(int argc, char* argv[]) {
	/*if(argc < 3)
		exit(0);

	auto N_I = atoi(argv[1]);
	auto type = atoi(argv[2]);*/

	srand(time(nullptr));
	auto n = 0; // кол-во вершин в графе

	/* создание G подграфов */
	for (auto i = 0; i < G; i++) {
		auto n_i = 10 + rand() % 10; // кол-во вершин в I подграфе
		N_I[i] = n_i; // кол-во вершин в I подграфе
		offset[i] = n; // номер вершины, с которой начинается I подграф
		n += n_i; // кол-во вершин в графе
		cout << "n_i of " << i << " subgr is " << n_i << endl;
	}

	graphDeg = new ulong[n]; // массив степеней вершин
	counterDistances = new ulong[n - 1]; // массив кол-в путей между вершиными (i - длина пути)
	graph.reserve(n);
	fill_n(counterDistances, n - 1, 0); // заполняем 0
	fill_n(graphDeg, n, 0); // заполняем 0
	fill_n(externConSubgr, G, 0); // заполняем 0

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
		}

		if (i != 0) {
			subgraphConnectBarAl(i, 1); // соединяем подграфы, если это ненулевой подграф
		}

		cout << "The " << i << " subgraph has been grown." << endl;
	}

	cout << endl << "The model has been grown." << endl;

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
        for (i = 1; i < n - 1; i++) 
        {
          if (counterDistances[i] > 0)
            max_exists_len = i;
        }
        //cout << "max_exists_len = " << max_exists_len << endl;
        
	// Среднее
        int lencount = 0;
        double sumlen = 0;
        for (i = 1; i <= max_exists_len; i++) 
        {
          lencount += counterDistances[i];
          sumlen   += counterDistances[i] * i;
        }        
        double zmean = sumlen/lencount;
                
        // Центральные моменты
        double centersum2 = 0;
        double centersum3 = 0;
        double centersum4 = 0;
        for (i = 1; i <= max_exists_len; i++) 
        {
          double i_centered = i - zmean;
          
          centersum2 += counterDistances[i] * i_centered * i_centered;
          centersum3 += counterDistances[i] * i_centered * i_centered * i_centered;
          centersum4 += counterDistances[i] * i_centered * i_centered * i_centered * i_centered;
        }                
        double zstd = sqrt(   centersum2 / (lencount - 1)   );
        double zskewness = centersum3 / (lencount * zstd*zstd*zstd );
        double zkurtosis = centersum4 / (lencount * zstd*zstd*zstd*zstd ) - 3;
#endif        
        	

	/* запись распределения путей от кол-ва в файл */
	out.open("graph_ch.txt");
	for (i = 1; i < n - 1; i++) {
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
	delete[] N_I;
	delete[] offset;
	delete[] externConSubgr;
	delete[] graphDeg;
	delete[] counterDistances;

	return 0;
}
