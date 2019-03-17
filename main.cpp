#include <algorithm>
#include <fstream>
#include <iostream>
#include <vector>
#include <sstream>
#include <iomanip>
#include <map>
#include <set>
#include <cstring>
#include <functional>
#include <typeinfo>
#include <stdlib.h>
#include <string>
#include <queue>
#include <math.h>
using namespace std;

ofstream simout;
pair<map<string, vector<pair<int, int> > >,  map<string, int> > load_data(string Process_name[], double seed, double lambda, double bound, double number_process){
     map<string, vector<pair<int, int> > > burst_io_time_temp;
    map<string, int> arrival_time_temp;
    double min = 0;     
  double max = 0;    
  double sum = 0;     
  double burst_times = 0;
  int count = 0;
  int iterations = 10000000;    
  pair<int, int> temp;
  srand48( seed );
  for ( int i = 0 ; i < iterations ; i++ )
  {
    
    double r = drand48();   
    double x = -log( r ) / lambda;  

    if ( x > bound ) { i--; continue; }
    //if ( i < 20 ) printf( "r is %lf\n", r );
    //if ( i < 20 ) printf( "x is %lf\n", x );
    sum += x;
    if ( i == 0 || x < min ) { min = x; }
    if ( i == 0 || x > max ) { max = x; }
    if (i == 0){
        //printf("arrival time for %s: %lf\n",Process_name[count].c_str(),floor(x));
        //printf( "arrival time: %lf\n", floor(x) );
        arrival_time_temp[Process_name[count]] = floor(x);
    }
    else if (i == 1){
        
        burst_times = trunc(r*100)+1;
        
        //printf( "burstl times: %lf\n", trunc(r*100)+1 );
    }
    else{
        if (i%2 == 0){
            temp.first = ceil(x);
            //printf( "CPU burstl times: %lf ---", ceil(x));
        }
        else if (i%2 == 1){
            temp.second = ceil(x);
            burst_io_time_temp[Process_name[count]].push_back(temp);
            
            //printf( "IO burstl times: %lf\n", ceil(x));
            
        }
        
        if ( i == 2.0*burst_times){
            //printf( "trunc %lf --- %lf", 2 * trunc(r*100), i);
            //printf( "CPU burstl times: %lf --- end\n", ceil(x));
            temp.second = -1;
            burst_io_time_temp[Process_name[count]].push_back(temp);
            i = -1;
            count ++;
            if (count == number_process){
                break; 
            }
        }
    }
  }
  return make_pair(burst_io_time_temp, arrival_time_temp);
      
     
    
}
void SJF_Algorithm(map<string, vector<pair<int, int> > > burst_io_time, map<string, int> arrival_time, double Tcs, double Alpha, double lambda){
    /*for(map<string, vector<pair<int, int> > >::iterator it = burst_io_time.begin(); it != burst_io_time.end(); ++it) //iterate through burst_io_time
   {                                                                                                                    //io time for last burst is -1
      std::cout << it->first << " = "; // keys
      for(unsigned i=0; i<it->second.size(); i++)  // values vec
         std::cout << it->second[i].first << "-" << it->second[i].second << "\n";
      std::cout << std::endl;
   }*/
    
}

void SRT_Algorithm(map<string, vector<pair<int, int> > > burst_io_time, map<string, int> arrival_time, double Tcs, double Alpha, double lambda){
    /*for(map<string, vector<pair<int, int> > >::iterator it = burst_io_time.begin(); it != burst_io_time.end(); ++it) //iterate through burst_io_time
   {                                                                                                                    //io time for last burst is -1
      std::cout << it->first << " = "; // keys
      for(unsigned i=0; i<it->second.size(); i++)  // values vec
         std::cout << it->second[i].first << "-" << it->second[i].second << "\n";
      std::cout << std::endl;
   }*/
    
}
void FCFS_Algorithm(map<string, vector<pair<int, int> > > burst_io_time, map<string, int> arrival_time, double Tcs){
    /*for(map<string, vector<pair<int, int> > >::iterator it = burst_io_time.begin(); it != burst_io_time.end(); ++it) //iterate through burst_io_time
   {                                                                                                                    //io time for last burst is -1
      std::cout << it->first << " = "; // keys
      for(unsigned i=0; i<it->second.size(); i++)  // values vec
         std::cout << it->second[i].first << "-" << it->second[i].second << "\n";
      std::cout << std::endl;
   }*/
    
}
void RR_Algorithm(map<string, vector<pair<int, int> > > burst_io_time, map<string, int> arrival_time, double Tcs, double T_slice, string RRadd){
    /*for(map<string, vector<pair<int, int> > >::iterator it = burst_io_time.begin(); it != burst_io_time.end(); ++it) //iterate through burst_io_time
   {                                                                                                                   //io time for last burst is -1
      std::cout << it->first << " = "; // keys
      for(unsigned i=0; i<it->second.size(); i++)  // values vec
         std::cout << it->second[i].first << "-" << it->second[i].second << "\n";
      std::cout << std::endl;                                              
   }*/
    
}
int main(int argc, char* argv[]){
    map<string, vector<pair<int, int> > > burst_io_time;
    map<string, int> arrival_time;
    double seed;
    double lambda;
    double bound;
    double number_process;
    double Tcs; //time of context switch
    double Alpha;
    double T_slice;
    string RRadd = "end"; 
    string Process_name[] = {"A","B","C","D","E","F","G","H","I","J","K","L","M","N","O","P","Q","R","S","T","U","V","W","X","Y","Z"};
    simout.open("simout.txt");
    if (argc == 9){
        seed = atof(argv[1]);
        lambda = atof(argv[2]);
        bound = atof(argv[3]);
        number_process = atof(argv[4]);
        Tcs = atof(argv[5]);
        Alpha = atof(argv[6]);
        T_slice = atof(argv[7]);
        RRadd = argv[8];
    }
    else if (argc == 8){
        seed = atof(argv[1]);
        lambda = atof(argv[2]);
        bound = atof(argv[3]);
        number_process = atof(argv[4]);
        Tcs = atof(argv[5]);
        Alpha = atof(argv[6]);
        T_slice = atof(argv[7]);        
    }
    else{
        perror( "ERROR: <invalid input>" );
        return EXIT_FAILURE;
    }
pair<map<string, vector<pair<int, int> > >,  map<string, int> > temp;
///////// SJF
temp = load_data(Process_name, seed, lambda, bound, number_process);
SJF_Algorithm(temp.first, temp.second, Tcs, Alpha, lambda);
///////// SRT
temp = load_data(Process_name, seed, lambda, bound, number_process);
SRT_Algorithm(temp.first, temp.second, Tcs, Alpha, lambda);
///////// FCFS
temp = load_data(Process_name, seed, lambda, bound, number_process);
FCFS_Algorithm(temp.first, temp.second, Tcs);
///////// RR
temp = load_data(Process_name, seed, lambda, bound, number_process);
RR_Algorithm(temp.first, temp.second, Tcs, T_slice, RRadd);

    
}
