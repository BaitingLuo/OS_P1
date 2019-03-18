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
#include <queue>
#include <numeric>
using namespace std;

ofstream simout;
typedef pair<string, vector<pair<int, int> > > process;

class CompareDist
{
public:
    bool operator()(pair<int,pair<string, vector<pair<int, int> > > > n1,pair<int,pair<string, vector<pair<int, int> > > > n2) {
        if(n1.first==n2.first) return n1.second.first>n2.second.first;
        return n1.first>n2.first;
    }
};

//print
void print_queue( priority_queue<pair<int, process>, vector<pair<int, process> >, CompareDist >& mypq)
{
    if(mypq.empty()){
        cout<<" <empty>";
        return;
    }
    priority_queue<pair<int, process>, vector<pair<int, process> >, CompareDist > pq = mypq;
    while (!pq.empty())
    {
        cout << " "<<pq.top().second.first;
        pq.pop();
    }
}

pair<map<string, vector<pair<int, int> > >,  map<string, int> > load_data(string Process_name[], double seed, double lambda, double bound, double number_process){
     map<string, vector<pair<int, int> > > burst_io_time_temp;
    map<string, int> arrival_time_temp;
    double min = 0;     
  double max = 0;    
  double sum = 0;     
  double burst_times = 0;
  int count = 0;
  int iterations = 1000000000;    
  pair<int, int> temp;
  srand48( seed );
  for ( int i = 0 ; i < iterations ; i++ )
  {
    
    double r = drand48();   
    double x = -log( r ) / lambda;  

    if ( x > bound && i != 1) { i--; continue; }
    //if ( i < 20 ) printf( "r is %lf\n", r );
    //if ( i < 20 ) printf( "x is %lf\n", x );
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

pair<int, vector<int> > nexteventtime(const map<string, int> & arrival_time, pair<string, int> contextSwitch, pair<string, int> runningProcess,const map<string, int> & blockedIO){
    map<int, vector<int> >mymap;
    for (map<string, int>::const_iterator itr = arrival_time.begin(); itr != arrival_time.end(); itr++) {
        mymap[itr->second].push_back(1);
    }
    if(contextSwitch.second>0 && contextSwitch.second<5000000) mymap[contextSwitch.second].push_back(2);
    if(runningProcess.second>0 && runningProcess.second<5000000) mymap[runningProcess.second].push_back(3);
    for (map<string, int>::const_iterator itr = blockedIO.begin(); itr != blockedIO.end(); itr++) {
        mymap[itr->second].push_back(4);
    }
    int ans_time = mymap.begin()->first;
    vector<int> ans_duplicate = mymap.begin()->second;
    if(ans_duplicate.size()==1) return make_pair(ans_time, ans_duplicate);
    vector<int> ans_type;
    ans_type.push_back(ans_duplicate[0]);
    for (int i = 1; i < ans_duplicate.size(); i++) {
        if(ans_duplicate[i]!=ans_duplicate[i-1]) ans_type.push_back(ans_duplicate[i]);
    }
    return make_pair(ans_time, ans_type);
}

void timePasses(int nextEventtime, map<string, int>& arrival_time, pair<string, int>& contextSwitch, pair<string, int>& runningProcess, map<string, int> & blockedIO){
    for (map<string, int>::iterator itr = arrival_time.begin(); itr != arrival_time.end(); itr++){
        itr->second = itr->second - nextEventtime;
    }
    contextSwitch.second = contextSwitch.second - nextEventtime;
    runningProcess.second = runningProcess.second - nextEventtime;
    for (map<string, int>::iterator itr = blockedIO.begin(); itr != blockedIO.end(); itr++) {
        itr->second = itr->second - nextEventtime;
    }
}
void SJF_Algorithm(map<string, vector<pair<int, int> > > burst_io_time, map<string, int> arrival_time, double Tcs, double Alpha, double lambda){
    vector<float> allBurst;
    for (map<string, vector<pair<int, int> > >::iterator itr = burst_io_time.begin(); itr != burst_io_time.end(); itr++) {
        for (int i = 0; i < itr->second.size(); i++) {
            allBurst.push_back((float)itr->second[i].first);
        }
    }
    float average = accumulate( allBurst.begin(), allBurst.end(), 0.0)/allBurst.size();
    simout<<"Algorithm SJF"<<endl;
    simout<<"-- average CPU burst time: "<<average<<" ms"<<endl;
    priority_queue<pair<int, process>, vector<pair<int, process> >, CompareDist > readyPQ;
    map<string, int> blockedIO;
    pair<string, int> runningProcess;
    pair<string, int> contextSwitch;
    map<string, int> estimatedTime = arrival_time;
    for (map<string, int>::iterator itr =  estimatedTime.begin(); itr != estimatedTime.end(); itr++) {
        itr->second = 1/lambda;
    }
    for (map<string, int>:: iterator itr = arrival_time.begin(); itr != arrival_time.end(); itr++) {
        cout<<"Process "<<itr->first<<" [NEW] (arrival time "<<itr->second<<" ms) "<<burst_io_time[itr->first].size()<<" CPU bursts"<<endl;
    }
    cout<<"time 0ms: Simulator started for SJF [Q <empty>]"<<endl;
    int time = 0;
    while (true) {
        bool contSwi = false;
        bool arriveStartcup = false;
        pair<int, vector<int> > nextEvent = nexteventtime(arrival_time, contextSwitch, runningProcess, blockedIO);
        int nextEventtime = nextEvent.first;
        time = time + nextEventtime;
        timePasses(nextEventtime, arrival_time, contextSwitch, runningProcess, blockedIO);
        vector<int> nextEventtype = nextEvent.second;//1 for a process arriving, 2 for a process starting to use cpu, 3 for a cpu burst ending, 4 for I/O complete
        for (int i = 0; i < nextEventtype.size(); i++) {
            if(nextEventtype[i] == 1){
                if(nextEventtype.size()>1 && nextEventtype[i+1] == 2){
                    arriveStartcup = true;
                }else{
                    for (map<string, int>::iterator itr = arrival_time.begin(); itr != arrival_time.end();) {
                        if(itr -> second == 0){
                            pair<int, process> arrivingProcess;
                            int firstBurst = 1 / lambda;
                            arrivingProcess.first = firstBurst;
                            arrivingProcess.second =make_pair(itr->first,burst_io_time[itr->first]) ;
                            readyPQ.push(arrivingProcess);
                            cout<<"time "<<time<<"ms: Process "<<itr->first<<" (tau "<<firstBurst<<"ms) arrived; added to ready queue [Q";
                            print_queue(readyPQ);
                            cout<<"]"<<endl;
                            if(((contextSwitch.second<0) || contextSwitch.second> 5000000) && ((runningProcess.second<0) || runningProcess.second> 5000000)){
                                pair<int, process> processToadd = readyPQ.top();
                                readyPQ.pop();
                                contextSwitch.second = Tcs/2;
                                contextSwitch.first = processToadd.second.first;
                            }
                            itr = arrival_time.erase(itr);
                        }else{
                            itr++;
                        }
                    }
                }
            }
            else if(nextEventtype[i] == 2){
                if(contextSwitch.first == "pending"){
                    contSwi = true;
                }
                else{
                    runningProcess = contextSwitch;
                    runningProcess.second = burst_io_time[runningProcess.first][0].first;
                    contextSwitch.second = 10000000;
                    cout<<"time "<<time<<"ms: Process "<<runningProcess.first<<" started using the CPU for "<<runningProcess.second<<"ms burst [Q";
                    print_queue(readyPQ);
                    cout<<"]"<<endl;
                }
            }
            else if(nextEventtype[i] == 3){
                string processName = runningProcess.first;
                runningProcess.second = 10000000;
                vector<pair<int, int> > BItime = burst_io_time[processName];
                pair<string, int> addIO;
                addIO.first = processName;
                addIO.second = BItime[0].second + Tcs/2;
                if(BItime.size()==1){
                    cout<<"time "<<time<<"ms: Process "<<processName<<" terminated [Q";
                    print_queue(readyPQ);
                    cout<<"]"<<endl;
                    burst_io_time.erase(processName);
                }else{
                    blockedIO.insert(addIO);
                    int tau = ceil(Alpha *  BItime[0].first+ (1-Alpha) * estimatedTime[processName]);
                    estimatedTime[processName] = tau;
                    burst_io_time[processName].erase(burst_io_time[processName].begin());
                    if(burst_io_time[processName].size() == 1){
                        cout<<"time "<<time<<"ms: Process "<<processName<<" completed a CPU burst; 1 burst to go [Q";
                        print_queue(readyPQ);
                        cout<<"]"<<endl;
                    }else{
                        cout<<"time "<<time<<"ms: Process "<<processName<<" completed a CPU burst; "<<burst_io_time[processName].size()<<" bursts to go [Q";
                        print_queue(readyPQ);
                        cout<<"]"<<endl;
                    }
                    cout<<"time "<<time<<"ms: Recalculated tau = "<<tau<<"ms for process "<<processName<<" [Q";
                    print_queue(readyPQ);
                    cout<<"]"<<endl;
                    cout<<"time "<<time<<"ms: Process "<<processName<<" switching out of CPU; will block on I/O until time "<<time+BItime[0].second+Tcs/2<<"ms [Q";
                    print_queue(readyPQ);
                    cout<<"]"<<endl;
                }
                if(!readyPQ.empty()&&i == nextEventtype.size()-1){
                    contextSwitch.first = "pending";
                    contextSwitch.second = Tcs/2;
                }
            }
            else if(nextEventtype[i] == 4){
                for (map<string, int>::iterator itr = blockedIO.begin(); itr != blockedIO.end();) {
                    if(itr->second == 0){
                        string processName = itr->first;
                        int tau = estimatedTime[processName];
                        process processToadd = make_pair(processName, burst_io_time[itr->first]);
                        readyPQ.push(make_pair(tau, processToadd));
                        cout<<"time "<<time<<"ms: Process "<<processName<<" (tau "<<tau<<"ms) completed I/O; added to ready queue [Q";
                        print_queue(readyPQ);
                        cout<<"]"<<endl;
                        itr = blockedIO.erase(itr);
                    }else{
                        itr++;
                    }
                    
                }
                if(contextSwitch.second>5000000 && runningProcess.second>5000000){
                    pair<int, process> processToadd = readyPQ.top();
                    if(i>0 && nextEventtype[i-1] == 3){
                        contextSwitch.second = Tcs/2;
                        contextSwitch.first = "pending";
                        
                    }
                    else {
                        contextSwitch.second = Tcs/2;
                        readyPQ.pop();
                        contextSwitch.first = processToadd.second.first;
                    }
                    
                }
            }
        }
        if(contSwi){
            pair<int, process> toAddpair = readyPQ.top();
            process toAdd = toAddpair.second;
            readyPQ.pop();
            contextSwitch.first = toAdd.first;
            contextSwitch.second = Tcs/2;
            contSwi = false;
        }
        if(arriveStartcup){
            arriveStartcup = false;
            for (map<string, int>::iterator itr = arrival_time.begin(); itr != arrival_time.end();) {
                if(itr -> second == 0){
                    pair<int, process> arrivingProcess;
                    int firstBurst = 1 / lambda;
                    arrivingProcess.first = firstBurst;
                    arrivingProcess.second =make_pair(itr->first,burst_io_time[itr->first]) ;
                    readyPQ.push(arrivingProcess);
                    cout<<"time "<<time<<"ms: Process "<<itr->first<<" (tau "<<firstBurst<<"ms) arrived; added to ready queue [Q";
                    print_queue(readyPQ);
                    cout<<"]"<<endl;
                    if(((contextSwitch.second<0) || contextSwitch.second> 5000000) && ((runningProcess.second<0) || runningProcess.second> 5000000)){
                        pair<int, process> processToadd = readyPQ.top();
                        readyPQ.pop();
                        contextSwitch.second = Tcs/2;
                        contextSwitch.first = processToadd.second.first;
                    }
                    itr = arrival_time.erase(itr);
                }else{
                    itr++;
                }
            }
        }
        if(readyPQ.empty() && blockedIO.empty() && runningProcess.second>5000000 && contextSwitch.second>5000000)
        {
            break;
        }
    }

    cout<<"time "<<time+Tcs/2<<"ms: Simulator ended for SJF [Q <empty>]";
    return;
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
