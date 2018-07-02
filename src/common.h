#ifndef COMMON_H
#define COMMON_H

#include <vector>
#include <string>
#include <algorithm>
#include <climits>

using namespace std;

typedef unsigned int TVertexNumber; // было int

// const int NodesCount = 1e5;
// const int NodesCount = 1e4;
const int NodesCount = 1e3;
// const int NodesCount = 3e2;
// const int NodesCount = 12;

const int minConnectionCount = 1;
//  const int minConnectionCount = 2;
// const int maxConnectionCount = 32;
// const int maxConnectionCount = 256;
//const int maxConnectionCount = 8;
const int maxConnectionCount = INT_MAX;

// const int startDel = 128;
const int startDel = 64;
const double pDel = 0.4;
// const double pAdd = 0.8;


class TGrowingNetworkBA;
class TGrowingNetworkLeafs; // присоединение по обратному правилу в ?50% случаев
class TNetworkWithDelete;
class TCombinedGraph;	// составной граф из графов

typedef TGrowingNetworkBA TCurrentModel;





#define ROOT_TARGET

// Расчёт характеристик распределения *****************************************

// Размер гистограммы
const int hist_width = 30;
const int hist_height = 16;

// Размер выборки оконечных узлов для частичного распределения
const int EndNodesCount = 550; // как для измерений (малые выборки)

class distProperties
{
protected:
	int max_exists_len;
	double lencount; 
	double zmean;
	double zstd;
	double zskewness;
	double zkurtosis;
	
	char hc[hist_width][hist_height];
	int max_y;	

public:	
	distProperties()
	{
		max_exists_len = 0; 
		lencount = 0; 
		zmean = 0;
		zstd = 0;
		zskewness = 0;
		zkurtosis = 0;
	
		for (int x=0; x < hist_width; ++x)
		{
			for(int y=0; y <hist_height; ++y)
			{
				hc[x][y] = ' ';
			}
		}	
		max_y = 0;	
	}
	
	void calcProperties(const ulong* counterDistances, int n)
	{
	
		// Максимальная длина
		//int 
		max_exists_len = 0;
		int i;
		for (i = 1; i < n - 1; i++) 
		{
			if (counterDistances[i] > 0)
				max_exists_len = i;
		}
		//cout << "max_exists_len = " << max_exists_len << endl;
		
		// Среднее
		//int 
		lencount = 0;
		double sumlen = 0;
		for (i = 1; i <= max_exists_len; i++) 
		{
			lencount += counterDistances[i];
			sumlen   += counterDistances[i] * i;
		}        
		//double 
		zmean = sumlen/lencount;
			
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
		//double 
		zstd = sqrt(   centersum2 / (lencount - 1)   );
		//double 
		zskewness = centersum3 / (lencount * zstd*zstd*zstd );
		//double 
		zkurtosis = centersum4 / (lencount * zstd*zstd*zstd*zstd ) - 3;
		
	
		int max_x = min(max_exists_len, hist_width-2);
		
		for (int x=0; x < hist_width; ++x)
		{
			for(int y=0; y <hist_height; ++y)
			{
				hc[x][y] = ' ';
			}
		}	
		
		//int 
		max_y = 0;
		for (int x = 0; x <= max_x; ++x)
		{
			double hist_col = counterDistances[x] * (double)hist_height / lencount;
			int y;
			for(y=1; y <hist_col; ++y)
			{
				hc[x][y-1] = '@';
			}
			// первое y >= hist_col		
			if (y - hist_col <= 0.3) 
			{
				hc[x][y-1] = '@';
			}		
			else if (y - hist_col <= 0.7) 
			{
				hc[x][y-1] = 'o';
			}		
			else
			{
				hc[x][y-1] = '.';
			}	
			if (y >= max_y)
				max_y = y;
		}		
	}
	
	void printHist(ostream &out) const
	{
		for(int y = max_y; y >=0; --y)
		{
			for (int x=0; x < hist_width; ++x)
			{
				out << hc[x][y];
			}
			out << endl;
		}		
	}
	
	void printMoments(ostream &out) const
	{
		out << setprecision(2) << "mean = " << zmean << "\tstd = " << zstd << "\tskewness = " << zskewness << "\tkurtosis-3 = " << zkurtosis << endl;
		
	}
	
	void printValues(ostream &out, const ulong* counterDistances, int n, ulong infDistance) 
	{
		//for (int i = 1; i < n - 1; i++) {
		//	if (counterDistances[i] > 0)
		// внутренние нули пишем!!!
		for (int i = 1; i <= max_exists_len; i++) 
		{
		
				out << i << " " << counterDistances[i] << endl;
		}
		out << "infPath " << infDistance;		
	}
	
};


#endif 


