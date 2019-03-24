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
#include <assert.h>
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
  //  double min = 0;
  //double max = 0;
  //double sum = 0;
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
    for (unsigned i = 1; i < ans_duplicate.size(); i++) {
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

void updateStarttime(map<string, vector<pair<int, int> > >& waitTime, string processName, int time){
    waitTime[processName].push_back(make_pair(time, 0));
}

void updateEndtime(map<string, vector<pair<int, int> > >& waitTime, string processName, int time, double Tcs){
    waitTime[processName][waitTime[processName].size()-1].second = time - Tcs/2;
}
void SJF_Algorithm(map<string, vector<pair<int, int> > > burst_io_time, map<string, int> arrival_time, double Tcs, double Alpha, double lambda){
    //write to file
    vector<float> allBurst;
    for (map<string, vector<pair<int, int> > >::iterator itr = burst_io_time.begin(); itr != burst_io_time.end(); itr++) {
        for (unsigned i = 0; i < itr->second.size(); i++) {
            allBurst.push_back((float)itr->second[i].first);
        }
    }
    float averageBurst = accumulate( allBurst.begin(), allBurst.end(), 0.0)/allBurst.size();
    simout << fixed << setprecision(3);
    simout<<"Algorithm SJF"<<endl;
    simout<<"-- average CPU burst time: "<<averageBurst<<" ms"<<endl;
    
    priority_queue<pair<int, process>, vector<pair<int, process> >, CompareDist > readyPQ;
    map<string, int> blockedIO;
    pair<string, int> runningProcess;
    pair<string, int> contextSwitch;
    map<string, int> estimatedTime = arrival_time;
    map<string, vector<pair<int, int> > > waitTime;
    int numContext = 0;
    for (map<string, int>::iterator itr =  estimatedTime.begin(); itr != estimatedTime.end(); itr++) {
        itr->second = 1/lambda;
    }
    for (map<string, int>:: iterator itr = arrival_time.begin(); itr != arrival_time.end(); itr++) {
        if(burst_io_time[itr->first].size()==1){
            cout<<"Process "<<itr->first<<" [NEW] (arrival time "<<itr->second<<" ms) "<<burst_io_time[itr->first].size()<<" CPU burst"<<endl;
        }else{
            cout<<"Process "<<itr->first<<" [NEW] (arrival time "<<itr->second<<" ms) "<<burst_io_time[itr->first].size()<<" CPU bursts"<<endl;
        }
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
        for (unsigned i = 0; i < nextEventtype.size(); i++) {
            if(nextEventtype[i] == 1){
                if(nextEventtype.size()>1 && nextEventtype[i+1] == 2){
                    arriveStartcup = true;
                }else{
                    for (map<string, int>::iterator itr = arrival_time.begin(); itr != arrival_time.end();) {
                        if(itr -> second == 0){
                            updateStarttime(waitTime, itr->first, time);
                            pair<int, process> arrivingProcess;
                            int firstBurst = 1 / lambda;
                            arrivingProcess.first = firstBurst;
                            arrivingProcess.second =make_pair(itr->first,burst_io_time[itr->first]) ;
                            readyPQ.push(arrivingProcess);
                            if(time >= 0 && time <= 999){
                                cout<<"time "<<time<<"ms: Process "<<itr->first<<" (tau "<<firstBurst<<"ms) arrived; added to ready queue [Q";
                                print_queue(readyPQ);
                                cout<<"]"<<endl;
                            }
                            
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
                    updateEndtime(waitTime, contextSwitch.first, time, Tcs);
                    runningProcess = contextSwitch;
                    runningProcess.second = burst_io_time[runningProcess.first][0].first;
                    contextSwitch.second = 10000000;
                    if(time >= 0 && time <= 999){
                        cout<<"time "<<time<<"ms: Process "<<runningProcess.first<<" started using the CPU for "<<runningProcess.second<<"ms burst [Q";
                        print_queue(readyPQ);
                        cout<<"]"<<endl;
                    }
                    
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
                        if(time >= 0 && time <= 999){
                            cout<<"time "<<time<<"ms: Process "<<processName<<" completed a CPU burst; 1 burst to go [Q";
                            print_queue(readyPQ);
                            cout<<"]"<<endl;
                        }
                        
                    }else{
                        if(time >= 0 && time <= 999){
                            cout<<"time "<<time<<"ms: Process "<<processName<<" completed a CPU burst; "<<burst_io_time[processName].size()<<" bursts to go [Q";
                            print_queue(readyPQ);
                            cout<<"]"<<endl;
                        }
                        
                    }
                    if(time >= 0 && time <= 999){
                        cout<<"time "<<time<<"ms: Recalculated tau = "<<tau<<"ms for process "<<processName<<" [Q";
                        print_queue(readyPQ);
                        cout<<"]"<<endl;
                        cout<<"time "<<time<<"ms: Process "<<processName<<" switching out of CPU; will block on I/O until time "<<time+BItime[0].second+Tcs/2<<"ms [Q";
                        print_queue(readyPQ);
                        cout<<"]"<<endl;
                    }
                    
                }
                if(!readyPQ.empty()&&i == nextEventtype.size()-1){
                    contextSwitch.first = "pending";
                    contextSwitch.second = Tcs/2;
                }
            }
            else if(nextEventtype[i] == 4){
                for (map<string, int>::iterator itr = blockedIO.begin(); itr != blockedIO.end();) {
                    if(itr->second == 0){
                        updateStarttime(waitTime, itr->first, time);
                        string processName = itr->first;
                        int tau = estimatedTime[processName];
                        process processToadd = make_pair(processName, burst_io_time[itr->first]);
                        readyPQ.push(make_pair(tau, processToadd));
                        if(time >= 0 && time <= 999){
                            cout<<"time "<<time<<"ms: Process "<<processName<<" (tau "<<tau<<"ms) completed I/O; added to ready queue [Q";
                            print_queue(readyPQ);
                            cout<<"]"<<endl;
                        }
                        
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
                    updateStarttime(waitTime, itr->first, time);
                    pair<int, process> arrivingProcess;
                    int firstBurst = 1 / lambda;
                    arrivingProcess.first = firstBurst;
                    arrivingProcess.second =make_pair(itr->first,burst_io_time[itr->first]) ;
                    readyPQ.push(arrivingProcess);
                    if(time >= 0 && time <= 999){
                        cout<<"time "<<time<<"ms: Process "<<itr->first<<" (tau "<<firstBurst<<"ms) arrived; added to ready queue [Q";
                        print_queue(readyPQ);
                        cout<<"]"<<endl;
                    }
                    
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

    cout<<"time "<<time+Tcs/2<<"ms: Simulator ended for SJF [Q <empty>]"<<endl;
    vector<float> tmpWait;
    for (map<string, vector<pair<int, int> > >:: iterator itr = waitTime.begin(); itr != waitTime.end(); itr++) {
        //simout<<itr->first<<endl;
        for (unsigned i = 0; i < itr->second.size(); i++) {
            tmpWait.push_back((float)itr->second[i].second - (float)itr->second[i].first);
            numContext++;
           // simout<<itr->second[i].first<<" "<<itr->second[i].second<<endl;
        }//simout<<endl;
    }
    float averageWaittime = accumulate( tmpWait.begin(), tmpWait.end(), 0.0)/tmpWait.size();
    float averageTtime = averageWaittime + Tcs + averageBurst;
    simout<<"-- average wait time: "<<averageWaittime<<" ms"<<endl;
    simout<<"-- average turnaround time: "<<averageTtime<<" ms"<<endl;
    simout<<"-- total number of context switches: "<<numContext<<endl;
    simout<<"-- total number of preemptions: 0"<<endl;
    return;
}




void SRT_Algorithm(map<string, vector<pair<int, int> > > burst_io_time, map<string, int> arrival_time, double Tcs, double Alpha, double lambda){
    vector<float> allBurst;
    for (map<string, vector<pair<int, int> > >::iterator itr = burst_io_time.begin(); itr != burst_io_time.end(); itr++) {
        for (unsigned i = 0; i < itr->second.size(); i++) {
            allBurst.push_back((float)itr->second[i].first);
        }
    }
    float average = accumulate( allBurst.begin(), allBurst.end(), 0.0)/allBurst.size();
    simout<<"Algorithm SRT"<<endl;
    simout<<"-- average CPU burst time: "<<average<<" ms"<<endl;
    int cpu_burt_total = 0;
    int total_process = 0;
    int numContext = 0;
    for (map<string, vector<pair<int, int> > >::iterator itr = burst_io_time.begin(); itr != burst_io_time.end(); itr++) {
        for (unsigned i = 0; i < itr->second.size(); i++) {
            //simout<<itr->second[i].first<<" "<<itr->second[i].second<<endl;
            cpu_burt_total+=itr->second[i].first;
            total_process++;
            numContext++;
        }
    }
    priority_queue<pair<int, process>, vector<pair<int, process> >, CompareDist > readyPQ;
    map<string, int> blockedIO;
    pair<string, int> runningProcess;
    pair<string, int> contextSwitch;
    map<string, int> estimatedTime = arrival_time;
    map<string, vector<pair<int, int> > > burst_io_time_copy = burst_io_time;
     map<string, vector<pair<int, int> > > waitTime;
    for (map<string, int>::iterator itr =  estimatedTime.begin(); itr != estimatedTime.end(); itr++) {
        itr->second = 1/lambda;
    }
    for (map<string, int>:: iterator itr = arrival_time.begin(); itr != arrival_time.end(); itr++) {
        if(burst_io_time[itr->first].size()==1){
            cout<<"Process "<<itr->first<<" [NEW] (arrival time "<<itr->second<<" ms) "<<burst_io_time[itr->first].size()<<" CPU burst"<<endl;
        }else{
            cout<<"Process "<<itr->first<<" [NEW] (arrival time "<<itr->second<<" ms) "<<burst_io_time[itr->first].size()<<" CPU bursts"<<endl;
        }
    }
    cout<<"time 0ms: Simulator started for SRT [Q <empty>]"<<endl;
    int time = 0;
    bool preemption = false;
    while (true) {
        
        bool contSwi = false;
        bool arriveStartcup = false;
        
        pair<int, vector<int> > nextEvent = nexteventtime(arrival_time, contextSwitch, runningProcess, blockedIO);
        int nextEventtime = nextEvent.first;
        time = time + nextEventtime;
        timePasses(nextEventtime, arrival_time, contextSwitch, runningProcess, blockedIO);
        vector<int> nextEventtype = nextEvent.second;//1 for a process arriving, 2 for a process starting to use cpu, 3 for a cpu burst ending, 4 for I/O complete
        for (unsigned i = 0; i < nextEventtype.size(); i++) {
            if(nextEventtype[i] == 1){
                if(nextEventtype.size()>1 && nextEventtype[i+1] == 2){
                    arriveStartcup = true;
                }else{
                    for (map<string, int>::iterator itr = arrival_time.begin(); itr != arrival_time.end();) {
                        if(itr -> second == 0){
                            updateStarttime(waitTime, itr->first, time);
                            pair<int, process> arrivingProcess;
                            int firstBurst = 1 / lambda;
                            arrivingProcess.first = firstBurst;
                            arrivingProcess.second =make_pair(itr->first,burst_io_time[itr->first]) ;
                            readyPQ.push(arrivingProcess);
                            cout<<"time "<<time<<"ms: Process "<<itr->first<<" (tau "<<firstBurst<<"ms) arrived; added to ready queue [Q";
                            print_queue(readyPQ);
                            cout<<"]"<<endl;
                            //check if preemptions
                            if(runningProcess.first.size()>0 && firstBurst < estimatedTime[runningProcess.first] - (burst_io_time_copy[runningProcess.first][0].first-burst_io_time[runningProcess.first][0].first)){
                                numContext++;
                                contextSwitch.first = "pending";
                                contextSwitch.second = 2;
                                burst_io_time[runningProcess.first][0].first = runningProcess.second;
                                preemption = true;
                                runningProcess.second = 10000000;

                            }
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
                    int expectTime3;
                    process afterProcess;
                    if(runningProcess.first.size()>0 && burst_io_time.find(runningProcess.first)!=burst_io_time.end()){
                        expectTime3 = estimatedTime[runningProcess.first] - (burst_io_time_copy[runningProcess.first][0].first-burst_io_time[runningProcess.first][0].first);
                        afterProcess.first = runningProcess.first;
                        afterProcess.second = burst_io_time[runningProcess.first];
                    }
                    if(preemption){

                        preemption = false;
                        readyPQ.push(make_pair(expectTime3, afterProcess));
                    }
                    string processTime = contextSwitch.first;
                    runningProcess = contextSwitch;
                    runningProcess.second = burst_io_time[runningProcess.first][0].first;
                    contextSwitch.second = 10000000;
                    
                    if(burst_io_time[processTime][0].first == burst_io_time_copy[processTime][0].first){
                        if(time >= 0 && time <= 999){
                            cout<<"time "<<time<<"ms: Process "<<runningProcess.first<<" started using the CPU for "<<runningProcess.second<<"ms burst [Q";
                            print_queue(readyPQ);
                            cout<<"]"<<endl;
                        }
                    }else{
                        if(time >= 0 && time <= 999){
                            cout<<"time "<<time<<"ms: Process "<<runningProcess.first<<" started using the CPU with "<<runningProcess.second<<"ms remaining [Q";
                            print_queue(readyPQ);
                            cout<<"]"<<endl;
                        }
                    }
                    
                    pair<int, process> top_process = readyPQ.top();

                    int expectTime1 = estimatedTime[top_process.second.first] - (burst_io_time_copy[top_process.second.first][0].first-burst_io_time[top_process.second.first][0].first);
                    int expectTime2 = estimatedTime[runningProcess.first] - (burst_io_time_copy[runningProcess.first][0].first-burst_io_time[runningProcess.first][0].first);
                    if(expectTime1 < expectTime2 && !readyPQ.empty()){
                        if(time >= 0 && time <= 999){
                            cout<<"time "<<time<<"ms: Process "<<top_process.second.first<<" (tau "<<expectTime1<<"ms) will preempt "<<runningProcess.first<<" [Q";
                            print_queue(readyPQ);
                            cout<<"]"<<endl;
                        }
                        
                        contextSwitch.first = "pending";
                        contextSwitch.second = 2;
                        preemption = true;
                        numContext++;
                        runningProcess.second = 10000000;
                        //readyPQ.push(make_pair(expectTime2, make_pair(runningProcess.first, burst_io_time[runningProcess.first])));
                    }
                    
                }

            }
            else if(nextEventtype[i] == 3){
                if(preemption) {
                    cout<<"here?"<<endl;
                    runningProcess.second = Tcs * 10;
                    continue;
                }
                updateEndtime(waitTime, runningProcess.first, time, Tcs);
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
                    burst_io_time_copy.erase(processName);
                }else{
                    blockedIO.insert(addIO);
                    int tau = ceil(Alpha *  burst_io_time_copy[processName][0].first+ (1-Alpha) * estimatedTime[processName]);
                    estimatedTime[processName] = tau;
                    burst_io_time[processName].erase(burst_io_time[processName].begin());
                    burst_io_time_copy[processName].erase(burst_io_time_copy[processName].begin());
                    if(burst_io_time[processName].size() == 1){
                        if(time >= 0 && time <= 999){
                            cout<<"time "<<time<<"ms: Process "<<processName<<" completed a CPU burst; 1 burst to go [Q";
                            print_queue(readyPQ);
                            cout<<"]"<<endl;
                        }
                        
                    }else{
                        if(time >= 0 && time <= 999){
                            cout<<"time "<<time<<"ms: Process "<<processName<<" completed a CPU burst; "<<burst_io_time[processName].size()<<" bursts to go [Q";
                            print_queue(readyPQ);
                            cout<<"]"<<endl;
                        }
                        
                    }
                    if(time >= 0 && time <= 999){
                        cout<<"time "<<time<<"ms: Recalculated tau = "<<tau<<"ms for process "<<processName<<" [Q";
                        print_queue(readyPQ);
                        cout<<"]"<<endl;
                        cout<<"time "<<time<<"ms: Process "<<processName<<" switching out of CPU; will block on I/O until time "<<time+BItime[0].second+Tcs/2<<"ms [Q";
                        print_queue(readyPQ);
                        cout<<"]"<<endl;
                    }
                    
                }
                if(!readyPQ.empty()&&i == nextEventtype.size()-1){
                    contextSwitch.first = "pending";
                    contextSwitch.second = Tcs/2;
                }
            }
            else if(nextEventtype[i] == 4){
                for (map<string, int>::iterator itr = blockedIO.begin(); itr != blockedIO.end();) {
                    if(itr->second == 0){
                        updateStarttime(waitTime, itr->first, time);
                        string processName = itr->first;
                        int tau = estimatedTime[processName];
                        process processToadd = make_pair(processName, burst_io_time[itr->first]);
                        int expectTime2 = -1;
                        if(contextSwitch.second > 5000000 && burst_io_time_copy.find(runningProcess.first) != burst_io_time_copy.end()){
                            expectTime2 = estimatedTime[runningProcess.first] - (burst_io_time_copy[runningProcess.first][0].first-runningProcess.second);
                        }else if(contextSwitch.second < 5000000 &&burst_io_time_copy.size()>1){
                            pair<int, process> top_process = readyPQ.top();
                            expectTime2= estimatedTime[top_process.second.first] - (burst_io_time_copy[top_process.second.first][0].first-burst_io_time[top_process.second.first][0].first);
                        }
                        if(tau < expectTime2 && runningProcess.second<5000000 && contextSwitch.second > 5000000){
                            numContext++;
                            contextSwitch.first = "pending";
                            contextSwitch.second = 2;
                            burst_io_time[runningProcess.first][0].first = runningProcess.second;
                            readyPQ.push(make_pair(tau, processToadd));
                            if(time >= 0 && time <= 999){
                                cout<<"time "<<time<<"ms: Process "<<processName<<" (tau "<<tau<<"ms) completed I/O and will preempt "<<runningProcess.first<<" [Q";
                                print_queue(readyPQ);
                                cout<<"]"<<endl;
                            }
                            
                            runningProcess.second = 10000000;
                            
                            preemption = true;
                        }else{
                            readyPQ.push(make_pair(tau, processToadd));
                            if(time >= 0 && time <= 999){
                                cout<<"time "<<time<<"ms: Process "<<processName<<" (tau "<<tau<<"ms) completed I/O; added to ready queue [Q";
                                print_queue(readyPQ);
                                cout<<"]"<<endl;
                            }
                            
                        }
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
                    updateStarttime(waitTime, itr->first, time);
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
    int process_total_time = 0;
    for (map<string, vector<pair<int, int> > >:: iterator itr = waitTime.begin(); itr != waitTime.end(); itr++) {
        for (unsigned i = 0; i < itr->second.size(); i++) {
            process_total_time+=itr->second[i].second-itr->second[i].first;
        }
    }
    float ttime;
    ttime = (process_total_time-cpu_burt_total)/(float)total_process + average + Tcs;
    int numpreemptions = numContext-total_process;
    float wtime = ttime - average - Tcs - numpreemptions*1.5*Tcs/total_process;
    simout<<"-- average wait time: "<<wtime<<" ms"<<endl;
    simout<<"-- average turnaround time: "<<ttime<<" ms"<<endl;
    simout<<"-- total number of context switches: "<<numContext<<endl;
    simout<<"-- total number of preemptions: "<<numpreemptions<<endl;
   /* simout<<"process_total_time "<<process_total_time<<endl;
    simout<<"cpu_burt_total "<<cpu_burt_total<<endl;
    simout<<"total_process "<<total_process<<endl;
    
    simout<<"-- total number of context switches: "<<(process_total_time-cpu_burt_total)/(float)total_process<<endl;*/
    cout<<"time "<<time+Tcs/2<<"ms: Simulator ended for SRT [Q <empty>]"<<endl;
    return;
    
}

void output_readyqueue(std::vector<std::string> ready_queue){
  std::cout << " [Q";
  if(ready_queue.size() == 0){
    std::cout << " <empty>]" << std::endl;
  }
  else{
    for(unsigned int i = 0; i < ready_queue.size(); i++){
      std::cout << " " << ready_queue[i];
    }
    std::cout << "]\n";
  }
}




void FCFS_Algorithm(map<string, vector<pair<int, int> > > burst_io_time, map<string, int> arrival_time, double Tcs){

    std::cout << std::endl;
    // caculate the average burst time
    vector<float> allBurst;
    for (map<string, vector<pair<int, int> > >::iterator itr = burst_io_time.begin(); itr != burst_io_time.end(); itr++) {
        for (unsigned i = 0; i < itr->second.size(); i++) {
            allBurst.push_back((float)itr->second[i].first);
        }
    }
    float average = accumulate( allBurst.begin(), allBurst.end(), 0.0)/allBurst.size();
    simout<<"Algorithm FCFS"<<endl;
    simout<<"-- average CPU burst time: "<<average<<" ms"<<endl;

    int cpu_burt_total = 0;
    int total_process = 0;
    int context_switch = 0;
    std::map<std::string, std::vector<int> > begin_end_time;

    for (map<string, vector<pair<int, int> > >::iterator itr = burst_io_time.begin(); itr != burst_io_time.end(); itr++) {
        for (unsigned i = 0; i < itr->second.size(); i++) {
            //simout<<itr->second[i].first<<" "<<itr->second[i].second<<endl;
            cpu_burt_total+=itr->second[i].first;
            total_process++;
        }
    }

    // print the process arrivial time
    pair<std::string, int> begin_process;
    for(map<string, int>::iterator itr = arrival_time.begin(); itr != arrival_time.end(); itr++){
        if(itr == arrival_time.begin()){
            begin_process.first = itr->first;
            begin_process.second = itr->second;
        }
        else{
            if(itr->second < begin_process.second){
                begin_process.first = itr->first;
                begin_process.second = itr->second;
            }
        }

        cout <<"Process "<<itr->first<<" [NEW] (arrival time "<<itr->second<<" ms) "<<burst_io_time[itr->first].size();
        if(burst_io_time[itr->first].size() == 1){
            std::cout <<" CPU burst"<<endl;
        }
        else{
            std::cout <<" CPU bursts"<<endl;
        }
    }
    // print FCFS begin message
    cout <<"time 0ms: Simulator started for FCFS [Q <empty>]" << endl;

    int time = begin_process.second;
    unsigned int num_completed = 0;
    string running_process = begin_process.first;
    std::vector<std::string> ready_queue;
    std::vector<pair<std::string, int> > Blocked_queue;

    // 1: Process arrival time
    // 2: Process starts using the CPU
    // 3: Process finishes using the CPU
    // 4: Process complete the IO
    bool all_Block = false;
    while(num_completed != arrival_time.size()){

        if((ready_queue.size() != 0) || (time == begin_process.second)){

            //pop the first element out of the ready queue
            string running_process;
            if(ready_queue.size() != 0){
                running_process = ready_queue[0];
            }
            else{
                running_process = begin_process.first;
            }

            // ready_queue.erase(ready_queue.begin());

            //save the burst time of CPU and IO
            int CPU_burst = burst_io_time[running_process][0].first;
            int IO_burst = burst_io_time[running_process][0].second;

            std::map<float, std::map<std::string, int> > arranged_time;
            int process_arrive = time;
            int process_leave = time + CPU_burst + Tcs;
           
            // 1. Save arrival time
            for(map<string, int>::iterator itr = arrival_time.begin(); itr != arrival_time.end(); itr++){
                if((itr->second >= process_arrive) && (itr->second < process_leave)){
                    arranged_time[float(itr->second)][itr->first] = 1; 
                }
            }

            // 2. Save time of starting using CPU
            arranged_time[float(process_arrive + 0.5*Tcs)][running_process] = 2;

            arranged_time[float(process_arrive) + float(0.5*Tcs) - 1.5][running_process] = 6;


            // 3. Save time of finishing using CPU
            arranged_time[float(process_arrive + 0.5*Tcs + CPU_burst)][running_process] = 3;

            // 4. Save time of completing IO
            for(unsigned int i = 0; i < Blocked_queue.size(); i++){
                if((Blocked_queue[i].second >= process_arrive) && (Blocked_queue[i].second < process_leave)){
                    arranged_time[float(Blocked_queue[i].second)][Blocked_queue[i].first] = 4;
                }
            }


            for(std::map<float, std::map<std::string, int> >::iterator itr = arranged_time.begin(); itr != arranged_time.end(); itr++){
                for(std::map<std::string, int>::iterator itr2 = (itr->second).begin(); itr2 != (itr->second).end(); itr2++){
                    if(itr2->second == 6){
                        ready_queue.erase(ready_queue.begin());
                    }
                }

                for(std::map<std::string, int>::iterator itr2 = (itr->second).begin(); itr2 != (itr->second).end(); itr2++){
                    if(itr2->second == 2){
                        // assert(ready_queue[0] == itr2->first);
                        // ready_queue.erase(ready_queue.begin());
                        begin_end_time[itr2->first].push_back(itr->first);
                        if(itr->first <= 999){
                            std::cout << "time " << itr->first << "ms: Process " << itr2->first << " started using the CPU for " << CPU_burst << "ms burst"; 
                            output_readyqueue(ready_queue);
                        }
                    }
                }

                for(std::map<std::string, int>::iterator itr2 = (itr->second).begin(); itr2 != (itr->second).end(); itr2++){
                    if(itr2->second == 3){
                        assert(running_process == itr2->first);
                        // complete a CPU burst
                        burst_io_time[running_process].erase(burst_io_time[running_process].begin());
                        if(burst_io_time[itr2->first].size() != 0){
                            if(itr->first <= 999){
                                std::cout << "time " << itr->first << "ms: Process " << itr2->first << " completed a CPU burst; " << burst_io_time[itr2->first].size();
                                if(burst_io_time[itr2->first].size() == 1){
                                    std::cout << " burst to go";
                                }
                                else{
                                    std::cout << " bursts to go";
                                }
                                output_readyqueue(ready_queue);
                            }
                        }

                        // switch out of CPU
                        if(IO_burst == -1){
                            num_completed += 1;
                            std::cout << "time " << itr->first << "ms: Process " << itr2->first << " terminated";
                            output_readyqueue(ready_queue);
                        }
                        else{
                            Blocked_queue.push_back(std::make_pair(itr2->first, itr->first + IO_burst + 0.5*Tcs));
                            context_switch += 1;
                            if(itr->first <= 999){
                                std::cout << "time " << itr->first << "ms: Process " << itr2->first << " switching out of CPU; will block on I/O until time " << (itr->first + IO_burst + 0.5*Tcs)<< "ms";
                                output_readyqueue(ready_queue);
                            }
                        }
                    }
                }

                for(std::map<std::string, int>::iterator itr2 = (itr->second).begin(); itr2 != (itr->second).end(); itr2++){
                    if(itr2->second == 4){
                        for(std::vector<pair<std::string, int> >::iterator block_itr = Blocked_queue.begin(); block_itr != Blocked_queue.end(); block_itr++){
                            if(block_itr->first == itr2->first){
                                assert(block_itr->second == itr->first);
                                Blocked_queue.erase(block_itr);
                                break;
                            }
                        }

                        if(all_Block != true){
                           ready_queue.push_back(itr2->first);
                        }
                        begin_end_time[itr2->first].push_back(itr->first);

                        if(itr->first <= 999){
                            std::cout << "time " << itr->first << "ms: Process " << itr2->first << " completed I/O; added to ready queue";
                            output_readyqueue(ready_queue); 
                        }
                        all_Block = false;
                    }
                }
                
                for(std::map<std::string, int>::iterator itr2 = (itr->second).begin(); itr2 != (itr->second).end(); itr2++){
                    if(itr2->second == 1){
                        ready_queue.push_back(itr2->first);
                        if(itr->first <= 999){
                            std::cout << "time " << itr->first << "ms: Process " << itr2->first << " arrived; added to ready queue";
                            output_readyqueue(ready_queue);
                        }
                        begin_end_time[itr2->first].push_back(itr->first);

                    }
                }

            }

            time = process_leave;
        }
        else{

            int min_block = numeric_limits<int>::max();
            for(unsigned int i = 0; i < Blocked_queue.size(); i++){
                if(min_block > Blocked_queue[i].second){
                    min_block = Blocked_queue[i].second;
                    running_process = Blocked_queue[i].first;
                }
            }
            time = min_block;
            ready_queue.push_back(running_process);
            all_Block = true;

        }
    }

    std::map<std::string, std::vector<int> >::iterator time_itr;
    float time_total = 0;
    int count = 0;
    for(time_itr = begin_end_time.begin(); time_itr != begin_end_time.end(); time_itr++){
        float time_each = 0;
        std::vector<int> temp_time = time_itr->second;
        sort(temp_time.begin(), temp_time.end());
        for(unsigned int i = 0; i < temp_time.size()-1; i+=2){
            time_each += (float)(temp_time[i+1] - (float)temp_time[i]);
            count++;
        }
        time_total += time_each;
    }
    float averageWaittime = float(time_total)/count - 2;
    float averageTtime = averageWaittime + Tcs + average;
    simout<<"-- average wait time: "<<averageWaittime<<" ms"<<endl;
    simout<<"-- average turnaround time: "<<averageTtime<<" ms"<<endl;
    simout<<"-- total number of context switches: "<<total_process<<endl;
    simout<<"-- total number of preemptions: "<< 0 <<endl;

    // print FCFS end message
    std::cout <<"time " << time << "ms: Simulator ended for FCFS [Q <empty>]" << std::endl; 
}

// bool check_intger(float number){
//     int new_bum = (int)number;
//     return (0 == (number-(float)new_bum));
// }

void RR_Algorithm(map<string, vector<pair<int, int> > > burst_io_time, map<string, int> arrival_time, double Tcs, double T_slice, string RRadd){
    
    std::cout << std::endl;
    // caculate the average burst time
    vector<float> allBurst;
    for (map<string, vector<pair<int, int> > >::iterator itr = burst_io_time.begin(); itr != burst_io_time.end(); itr++) {
        for (unsigned i = 0; i < itr->second.size(); i++) {
            allBurst.push_back((float)itr->second[i].first);
        }
    }
    float average = accumulate( allBurst.begin(), allBurst.end(), 0.0)/allBurst.size();
    simout<<"Algorithm RR"<<endl;
    simout<<"-- average CPU burst time: "<<average<<" ms"<<endl;

    int cpu_burt_total = 0;
    int total_process = 0;
    int preempted = 0;
    std::map<std::string, std::vector<int> > begin_end_time;

    for (map<string, vector<pair<int, int> > >::iterator itr = burst_io_time.begin(); itr != burst_io_time.end(); itr++) {
        for (unsigned i = 0; i < itr->second.size(); i++) {
            //simout<<itr->second[i].first<<" "<<itr->second[i].second<<endl;
            cpu_burt_total+=itr->second[i].first;
            total_process++;
        }
    }

    // print the process arrivial time
    pair<std::string, int> begin_process;
    for(map<string, int>::iterator itr = arrival_time.begin(); itr != arrival_time.end(); itr++){
        if(itr == arrival_time.begin()){
            begin_process.first = itr->first;
            begin_process.second = itr->second;
        }
        else{
            if(itr->second < begin_process.second){
                begin_process.first = itr->first;
                begin_process.second = itr->second;
            }
        }

        cout <<"Process "<<itr->first<<" [NEW] (arrival time "<<itr->second<<" ms) "<<burst_io_time[itr->first].size();
        if(burst_io_time[itr->first].size() == 1){
            std::cout <<" CPU burst"<<endl;
        }
        else{
            std::cout <<" CPU bursts"<<endl;
        }
    }
    // print FCFS begin message
    cout <<"time 0ms: Simulator started for RR [Q <empty>]" << endl;

    int time = begin_process.second;
    unsigned int num_completed = 0;
    string running_process = begin_process.first;
    std::vector<std::string> ready_queue;
    std::vector<pair<std::string, int> > Blocked_queue;

    // 1: Process arrival time
    // 2: Process starts using the CPU
    // 3: Process finishes using the CPU
    // 4: Process complete the IO
    bool all_Block = false;
    std::map<std::string, bool> remaining;
    for(map<string, int>::iterator itr = arrival_time.begin(); itr != arrival_time.end(); itr++){
        remaining[itr->first] = false;
    }
    while(num_completed != arrival_time.size()){
        if((ready_queue.size() != 0) || (time == begin_process.second)){

            //pop the first element out of the ready queue
            string running_process;
            if(ready_queue.size() != 0){
                running_process = ready_queue[0];
            }
            else{
                running_process = begin_process.first;
            }

            //save the burst time of CPU and IO
            int CPU_burst = burst_io_time[running_process][0].first;
            int IO_burst = burst_io_time[running_process][0].second;

            ///////////////////////judge the next step//////////////////////////
            float next = numeric_limits<float>::max();
            float process_arrive = time;
            float process_leave;
            bool preemption;
            std::map<float, std::map<std::string, int> > arranged_time;

            // 7. Save time of preemption
            if(CPU_burst <= T_slice){
                process_leave = time + CPU_burst + Tcs;
                preemption = false;
                arranged_time[float(process_leave - 0.5*Tcs)][running_process] = 3;
            }
            else{
                bool ready_queue_other = false;
                bool Blocked_queue_other = false;
                for(unsigned int i = 0; i < ready_queue.size(); i++){
                    if(ready_queue[i] != running_process){
                        ready_queue_other = true;
                    }
                }

                for(unsigned int i = 0; i < Blocked_queue.size(); i++){
                    if(Blocked_queue[i].first != running_process){
                        Blocked_queue_other = true;
                    }
                }

                if((!ready_queue_other) && (!Blocked_queue_other)){
                   process_leave = time + CPU_burst + Tcs;
                   preemption = false; 
                   arranged_time[float(process_arrive + 0.5*Tcs + T_slice)][running_process] = 7;
                   arranged_time[float(process_leave - 0.5*Tcs)][running_process] = 3;
                }
                else{
                    if(ready_queue_other){
                        next = 0;
                    }
                    else{
                        //find the min value of arrival_time and blocked_queue
                        for(map<string, int>::iterator itr = arrival_time.begin(); itr != arrival_time.end(); itr++){
                            if((itr->second > time) && (itr->second < next)){
                                next = itr->second;
                            }
                        }

                        for(unsigned int i = 0; i < Blocked_queue.size(); i++){
                            if((Blocked_queue[i].second > time) && (Blocked_queue[i].second < next)){
                                next = Blocked_queue[i].second;
                            }
                        }
                    }

                    if((next - time - 0.5*Tcs) <= T_slice){
                        process_leave = time + T_slice + Tcs; 
                        preemption = true; 
                    }
                    else{
                        process_leave = time + CPU_burst + Tcs;
                        preemption = false;
                    }
                    
                    arranged_time[float(process_arrive + 0.5*Tcs + T_slice)][running_process] = 7;
                    arranged_time[float(process_leave - 0.5*Tcs)][running_process] = 3;
                }
            }
           
            // 1. Save arrival time
            for(map<string, int>::iterator itr = arrival_time.begin(); itr != arrival_time.end(); itr++){
                if((itr->second >= process_arrive) && (itr->second < process_leave)){
                    arranged_time[float(itr->second)][itr->first] = 1; 
                }
            }

            // 2. Save time of actually starting using CPU
            arranged_time[float(process_arrive + 0.5*Tcs)][running_process] = 2;

            // 8. Save time of actually leaving CPU
            arranged_time[float(process_leave - 0.5)][running_process] = 8;

            // 6. Save time of starting using CPU
            arranged_time[float(process_arrive) + float(0.5*Tcs) - 1.5][running_process] = 6;

            // 4. Save time of completing IO
            for(unsigned int i = 0; i < Blocked_queue.size(); i++){
                if((Blocked_queue[i].second >= process_arrive) && (Blocked_queue[i].second < process_leave)){
                    arranged_time[float(Blocked_queue[i].second)][Blocked_queue[i].first] = 4;
                }
            }


            for(std::map<float, std::map<std::string, int> >::iterator itr = arranged_time.begin(); itr != arranged_time.end(); itr++){
                for(std::map<std::string, int>::iterator itr2 = (itr->second).begin(); itr2 != (itr->second).end(); itr2++){
                    if(itr2->second == 6){
                        ready_queue.erase(ready_queue.begin());
                    }
                }

                for(std::map<std::string, int>::iterator itr2 = (itr->second).begin(); itr2 != (itr->second).end(); itr2++){
                    if(itr2->second == 2){
                        // assert(ready_queue[0] == itr2->first);
                        // ready_queue.erase(ready_queue.begin());
                        begin_end_time[itr2->first].push_back(itr->first);
                        if(itr->first <= 999){
                            if(remaining[running_process] == false){
                                std::cout << "time " << itr->first << "ms: Process " << itr2->first << " started using the CPU for " << CPU_burst << "ms burst";    
                            }
                            else{
                                std::cout << "time " << itr->first << "ms: Process " << itr2->first << " started using the CPU with " << CPU_burst << "ms remaining"; 
                            } 
                            output_readyqueue(ready_queue);
                        }
                    }
                }

                for(std::map<std::string, int>::iterator itr2 = (itr->second).begin(); itr2 != (itr->second).end(); itr2++){
                    if(itr2->second == 3){
                        assert(running_process == itr2->first);
                        // complete a CPU burst
                        if(CPU_burst > T_slice){
                            if(preemption == false){
                                burst_io_time[running_process].erase(burst_io_time[running_process].begin()); 
                            }
                            else{
                                (burst_io_time[running_process].begin())->first -= T_slice;
                                assert((burst_io_time[running_process].begin())->first > 0);
                            }
                        }
                        else{
                            burst_io_time[running_process].erase(burst_io_time[running_process].begin());
                        }

                        if(((process_arrive + 0.5*Tcs + T_slice) == (process_leave - 0.5*Tcs)) && (CPU_burst != T_slice)){
                            if(itr->first <= 999){
                                if(preemption == false){
                                    std::cout << "time " << itr->first << "ms: Time slice expired; no preemption because ready queue is empty [Q <empty>]" << std::endl;   
                                }
                                else{
                                    std::cout << "time " << itr->first << "ms: Time slice expired; process " << itr2->first << " preempted with " << (CPU_burst-T_slice) << "ms to go";
                                    output_readyqueue(ready_queue);   
                                }
                            }

                            if(preemption){
                                preempted += 1;
                                begin_end_time[itr2->first].push_back(itr->first);
                            }
                            
                            remaining[running_process] = true;
                        }
                        else{
                            if(burst_io_time[itr2->first].size() != 0){
                                if(itr->first <= 999){
                                    std::cout << "time " << itr->first << "ms: Process " << itr2->first << " completed a CPU burst; " << burst_io_time[itr2->first].size();
                                    if(burst_io_time[itr2->first].size() == 1){
                                        std::cout << " burst to go";
                                    }
                                    else{
                                        std::cout << " bursts to go";
                                    }
                                    output_readyqueue(ready_queue);
                                }
                            }

                            // switch out of CPU
                            if(IO_burst == -1){
                                num_completed += 1;
                                std::cout << "time " << itr->first << "ms: Process " << itr2->first << " terminated";
                                output_readyqueue(ready_queue);
                            }
                            else{
                                Blocked_queue.push_back(std::make_pair(itr2->first, itr->first + IO_burst + 0.5*Tcs));
                                if(itr->first <= 999){
                                    std::cout << "time " << itr->first << "ms: Process " << itr2->first << " switching out of CPU; will block on I/O until time " << (itr->first + IO_burst + 0.5*Tcs)<< "ms";
                                    output_readyqueue(ready_queue);
                                }
                            }
                            remaining[running_process] = false;
                        }
                    }
                }

                for(std::map<std::string, int>::iterator itr2 = (itr->second).begin(); itr2 != (itr->second).end(); itr2++){
                    if(itr2->second == 4){
                        for(std::vector<pair<std::string, int> >::iterator block_itr = Blocked_queue.begin(); block_itr != Blocked_queue.end(); block_itr++){
                            if(block_itr->first == itr2->first){
                                assert(block_itr->second == itr->first);
                                Blocked_queue.erase(block_itr);
                                break;
                            }
                        }

                        if(all_Block != true){
                           ready_queue.push_back(itr2->first);
                        }
                        begin_end_time[itr2->first].push_back(itr->first);

                        if(itr->first <= 999){
                            std::cout << "time " << itr->first << "ms: Process " << itr2->first << " completed I/O; added to ready queue";
                            output_readyqueue(ready_queue); 
                        }
                        all_Block = false;
                    }
                }
                
                for(std::map<std::string, int>::iterator itr2 = (itr->second).begin(); itr2 != (itr->second).end(); itr2++){
                    if(itr2->second == 1){
                        ready_queue.push_back(itr2->first);
                        if(itr->first <= 999){
                            std::cout << "time " << itr->first << "ms: Process " << itr2->first << " arrived; added to ready queue";
                            output_readyqueue(ready_queue);
                        }
                        begin_end_time[itr2->first].push_back(itr->first);

                    }
                }

                for(std::map<std::string, int>::iterator itr2 = (itr->second).begin(); itr2 != (itr->second).end(); itr2++){
                    if(itr2->second == 7){
                        if(itr->first <= 999){
                            if(preemption == false){
                                std::cout << "time " << itr->first << "ms: Time slice expired; no preemption because ready queue is empty [Q <empty>]" << std::endl;   
                            }
                            else{
                                std::cout << "time " << itr->first << "ms: Time slice expired; process " << itr2->first << " preempted with " << (CPU_burst-T_slice) << "ms to go";
                                output_readyqueue(ready_queue);   
                            }
                        }
                        if(preemption == true){
                            ready_queue.push_back(itr2->first); 
                        }
                    }
                }

                for(std::map<std::string, int>::iterator itr2 = (itr->second).begin(); itr2 != (itr->second).end(); itr2++){
                    if(itr2->second == 8){
                        if(((process_arrive + 0.5*Tcs + T_slice) == (process_leave - 0.5*Tcs)) && (CPU_burst != T_slice)){
                            if(preemption == true){
                                ready_queue.push_back(itr2->first);  
                            }
                        }
                    }
                }

            }

            time = process_leave;
        }
        else{

            int min_block = numeric_limits<int>::max();
            for(unsigned int i = 0; i < Blocked_queue.size(); i++){
                if(min_block > Blocked_queue[i].second){
                    min_block = Blocked_queue[i].second;
                    running_process = Blocked_queue[i].first;
                }
            }
            time = min_block;
            ready_queue.push_back(running_process);
            all_Block = true;

        }
    }

    std::map<std::string, std::vector<int> >::iterator time_itr;
    float time_total = 0;
    int count = 0;
    for(time_itr = begin_end_time.begin(); time_itr != begin_end_time.end(); time_itr++){
        float time_each = 0;
        std::vector<int> temp_time = time_itr->second;
        sort(temp_time.begin(), temp_time.end());
        for(unsigned int i = 0; i < temp_time.size()-1; i+=2){
            time_each += (float)(temp_time[i+1] - (float)temp_time[i]);
            count++;
        }
        time_total += time_each;
    }
    float averageWaittime = float(time_total)/count - 2;
    float averageTtime = averageWaittime + Tcs + average;
    simout<<"-- average wait time: "<<averageWaittime<<" ms"<<endl;
    simout<<"-- average turnaround time: "<<averageTtime<<" ms"<<endl;
    simout<<"-- total number of context switches: "<<(preempted + total_process)<<endl;
    simout<<"-- total number of preemptions: "<< preempted <<endl;

    // print RR end message
    std::cout <<"time " << time << "ms: Simulator ended for RR [Q <empty>]" << std::endl; 

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
    string RRadd = "END"; 
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
    cout<<endl;
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
