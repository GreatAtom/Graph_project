#include "common.h"

// Растущая сеть
class TGrowingNetwork
{
protected:
	vector<vector<TVertexNumber>> adjMatrix;
	vector<long> nodesDeg;
	unsigned long sumDeg;

	
	bool isConnected(TVertexNumber i, TVertexNumber j)
	{
		int i_deg = adjMatrix[i].size();
		for (auto k = 0; k < i_deg; k++) // проверка на отсутствие связи с этой вершиной
		{
			if (adjMatrix[i][k] == j) {
				return true;
			}
		}
		
		return false;
	}

	void connect(TVertexNumber i, TVertexNumber j)
	{
		// добавляем i к j и j к i вершинам в матрицу смежности
		adjMatrix[i].push_back(j);
		adjMatrix[j].push_back(i);
		//updatenodesDegAfterConnect(i, j);
	}
	
	
	virtual int tryConnect(TVertexNumber i)
	{
		auto j = rouletteSelect();
		
		if (isConnected(i, j) || adjMatrix[j].size() >= maxConnectionCount) // уже соединены с этой вершиной или у нее достигнут предел степени
		{
			return -1;
		}
		
		connect(i, j);		
		
		return j;
	}
	
	//virtual void doAfterAddNode(int i)
	//{
	//}
	
	
public:
	virtual const char* title()
	{
		return "ba";
	}
	
	// Вектор векторов смежности ************************************************
	const vector<vector<TVertexNumber>>& AdjacencyMatrix() const
	{
		return adjMatrix;
	}

	// Создание пустого графа ***************************************************
	TGrowingNetwork()
	{
		sumDeg = 0;
	}

	// Число вершин *************************************************************
	virtual int nodesCount() const
	{
		return adjMatrix.size();
	}


	// Отладочная печать *******************************************************
	void print(const char *title, int LogLevel = 0, char newln = '\n')  const
	{
		cout << title << ": nodes = " << nodesCount() << "; sumDeg = " << sumDeg << newln;

		if (LogLevel < 1)
			return;

		int vCount = adjMatrix.size(); // не nodesCount(), если реализована модель с удалением
		for(int v = 0; v < vCount; ++v)
		{
			cout << v << ": ";

			int iCount = adjMatrix[v].size();
			for(int i = 0; i < iCount; ++i)
			{
				cout << adjMatrix[v][i] << " ";
			};
			cout << newln;

		}; // цикл по вершинам

	}  // print



	// Затравка *****************************************************************
	virtual int init()
	{
		/* создаём и соединяем первые 2 вершины (0 и 1) */
		vector<TVertexNumber> line0;
		line0.push_back(1);
		adjMatrix.push_back(line0);
		line0.pop_back();
		line0.push_back(0);
		adjMatrix.push_back(line0);
		
		//updatenodesDegAfterConnect(0, 1)

		// Две начальные вершины равноценны при любом алгоритме выращивания
		nodesDeg.push_back(1); // степень 0 вершины
		nodesDeg.push_back(1); // степень 1 вершины
		
		sumDeg = 2; // суммарная степень в подграфе    
		
		return nodesCount();
	}


	// Выбор по БА **************************************************************
	int rouletteSelect() const
	{
		double rouletteMax = sumDeg;
		double rouletteValue = rand() * rouletteMax / (RAND_MAX + 1.0);
		
		//cout << "rouletteMax = " << rouletteMax << " rouletteValue = " << rouletteValue << endl;

		int imax = nodesDeg.size();
		int rouletteSector = 0;
		for(int i = 0; i < imax; ++i)
		{
			rouletteSector += nodesDeg[i];
			if (rouletteValue < rouletteSector)
			{
				return i;
			}      
		}    
		return imax-1; // а сюда мы не дойдём
	} // rouletteSelect  
	
	// Обратный выбор — чем меньше степень, тем больше вероятность ********
	int rouletteInvSelect() const
	{
		int imax = nodesDeg.size();

		double rouletteMax = 0;
		for(int i = 0; i < imax; ++i)
		{
			if (nodesDeg[i] > 0)
				rouletteMax += 100.0/nodesDeg[i];
  
		}   		
		
		double rouletteValue = rand() * rouletteMax / (RAND_MAX + 1.0);
		
		int rouletteSector = 0;
		for(int i = 0; i < imax; ++i)
		{
			if (nodesDeg[i] > 0)
			{
				rouletteSector += 100.0/nodesDeg[i];
				if (rouletteValue < rouletteSector)
				{
					while (nodesDeg[i] <= 0)
						++i;
					return i;
				}      
			}
		}    
		return imax-1; // а сюда мы не дойдём
	} // rouletteInvSelect  	
	
	// Выбор оконечного узла (пользователя, поэтому, вероятно, с минимальной степенью)
	virtual int selectEndNode() const
	{
		return rouletteInvSelect();
	}


	// Добавление вершины к графу *****************************************
	void addNode()
	{
		// *(т.к. изначально нельзя присоединиться к больше вершинам, чем их всего в графе)
		auto minCC = min(static_cast<int>(adjMatrix.size()), minConnectionCount);

		vector<TVertexNumber> line;
		int i = adjMatrix.size();
		//cout << " addNode " << i;

		//line.reserve(maxConnectionCount);
		adjMatrix.push_back(line);

		while (adjMatrix[i].size() < minCC) // цикл перебора существующих вершин для дальнейшего не/присоединения
		{
			auto j = tryConnect(i);		
		}
		
		// Обновление массива степеней
		int i_deg = adjMatrix[i].size();
		nodesDeg.push_back(i_deg);
		sumDeg += i_deg;		
		for (auto k = 0; k < i_deg; k++) 
		{
			int j = adjMatrix[i][k];
			nodesDeg[j]++;
			++sumDeg;
		}		
		//cout << " /addNode " << i;
		//doAfterAddNode(i);
	}
	
	// Рост графа
	virtual int grow(int maxNodes)
	{
		init();
		while (nodesCount() < maxNodes)
			addNode();

	}
	// Упаковка выращенного графа в массив для расчёта длин *********************
	int exportToPlain(vector<vector<TVertexNumber>> &graph) const
	{
		graph = adjMatrix;
	}

}; // TGrowingNetwork

// template<int minConnectionCount, int maxConnectionCount>
class TGrowingNetworkBA: public TGrowingNetwork
// <minConnectionCount, maxConnectionCount>
{
public:
	virtual const char* title()
	{
		return "ba";
	}
	
}; // TGrowingNetworkBA

// typedef TGrowingNetwork TGrowingNetworkBA;

class TGrowingNetworkLeafs: public TGrowingNetwork
{
public:
	virtual const char* title()
	{
		return "l";
	}	
protected:
	
	virtual int tryConnect(TVertexNumber i)
	{
		int j;
		if( rand()%2 > 0)
			j = rouletteSelect();
		else
			j = rouletteInvSelect();
		
		while (isConnected(i, j) || adjMatrix[j].size() >= maxConnectionCount) // уже соединены с этой вершиной или у нее достигнут предел степени
		{
			j = rouletteInvSelect();
		}
		
		// добавляем i к j и j к i вершинам в матрицу смежности
		adjMatrix[i].push_back(j);
		adjMatrix[j].push_back(i);		
		
		return j;
	}

	
}; // TGrowingNetworkLeafs


class TNetworkWithDelete: public 
TGrowingNetworkBA
// TGrowingNetworkLeafs
{
public:
	virtual const char* title()
	{
		return "del";
	}	vector<TVertexNumber> deadNodes;
	
protected:	
	// Удаление узла
	void deleteNode()
	{
		//int i = rand() % adjMatrix.size();
		int i = rouletteInvSelect();
		//int i = rouletteSelect();
		if (0 == nodesDeg[i])
			return;
		
		deadNodes.push_back(i);
		nodesDeg[i] = 0;
		
		int i_deg = adjMatrix[i].size();	
		
		
		
		int i_inheritor =
		adjMatrix[i][ rand() % i_deg ];
// 		adjMatrix[i][0]; 
// 		for (auto k = 1; k < i_deg; ++k) 
// 		{
// 			int j = adjMatrix[i][k];
// 			
// 			if (nodesDeg[j] > nodesDeg[i_inheritor])
// 				i_inheritor = j;
// 		}	
		
		
		//cout << " deleteNode " << i << " inheritor " << i_inheritor;
		//print("", 1);
		
		// Передача связей i → i_inheritor
		for (auto k = i_deg-1; k >= 0; --k) 
		{
			int j = adjMatrix[i][k];
			adjMatrix[i].pop_back();
			vector<TVertexNumber> &adjCurrent = adjMatrix[j];
			
			if (j != i_inheritor)
			{
				replace_if(adjCurrent.begin(), adjCurrent.end(), bind2nd(equal_to<TVertexNumber>(), i), i_inheritor);		
				adjMatrix[i_inheritor].push_back(j);
			}
			else
			{
				adjCurrent.erase(remove(adjCurrent.begin(), adjCurrent.end(), i), adjCurrent.end()); 				
			}
		}	
		sumDeg -= 2;

		//print("", 1);
		
	}
	

	
	void reanimateNode()
	{
		// *(т.к. изначально нельзя присоединиться к больше вершинам, чем их всего в графе)
		auto minCC = min(static_cast<int>(nodesCount()), minConnectionCount);

		int i = deadNodes[ deadNodes.size()-1 ];
		deadNodes.pop_back();
		//cout << " reanimateNode " << i;
		
		while (adjMatrix[i].size() < minCC) // цикл перебора существующих вершин для дальнейшего не/присоединения
		{
			auto j = tryConnect(i);		
		}
		
		// Обновление массива степеней
		int i_deg = adjMatrix[i].size();
		nodesDeg[i] = i_deg;
		sumDeg += i_deg;		
		for (auto k = 0; k < i_deg; k++) 
		{
			int j = adjMatrix[i][k];
			nodesDeg[j]++;
			++sumDeg;
		}				
	}
	


public:	
	// Число вершин *************************************************************
	virtual int nodesCount() const
	{
		return adjMatrix.size() - deadNodes.size();
	}	
	
	virtual int grow(int maxNodes)
	{
		init();
		while (nodesCount() < startDel)
			addNode();
		
		while (nodesCount() < maxNodes)
		{
			if (rand() / (RAND_MAX + 1.0) < pDel)
				deleteNode();
			else
			//if (rand() / (RAND_MAX + 1.0) < pAdd)
				if (deadNodes.size() > 0)
					reanimateNode();
				else
					addNode();
		}

	}	
};






















// typedef TGrowingNetworkBA TSolidGraph;

// Подграф как элемент составного графа
class TSolidGraph: public TGrowingNetworkBA
{
protected:

public:

	// Исходящая вершина для присоединения этого графа к другому ****************
	int selectVertexFrom() const
	{
		return rouletteSelect();

	}

	// Входящая вершина для присоединения другого графа к данному ***************
	int selectVertexTo() const
	{
		return rouletteInvSelect();
	}
};

// Составной граф
// template<int minConnectionCount, int maxConnectionCount>
class TCombinedGraph: TGrowingNetworkBA
{
public:
	virtual const char* title()
	{
		return "pf";
	}	
	
  // Внешняя связь
  struct TConnectionTo
  {
    int localVertex;    // вершина текущего подграфа
    int farGraph;
    int farVertex;
    
    TConnectionTo(int lv, int fg, int fv)
    {
      localVertex = lv;
      farGraph = fg;
      farVertex = fv;
    }
  };
private:
	vector<vector<TVertexNumber>> *pexportedGraph;
	
protected:
  vector<TSolidGraph> subGraphs;
  int totalVerticesCount;
  
  vector<vector<TConnectionTo>> externgraph;

public:
  TCombinedGraph()
  {
    totalVerticesCount = 0;
    pexportedGraph = NULL;
  }
  
  // Общее число вершин во всех подграфах *************************************
  int verticesCount() const
  {
    return totalVerticesCount;
  }
  
  // Число подграфов **********************************************************
  int subGrapsCount() const
  {
    return subGraphs.size();
  }
  
  
  // Отладочная печать ********************************************************
  void print(const char *title, int LogLevel = 0, char newln = '\n') const
  {
    int graphsCount = subGrapsCount(), g = 0;
    
    cout <<title << ": subGrapsCount = " << graphsCount << "; vert. count = " << totalVerticesCount << newln;
   
    if (LogLevel < 1)
      return;
    for(g = 0; g < graphsCount; ++g)
    {
      subGraphs[g].print("^", LogLevel);
    };  
   }  // print

  
  // Добавление нового подграфа ***********************************************
  void addSubGraph()
  {
    TSolidGraph g;
    int gVerticesCount = g.init();
    
    
    subGraphs.push_back(g);
    totalVerticesCount += gVerticesCount;
    
    addNode();
    
    g.print("TCombinedGraph::addSubGraph", 0);
  }
  
// 	virtual void doAfterAddNode(int i)
// 	{
// 		addSubGraph();
// 	}
	
  // Добавление одной вершины к составному графу *****************************
  // вероятность выбора подграфа G_i пропорциональна числу вершин G_i
  void addVertex()
  {
    //print("TCombinedGraph::addVertex start", 0);

    // Вероятность образования нового подграфа (ненормированная!)
    int subGraphCreationWeight = 2 ; //verticesCount() / 4 + 2;  
    
    int rouletteMax = verticesCount() + subGraphCreationWeight;
    int rouletteValue = rand() % rouletteMax;

    int imax = subGrapsCount();
    int rouletteSector = 0;
    for(int i = 0; i < imax; ++i)
    {
      rouletteSector += subGraphs[i].nodesCount();   // (*)
      if (rouletteValue < rouletteSector)
      {
        subGraphs[i].addNode();
        ++totalVerticesCount;
        //return 1;
        return;
      }      
    }

    addSubGraph();    
  } // addVertex
  
  
  // Соединение двух подграфов
  void connectSubGraphs(int g1, int v1, int g2, int v2)
  {
    externgraph[g1].push_back(TConnectionTo(v1, g2, v2));
    externgraph[g2].push_back(TConnectionTo(v2, g1, v1));
    
     //subGraphs[g1].print("connect >", 0, '\t');
    // subGraphs[g2].print("connect <");
  } // connectSubGraphs
  
  
  // Выбор подграфа для присоединения к iSubGraphFrom
  int selectSubGraphTo(int iSubGraphFrom) const
  {
    /* из всех, кроме iSubGraphFrom
    int rouletteMax = verticesCount() - subGraphs[iSubGraphFrom].verticesCount();
    int rouletteValue = rand() % rouletteMax;

    int imax = subGrapsCount();
    int rouletteSector = 0;
    for(int i = 0; i < imax; ++i)
    {
      if (i==iSubGraphFrom)
        continue;
      rouletteSector += subGraphs[i].verticesCount();
      if (rouletteValue < rouletteSector)
      {
        return i;
      }      
    }    
    */
    
    // из более старых, чем iSubGraphFrom
    
    int imax = iSubGraphFrom;
    
    int rouletteMax = 0;
    for(int i = 0; i < imax; ++i)
    {
      rouletteMax += subGraphs[i].nodesCount();
    }     
    
    int rouletteValue = rand() % rouletteMax;

    int rouletteSector = 0;
    for(int i = 0; i < imax; ++i)
    {
      rouletteSector += subGraphs[i].nodesCount();
      if (rouletteValue < rouletteSector)
      {
        return i;
      }      
    }
    return imax-1;
  } // selectSubGraphTo
  
  
  // Рост графа
  int grow(int maxVertices)
  {
    print("TCombinedGraph::grow start", 0);

    init();
    // Рост подграфов
    addSubGraph();
    while (verticesCount() < maxVertices)
      addVertex();
    
    print("TCombinedGraph::grow ex", 0);
    
    // Соединение подграфов
    int imax = subGrapsCount();
    
    // Инициализация списка связей для всех подграфов
    vector<TConnectionTo> empty;
    for(int i = 0; i < imax; ++i)
    {
      externgraph.push_back(empty);
    };    
    
    // Заполнение списка связей
    for(int i = 1; i < imax; ++i)
    {
      int j = selectSubGraphTo(i);      
      connectSubGraphs(i, subGraphs[i].selectVertexFrom(), j, subGraphs[j].selectVertexTo());   
    };
    
    print("TCombinedGraph::grow end", 0);
  }
  
  
  // Упаковка выращенного графа в массив для расчёта длин *********************
  int exportToPlain(vector<vector<TVertexNumber>> &graph) 
  {
    int graphsCount = subGrapsCount(), g = 0;
    int currentSubGraphShift = 0;    
    vector<int> subGraphShift;
    
    // 
    for(g = 0; g < graphsCount; ++g)
    {
      subGraphShift.push_back(currentSubGraphShift);
      currentSubGraphShift += subGraphs[g].nodesCount();
    };   
    
    graph.clear();
    
    for(g = 0; g < graphsCount; ++g)
    {
      currentSubGraphShift = subGraphShift[g];
      
      int vCount = subGraphs[g].nodesCount();
      for(int v = 0; v < vCount; ++v)
      {
        vector<TVertexNumber> vAdjacentGlobal;
        vector<TVertexNumber> vAdjacentLocal = subGraphs[g].AdjacencyMatrix()[v];

        int imax = vAdjacentLocal.size();
        for(int i = 0; i < imax; ++i)
        {
          vAdjacentGlobal.push_back(vAdjacentLocal[i] + currentSubGraphShift);
        }
        
        graph.push_back(vAdjacentGlobal);
        
      }; // цикл по вершинам подграфа g 
      
      int exCount = externgraph[g].size(); // количество внешних связей g
      for(int ex = 0; ex < exCount; ++ex)
      {
        int lv = externgraph[g][ex].localVertex + subGraphShift[g];
        int fg = externgraph[g][ex].farGraph;
        int fv = externgraph[g][ex].farVertex + subGraphShift[fg];
        
        graph[lv].push_back(fv);              
      }; // цикл по внешним связям подграфа g       
    
    }; // цикл по подграфам     
    
    pexportedGraph = &graph;
    
  } // упаковка
  
	
	// Выбор оконечного узла (пользователя, поэтому, вероятно, с минимальной степенью)
	virtual int selectEndNode() const
	{
		if (!pexportedGraph)
		{
			cout << "call exportToPlain()!!!" << endl;
			return rand() % totalVerticesCount;
		}
		
		int imax = pexportedGraph->size();

		double rouletteMax = 0;
		for(int i = 0; i < imax; ++i)
		{
			auto ideg = (*pexportedGraph)[i].size();
			if (ideg > 0)
				rouletteMax += 100.0/ideg;
  
		}   		
		
		double rouletteValue = rand() * rouletteMax / (RAND_MAX + 1.0);
		
		int rouletteSector = 0;
		for(int i = 0; i < imax; ++i)
		{
			auto ideg = (*pexportedGraph)[i].size();
			if (ideg > 0)
			{
				rouletteSector += 100.0/ideg;
				if (rouletteValue < rouletteSector)
				{
					return i;
				}      
			}
		}    
		return imax-1; // а сюда мы не дойдём	
		
	}


}; //TCombinedGraph
