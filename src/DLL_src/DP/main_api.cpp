//  Portions Copyright 2019
// Xuesong Zhou
//   If you help write or modify the code, please also list your names here.
//   The reason of having Copyright info here is to ensure all the modified version, as a whole, under the GPL 
//   and further prevent a violation of the GPL.

// More about "How to use GNU licenses for your own software"
// http://www.gnu.org/licenses/gpl-howto.html

#pragma warning( disable : 4305 4267) 
#include <iostream>
#include <fstream>
#include <list> 
#include <omp.h>
#include <algorithm>
#include <time.h>
#include <functional>
#include <stdio.h>   
#include <math.h>


#include <stack>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <iostream>
#include <iomanip>
using namespace std;
using std::string;
using std::ifstream;
using std::vector;
using std::map;
using std::istringstream;
using std::max;
template <typename T>

// some basic parameters setting

//Pls make sure the _MAX_K_PATH > Agentlite.cpp's g_number_of_K_paths+g_reassignment_number_of_K_paths and the _MAX_ZONE remain the same with .cpp's defination
#define _MAX_LABEL_COST 99999.0
#define _MAX_K_PATH 40 
#define _MAX_ZONE 70

#define _MAX_AGNETTYPES 5 //because of the od demand store format,the MAX_demandtype must >=g_DEMANDTYPES.size()+1
#define _MAX_TIMEPERIODS 6

#define _MAX_TIMESLOT_PerPeriod 100 // max 96 15-min slots per day

#define MIN_PER_TIMESLOT 15

// Linear congruential generator 
#define LCG_a 17364
#define LCG_c 0
#define LCG_M 65521  // it should be 2^32, but we use a small 16-bit number to save memory


#define sprintf_s sprintf

FILE* g_pFileOutputLog = NULL;

void fopen_ss(FILE **file, const char *fileName, const char *mode)
{
	*file = fopen(fileName, mode);
}


void g_ProgramStop();


//below shows where the functions used in Agentlite.cpp come from!
//Utility.cpp

#pragma warning(disable: 4244)  // stop warning: "conversion from 'int' to 'float', possible loss of data"


class CCSVParser
{
public:
	char Delimiter;
	bool IsFirstLineHeader;
	ifstream inFile;
	string mFileName;
	vector<string> LineFieldsValue;
	vector<string> Headers;
	map<string, int> FieldsIndices;

	vector<int> LineIntegerVector;

public:
	void  ConvertLineStringValueToIntegers()
	{
		LineIntegerVector.clear();
		for (unsigned i = 0; i < LineFieldsValue.size(); i++)
		{
			std::string si = LineFieldsValue[i];
			int value = atoi(si.c_str());

			if (value >= 1)
				LineIntegerVector.push_back(value);

		}
	}
	vector<string> GetHeaderVector()
	{
		return Headers;
	}

	int m_EmptyLineCount;
	bool m_bDataHubSingleCSVFile;
	string m_DataHubSectionName;
	bool m_bLastSectionRead;

	bool m_bSkipFirstLine;  // for DataHub CSV files

	CCSVParser(void)
	{
		Delimiter = ',';
		IsFirstLineHeader = true;
		m_bSkipFirstLine = false;
		m_bDataHubSingleCSVFile = false;
		m_bLastSectionRead = false;
		m_EmptyLineCount++;
	}

	~CCSVParser(void)
	{
		if (inFile.is_open()) inFile.close();
	}


	bool OpenCSVFile(string fileName, bool b_required)
	{
		mFileName = fileName;
		inFile.open(fileName.c_str());

		if (inFile.is_open())
		{
			if (IsFirstLineHeader)
			{
				string s;
				std::getline(inFile, s);
				vector<string> FieldNames = ParseLine(s);

				for (size_t i = 0;i < FieldNames.size();i++)
				{
					string tmp_str = FieldNames.at(i);
					size_t start = tmp_str.find_first_not_of(" ");

					string name;
					if (start == string::npos)
					{
						name = "";
					}
					else
					{
						name = tmp_str.substr(start);
						//			TRACE("%s,", name.c_str());
					}


					FieldsIndices[name] = (int)i;
				}
			}

			return true;
		}
		else
		{
			if (b_required)
			{

				cout << "File " << fileName << " does not exist. Please check." << endl;
				//g_ProgramStop();
			}
			return false;
		}
	}


	void CloseCSVFile(void)
	{
		inFile.close();
	}



	bool ReadRecord()
	{
		LineFieldsValue.clear();

		if (inFile.is_open())
		{
			string s;
			std::getline(inFile, s);
			if (s.length() > 0)
			{

				LineFieldsValue = ParseLine(s);

				return true;
			}
			else
			{

				return false;
			}
		}
		else
		{
			return false;
		}
	}

	vector<string> ParseLine(string line)
	{
		vector<string> SeperatedStrings;
		string subStr;

		if (line.length() == 0)
			return SeperatedStrings;

		istringstream ss(line);


		if (line.find_first_of('"') == string::npos)
		{

			while (std::getline(ss, subStr, Delimiter))
			{
				SeperatedStrings.push_back(subStr);
			}

			if (line.at(line.length() - 1) == ',')
			{
				SeperatedStrings.push_back("");
			}
		}
		else
		{
			while (line.length() > 0)
			{
				size_t n1 = line.find_first_of(',');
				size_t n2 = line.find_first_of('"');

				if (n1 == string::npos && n2 == string::npos) //last field without double quotes
				{
					subStr = line;
					SeperatedStrings.push_back(subStr);
					break;
				}

				if (n1 == string::npos && n2 != string::npos) //last field with double quotes
				{
					size_t n3 = line.find_first_of('"', n2 + 1); // second double quote

					//extract content from double quotes
					subStr = line.substr(n2 + 1, n3 - n2 - 1);
					SeperatedStrings.push_back(subStr);

					break;
				}

				if (n1 != string::npos && (n1 < n2 || n2 == string::npos))
				{
					subStr = line.substr(0, n1);
					SeperatedStrings.push_back(subStr);
					if (n1 < line.length() - 1)
					{
						line = line.substr(n1 + 1);
					}
					else // comma is the last char in the line string, push an empty string to the back of vector
					{
						SeperatedStrings.push_back("");
						break;
					}
				}

				if (n1 != string::npos && n2 != string::npos && n2 < n1)
				{
					size_t n3 = line.find_first_of('"', n2 + 1); // second double quote
					subStr = line.substr(n2 + 1, n3 - n2 - 1);
					SeperatedStrings.push_back(subStr);
					size_t idx = line.find_first_of(',', n3 + 1);

					if (idx != string::npos)
					{
						line = line.substr(idx + 1);
					}
					else
					{
						break;
					}
				}
			}

		}

		return SeperatedStrings;
	}

	template <class T> bool GetValueByFieldName(string field_name, T& value, bool NonnegativeFlag = true)
	{


		bool required_field = true;
		bool print_out = false;
		if (FieldsIndices.find(field_name) == FieldsIndices.end())
		{
			if (required_field)
			{
				cout << "Field " << field_name << " in file " << mFileName << " does not exist. Please check the file." << endl;

				g_ProgramStop();
			}
			return false;
		}
		else
		{
			if (LineFieldsValue.size() == 0)
			{
				return false;
			}

			int size = (int)(LineFieldsValue.size());
			if (FieldsIndices[field_name] >= size)
			{
				return false;
			}

			string str_value = LineFieldsValue[FieldsIndices[field_name]];

			if (str_value.length() <= 0)
			{
				return false;
			}

			istringstream ss(str_value);

			T converted_value;
			ss >> converted_value;

			if (/*!ss.eof() || */ ss.fail())
			{
				return false;
			}

			if (NonnegativeFlag && converted_value < 0)
				converted_value = 0;

			value = converted_value;
			return true;
		}
	}


	bool GetValueByFieldName(string field_name, string& value)
	{
		if (FieldsIndices.find(field_name) == FieldsIndices.end())
		{
			return false;
		}
		else
		{
			if (LineFieldsValue.size() == 0)
			{
				return false;
			}

			unsigned int index = FieldsIndices[field_name];
			if (index >= LineFieldsValue.size())
			{
				return false;
			}
			string str_value = LineFieldsValue[index];

			if (str_value.length() <= 0)
			{
				return false;
			}

			value = str_value;
			return true;
		}
	}

};



template <typename T>
T **AllocateDynamicArray(int nRows, int nCols)
{
	T **dynamicArray;

	dynamicArray = new (std::nothrow) T*[nRows];

	if (dynamicArray == NULL)
	{
		cout << "Error: insufficient memory.";
		g_ProgramStop();

	}

	for (int i = 0; i < nRows; i++)
	{
		dynamicArray[i] = new (std::nothrow) T[nCols];

		if (dynamicArray[i] == NULL)
		{
			cout << "Error: insufficient memory.";
			g_ProgramStop();
		}


	}

	return dynamicArray;
}

template <typename T>
void DeallocateDynamicArray(T** dArray, int nRows, int nCols)
{
	if (!dArray)
		return;

	for (int x = 0; x < nRows; x++)
	{
		delete[] dArray[x];
	}

	delete[] dArray;

}

template <typename T>
T ***Allocate3DDynamicArray(int nX, int nY, int nZ)
{
	T ***dynamicArray;

	dynamicArray = new (std::nothrow) T**[nX];

	if (dynamicArray == NULL)
	{
		cout << "Error: insufficient memory.";
		g_ProgramStop();
	}

	for (int x = 0; x < nX; x++)
	{
		if (x % 1000 == 0)
		{
			cout << "allocating 3D memory for " << x << endl;
		}


		dynamicArray[x] = new (std::nothrow) T*[nY];

		if (dynamicArray[x] == NULL)
		{
			cout << "Error: insufficient memory.";
			g_ProgramStop();
		}

		for (int y = 0; y < nY; y++)
		{
			dynamicArray[x][y] = new (std::nothrow) T[nZ];
			if (dynamicArray[x][y] == NULL)
			{
				cout << "Error: insufficient memory.";
				g_ProgramStop();
			}
		}
	}

	for (int x = 0; x < nX; x++)
		for (int y = 0; y < nY; y++)
			for (int z = 0; z < nZ; z++)
			{
				dynamicArray[x][y][z] = 0;
			}
	return dynamicArray;

}

template <typename T>
void Deallocate3DDynamicArray(T*** dArray, int nX, int nY)
{
	if (!dArray)
		return;
	for (int x = 0; x < nX; x++)
	{
		for (int y = 0; y < nY; y++)
		{
			delete[] dArray[x][y];
		}

		delete[] dArray[x];
	}

	delete[] dArray;

}



template <typename T>
T ****Allocate4DDynamicArray(int nM, int nX, int nY, int nZ)
{
	T ****dynamicArray;

	dynamicArray = new (std::nothrow) T***[nX];

	if (dynamicArray == NULL)
	{
		cout << "Error: insufficient memory.";
		g_ProgramStop();
	}
	for (int m = 0; m < nM; m++)
	{
		if (m % 100 == 0)
			cout << "allocating 4D memory for " << m << endl;

		dynamicArray[m] = new (std::nothrow) T**[nX];

		if (dynamicArray[m] == NULL)
		{
			cout << "Error: insufficient memory.";
			g_ProgramStop();
		}

		for (int x = 0; x < nX; x++)
		{
			dynamicArray[m][x] = new (std::nothrow) T*[nY];

			if (dynamicArray[m][x] == NULL)
			{
				cout << "Error: insufficient memory.";
				g_ProgramStop();
			}

			for (int y = 0; y < nY; y++)
			{
				dynamicArray[m][x][y] = new (std::nothrow) T[nZ];
				if (dynamicArray[m][x][y] == NULL)
				{
					cout << "Error: insufficient memory.";
					g_ProgramStop();
				}
			}
		}
	}
	return dynamicArray;

}

template <typename T>
void Deallocate4DDynamicArray(T**** dArray, int nM, int nX, int nY)
{
	if (!dArray)
		return;
	for (int m = 0; m < nM; m++)
	{
		for (int x = 0; x < nX; x++)
		{
			for (int y = 0; y < nY; y++)
			{
				delete[] dArray[m][x][y];
			}

			delete[] dArray[m][x];
		}
		delete[] dArray[m];
	}
	delete[] dArray;

}


//struct MyException : public exception {
//	const char * what() const throw () {
//		return "C++ Exception";
//	}
//};
//

class CDemand_Period {
public:

	CDemand_Period()
	{
		demand_period_id = 0;
		starting_time_slot_no = 0;
		ending_time_slot_no = 0;

	}
	string demand_period;
	string time_period;
	int demand_period_id;
	int starting_time_slot_no;
	int ending_time_slot_no;

};


class CAgent_type {
public:
	CAgent_type()
	{
		value_of_time = 1;
		agent_type_no = 0;
		bFixed = 0;
	}

	int agent_type_no;
	string agent_type;
	float value_of_time;  // dollar per hour
	std::map<int, float> PCE_link_type_map;  // link type, product consumption equivalent used, for travel time calculation
	std::map<int, float> CRU_link_type_map;  // link type, 	Coefficient of Resource Utilization - CRU, for resource constraints 
	int bFixed; // not enter the column_pool optimization process. 

};

class CLinkType
{
public:
	int link_type;
	string link_type_name;
	string agent_type_list;

	int number_of_links;

	CLinkType()
	{
		number_of_links = 0;

	}

	bool AllowAgentType(string agent_type)
	{
		if (agent_type_list.size() == 0)  // if the agent_type_list is empty then all types are allowed.
			return true;
		else
		{
			if (agent_type_list.find(agent_type) != string::npos)  // otherwise, only an agent type is listed in this "white list", then this agent is allowed to travel on this link
				return true;
			else
				return false;


		}
	}


};


class CColumnPath {
public:
vector<int> path_node_vector;
vector<int> path_link_vector;
vector<float> path_time_vector;

int path_seq_no;

float path_volume;  // path volume
float path_switch_volume;  // path volume
float path_travel_time;
float path_distance;
float path_cost;
float path_gradient_cost;  // first order graident cost.
float path_gradient_cost_difference;  // first order graident cost - least gradient cost
float path_gradient_cost_relative_difference;  // first order graident cost - least gradient cost


CColumnPath()
{
	path_switch_volume = 0;
	path_seq_no = 0;
	path_cost = 0;
	path_volume = 0;
	path_travel_time = 0;
	path_distance = 0;
	path_gradient_cost = 0;
	path_gradient_cost_difference = 0;
	path_gradient_cost_relative_difference = 0;
}

};


class CColumnVector {
public:
	float cost;
	float time;
	float distance;
	float od_volume;  // od volume

	std::map <int, CColumnPath> path_node_sequence_map;  // first key is the sum of node id;. e.g. node 1, 3, 2, sum of those node ids is 6, 1, 4, 2 then node sum is 7.
	// this is colletion of unique paths

	CColumnVector()
	{
		od_volume = 0;
		cost = 0;
		time = 0;
		distance = 0;
	}
};

class CAgent_Column {
public:
	
	
	int agent_id;
	int o_zone_id;
	int d_zone_id;
	int o_node_id;
	int d_node_id;
	string agent_type;
	string demand_period;
	float volume;	
	float cost;
	float travel_time;
	float distance;
	vector<int> path_node_vector;
	vector<int> path_link_vector;
	vector<float> path_time_vector;

	CAgent_Column()
	{
	
	}


};

class Assignment {
public:
	Assignment()
	{
		g_column_pool = NULL;
		g_origin_demand_array = NULL;
		//pls check following 7 settings before running programmer
		g_number_of_threads = 4000;
		g_number_of_K_paths = 20;
		g_number_of_demand_periods = 24;
		g_reassignment_tau0 = 999;

		g_number_of_links = 0;
		g_number_of_nodes = 0;
		g_number_of_zones = 0;
		g_number_of_agent_types = 0;

		b_debug_detail_flag = 1;

		g_pFileDebugLog = NULL;
		assignment_mode = 0;  // default is UE
	}

	void InitializeDemandMatrix(int number_of_zones, int number_of_agent_types)
	{
		g_number_of_zones = number_of_zones;
		g_number_of_agent_types = number_of_agent_types;

		g_column_pool = Allocate4DDynamicArray<CColumnVector>(number_of_zones, number_of_zones, max(1, number_of_agent_types), _MAX_TIMEPERIODS);
		g_origin_demand_array = Allocate3DDynamicArray<float>(number_of_zones, max(1, number_of_agent_types), _MAX_TIMEPERIODS);


		for (int i = 0;i < number_of_zones;i++)
		{
			for (int at = 0;at < number_of_agent_types;at++)
			{
				for (int tau = 0;tau < g_number_of_demand_periods;tau++)
				{

					g_origin_demand_array[i][at][tau] = 0.0;
				}
			}

		}
		total_demand_volume = 0.0;
		for (int i = 0;i < number_of_agent_types;i++)
		{
			for (int tau = 0;tau < g_number_of_demand_periods;tau++)
			{
				total_demand[i][tau] = 0.0;
			}
		}

		g_DemandGlobalMultiplier = 1.0f;


	};
	~Assignment()
	{

		if (g_column_pool != NULL)
			Deallocate4DDynamicArray(g_column_pool, g_number_of_zones, g_number_of_zones, g_number_of_agent_types);
		if (g_origin_demand_array != NULL)
			Deallocate3DDynamicArray(g_origin_demand_array, g_number_of_zones, g_number_of_agent_types);

		if (g_pFileDebugLog == NULL)
			fclose(g_pFileDebugLog);

	}
	int g_number_of_threads;
	int g_number_of_K_paths;
	int assignment_mode;

	int g_reassignment_tau0;

	int b_debug_detail_flag;
	std::map<int, int> g_internal_node_to_seq_no_map;  // hash table, map external node number to internal node sequence no. 
	std::map<int, int> g_zoneid_to_zone_seq_no_mapping;// from integer to integer map zone_id to zone_seq_no

	CColumnVector**** g_column_pool;
	float*** g_origin_demand_array;

	//StatisticOutput.cpp
	float total_demand_volume;
	//NetworkReadInput.cpp and ShortestPath.cpp



	std::vector<CDemand_Period> g_DemandPeriodVector;
	std::vector<CAgent_type> g_AgentTypeVector;
	std::map<int, CLinkType> g_LinkTypeMap;

	std::map<string, int> demand_period_to_seqno_mapping;
	std::map<string, int> agent_type_2_seqno_mapping;


	float total_demand[_MAX_AGNETTYPES][_MAX_TIMEPERIODS];
	float g_DemandGlobalMultiplier;

	int g_number_of_links;
	int g_number_of_nodes;
	int g_number_of_zones;
	int g_number_of_agent_types;
	int g_number_of_demand_periods;

	FILE* g_pFileDebugLog = NULL;

};

Assignment assignment;

class CVDF_Period
{
public:

	CVDF_Period()
	{
		m = 0.5;
		VOC = 0;
		gamma = 3.47f;
		mu = 1000;
		theta = 1;
		alpha = 0.15f;
		beta = 4;
		rho = 1;
		marginal_base = 1;
		ruc_base_resource = 0;

		ruc_type = 1;
	}


	int starting_time_slot_no;  // in 15 min slot
	int ending_time_slot_no;
	string period;


	//standard BPR parameter 
	float alpha;
	float beta;
	float theta;
	float capacity;
	float FFTT;
	float VOC;
	float rho;
	float ruc_base_resource;
	int   ruc_type;

	float marginal_base;
	//updated BPR-X parameters
	float gamma;
	float mu;
	float m;
	float congestion_period_P;
	// inpput
	float volume;

	//output
	float avg_delay;
	float avg_travel_time = 0;
	float avg_waiting_time = 0;

	//float Q[_MAX_TIMESLOT_PerPeriod];  // t starting from starting_time_slot_no if we map back to the 24 hour horizon 
	float waiting_time[_MAX_TIMESLOT_PerPeriod];
	float arrival_rate[_MAX_TIMESLOT_PerPeriod];

	float discharge_rate[_MAX_TIMESLOT_PerPeriod];
	float travel_time[_MAX_TIMESLOT_PerPeriod];

	
	float get_waiting_time(int relative_time_slot_no)
	{
		if (relative_time_slot_no >=0 && relative_time_slot_no < _MAX_TIMESLOT_PerPeriod)
			return waiting_time[relative_time_slot_no];
		else
			return 0;

	}
	int t0, t3;

	void Setup()
	{

	}

	float  PerformBPR(float volume)
	{
		volume = max(0, volume);  // take nonnegative values

		VOC = volume / max(0.00001f, capacity);
		avg_travel_time = FFTT * theta + FFTT * alpha * pow(volume / max(0.00001f, capacity), beta);

		marginal_base = FFTT * alpha * beta*pow(volume / max(0.00001f, capacity), beta - 1);



	return avg_travel_time;

		// volume --> avg_traveltime
	
	}

	
	float PerformBPR_X(float volume)
	{
		congestion_period_P = 0;
		// Step 1: Initialization
		int L = ending_time_slot_no - starting_time_slot_no;  // in 15 min slot

		if (L >= _MAX_TIMESLOT_PerPeriod - 1)
			return 0;

		float mid_time_slot_no = starting_time_slot_no + L / 2.0;  // t1;
		for (int t = 0; t <= L; t++)
		{
			waiting_time[t] = 0;
			arrival_rate[t] = 0;
			discharge_rate[t]= mu/2.0;
			travel_time[t] = FFTT;
		}
		avg_waiting_time = 0;
		avg_travel_time = FFTT + avg_waiting_time;

		//int L = ending_time_slot_no - starting_time_slot_no;  // in 15 min slot

		// Case 1
		if (volume <= L * mu / 2)
		{
			// still keep 0 waiting time for all time period
			congestion_period_P = 0;

		}
		else
		{
			//if (volume > L * mu / 2 ) // Case 2
			float P = min(L, volume * 2 / mu - L);  // if  volume > L * mu then P is set the the maximum of L: case 3
			congestion_period_P = P/4.0;  // unit: hour

			t0 = mid_time_slot_no - P / 2.0;
			t3 = mid_time_slot_no + P / 2.0;
			int t2 = m * (t3 - t0) + t0;
			for (int tt = 0; tt <= L; tt++)
			{
				int time = starting_time_slot_no + tt;
				if (time < t0)
				{
					waiting_time[tt] = 0;
					arrival_rate[tt] = mu / 2;
					discharge_rate[tt] = mu / 2.0;
					travel_time[tt] = FFTT;

				}
				if (time >= t0 && time <= t3)
				{
					waiting_time[tt] = 1 / (4.0*mu) *gamma *(time - t0)*(time - t0) * (time - t3)*(time - t3);
					arrival_rate[tt] = gamma * (time - t0)*(time - t2)*(time - t3) + mu;
					discharge_rate[tt] = mu;
					travel_time[tt] = FFTT + waiting_time[tt];
				}
				if (time > t3)
				{
					waiting_time[tt] = 0;
					arrival_rate[tt] = mu / 2;
					discharge_rate[tt] = mu / 2.0;
					travel_time[tt] = FFTT;
				}
				avg_waiting_time = gamma / (120 * mu)*pow(P, 4.0);
				//cout << avg_waiting_time << endl;
				avg_travel_time = FFTT + avg_waiting_time;
			}
		}

		return avg_travel_time;

	}
};


class CLink
{
public:
	CLink()  // construction 
	{

		free_flow_travel_time_in_min = 1;
		cost = 0;
		for (int tau = 0; tau < _MAX_TIMEPERIODS; tau++)
		{
			flow_volume_per_period[tau] = 0;
			resource_per_period[tau] = 0;

			queue_length_perslot[tau] = 0;
			travel_time_per_period[tau] = 0;

			for(int at = 0; at < _MAX_AGNETTYPES; at++)
			{
				volume_per_period_per_at[tau][at] = 0;
				resource_per_period_per_at[tau][at] = 0;

			}

			TDBaseTT[tau] = 0;
			TDBaseCap[tau] = 0;
			TDBaseFlow[tau] = 0;
			TDBaseQueue[tau] = 0;


			//cost_perhour[tau] = 0;
		}
		link_spatial_capacity = 100;
		RUC_type = 1;
	}

	~CLink()
	{
		//if (flow_volume_for_each_o != NULL)
		//	delete flow_volume_for_each_o;
	}

	void free_memory()
	{
	}

	void AddAgentsToLinkVolume()
	{


	}




	std::vector<int> m_total_volume_vector;
	std::vector<float> m_avg_travel_time;

	// 1. based on BPR. 


	int m_LeftTurn_link_seq_no;

	int m_RandomSeed;
	int link_seq_no;
	int link_id;
	int from_node_seq_no;
	int to_node_seq_no;
	int link_type;
	float cost;


	float fftt;
	float free_flow_travel_time_in_min;


	CVDF_Period VDF_period[_MAX_TIMEPERIODS];

	float TDBaseTT[_MAX_TIMEPERIODS];
	float TDBaseCap[_MAX_TIMEPERIODS];
	float TDBaseFlow[_MAX_TIMEPERIODS];
	float TDBaseQueue[_MAX_TIMEPERIODS];


	int type;
	float link_spatial_capacity;

	//static
	//float flow_volume;
	//float travel_time;

	float flow_volume_per_period[_MAX_TIMEPERIODS];
	float volume_per_period_per_at[_MAX_TIMEPERIODS][_MAX_AGNETTYPES];

	float resource_per_period[_MAX_TIMEPERIODS];
	float resource_per_period_per_at[_MAX_TIMEPERIODS][_MAX_AGNETTYPES];

	float queue_length_perslot[_MAX_TIMEPERIODS];  // # of vehicles in the vertical point queue
	float travel_time_per_period[_MAX_TIMEPERIODS];
	float travel_marginal_cost_per_period[_MAX_TIMEPERIODS][_MAX_AGNETTYPES];

	float exterior_penalty_cost_per_period[_MAX_TIMEPERIODS][_MAX_AGNETTYPES];
	float exterior_penalty_derviative_per_period[_MAX_TIMEPERIODS][_MAX_AGNETTYPES];

	int RUC_type;
	int number_of_periods;

	float length;
	//std::vector <SLinkMOE> m_LinkMOEAry;
	//beginning of simulation data 

	//toll related link
	//int m_TollSize;
	//Toll *pTollVector;  // not using SLT here to avoid issues with OpenMP
	std::map<int, float> TollMAP;

	void CalculateTD_VDFunction();

	float get_VOC_ratio(int tau)
	{

		return (flow_volume_per_period[tau] + TDBaseFlow[tau]) / max(0.00001f, TDBaseCap[tau]);
	}

	float get_net_resource(int tau)
	{

		return resource_per_period[tau];
	}

	float get_speed(int tau)
	{
		return length / max(travel_time_per_period[tau], 0.0001f) * 60;  // per hour
	}


	void calculate_marginal_cost_for_agent_type(int tau, int agent_type_no, float PCE_agent_type)
	{
		// volume * dervative 
		// BPR_term: volume * FFTT * alpha * (beta) * power(v/c, beta-1), 
		
		travel_marginal_cost_per_period[tau][agent_type_no] = VDF_period[tau].marginal_base*PCE_agent_type;
	}

	void calculate_penalty_for_agent_type(int tau, int agent_type_no, float CRU_agent_type)
	{
		// volume * dervative 

		float resource = 0;
			
		if (RUC_type == 0)  // equality constraints 
			resource = resource_per_period[tau];
		else
			resource = min(0, resource_per_period[tau]);  // inequality
		
		exterior_penalty_derviative_per_period[tau][agent_type_no] = VDF_period[tau].rho * resource * CRU_agent_type;
	}

	float get_generalized_first_order_gradient_cost_for_agent_type(int tau, int agent_type_no)
	{
		float generalized_cost = travel_time_per_period[tau] + cost / assignment.g_AgentTypeVector[agent_type_no].value_of_time * 60;  // *60 as 60 min per hour

		if (assignment.assignment_mode == 1)  // system optimal mode
		{
			generalized_cost += travel_marginal_cost_per_period[tau][agent_type_no];
		}

		if (assignment.assignment_mode == 2)  // exterior panalty mode
		{
			generalized_cost += exterior_penalty_derviative_per_period[tau][agent_type_no];

		}

		return generalized_cost;
	}

};


class CNode
{
public:
	CNode()
	{
		zone_id = -1;
		//accessible_node_count = 0;
	}

	//int accessible_node_count;

	int node_seq_no;  // sequence number 
	int node_id;      //external node number 
	int zone_id = -1;

	double x;
	double y;

	std::vector<int> m_outgoing_link_seq_no_vector;
	std::vector<int> m_to_node_seq_no_vector;
	std::map<int, int> m_to_node_seq_no_map;



};


extern std::vector<CNode> g_node_vector;
extern std::vector<CLink> g_link_vector;


class COZone
{
public:
	int zone_seq_no;  // 0, 1, 
	int zone_id;  // external zone id // this is origin zone
	int node_seq_no;


};

extern std::vector<COZone> g_zone_vector;
extern std::map<int, int> g_zoneid_to_zone_seq_no_mapping;

class CAGBMAgent
{
public:

	int agent_id;
	int income;
	int gender;
	int vehicle;
	int purpose;
	int flexibility;
	float preferred_arrival_time;
	float travel_time_in_min;
	float free_flow_travel_time;
	int from_zone_seq_no;
	int to_zone_seq_no;
	int type;
	int time_period;
	int k_path;
	float volume;
	float arrival_time_in_min;



};
extern std::vector<CAGBMAgent> g_agbmagent_vector;


class NetworkForSP  // mainly for shortest path calculation
{
public:

	NetworkForSP()
	{
	}


	int  m_origin_node; // assigned nodes for computing 
	int  m_origin_zone_seq_no;
	int  m_demand_time_period_no; // assigned nodes for computing 
	int  m_agent_type_no; // assigned nodes for computing 
	float m_value_of_time;

	std::vector<CNode> m_node_vector;  // local copy of node vector, based on agent type and origin node

	int m_threadNo;  // internal thread number 

	int m_ListFront; // used in coding SEL
	int m_ListTail;  // used in coding SEL
	int* m_SENodeList; // used in coding SEL

	float* m_node_label_cost;  // label cost // for shortest path calcuating
	float* m_label_time_array;  // time-based cost
	float* m_label_distance_array;  // distance-based cost

	int * m_node_predecessor;  // predecessor for nodes
	int* m_node_status_array; // update status 
	int* m_link_predecessor;  // predecessor for this node points to the previous link that updates its label cost (as part of optimality condition) (for easy referencing)

	int  m_iteration_k;

	// major function 1:  allocate memory and initialize the data 
	void AllocateMemory(int number_of_nodes)
	{

		m_SENodeList = new int[number_of_nodes];  //1
		m_node_status_array = new int[number_of_nodes];  //2
		m_label_time_array = new float[number_of_nodes];  //3
		m_label_distance_array = new float[number_of_nodes];  //4
		m_node_predecessor = new int[number_of_nodes];  //5
		m_link_predecessor = new int[number_of_nodes];  //6
		m_node_label_cost = new float[number_of_nodes];  //7

	}

	void BuildNetwork(Assignment& assignment)
	{

		for (int i = 0; i < assignment.g_number_of_nodes; i++) //Initialization for all non-origin nodes
		{
			CNode node;  // create a node object 

			node.node_id = g_node_vector[i].node_id;
			node.node_seq_no = g_node_vector[i].node_seq_no;

			for (int j = 0; j < g_node_vector[i].m_outgoing_link_seq_no_vector.size(); j++)
			{
				

				int link_seq_no = g_node_vector[i].m_outgoing_link_seq_no_vector[j];

				
				if(assignment.g_LinkTypeMap[g_link_vector[link_seq_no].link_type].AllowAgentType (assignment.g_AgentTypeVector[m_agent_type_no].agent_type ))  // only predefined allowed agent type can be considered
				{ 
					int from_node_seq_no = g_link_vector[link_seq_no].from_node_seq_no;

					int zone_seq_no = -1;
					
					
				if(g_node_vector[i].zone_id >= 1)
					zone_seq_no  = assignment.g_zoneid_to_zone_seq_no_mapping[g_node_vector[from_node_seq_no].zone_id];

					//if (zone_seq_no != -1 && zone_seq_no != m_origin_zone_seq_no)
					//{
					//	//skip the incoming node if it is belongs to a zone with zone id/no different from the current zone id
					//}else
					//{ 

					node.m_outgoing_link_seq_no_vector.push_back(link_seq_no);
					node.m_to_node_seq_no_vector.push_back(g_node_vector[i].m_to_node_seq_no_vector[j]);

					//}
				}

			}

			m_node_vector.push_back(node);

		}
		m_value_of_time = assignment.g_AgentTypeVector[m_agent_type_no].value_of_time;
	}
	



	~NetworkForSP()
	{
		if (m_SENodeList != NULL)  //1
			delete m_SENodeList;

		if (m_node_status_array != NULL)  //2
			delete m_node_status_array;

		if (m_label_time_array != NULL)  //3
			delete m_label_time_array;

		if (m_label_distance_array != NULL)  //4
			delete m_label_distance_array;

		if (m_node_predecessor != NULL)  //5
			delete m_node_label_cost;

		if (m_link_predecessor != NULL)  //6
			delete m_link_predecessor;

		if (m_node_label_cost != NULL)  //7
			delete m_node_label_cost;

			   
		if (m_link_predecessor != NULL)
			delete m_link_predecessor;

	}


	// SEList: scan eligible List implementation: the reason for not using STL-like template is to avoid overhead associated pointer allocation/deallocation
	void SEList_clear()
	{
		m_ListFront = -1;
		m_ListTail = -1;
	}

	void SEList_push_front(int node)
	{
		if (m_ListFront == -1)  // start from empty
		{
			m_SENodeList[node] = -1;
			m_ListFront = node;
			m_ListTail = node;
		}
		else
		{
			m_SENodeList[node] = m_ListFront;
			m_ListFront = node;
		}
	}
	void SEList_push_back(int node)
	{
		if (m_ListFront == -1)  // start from empty
		{
			m_ListFront = node;
			m_ListTail = node;
			m_SENodeList[node] = -1;
		}
		else
		{
			m_SENodeList[m_ListTail] = node;
			m_SENodeList[node] = -1;
			m_ListTail = node;
		}
	}

	bool SEList_empty()
	{
		return(m_ListFront == -1);
	}

	int SEList_front()
	{
		return m_ListFront;
	}

	void SEList_pop_front()
	{
		int tempFront = m_ListFront;
		m_ListFront = m_SENodeList[m_ListFront];
		m_SENodeList[tempFront] = -1;
	}


	//major function: update the cost for each node at each SP tree, using a stack from the origin structure 

	void calculate_TD_link_flow(Assignment& assignment, int iteration_number);

	//major function 2: // time-dependent label correcting algorithm with double queue implementation
	int optimal_label_correcting(Assignment& assignment, int iteration_k)

	{
		if (iteration_k == 0)
		{
			BuildNetwork(assignment);  // based on agent type and link type
		}

		int shortest_path_debugging_flag = 0;

		if (m_iteration_k != iteration_k)
			return 0;

		int origin_node = m_origin_node; // assigned nodes for computing 
		int agent_type = m_agent_type_no; // assigned nodes for computing 

		for (int i = 0; i < assignment.g_number_of_nodes; i++) //Initialization for all non-origin nodes
		{
			m_node_status_array[i] = 0;  // not scanned
			m_node_label_cost[i] = _MAX_LABEL_COST;
			//m_node_label_cost_withouttoll[i] = _MAX_LABEL_COST;
			m_label_time_array[i] = 0;
			m_label_distance_array[i] = 0;
			m_node_predecessor[i] = -1;  // pointer to previous NODE INDEX from the current label at current node and time
			m_link_predecessor[i] = -1;  // pointer to previous NODE INDEX from the current label at current node and time
		}



		int internal_debug_flag = 0;
		if (m_node_vector[origin_node].m_outgoing_link_seq_no_vector.size() == 0)
		{
			return 0;
		}

		//Initialization for origin node at the preferred departure time, at departure time, cost = 0, otherwise, the delay at origin node
		m_label_time_array[origin_node] = 0;
		m_node_label_cost[origin_node] = 0.0;
		//m_node_label_cost_withouttoll[origin_node] = 0.0;

		SEList_clear();
		SEList_push_back(origin_node);

		clock_t start_t = clock();
		while (!SEList_empty())
		{
			int from_node = SEList_front();//pop a node FromID for scanning

			SEList_pop_front();  // remove current node FromID from the SE list
			m_node_status_array[from_node] = 2;

			if (shortest_path_debugging_flag && assignment.g_pFileDebugLog !=NULL)
				fprintf(assignment.g_pFileDebugLog, "SP: SE node: %d\n", g_node_vector[from_node].node_id);

			//scan all outbound nodes of the current node
			for (int i = 0; i < m_node_vector[from_node].m_outgoing_link_seq_no_vector.size(); i++)  // for each link (i,j) belong A(i)
			{

				int to_node = m_node_vector[from_node].m_to_node_seq_no_vector[i];

				/*if (to_node == origin_node)
					continue;*/

				bool  b_node_updated = false;
				/*if (g_link_vector[m_node_vector[from_node].m_outgoing_link_seq_no_vector[i].link_seq_no].TollMAP[assignment.g_AgentTypeVector[agent_type]] / max(0.0001, assignment.g_VOT_PerDemandType_MAP[assignment.g_AgentTypeVector[agent_type]]) * 60.0f > 0)
				{
					int a = g_link_vector[m_node_vector[from_node].m_outgoing_link_seq_no_vector[i].link_seq_no].TollMAP[assignment.g_AgentTypeVector[agent_type]] / max(0.0001, assignment.g_VOT_PerDemandType_MAP[assignment.g_AgentTypeVector[agent_type]]) * 60.0f;
					cout << "a" << endl;
				}*/

				int link_sqe_no = m_node_vector[from_node].m_outgoing_link_seq_no_vector[i];
				float new_time = m_label_time_array[from_node] + g_link_vector[link_sqe_no].travel_time_per_period[m_demand_time_period_no];
				float new_distance = m_label_distance_array[from_node] + g_link_vector[link_sqe_no].length;

				float new_to_node_cost = m_node_label_cost[from_node] + g_link_vector[link_sqe_no].travel_time_per_period[m_demand_time_period_no] + g_link_vector[link_sqe_no].cost /m_value_of_time * 60;  // *60 as 60 min per hour

				if(assignment.assignment_mode == 1)  // system optimal mode
				{ 
					new_to_node_cost += g_link_vector[link_sqe_no].travel_marginal_cost_per_period[m_demand_time_period_no][m_agent_type_no];
				}

				if (assignment.assignment_mode == 2)  // exterior panalty mode
				{
					new_to_node_cost += g_link_vector[link_sqe_no].exterior_penalty_derviative_per_period[m_demand_time_period_no][m_agent_type_no];
				
				}

				//g_link_vector[m_node_vector[from_node].m_outgoing_link_seq_no_vector[i].link_seq_no].TollMAP[agent_type]] / max(0.0001, assignment.g_AgentTypeVector [agent_type]);
								//float new_to_node_cost_withouttoll = m_node_label_cost_withouttoll[from_node] + m_link_cost_withouttoll_array[link_entering_time_interval][agent_type][m_node_vector[from_node].m_outgoing_link_seq_no_vector[i].link_seq_no];

								/*if (shortest_path_debugging_flag && g_pFileDebugLog != NULL)
								{
									fprintf(g_pFileDebugLog, "SP: checking from node %d, to node %d  cost = %d\n",
										g_node_vector[from_node].node_id,
										g_node_vector[to_node].node_id,
										new_to_node_cost, g_node_vector[from_node].m_outgoing_link_seq_no_vector[i].cost);
								}*/

				if (new_to_node_cost < m_node_label_cost[to_node]) // we only compare cost at the downstream node ToID at the new arrival time t
				{

					if (shortest_path_debugging_flag && assignment.g_pFileDebugLog != NULL)
					{
						fprintf(assignment.g_pFileDebugLog, "SP: updating node: %d current cost: %.2f, new cost %.2f\n",
							m_node_vector[to_node].node_id,
							m_node_label_cost[to_node], new_to_node_cost);
					}

					// update cost label and node/time predecessor
					m_label_time_array[to_node] = new_time;
					m_label_distance_array[to_node] = new_distance;

					m_node_label_cost[to_node] = new_to_node_cost;
					//m_node_label_cost_withouttoll[to_node] = new_to_node_cost_withouttoll;
					int link_seq_no = m_node_vector[from_node].m_outgoing_link_seq_no_vector[i];

					m_node_predecessor[to_node] = from_node;  // pointer to previous physical NODE INDEX from the current label at current node and time
					m_link_predecessor[to_node] = m_node_vector[from_node].m_outgoing_link_seq_no_vector[i];  // pointer to previous physical NODE INDEX from the current label at current node and time

					b_node_updated = true;

					if (shortest_path_debugging_flag && assignment.g_pFileDebugLog != NULL)
						fprintf(assignment.g_pFileDebugLog, "SP: add node %d into SE List\n",
							m_node_vector[to_node].node_id);

					//to_node is zone centroid and not origin_node,is to make sure no passing zones, only needed in network with connector
					/*if (m_node_vector[to_node].zone_id != -1 && to_node != origin_node)
					{
						m_node_status_array[to_node] = 1;
					}*/

					if (m_node_status_array[to_node] == 0)
					{
						SEList_push_back(to_node);
						m_node_status_array[to_node] = 1;
					}
					if (m_node_status_array[to_node] == 2)
					{
						SEList_push_front(to_node);
						m_node_status_array[to_node] = 1;
					}

				}

			}
		}

		//clock_t end_t = clock();
		//clock_t total_t = (end_t - start_t);
		//		cout << "( total time of shortest path calculation = " << total_t << " milliseconds" << " " << ")" << endl;



		return 1;  // one to all shortest pat
	}


};

std::vector<CNode> g_node_vector;
std::vector<CLink> g_link_vector;
std::vector<COZone> g_zone_vector;
std::vector<CAGBMAgent> g_agbmagent_vector;
std::vector<NetworkForSP*> g_NetworkForSP_vector;

//    This file is part of FlashDTA.

//    FlashDTA is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.

//    FlashDTA  is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.

//    You should have received a copy of the GNU General Public License
//    along with DTALite.  If not, see <http://www.gnu.org/licenses/>.


int g_read_integer(FILE* f, bool speicial_char_handling)
// read an integer from the current pointer of the file, skip all spaces
{
	char ch, buf[32];
	int i = 0;
	int flag = 1;
	/* returns -1 if end of file is reached */

	while (true)
	{
		ch = getc(f);
		//cout << "get from node successful: " << ch;
		if (ch == EOF || (speicial_char_handling && (ch == '*' || ch == '$')))
			return -1; // * and $ are special characters for comments
		if (isdigit(ch))
			break;
		if (ch == '-')
			flag = -1;
		else
			flag = 1;
	};
	if (ch == EOF) return -1;


	while (isdigit(ch)) {
		buf[i++] = ch;
		//cout << "isdigit" << buf[i++] << endl;
		ch = fgetc(f);
		//cout << "new ch" << ch;
	}
	buf[i] = 0;


	return atoi(buf) * flag;

}


float g_read_float(FILE *f)
//read a floating point number from the current pointer of the file,
//skip all spaces

{
	char ch, buf[32];
	int i = 0;
	int flag = 1;

	/* returns -1 if end of file is reached */

	while (true)
	{
		ch = getc(f);
		if (ch == EOF || ch == '*' || ch == '$') return -1;
		if (isdigit(ch))
			break;

		if (ch == '-')
			flag = -1;
		else
			flag = 1;

	};
	if (ch == EOF) return -1;
	while (isdigit(ch) || ch == '.') {
		buf[i++] = ch;
		ch = fgetc(f);

	}
	buf[i] = 0;

	/* atof function converts a character string (char *) into a doubleing
	pointer equivalent, and if the string is not a floting point number,
	a zero will be return.
	*/

	return (float)(atof(buf) * flag);

}



//split the string by "_"
vector<string> split(const string &s, const string &seperator) {
	vector<string> result;
	typedef string::size_type string_size;
	string_size i = 0;

	while (i != s.size()) {
		int flag = 0;
		while (i != s.size() && flag == 0) {
			flag = 1;
			for (string_size x = 0; x < seperator.size(); ++x)
				if (s[i] == seperator[x]) {
					++i;
					flag = 0;
					break;
				}
		}

		flag = 0;
		string_size j = i;
		while (j != s.size() && flag == 0) {
			for (string_size x = 0; x < seperator.size(); ++x)
				if (s[j] == seperator[x]) {
					flag = 1;
					break;
				}
			if (flag == 0)
				++j;
		}
		if (i != j) {
			result.push_back(s.substr(i, j - i));
			i = j;
		}
	}
	return result;
}

vector<float> g_time_parser(vector<string>& inputstring)
{
	vector<float> output_global_minute;

	for (int k = 0; k < inputstring.size(); k++)
	{
		vector<string> sub_string = split(inputstring[k], "_");

		for (int i = 0; i < sub_string.size(); i++)
		{
			//HHMM
			//012345
			char hh1 = sub_string[i].at(0);
			char hh2 = sub_string[i].at(1);
			char mm1 = sub_string[i].at(2);
			char mm2 = sub_string[i].at(3);

			float hhf1 = ((float)hh1 - 48);
			float hhf2 = ((float)hh2 - 48);
			float mmf1 = ((float)mm1 - 48);
			float mmf2 = ((float)mm2 - 48);

			float hh = hhf1 * 10 * 60 + hhf2 * 60;
			float mm = mmf1 * 10 + mmf2;
			float global_mm_temp = hh + mm;
			output_global_minute.push_back(global_mm_temp);
		}
	}

	return output_global_minute;
} // transform hhmm to minutes 


string g_time_coding(float time_stamp)
{
	int hour = time_stamp / 60;
	int minute = time_stamp - hour * 60;

	ostringstream strm;
	strm.fill('0');
	strm << setw(2) << hour << setw(2) << minute;

	return strm.str();
} // transform hhmm to minutes 


void g_ProgramStop()
{

	cout << "AgentLite Program stops. Press any key to terminate. Thanks!" << endl;
	getchar();
	exit(0);
};



//void ReadLinkTollScenarioFile(Assignment& assignment)
//{
//
//	for (unsigned li = 0; li < g_link_vector.size(); li++)
//	{
//
//		g_link_vector[li].TollMAP.erase(g_link_vector[li].TollMAP.begin(), g_link_vector[li].TollMAP.end()); // remove all previouly read records
//	}
//
//	// generate toll based on demand type code in input_link.csv file
//	int demand_mode_type_count = 0;
//
//	for (unsigned li = 0; li < g_link_vector.size(); li++)
//	{
//		if (g_link_vector[li].agent_type_code.size() >= 1)
//		{  // with data string
//
//			std::string agent_type_code = g_link_vector[li].agent_type_code;
//
//			vector<float> TollRate;
//			for (int at = 0; at < assignment.g_AgentTypeVector.size(); at++)
//			{
//				CString number;
//				number.Format(_T("%d"), at);
//
//				std::string str_number = CString2StdString(number);
//				if (agent_type_code.find(str_number) == std::string::npos)   // do not find this number
//				{
//					g_link_vector[li].TollMAP[at] = 999;
//					demand_mode_type_count++;
//				}
//				else
//				{
//					g_link_vector[li].TollMAP[at] = 0;
//				}
//
//			}  //end of pt
//		}
//	}
//}



int g_ParserIntSequence(std::string str, std::vector<int>& vect)
{

	std::stringstream ss(str);

	int i;

	while (ss >> i)
	{
		vect.push_back(i);

		if (ss.peek() == ';')
			ss.ignore();
	}

	return vect.size();
}

void g_ReadDemandFileBasedOnDemandFileList(Assignment& assignment)
{

	for (int i = 0; i < g_node_vector.size(); i++)
	{

		if (g_node_vector[i].zone_id >= 1 && assignment.g_zoneid_to_zone_seq_no_mapping.find(g_node_vector[i].zone_id) == assignment.g_zoneid_to_zone_seq_no_mapping.end())  // create a new zone  // we assume each zone only has one node
		{ // we need to make sure we only create a zone in the memory if only there is positive demand flow from the (new) OD table
			COZone ozone;
			ozone.node_seq_no = g_node_vector[i].node_seq_no;
			ozone.zone_id = g_node_vector[i].zone_id;
			ozone.zone_seq_no = g_zone_vector.size();
			assignment.g_zoneid_to_zone_seq_no_mapping[ozone.zone_id] = assignment.g_zoneid_to_zone_seq_no_mapping.size();  // create the zone id to zone seq no mapping

			g_zone_vector.push_back(ozone);  // add element into vector
											 //	cout << ozone.zone_id << ' ' << ozone.zone_seq_no << endl;
		}
	}

	cout << "number of zones = " << g_zone_vector.size() << endl;
//	fprintf(g_pFileOutputLog, "number of zones =,%lu\n", g_zone_vector.size());

	assignment.InitializeDemandMatrix(g_zone_vector.size(), assignment.g_AgentTypeVector.size());

	float total_demand_in_demand_file = 0;

	CCSVParser parser;
	cout << "Step 4: Reading file demand_file_list.csv..." << endl;

	if (parser.OpenCSVFile("demand_file_list.csv", true))
	{
		int i = 0;

		while (parser.ReadRecord())
		{

			int file_sequence_no = 1;
			string file_name;
			string format_type = "null";

			string demand_period, agent_type;

			int demand_format_flag = 0;

			if (parser.GetValueByFieldName("file_sequence_no", file_sequence_no) == false)
				break;

			if (file_sequence_no <= -1)  // skip negative sequence no 
				continue;

			parser.GetValueByFieldName("file_name", file_name);

			parser.GetValueByFieldName("demand_period", demand_period);


			parser.GetValueByFieldName("format_type", format_type);
			if (format_type.find("null") != string::npos)  // skip negative sequence no 
			{
				cout << "Please provide format_type in file demand_file_list.csv" << endl;
				g_ProgramStop();
			}


			double total_ratio = 0;

			parser.GetValueByFieldName("agent_type", agent_type);


			int agent_type_no = 0;
			int demand_period_no = 0;

			if (assignment.demand_period_to_seqno_mapping.find(demand_period) != assignment.demand_period_to_seqno_mapping.end())
			{
				demand_period_no = assignment.demand_period_to_seqno_mapping[demand_period];

			}
			else
			{
				cout << "Error: demand period in demand_file_list " << demand_period << "cannot be found." << endl;
				g_ProgramStop();

			}

			if (assignment.agent_type_2_seqno_mapping.find(agent_type) != assignment.agent_type_2_seqno_mapping.end())
			{
				agent_type_no = assignment.agent_type_2_seqno_mapping[agent_type];

			}
			else
			{
				cout << "Error: agent_type in agent_type " << agent_type << "cannot be found." << endl;
				g_ProgramStop();
			}


			if (demand_period_no > _MAX_TIMEPERIODS)
			{
				cout << "demand_period_no should be less than settings in demand_period.csv. Please change the parameter settings in the source code." << endl;
				g_ProgramStop();
			}

			if (format_type.find("column") != string::npos)  // or muliti-column
			{


				bool bFileReady = false;
				
				FILE* st;
				// read the file formaly after the test. 

				fopen_ss(&st, file_name.c_str(), "r");
				if (st != NULL)
				{

					bFileReady = true;
					int line_no = 0;

					while (true)
					{
						int origin_zone = g_read_integer(st, true);

						if (origin_zone <= 0)
						{

							if (line_no == 1 && !feof(st))  // read only one line, but has not reached the end of the line
							{
								cout << endl << "Error: Only one line has been read from file. Are there multiple columns of demand type in file " << file_name << " per line?" << endl;
								g_ProgramStop();

							}
							break;
						}

						if (assignment.g_zoneid_to_zone_seq_no_mapping.find(origin_zone) == assignment.g_zoneid_to_zone_seq_no_mapping.end())
						{
							cout << endl << "Warning: origin zone " << origin_zone << "  has not been defined in input_node.csv" << endl;

							continue; // origin zone  has not been defined, skipped. 
						}

						int destination_zone = g_read_integer(st, true);

						if (assignment.g_zoneid_to_zone_seq_no_mapping.find(destination_zone) == assignment.g_zoneid_to_zone_seq_no_mapping.end())
						{
							cout << endl << "Warning: destination zone " << destination_zone << "  has not been defined in input_node.csv" << endl;

							continue; // destination zone  has not been defined, skipped. 
						}

						int from_zone_seq_no = 0;
						int to_zone_seq_no = 0;
						from_zone_seq_no = assignment.g_zoneid_to_zone_seq_no_mapping[origin_zone];
						to_zone_seq_no = assignment.g_zoneid_to_zone_seq_no_mapping[destination_zone];

						float demand_value = g_read_float(st);

						if (demand_value < -99) // encounter return 
						{
							break;
						}

						assignment.total_demand[agent_type_no][demand_period_no] += demand_value;
						assignment.g_column_pool[from_zone_seq_no][to_zone_seq_no][agent_type_no][demand_period_no].od_volume += demand_value;
						assignment.total_demand_volume += demand_value;
						assignment.g_origin_demand_array[from_zone_seq_no][agent_type_no][demand_period_no] += demand_value;

						// we generate vehicles here for each OD data line
						if (line_no <= 5)  // read only one line, but has not reached the end of the line
							cout << "o_zone_id:" << origin_zone << ", d_zone_id: " << destination_zone << ", value = " << demand_value << endl;

						if (line_no % 100000 == 0)
						{
							cout << "Reading file no." << file_sequence_no << ": " << file_name << " at " << line_no / 1000 << "K lines..." << endl;
						}


						line_no++;
					}  // scan lines


					fclose(st);

					cout << "total_demand_volume is " << assignment.total_demand_volume << endl;
				}
				else  //open file
				{
					cout << "Error: File " << file_name << " cannot be opened.\n It might be currently used and locked by EXCEL." << endl;
					g_ProgramStop();

				}
			}

			else if (format_type.compare("agent_csv") == 0)
			{

				CCSVParser parser;

				if (parser.OpenCSVFile(file_name, false))
				{
					int total_demand_in_demand_file = 0;


					// read agent file line by line,

					int agent_id, o_zone_id, d_zone_id;
					string agent_type, demand_period;
					
					std::vector <int> node_sequence;

					while (parser.ReadRecord())
					{
						total_demand_in_demand_file++;

						if (total_demand_in_demand_file % 1000 == 0)
							cout << "demand_volume is " << total_demand_in_demand_file << endl;

						parser.GetValueByFieldName("agent_id", agent_id);

						parser.GetValueByFieldName("from_zone_id", o_zone_id);
						parser.GetValueByFieldName("to_zone_id", d_zone_id);

						if (assignment.g_zoneid_to_zone_seq_no_mapping.find(o_zone_id) == assignment.g_zoneid_to_zone_seq_no_mapping.end())
						{
							continue; // origin zone  has not been defined, skipped. 
						}

						if (assignment.g_zoneid_to_zone_seq_no_mapping.find(d_zone_id) == assignment.g_zoneid_to_zone_seq_no_mapping.end())
						{
							continue; // origin zone  has not been defined, skipped. 
						}

						float volume = 1.0f;

						parser.GetValueByFieldName("volume", volume);

						std::string path_node_sequence;
						parser.GetValueByFieldName("node_sequence", path_node_sequence);

						std::vector<int> node_id_sequence;

						g_ParserIntSequence(path_node_sequence, node_id_sequence);

						std::vector<int> node_no_sequence;
						std::vector<int> link_no_sequence;

						bool bValid = true;
						int node_sum = 0;
						for (int i = 0; i < node_id_sequence.size(); i++)
						{
							
							if (assignment.g_internal_node_to_seq_no_map.find(node_id_sequence[i]) == assignment.g_internal_node_to_seq_no_map.end())
							{
								bValid = false;
								continue; //has not been defined

								// warning
							}

							int internal_node_seq_no = assignment.g_internal_node_to_seq_no_map[node_id_sequence[i]];  // map external node number to internal node seq no. 

							node_sum += internal_node_seq_no;
							if (i >= 1)
							{ // check if a link exists

								int link_seq_no = -1;
								int prev_node_seq_no = assignment.g_internal_node_to_seq_no_map[node_id_sequence[i-1]];  // map external node number to internal node seq no. 


								if (g_node_vector[prev_node_seq_no].m_to_node_seq_no_map.find(node_no_sequence[i]) != g_node_vector[prev_node_seq_no].m_to_node_seq_no_map.end())
								{
									link_seq_no = g_node_vector[prev_node_seq_no].m_to_node_seq_no_map[node_no_sequence[i]];

									link_no_sequence.push_back(link_seq_no);
								}
								else
								{
									bValid = false;
								}


							}
							node_no_sequence.push_back(internal_node_seq_no);
						}


						if(bValid == true)
						{
						int from_zone_seq_no = 0;
						int to_zone_seq_no = 0;
						from_zone_seq_no = assignment.g_zoneid_to_zone_seq_no_mapping[o_zone_id];
						to_zone_seq_no = assignment.g_zoneid_to_zone_seq_no_mapping[d_zone_id];


						assignment.total_demand[agent_type_no][demand_period_no] += volume;
						assignment.g_column_pool[from_zone_seq_no][to_zone_seq_no][agent_type_no][demand_period_no].od_volume += volume;

						assignment.g_column_pool[from_zone_seq_no][to_zone_seq_no][agent_type_no][demand_period_no].path_node_sequence_map[node_sum].path_node_vector = node_no_sequence;
						assignment.g_column_pool[from_zone_seq_no][to_zone_seq_no][agent_type_no][demand_period_no].path_node_sequence_map[node_sum].path_link_vector = link_no_sequence;

						assignment.total_demand_volume += volume;
						assignment.g_origin_demand_array[from_zone_seq_no][agent_type_no][demand_period_no] += volume;
						}
					}


				}
				else  //open file
				{
					cout << "Error: File " << file_name << " cannot be opened.\n It might be currently used and locked by EXCEL." << endl;
					g_ProgramStop();

				}

			}

			else
			{
				cout << "Error: format_type = " << format_type << " is not supported. Currently DTALite supports multi_column, matrix, full_matrix, dynasmart, agent_csv, agent_bin, trip_csv,transims_trip_file." << endl;
				g_ProgramStop();
			}
		}

	}

}



void g_ReadInputData(Assignment& assignment)
{

	//step 0:read demand period file
	CCSVParser parser_demand_period;
	cout << "Step 1: Reading file demand_period.csv..." << endl;
	//g_LogFile << "Step 7.1: Reading file input_agent_type.csv..." << g_GetUsedMemoryDataInMB() << endl;
	if (!parser_demand_period.OpenCSVFile("demand_period.csv", true))
	{
		cout << "demand_period.csv cannot be opened. " << endl;
		g_ProgramStop();

	}

	if (parser_demand_period.inFile.is_open() || parser_demand_period.OpenCSVFile("demand_period.csv", true))
	{

		while (parser_demand_period.ReadRecord())
		{

			CDemand_Period demand_period;

			if (parser_demand_period.GetValueByFieldName("demand_period_id", demand_period.demand_period_id) == false)
			{
				cout << "Error: Field demand_period_id in file demand_period cannot be read." << endl;
				g_ProgramStop();
				break;
			}

			if (parser_demand_period.GetValueByFieldName("demand_period", demand_period.demand_period) == false)
			{
				cout << "Error: Field demand_period in file demand_period cannot be read." << endl;
				g_ProgramStop();
				break;
			}
			


			vector<float> global_minute_vector;

			if (parser_demand_period.GetValueByFieldName("time_period", demand_period.time_period) == false)
			{ 
				cout << "Error: Field time_period in file demand_period cannot be read." << endl;
				g_ProgramStop();
				break;
			}

			vector<string> input_string;
			input_string.push_back(demand_period.time_period);
			//input_string includes the start and end time of a time period with hhmm format
			global_minute_vector = g_time_parser(input_string); //global_minute_vector incldue the starting and ending time
			if (global_minute_vector.size() == 2)
			{

				demand_period.starting_time_slot_no = global_minute_vector[0] / MIN_PER_TIMESLOT;
				demand_period.ending_time_slot_no = global_minute_vector[1] / MIN_PER_TIMESLOT;

				//cout << global_minute_vector[0] << endl;
				//cout << global_minute_vector[1] << endl;
			}

			assignment.demand_period_to_seqno_mapping[demand_period.demand_period] = assignment.g_DemandPeriodVector.size();

			assignment.g_DemandPeriodVector.push_back(demand_period);


		}
		parser_demand_period.CloseCSVFile();

		if(assignment.g_DemandPeriodVector.size() == 0)
		{
		cout << "Error:  File demand_period.csv has no information." << endl;
		g_ProgramStop();
		}

	}
	else
	{
		cout << "Error: File demand_period.csv cannot be opened.\n It might be currently used and locked by EXCEL." << endl;
		g_ProgramStop();
	}


	assignment.g_number_of_demand_periods = assignment.g_DemandPeriodVector.size();
	//step 1:read demand type file

	cout << "Reading file link_type.csv..." << endl;

	CCSVParser parser_link_type;

	if (parser_link_type.OpenCSVFile("link_type.csv", true))
	{

		int line_no = 0;

		while (parser_link_type.ReadRecord())
		{
			CLinkType element;

			if (parser_link_type.GetValueByFieldName("link_type", element.link_type) == false)
			{
				if (line_no == 0)
				{
					cout << "Error: Field link_type cannot be found in file link_type.csv." << endl;
					g_ProgramStop();
				}
				else
				{  // read empty line
					break;
				}
			}

			if (assignment.g_LinkTypeMap.find(element.link_type) != assignment.g_LinkTypeMap.end())
			{
				cout << "Error: Field link_type " << element.link_type << " has been defined more than once in file link_type.csv." << endl;
				g_ProgramStop();

				break;
			}


			parser_link_type.GetValueByFieldName("agent_type_list", element.agent_type_list);

			assignment.g_LinkTypeMap[element.link_type] = element;

			line_no++;
		}
	}
	else
	{
		cout << "Error: File link_type.csv cannot be opened.\n It might be currently used and locked by EXCEL." << endl;


	}


	CCSVParser parser_agent_type;
	cout << "Step 2: Reading file agent_type.csv..." << endl;
	//g_LogFile << "Step 7.1: Reading file input_agent_type.csv..." << g_GetUsedMemoryDataInMB() << endl;
	if (!parser_agent_type.OpenCSVFile("agent_type.csv", true))
	{
		cout << "agent_type.csv cannot be opened. " << endl;
		g_ProgramStop();

	}

	if (parser_agent_type.inFile.is_open() || parser_agent_type.OpenCSVFile("agent_type.csv", true))
	{
		assignment.g_AgentTypeVector.clear();
		while (parser_agent_type.ReadRecord())
		{

			CAgent_type agent_type;
			agent_type.agent_type_no = assignment.g_AgentTypeVector.size();

			if (parser_agent_type.GetValueByFieldName("agent_type", agent_type.agent_type) == false)
			{
				break;
			}

			parser_agent_type.GetValueByFieldName("VOT", agent_type.value_of_time);

			parser_agent_type.GetValueByFieldName("fixed_flag", agent_type.bFixed);

			float value;
			

			std::map<int, CLinkType>::iterator it;

			// scan through the map with different node sum for different paths
			for (it = assignment.g_LinkTypeMap.begin();
				it != assignment.g_LinkTypeMap.end(); it++)
			{

				char field_name[20];

				sprintf_s(field_name, "PCE_link_type%d", it->first);
				parser_agent_type.GetValueByFieldName(field_name, value,false);

				agent_type.PCE_link_type_map[it->first] = value;

				sprintf_s(field_name, "CRU_link_type%d", it->first);
				parser_agent_type.GetValueByFieldName(field_name, value, false);

				agent_type.CRU_link_type_map[it->first] = value;



			}
	

			assignment.agent_type_2_seqno_mapping[agent_type.agent_type] = assignment.g_AgentTypeVector.size();



			assignment.g_AgentTypeVector.push_back(agent_type);
			assignment.g_number_of_agent_types = assignment.g_AgentTypeVector.size();

		}
		parser_agent_type.CloseCSVFile();

		if (assignment.g_AgentTypeVector.size() == 0 )
		{
			cout << "Error: File agent_type.csv does not contain information." << endl;
		}

	}
	else
	{
		cout << "Error: File agent_type.csv cannot be opened.\n It might be currently used and locked by EXCEL." << endl;
		g_ProgramStop();
	}


	if (assignment.g_AgentTypeVector.size() >= _MAX_AGNETTYPES)
	{
		cout << "Error: agent_type = " << assignment.g_AgentTypeVector.size() << " in file agent_type.csv is too large. " << "_MAX_AGNETTYPES = " << _MAX_AGNETTYPES << "Please contact program developers!";

		g_ProgramStop();
	}



	assignment.g_number_of_nodes = 0;
	assignment.g_number_of_links = 0;  // initialize  the counter to 0

	if (assignment.g_number_of_K_paths > _MAX_K_PATH)
	{
		cout << "g_number_of_K_paths >= _MAX_K_PATH" << endl;
		g_ProgramStop();
	}

	int internal_node_seq_no = 0;
	// step 3: read node file 
	CCSVParser parser;
	if (parser.OpenCSVFile("node.csv", true))
	{

		while (parser.ReadRecord())  // if this line contains [] mark, then we will also read field headers.
		{

			int node_id;

			if (parser.GetValueByFieldName("node_id", node_id) == false)
				continue;

			if (assignment.g_internal_node_to_seq_no_map.find(node_id) != assignment.g_internal_node_to_seq_no_map.end())
			{
				continue; //has been defined
			}
			assignment.g_internal_node_to_seq_no_map[node_id] = internal_node_seq_no;


			CNode node;  // create a node object 

			node.node_id = node_id;
			node.node_seq_no = internal_node_seq_no;
			parser.GetValueByFieldName("zone_id", node.zone_id);



			/*node.x = x;
			node.y = y;*/
			internal_node_seq_no++;

			g_node_vector.push_back(node);  // push it to the global node vector

			assignment.g_number_of_nodes++;
			if (assignment.g_number_of_nodes % 1000 == 0)
				cout << "reading " << assignment.g_number_of_nodes << " nodes.. " << endl;
		}

		cout << "number of nodes = " << assignment.g_number_of_nodes << endl;

	//	fprintf(g_pFileOutputLog, "number of nodes =,%d\n", assignment.g_number_of_nodes);


		parser.CloseCSVFile();
	}


	// step 4: read link file 

	CCSVParser parser_link;

	if (parser_link.OpenCSVFile("road_link.csv", true))
	{
		while (parser_link.ReadRecord())  // if this line contains [] mark, then we will also read field headers.
		{
			int from_node_id;
			int to_node_id;
			if (parser_link.GetValueByFieldName("from_node_id", from_node_id) == false)
				continue;
			if (parser_link.GetValueByFieldName("to_node_id", to_node_id) == false)
				continue;

			int linkID = 0;
			parser_link.GetValueByFieldName("road_link_id", linkID);


			// add the to node id into the outbound (adjacent) node list

			int internal_from_node_seq_no = assignment.g_internal_node_to_seq_no_map[from_node_id];  // map external node number to internal node seq no. 
			int internal_to_node_seq_no = assignment.g_internal_node_to_seq_no_map[to_node_id];

			CLink link;  // create a link object 

			link.from_node_seq_no = internal_from_node_seq_no;
			link.to_node_seq_no = internal_to_node_seq_no;
			link.link_seq_no = assignment.g_number_of_links;
			link.to_node_seq_no = internal_to_node_seq_no;
			link.link_id = linkID;

			parser_link.GetValueByFieldName("facility_type", link.type);
			parser_link.GetValueByFieldName("link_type", link.link_type);
			parser_link.GetValueByFieldName("cost", link.cost);

				float length = 1.0; // km or mile
			float free_speed = 1.0;
			float k_jam = 200;
			parser_link.GetValueByFieldName("length", length);
			parser_link.GetValueByFieldName("free_speed", free_speed);

			int number_of_lanes = 1;
			parser_link.GetValueByFieldName("lanes", number_of_lanes);

			char VDF_field_name[20];

			for (int tau = 0; tau < assignment.g_number_of_demand_periods; tau++)
			{
				int demand_period_id = assignment.g_DemandPeriodVector[tau].demand_period_id;
				sprintf_s (VDF_field_name, "VDF_fftt%d", demand_period_id);
				parser_link.GetValueByFieldName(VDF_field_name, link.VDF_period[tau].FFTT);

				sprintf_s (VDF_field_name, "VDF_cap%d", demand_period_id);
				parser_link.GetValueByFieldName(VDF_field_name, link.VDF_period[tau].capacity);

				sprintf_s (VDF_field_name, "VDF_alpha%d", demand_period_id);
				parser_link.GetValueByFieldName(VDF_field_name, link.VDF_period[tau].alpha);

				sprintf_s (VDF_field_name, "VDF_beta%d", demand_period_id);
				parser_link.GetValueByFieldName(VDF_field_name, link.VDF_period[tau].beta);

				sprintf_s(VDF_field_name, "VDF_theta%d", demand_period_id);
				parser_link.GetValueByFieldName(VDF_field_name, link.VDF_period[tau].theta);

				sprintf_s (VDF_field_name, "VDF_mu%d", demand_period_id);
				parser_link.GetValueByFieldName(VDF_field_name, link.VDF_period[tau].mu);

				sprintf_s (VDF_field_name, "VDF_gamma%d", demand_period_id);
				parser_link.GetValueByFieldName(VDF_field_name, link.VDF_period[tau].gamma);

				sprintf_s(VDF_field_name, "RUC_rho%d", demand_period_id);
				parser_link.GetValueByFieldName(VDF_field_name, link.VDF_period[tau].rho);

				sprintf_s(VDF_field_name, "RUC_resource%d", demand_period_id);
				parser_link.GetValueByFieldName(VDF_field_name, link.VDF_period[tau].ruc_base_resource,false);

				link.VDF_period[tau].starting_time_slot_no = assignment.g_DemandPeriodVector[tau].starting_time_slot_no;
				link.VDF_period[tau].ending_time_slot_no = assignment.g_DemandPeriodVector[tau].ending_time_slot_no;
				link.VDF_period[tau].period = assignment.g_DemandPeriodVector[tau].time_period;


			}

			parser_link.GetValueByFieldName("RUC_type", link.RUC_type);
			

			// for each period

			float default_cap = 1000;
			float default_BaseTT = 1;

			// setup default value
			for (int tau = 0; tau < assignment.g_number_of_demand_periods; tau++)
			{
				link.TDBaseTT[tau] = default_BaseTT;
				link.TDBaseCap[tau] = default_cap;
			}

			//link.m_OutflowNumLanes = number_of_lanes;//visum lane_cap is actually link_cap

			link.link_spatial_capacity = k_jam * number_of_lanes*length;

			link.free_flow_travel_time_in_min = default_BaseTT;

			link.length = length;
			for (int tau = 0; tau < assignment.g_number_of_demand_periods; tau++)
			{
				link.travel_time_per_period[tau] = length / free_speed * 60;
			}
			// min // calculate link cost based length and speed limit // later we should also read link_capacity, calculate delay 

			//int sequential_copying = 0;
			//
			//parser_link.GetValueByFieldName("sequential_copying", sequential_copying);

			g_node_vector[internal_from_node_seq_no].m_outgoing_link_seq_no_vector.push_back(link.link_seq_no);  // add this link to the corresponding node as part of outgoing node/link
			g_node_vector[internal_from_node_seq_no].m_to_node_seq_no_vector .push_back(link.to_node_seq_no);  // add this link to the corresponding node as part of outgoing node/link
			g_node_vector[internal_from_node_seq_no].m_to_node_seq_no_map[link.to_node_seq_no] = link.link_seq_no;  // add this link to the corresponding node as part of outgoing node/link

			g_link_vector.push_back(link);

			assignment.g_number_of_links++;

			if (assignment.g_number_of_links % 1000 == 0)
				cout << "reading " << assignment.g_number_of_links << " links.. " << endl;
		}

		parser_link.CloseCSVFile();
	}
	// we now know the number of links
	cout << "number of links = " << assignment.g_number_of_links << endl;

//	fprintf(g_pFileOutputLog, "number of links =,%d\n", assignment.g_number_of_links);

	parser_link.CloseCSVFile();


};


float total_tree_cost[_MAX_K_PATH];
float total_tree_distance[_MAX_K_PATH];
float total_pi_cost[_MAX_K_PATH];
float total_experienced_cost[_MAX_K_PATH];

float _gap_[_MAX_K_PATH];
float _gap_relative_[_MAX_K_PATH];

void g_reset_link_volume(int number_of_links)
{
	for (int l = 0; l < number_of_links; l++)
	{
		for (int tau = 0; tau < assignment.g_number_of_demand_periods; tau++)
		{
			g_link_vector[l].flow_volume_per_period[tau] = 0;
			g_link_vector[l].queue_length_perslot[tau] = 0;
			g_link_vector[l].resource_per_period[tau] = g_link_vector[l].VDF_period[tau].ruc_base_resource; // base as the reference value

			for (int at = 0; at < _MAX_AGNETTYPES; at++)
			{
				g_link_vector[l].volume_per_period_per_at[tau][at] = 0;
				g_link_vector[l].resource_per_period_per_at[tau][at] = 0;
			}

		}
	}


			for (int at = 0; at < assignment.g_AgentTypeVector.size(); at++)  //m
			{
				if(assignment.g_AgentTypeVector[at].bFixed == 1)  // we take into account the prespecified agent volume into the link volume and link resource in this initialization stage.
				{ 
					for (int o = 0; o < g_zone_vector.size(); o++)  // o
					for (int d = 0; d < g_zone_vector.size(); d++) //d

				for (int tau = 0; tau < assignment.g_DemandPeriodVector.size(); tau++)  //tau
				{
					if (assignment.g_column_pool[o][d][at][tau].od_volume > 0)
					{
						std::map<int, CColumnPath>::iterator it;

						float column_vector_size = assignment.g_column_pool[o][d][at][tau].path_node_sequence_map.size();


						// scan through the map with different node sum for different paths
						for (it = assignment.g_column_pool[o][d][at][tau].path_node_sequence_map.begin(); //k
							it != assignment.g_column_pool[o][d][at][tau].path_node_sequence_map.end(); it++)
						{

							for (int nl = it->second.path_link_vector.size() - 1; nl >= 0; nl--)  // arc a
							{
								int link_seq_no = it->second.path_link_vector[nl];

								float volume = it->second.path_volume;

								float PCE_ratio = assignment.g_AgentTypeVector[at].PCE_link_type_map[g_link_vector[link_seq_no].link_type];
								g_link_vector[link_seq_no].flow_volume_per_period[tau] += volume * PCE_ratio;
								g_link_vector[link_seq_no].volume_per_period_per_at[tau][at] += volume;  // pure volume, not consider PCE


								float CRU = assignment.g_AgentTypeVector[at].CRU_link_type_map[g_link_vector[link_seq_no].link_type];
								g_link_vector[link_seq_no].resource_per_period[tau] += volume * CRU;
								g_link_vector[link_seq_no].resource_per_period_per_at[tau][at] += volume * CRU;


							}
						}
					}
				}
				}

			}

}

void update_link_volume_and_cost()
{

	for (int l = 0; l < g_link_vector.size(); l++)
	{
		//g_link_vector[l].tally_flow_volume_across_all_processors();
		g_link_vector[l].CalculateTD_VDFunction();

		for (int tau = 0; tau < assignment.g_DemandPeriodVector.size(); tau++)
			for (int at = 0; at < assignment.g_AgentTypeVector.size(); at++)
			{

				float PCE_agent_type = assignment.g_AgentTypeVector[at].PCE_link_type_map[g_link_vector[l].link_type];

				g_link_vector[l].calculate_marginal_cost_for_agent_type(tau, at, PCE_agent_type);

				float CRU_agent_type = assignment.g_AgentTypeVector[at].CRU_link_type_map[g_link_vector[l].link_type];

				g_link_vector[l].calculate_penalty_for_agent_type(tau, at, CRU_agent_type);

			}
	}
}
void g_update_gradient_cost_and_assigned_flow_in_column_pool(Assignment& assignment, int inner_iteration_number)
{
	float total_gap = 0;
	float total_relative_gap = 0;
	float total_gap_count = 0;

	for (int o = 0; o < g_zone_vector.size(); o++)  // o
		for (int d = 0; d < g_zone_vector.size(); d++) //d
			for (int at = 0; at < assignment.g_AgentTypeVector.size(); at++)  //m
				for (int tau = 0; tau < assignment.g_DemandPeriodVector.size(); tau++)  //tau
				{
					if (assignment.g_column_pool[o][d][at][tau].od_volume > 0)
					{
						std::map<int, CColumnPath>::iterator it;

						int column_vector_size = assignment.g_column_pool[o][d][at][tau].path_node_sequence_map.size();


						// scan through the map with different node sum for different paths
						/// step 1: update gradient cost for each column path
						//if (o = 7 && d == 15)
						//{

						//	if (assignment.g_pFileDebugLog != NULL)
						//		fprintf(assignment.g_pFileDebugLog, "CU: iteration %d: total_gap=, %f,total_relative_gap,%f,\n", inner_iteration_number, total_gap, total_gap / max(0.00001, total_gap_count));
						//}
						float least_gradient_cost = 999999;
						int least_gradient_cost_path_seq_no = -1;
						int least_gradient_cost_path_node_sum_index = -1;
						int path_seq_count = 0;
						for (it = assignment.g_column_pool[o][d][at][tau].path_node_sequence_map.begin(); //k
							it != assignment.g_column_pool[o][d][at][tau].path_node_sequence_map.end(); it++)
						{


							float path_cost = 0;
							float path_gradient_cost = 0;
							float path_distance = 0;
							float path_travel_time = 0;

							for (int nl = it->second.path_link_vector.size() - 1; nl >= 0; nl--)  // arc a
							{
								int link_seq_no = it->second.path_link_vector[nl];
								path_cost += g_link_vector[link_seq_no].cost;
								path_distance += g_link_vector[link_seq_no].length;
								float link_travel_time = g_link_vector[link_seq_no].travel_time_per_period[tau];
								path_travel_time += link_travel_time;

								path_gradient_cost += g_link_vector[link_seq_no].get_generalized_first_order_gradient_cost_for_agent_type(tau, at);
							}


							it->second.path_cost = path_cost;
							it->second.path_travel_time = path_travel_time;
							it->second.path_gradient_cost = path_gradient_cost;

							if (column_vector_size == 1)  // only one path
							{
								total_gap_count += (it->second.path_gradient_cost * it->second.path_volume);
								break;
							}


							if (path_gradient_cost < least_gradient_cost)
							{
								least_gradient_cost = path_gradient_cost;
								least_gradient_cost_path_seq_no = it->second.path_seq_no;
								least_gradient_cost_path_node_sum_index = it->first;
							}


						}


						if (column_vector_size >= 2)  
						{


						// step 2: calculate gradient cost difference for each column path
						float total_switched_out_path_volume = 0;
						for (it = assignment.g_column_pool[o][d][at][tau].path_node_sequence_map.begin(); //m
							it != assignment.g_column_pool[o][d][at][tau].path_node_sequence_map.end(); it++)
						{
							if (it->second.path_seq_no != least_gradient_cost_path_seq_no)  //for non-least cost path
							{
							
							it->second.path_gradient_cost_difference = it->second.path_gradient_cost - least_gradient_cost;
							it->second.path_gradient_cost_relative_difference = it->second.path_gradient_cost_difference / max(0.0001, least_gradient_cost);

							total_gap += (it->second.path_gradient_cost_difference * it->second.path_volume);
							total_gap_count+= (it->second.path_gradient_cost * it->second.path_volume);

							float step_size = 1.0/(inner_iteration_number+2) * assignment.g_column_pool[o][d][at][tau].od_volume;
								
							float previous_path_volume = it->second.path_volume;
							
							 //recall that it->second.path_gradient_cost_difference >=0 
							it->second.path_volume = max(0, it->second.path_volume - step_size * it->second.path_gradient_cost_relative_difference);

							 //we use min(step_size to ensure a path is not switching more than 1/n proportion of flow 
							it->second.path_switch_volume = (previous_path_volume - it->second.path_volume);
							total_switched_out_path_volume +=  (previous_path_volume - it->second.path_volume);
							
							}

						}

						//consider least cost path
						if (least_gradient_cost_path_seq_no != -1)
						{
							assignment.g_column_pool[o][d][at][tau].path_node_sequence_map[least_gradient_cost_path_node_sum_index].path_volume += total_switched_out_path_volume;
							total_gap_count += (assignment.g_column_pool[o][d][at][tau].path_node_sequence_map[least_gradient_cost_path_node_sum_index].path_gradient_cost * 
								assignment.g_column_pool[o][d][at][tau].path_node_sequence_map[least_gradient_cost_path_node_sum_index].path_volume);
						}
						
						}

					
					}

				}

	if(assignment.g_pFileDebugLog != NULL)
	fprintf(assignment.g_pFileDebugLog, "CU: iteration %d: total_gap=, %f,total_relative_gap=, %f,\n", inner_iteration_number, total_gap, total_gap / max(0.0001, total_gap_count));
		
	// update volume based travel time, and update volume based resource balance, update gradie
	g_reset_link_volume(g_link_vector.size());  // we can have a recursive formulat to reupdate the current link volume by a factor of k/(k+1), and use the newly generated path flow to add the additional 1/(k+1)


	// step 3: given assigned path flow volume, calculate link volume
			for (int at = 0; at < assignment.g_AgentTypeVector.size(); at++)  //m
				if (assignment.g_AgentTypeVector[at].bFixed == 0)  // we only take into account the MAS generated agent volume into the link volume and link resource in this second optimization stage.
				{
					for (int o = 0; o < g_zone_vector.size(); o++)  // o
						for (int d = 0; d < g_zone_vector.size(); d++) //d

							for (int tau = 0; tau < assignment.g_DemandPeriodVector.size(); tau++)  //tau
							{
								if (assignment.g_column_pool[o][d][at][tau].od_volume > 0)
								{
									std::map<int, CColumnPath>::iterator it;

									float column_vector_size = assignment.g_column_pool[o][d][at][tau].path_node_sequence_map.size();

									// scan through the map with different node sum for different paths
									for (it = assignment.g_column_pool[o][d][at][tau].path_node_sequence_map.begin(); //k
										it != assignment.g_column_pool[o][d][at][tau].path_node_sequence_map.end(); it++)
									{

										for (int nl = it->second.path_link_vector.size() - 1; nl >= 0; nl--)  // arc a
										{
											int link_seq_no = it->second.path_link_vector[nl];

											float volume = it->second.path_volume;

											float PCE_ratio = assignment.g_AgentTypeVector[at].PCE_link_type_map[g_link_vector[link_seq_no].link_type];
											g_link_vector[link_seq_no].flow_volume_per_period[tau] += volume * PCE_ratio;
											g_link_vector[link_seq_no].volume_per_period_per_at[tau][at] += volume;  // pure volume, not consider PCE


											float CRU = assignment.g_AgentTypeVector[at].CRU_link_type_map[g_link_vector[link_seq_no].link_type];
											g_link_vector[link_seq_no].resource_per_period[tau] += volume * CRU;
											g_link_vector[link_seq_no].resource_per_period_per_at[tau][at] += volume * CRU;



										}
									}
								}
							}
				}


	update_link_volume_and_cost();  // initialization at the first iteration of shortest path

}


void g_column_pool_optimization(Assignment& assignment, int column_updating_iterations)
{

	for (int n = 0; n < column_updating_iterations; n++)
	{ 
		g_update_gradient_cost_and_assigned_flow_in_column_pool(assignment, n);
	}
}

void g_output_simulation_result(Assignment& assignment)
{


	int b_debug_detail_flag = 0;
	FILE* g_pFileLinkMOE = NULL;
	fopen_ss(&g_pFileLinkMOE,"link_performance.csv", "w");
	if (g_pFileLinkMOE == NULL)
	{
		cout << "File link_performance.csv cannot be opened." << endl;
		g_ProgramStop();
	}
	else
	{
		// Option 2: BPR-X function
		fprintf(g_pFileLinkMOE, "road_link_id,from_node_id,to_node_id,time_period,volume,travel_time,speed,VOC,");

		for (int at = 0; at < assignment.g_AgentTypeVector.size(); at++)
		{
			fprintf(g_pFileLinkMOE, "volume_at_%s,", assignment.g_AgentTypeVector[at].agent_type.c_str());
		}

		fprintf(g_pFileLinkMOE, "resource_balance,");

		fprintf(g_pFileLinkMOE, "notes\n");
		

		for (int l = 0; l < g_link_vector.size(); l++) //Initialization for all nodes
		{
			for (int tau = 0; tau < assignment.g_number_of_demand_periods; tau++)
			{
				float speed = g_link_vector[l].length / (max(0.001,g_link_vector[l].VDF_period[tau].avg_travel_time )/ 60.0);
				fprintf(g_pFileLinkMOE, "%d,%d,%d,%s,%.3f,%.3f,%.3f,%.3f,",
					g_link_vector[l].link_id,

					g_node_vector[g_link_vector[l].from_node_seq_no].node_id,
					g_node_vector[g_link_vector[l].to_node_seq_no].node_id,

					g_link_vector[l].VDF_period[tau].period.c_str(),
					g_link_vector[l].flow_volume_per_period[tau],
					g_link_vector[l].VDF_period[tau].avg_travel_time,
					speed,  /* 60.0 is used to convert min to hour */
					g_link_vector[l].VDF_period[tau].VOC);


				for (int at = 0; at < assignment.g_AgentTypeVector.size(); at++)
				{ 
					fprintf(g_pFileLinkMOE, "%.3f,", g_link_vector[l].volume_per_period_per_at[tau][at]);
				}

				fprintf(g_pFileLinkMOE, "%.3f,", g_link_vector[l].resource_per_period[tau]);

			}



				fprintf(g_pFileLinkMOE, "period-based\n");
		}

		//for (int l = 0; l < g_link_vector.size(); l++) //Initialization for all nodes
		//{
		//	for (int tau = 0; tau < assignment.g_number_of_demand_periods; tau++)
		//	{
		//		
		//			int starting_time = g_link_vector[l].VDF_period[tau].starting_time_slot_no;
		//			int ending_time = g_link_vector[l].VDF_period[tau].ending_time_slot_no;

		//			for (int t = 0; t <= ending_time - starting_time; t++)
		//			{
		//				fprintf(g_pFileLinkMOE, "%s,%s,%s,%d,%.3f,%.3f,%.3f,%.3f,%s\n",

		//					g_link_vector[l].link_id.c_str(),
		//					g_node_vector[g_link_vector[l].from_node_seq_no].node_id.c_str(),
		//					g_node_vector[g_link_vector[l].to_node_seq_no].node_id.c_str(),
		//					t,
		//					g_link_vector[l].VDF_period[tau].discharge_rate[t],
		//					g_link_vector[l].VDF_period[tau].travel_time[t],
		//					g_link_vector[l].length / g_link_vector[l].VDF_period[tau].travel_time[t] * 60.0,
		//					g_link_vector[l].VDF_period[tau].congestion_period_P,
		//					"timeslot-dependent");
		//			}

		//		}

		//}

		


	fclose(g_pFileLinkMOE);
	}

	//added by zhuge,to output the ODMOE 
	FILE* g_pFileODMOE = NULL;
	fopen_ss(&g_pFileODMOE,"agent.csv", "w");
	if (g_pFileODMOE == NULL)
	{
		cout << "File agent.csv cannot be opened." << endl;
		g_ProgramStop();
	}
	else
	{
		fprintf(g_pFileODMOE, "agent_id,o_zone_id,d_zone_id,path_id,o_node_id,d_node_id,agent_type,demand_period,volume,cost,travel_time,distance,diff,relative_diff,switched_volume,node_sequence,link_sequence,time_sequence,time_decimal_sequence,\n");

		int count = 1;
		for (int o = 0; o < g_zone_vector.size(); o++)
			for (int d = 0; d < g_zone_vector.size(); d++)
				for (int at = 0; at < assignment.g_AgentTypeVector.size(); at++)
					for (int tau = 0; tau < assignment.g_DemandPeriodVector.size(); tau++)
					{

						if (assignment.g_column_pool[o][d][at][tau].od_volume > 0)
						{
							std::map<int, CColumnPath>::iterator it;

							// scan through the map with different node sum for different paths
							for (it = assignment.g_column_pool[o][d][at][tau].path_node_sequence_map.begin();
								it != assignment.g_column_pool[o][d][at][tau].path_node_sequence_map.end(); it++)
							{

								float path_cost = 0;
								float path_distance = 0;
								float path_travel_time = 0;

								vector<float> path_time_vector;

								float time_stamp = (assignment.g_DemandPeriodVector[tau].starting_time_slot_no + assignment.g_DemandPeriodVector[tau].ending_time_slot_no) / 2.0 * MIN_PER_TIMESLOT;

								path_time_vector.push_back(time_stamp);

								for (int nl = it->second.path_link_vector.size() - 1; nl >= 0; nl--)  //backward
								{
									int link_seq_no = it->second.path_link_vector[nl];
									path_cost += g_link_vector[link_seq_no].cost;
									path_distance += g_link_vector[link_seq_no].length;
									float link_travel_time = g_link_vector[link_seq_no].travel_time_per_period[tau];
									path_travel_time += link_travel_time;
									time_stamp += link_travel_time;
									path_time_vector.push_back(time_stamp);
								}


								fprintf(g_pFileODMOE, "%d,%d,%d,%d,%d,%d,%s,%s,%.1f,%.1f,%.1f,%.4f,%.4f,%.4f,%.4f,",
									count,
									g_zone_vector[o].zone_id,
									g_zone_vector[d].zone_id,
									it->second.path_seq_no,
									g_node_vector[g_zone_vector[o].node_seq_no].node_id,
									g_node_vector[g_zone_vector[d].node_seq_no].node_id,
									assignment.g_AgentTypeVector[at].agent_type.c_str(),
									assignment.g_DemandPeriodVector[tau].demand_period.c_str(),
									it->second.path_volume ,
									path_cost,
									path_travel_time,
									it->second.path_distance,
									it->second.path_gradient_cost_difference,
									it->second.path_gradient_cost_relative_difference * it->second.path_volume,
									it->second.path_switch_volume);


								for (int ni = it->second.path_node_vector.size() - 1; ni >= 0; ni--)
								{
									fprintf(g_pFileODMOE, "%d;", g_node_vector[it->second.path_node_vector[ni]].node_id);

								}

								fprintf(g_pFileODMOE, ",");

							
								for (int nl = it->second.path_link_vector.size() - 1; nl >= 0; nl--)
								{
									int link_seq_no = it->second.path_link_vector[nl];
									fprintf(g_pFileODMOE, "%d;", g_link_vector[link_seq_no].link_id);

								}
								fprintf(g_pFileODMOE, ",");

								for (int nt = 0; nt<path_time_vector.size(); nt++)
								{
									fprintf(g_pFileODMOE, "%s;", g_time_coding(path_time_vector[nt]).c_str());
								}
								fprintf(g_pFileODMOE, ",");

								for (int nt = 0; nt < path_time_vector.size(); nt++)
								{
									fprintf(g_pFileODMOE, "%.2f;", path_time_vector[nt]);
								}

								fprintf(g_pFileODMOE, "\n");

								count++;
							}

						}
					}
	}
	fclose(g_pFileODMOE);
}

//***
// major function 1:  allocate memory and initialize the data
// void AllocateMemory(int number_of_nodes)
//
//major function 2: // time-dependent label correcting algorithm with double queue implementation
//int optimal_label_correcting(int origin_node, int destination_node, int departure_time, int shortest_path_debugging_flag, FILE* g_pFileDebugLog, Assignment& assignment, int time_period_no = 0, int agent_type = 1, float VOT = 10)

//	//major function: update the cost for each node at each SP tree, using a stack from the origin structure 
//int tree_cost_updating(int origin_node, int departure_time, int shortest_path_debugging_flag, FILE* g_pFileDebugLog, Assignment& assignment, int time_period_no = 0, int agent_type = 1)

//***

// The one and only application object
using namespace std;

int g_number_of_CPU_threads()
{
	int number_of_threads = omp_get_max_threads();

	int max_number_of_threads = 4000;

	if (number_of_threads > max_number_of_threads)
		number_of_threads = max_number_of_threads;

	return number_of_threads;

}


void g_assign_computing_tasks_to_memory_blocks(Assignment& assignment)
{
	//fprintf(g_pFileDebugLog, "-------g_assign_computing_tasks_to_memory_blocks-------\n");
	// step 2: assign node to thread
	for (int k = 0; k < assignment.g_number_of_K_paths; k++)
	{
		for (int i = 0; i < g_node_vector.size(); i++)  //assign all nodes to the corresponding thread
		{
			if (g_node_vector[i].zone_id >= 1)
			{
				int zone_seq_no = assignment.g_zoneid_to_zone_seq_no_mapping[g_node_vector[i].zone_id];


				for (int at = 0; at < assignment.g_AgentTypeVector.size(); at++)
				{

					for (int tau = 0; tau < assignment.g_DemandPeriodVector.size(); tau++)
					{
						if (assignment.g_origin_demand_array[zone_seq_no][at][tau] > 0) // with feasible flow
						{
							//fprintf(g_pFileDebugLog, "%f\n",g_origin_demand_array[zone_seq_no][at][tau]);

								//cout << assignment.g_origin_demand_array[zone_seq_no][at][tau] << endl;

							NetworkForSP* p_NetworkForSP = new NetworkForSP();
							g_NetworkForSP_vector.push_back(p_NetworkForSP);
							p_NetworkForSP->m_iteration_k = k;
							p_NetworkForSP->m_origin_node = i;
							p_NetworkForSP->m_origin_zone_seq_no = zone_seq_no;

							p_NetworkForSP->m_agent_type_no = at;
							p_NetworkForSP->m_demand_time_period_no = tau;
							p_NetworkForSP->AllocateMemory(assignment.g_number_of_nodes);

						}
					}
				}


			}
		}
	}

}





//major function: update the cost for each node at each SP tree, using a stack from the origin structure 

void NetworkForSP::calculate_TD_link_flow(Assignment& assignment, int iteration_number_outterloop)
{
	

	int origin_node = m_origin_node;
	int departure_time = m_demand_time_period_no;

	int agent_type = m_agent_type_no;


	if (m_iteration_k > iteration_number_outterloop)  // we only update available path tree;
		return;

	if (g_node_vector[origin_node].m_outgoing_link_seq_no_vector.size() == 0)
	{
		return;
	}

	// given,  m_node_label_cost[i]; is the gradient cost , for this tree's, from its origin to the destination node i'. 

	//	fprintf(g_pFileDebugLog, "------------START: origin:  %d  ; Departure time: %d  ; demand type:  %d  --------------\n", origin_node + 1, departure_time, agent_type);
	float k_path_prob = float(1) / float(iteration_number_outterloop + 1);  //XZ: use default value as MSA, this is equivalent to 1/(n+1)
	// to do, this is for each nth tree. 

    //change of path flow is a function of cost gap (the updated node cost for the path generated at the previous iteration -m_node_label_cost[i] at the current iteration)
   // current path flow - change of path flow, 
   // new propability for flow staying at this path
   // for current shortest path, collect all the switched path from the other shortest paths for this ODK pair.
   // check demand flow balance constraints 

	int num = 0;
	for (int i = 0;i < assignment.g_number_of_nodes;i++)
	{

		if (g_node_vector[i].zone_id >= 1)
		{
			//			fprintf(g_pFileDebugLog, "--------origin  %d ; destination node: %d ; (zone: %d) -------\n", origin_node + 1, i+1, g_node_vector[i].zone_id);
			//fprintf(g_pFileDebugLog, "--------iteration number outterloop  %d ;  -------\n", iteration_number_outterloop);
			int destination_zone_seq_no = assignment.g_zoneid_to_zone_seq_no_mapping[g_node_vector[i].zone_id];

			float volume = assignment.g_column_pool[m_origin_zone_seq_no][destination_zone_seq_no][agent_type][m_demand_time_period_no].od_volume *k_path_prob;
			// this is contributed path volume from OD flow (O, D, k, per time period

			if (volume > 0.000001)
			{

				float path_travel_time = 0;
				float path_distance = 0;

				vector<int> temp_path_node_vector; //node seq vector for each ODK
				vector<int> temp_path_link_vector; //node seq vector for each ODK
				vector<float> temp_path_link_travel_time_vector; //node seq vector for each ODK

				int current_node_seq_no = i;  // destination node
				int current_link_seq_no = -1;

				// backtrace the sp tree from the destination to the root (at origin) 
				while (current_node_seq_no >= 0 && current_node_seq_no < assignment.g_number_of_nodes)
				{

					temp_path_node_vector.push_back(current_node_seq_no);
					if (current_node_seq_no >= 0 && current_node_seq_no < assignment.g_number_of_nodes)  // this is valid node 
					{
						current_link_seq_no = m_link_predecessor[current_node_seq_no];

						// fetch m_link_predecessor to mark the link volume

						if (current_link_seq_no >= 0 && current_link_seq_no < assignment.g_number_of_links)
						{
							temp_path_link_vector.push_back(current_link_seq_no);

							path_travel_time += g_link_vector[current_link_seq_no].travel_time_per_period[m_demand_time_period_no];

							path_distance += g_link_vector[current_link_seq_no].length;

							//static
							//link_volume[current_link_seq_no] += (destination_k_path_prob[d] * destination_OD_volume[d][assignment.g_AgentTypeVector[i]][tau]);

							float value_before = g_link_vector[current_link_seq_no].flow_volume_per_period[m_demand_time_period_no];

							float PCE_ratio = assignment.g_AgentTypeVector[agent_type].PCE_link_type_map[g_link_vector[current_link_seq_no].link_type];
							g_link_vector[current_link_seq_no].flow_volume_per_period[m_demand_time_period_no] += volume* PCE_ratio;
							int at = assignment.g_AgentTypeVector[agent_type].agent_type_no;
							g_link_vector[current_link_seq_no].volume_per_period_per_at[m_demand_time_period_no][at] += volume;  // pure volume, not consider PCE


							float CRU = assignment.g_AgentTypeVector[agent_type].CRU_link_type_map[g_link_vector[current_link_seq_no].link_type];
							g_link_vector[current_link_seq_no].resource_per_period[m_demand_time_period_no] += volume * CRU;
							g_link_vector[current_link_seq_no].resource_per_period_per_at[m_demand_time_period_no][at] += volume * CRU;


							////								fprintf(g_pFileDebugLog, "\n o_k = %d; t_k=%d; origin  %d ; destination node: %d ; (zone: %d); link (%d,%d, t=%d)---volume = %f + %f = %f", 
							//									iteration_number_outterloop, m_iteration_k, origin_node + 1, i + 1, g_node_vector[i].zone_id,
							//									g_link_vector[current_link_seq_no].from_node_seq_no + 1, g_link_vector[current_link_seq_no].to_node_seq_no + 1, m_demand_time_period_no,
							//									value_before, volume, g_link_vector[current_link_seq_no].flow_volume_per_period[m_demand_time_period_no]);
						}
					}
					current_node_seq_no = m_node_predecessor[current_node_seq_no];  // update node seq no	
				}
				//fprintf(g_pFileDebugLog, "\n");

				// we obtain the cost, time, distance from the last tree-k 

				if (iteration_number_outterloop == assignment.g_number_of_K_paths-1)  // we only use the last newly generated path from the path tree;
				{

					// get node sum
						int node_sum = 0;
						for (int i = 0; i < temp_path_node_vector.size(); i++)
						{
							node_sum += temp_path_node_vector[i];
						}


						if( assignment.g_column_pool[m_origin_zone_seq_no][destination_zone_seq_no][agent_type][m_demand_time_period_no].path_node_sequence_map .find(node_sum) == assignment.g_column_pool[m_origin_zone_seq_no][destination_zone_seq_no][agent_type][m_demand_time_period_no].path_node_sequence_map.end())
							// we cannot find a path with the same node sum, so we need to add this path into the map, 
						{ 
							// add this unique path
							int path_count = assignment.g_column_pool[m_origin_zone_seq_no][destination_zone_seq_no][agent_type][m_demand_time_period_no].path_node_sequence_map.size();
							assignment.g_column_pool[m_origin_zone_seq_no][destination_zone_seq_no][agent_type][m_demand_time_period_no].path_node_sequence_map[node_sum].path_seq_no = path_count;
							assignment.g_column_pool[m_origin_zone_seq_no][destination_zone_seq_no][agent_type][m_demand_time_period_no].path_node_sequence_map[node_sum].path_volume = 0;
							assignment.g_column_pool[m_origin_zone_seq_no][destination_zone_seq_no][agent_type][m_demand_time_period_no].time = m_label_time_array[i];
							assignment.g_column_pool[m_origin_zone_seq_no][destination_zone_seq_no][agent_type][m_demand_time_period_no].path_node_sequence_map[node_sum].path_distance = m_label_distance_array[i];
							assignment.g_column_pool[m_origin_zone_seq_no][destination_zone_seq_no][agent_type][m_demand_time_period_no].path_node_sequence_map[node_sum].path_cost = m_node_label_cost[i];
							assignment.g_column_pool[m_origin_zone_seq_no][destination_zone_seq_no][agent_type][m_demand_time_period_no].path_node_sequence_map[node_sum].path_node_vector = temp_path_node_vector;
							assignment.g_column_pool[m_origin_zone_seq_no][destination_zone_seq_no][agent_type][m_demand_time_period_no].path_node_sequence_map[node_sum].path_link_vector = temp_path_link_vector;


						}

						assignment.g_column_pool[m_origin_zone_seq_no][destination_zone_seq_no][agent_type][m_demand_time_period_no].path_node_sequence_map[node_sum].path_volume += volume;


				}


			}
		}

	}

	//fprintf(g_pFileDebugLog, "-------------END---------------- \n");
}


void  CLink::CalculateTD_VDFunction()
{


	for (int tau = 0; tau < assignment.g_number_of_demand_periods; tau++)
	{
		float starting_time_slot_no = assignment.g_DemandPeriodVector[tau].starting_time_slot_no;
		float ending_time_slot_no = assignment.g_DemandPeriodVector[tau].ending_time_slot_no;
		travel_time_per_period[tau] = VDF_period[tau].PerformBPR(flow_volume_per_period[tau]);
		//travel_time_per_period[tau] = VDF_period[tau].PerformBPR_X(flow_volume_per_period[tau]);

	}
}


double network_assignment(int iteration_number, int assignment_mode, int column_updating_iterations)
{


	//fopen_ss(&g_pFileOutputLog, "output_solution.csv", "w");
	//if (g_pFileOutputLog == NULL)
	//{
	//	cout << "File output_solution.csv cannot be opened." << endl;
	//	g_ProgramStop();
	//}

	if (assignment.g_pFileDebugLog == NULL)
		fopen_ss(&assignment.g_pFileDebugLog, "STALite_log.txt", "w");

		//{
	//	cout << "File output_solution.csv cannot be opened." << endl;
	//	g_ProgramStop();
	//}
	

	assignment.g_number_of_K_paths = iteration_number;
	assignment.assignment_mode = assignment_mode;


	// step 1: read input data of network / demand tables / Toll
	g_ReadInputData(assignment);
	g_ReadDemandFileBasedOnDemandFileList(assignment);

	// definte timestamps
	clock_t start_t, end_t, total_t;

	//step 2: allocate memory and assign computing tasks
	g_assign_computing_tasks_to_memory_blocks(assignment);
	start_t = clock();

	//step 3: find shortest path and update path cost of tree using TD travel time
	for (int iteration_number = 0; iteration_number < assignment.g_number_of_K_paths; iteration_number++)
	{
		//TRACE("Loop 1: assignment iteration %d", iteration_number);
		//step 3: compute K SP tree

		g_reset_link_volume(g_link_vector.size());  // we can have a recursive formulat to reupdate the current link volume by a factor of k/(k+1), and use the newly generated path flow to add the additional 1/(k+1)

		if(iteration_number == 0)
			update_link_volume_and_cost();  // initialization at the first iteration of shortest path

		//g_setup_link_cost_in_each_memory_block(iteration_number, assignment);
#pragma omp parallel for  // step 3: C++ open mp automatically create n threads., each thread has its own computing thread on a cpu core 
		for (int ProcessID = 0; ProcessID < g_NetworkForSP_vector.size(); ProcessID++) //changed by zhuge
		{
			g_NetworkForSP_vector[ProcessID]->optimal_label_correcting(assignment, iteration_number);
		}


		for (int ProcessID = 0; ProcessID < g_NetworkForSP_vector.size(); ProcessID++)
		{
			g_NetworkForSP_vector[ProcessID]->calculate_TD_link_flow(assignment, iteration_number);
		}



		//					cout << "CPU Running Time for SP = " << total_t << " milliseconds" << " " << "k=" << iteration_number << endl;
		//					fprintf(g_pFileDebugLog, "CPU Running Time for SP  = %ld milliseconds\n", total_t);

		////step 3.2: calculate TD link travel time using TD inflow flow and capacity  
		//					start_t_1 = clock();

//#pragma omp parallel for  // step collect all partial link volume to compute link volume across all zones
		update_link_volume_and_cost();  // at the end of shortest path
	}


	g_column_pool_optimization(assignment, column_updating_iterations);

	end_t = clock();
	total_t = (end_t - start_t);

	cout << "Output for assignment with " << assignment.g_number_of_K_paths << " iterations. Done!" << endl;
	cout << "CPU Running Time for assignment= " << total_t/1000.0 << " s" << endl;

	//step 4: output simulation results of the new demand 
	g_output_simulation_result(assignment);

	cout << "CPU Running Time for Reassignment= " << total_t/1000.0 << " s" << endl;

	cout << "free memory.." << endl;
	cout << "done." << endl;

	g_node_vector.clear();

	for (int l = 0; l < g_link_vector.size(); l++)
	{
		g_link_vector[l].free_memory();
	}
	g_link_vector.clear();

	if (assignment.g_pFileDebugLog != NULL)
		fclose(assignment.g_pFileDebugLog);
	getchar();
	return 1;

}
