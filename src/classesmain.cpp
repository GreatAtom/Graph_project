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

#include "classesgrow_roulette.h"

#define INF 2 << 22
#define uint  unsigned int
#define ulong unsigned long

using namespace std;

vector<vector<uint>> graph;
ulong* counterDistances = nullptr; // массив кол-в путей между вершиными (i - длина пути)
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




int main(int argc, char* argv[]) {
	/*if(argc < 3)
		exit(0);

	auto N_I = atoi(argv[1]);
	auto type = atoi(argv[2]);*/

	srand(time(nullptr));


	auto n = NodesCount;

	
	counterDistances = new ulong[n - 1]; // массив кол-в путей между вершиными (i - длина пути)
	graph.reserve(n);
	fill_n(counterDistances, n - 1, 0); // заполняем 0
	

	TCurrentModel G;
	
	G.grow(n);
	G.exportToPlain(graph);

	cout << endl << "The model has been grown." << endl;
	
	char graph_filename[1024];
	char graph_paths_filename[1024];
	
	const int uniq_id = time(nullptr);
	sprintf(graph_filename, "graph_n%d_%s_%d_%d_id%d.txt", n, G.title(), minConnectionCount, maxConnectionCount, uniq_id);
	sprintf(graph_paths_filename, "graph_n%d_%s_%d_%d_id%d_ch.txt", n, G.title(), minConnectionCount, maxConnectionCount, uniq_id);
	
	
	/* запись графа в файл */
	ofstream out(graph_filename);
	auto k = 0;
	for (auto j = graph.begin(); j != graph.end(); ++j , k++) {
		out << k << ") ";
		for (auto e = (*j).begin(); e != (*j).end(); ++e)
			out << *e << " ";
		out << endl;
	}
	out.close();
	cout << "The model has been writen." << endl << endl;
	

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
	
	
	distProperties props;
	// Расчёт характеристик распределения
	props.calcProperties(counterDistances, n-1);
	

	
	/* запись распределения путей от кол-ва в файл */
	out.open(graph_paths_filename);
	
	props.printValues(out, counterDistances, n, infDistance);
        out << endl;
	props.printHist(out);
	props.printMoments(out);
	//out.close();
	cout << "The paths have been found." << endl;

	props.printHist(cout);
	cout << "infPath = " << infDistance << " ";
	props.printMoments(cout);
        
	
#ifdef ROOT_TARGET
	fill_n(counterDistances, n - 1, 0); // заполняем 0
	infDistance = 0;
	//out.open("graph_ch.txt", ios_base::ate);
#ifdef CALC_MOMENTS 
	for (k = 0; k < 3; ++k)
	{
		//for (i = 0; i < graph.size() - 1; i++) // -1, с. м. функцию обхода графа (например от 1 к 0 и от 0 к 1)
		//i = rand()  % graph.size();
		i = G.rouletteInvSelect();
		findShortestPaths(i);
		
	
		// Расчёт характеристик распределения
		props.calcProperties(counterDistances, n);
		cout << endl << "root " << i << endl;
		props.printHist(cout);
		cout << "infPath = " << infDistance << " ";
		props.printMoments(cout);

		out << endl << endl << "root " << i << endl;
		
		props.printValues(out, counterDistances, n, infDistance);
		out << endl;
		props.printHist(out);
		props.printMoments(out);
	
	}
	
#endif     	
	out.close();

#endif     	
	
	delete[] counterDistances;

	return 0;
}
