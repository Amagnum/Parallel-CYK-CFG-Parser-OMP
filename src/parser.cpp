/*

Adarsh Baghel
180001001

ASSIGNMENT-1 CS 359(2021)
__________________________________________

Problem Statement:

Write a parallel program to parse a string of symbols. 
The inputs are a context-free grammar G in Chomsky Normal Form 
and a string of symbols. In the end, the program should print 
yes if the string of symbols can be derived by the rules of 
the grammar and no otherwise. Write a sequential program 
(no OpenMP directives at all) as well. 

Compare the running time of the sequential program with 
the running time of the parallel program and compute the 
speedup for different grammars and different string lengths.
(OpenMP, 10 marks)

How to run?
$ g++ -fopenmp main.cpp -o main
$ ./main ./gram1.in < input1.in

*/

#include <bits/stdc++.h>
#include <time.h>
#include <omp.h>

using namespace std;
#define MAX 100

string gram[MAX][MAX];  //to store entered grammar
int np;                 //no of productions

double serialTime;

void cyk_parser(string str, char start){
    
    int strLen = str.length();
    vector<vector<vector<bool>>> table(strLen+1, vector<vector<bool>>(strLen+1, vector<bool>(127, false)));
    
    double start_time = omp_get_wtime();

    for (int i = 1; i <= strLen; i++)
    {
        for (int j = 0; j < np; j++)
            for (int k = 1; gram[j][k] != ""; k++)
                if (gram[j][k][0] == str[i-1]){
                    table[1][i][gram[j][0][0]]=true;
                }
    }

    for(int l = 2; l <= strLen; ++l){
        for(int s = 1; s <= (strLen - l + 1); ++s){
            for(int p = 1; p <= (l - 1); ++p){
                for (int j = 0; j < np; j++)
                    for (int k = 1; gram[j][k] != ""; k++)
                        if(gram[j][k].length()>1){ // ie dual productions
                            if(table[p][s][gram[j][k][0]] and table[l - p][s + p][gram[j][k][1]]){
                                table[l][s][gram[j][0][0]] = true;
                                
                            }
                        }
            }
        }
    }


    if (table[strLen][1][start])
        cout << "YES\n";
    else
        cout << "NO\n";

    double time = omp_get_wtime() - start_time;
    serialTime=time;
    printf("Total time for Serial (in sec): %.8f\n", time);  

}

void cyk_parser_parallel(string str, char start){
    
    int strLen = str.length();
    vector<vector<vector<bool>>> table(strLen+1, vector<vector<bool>>(strLen+1, vector<bool>(127, false)));
    
    double start_time = omp_get_wtime();

    for(int nthreads=1; nthreads <= 10; nthreads+=1) {
        omp_set_num_threads(nthreads);
        double start_time = omp_get_wtime();
        
        #pragma omp parallel shared(table)

        #pragma omp parallel for schedule(dynamic)
        for (int i = 1; i <= strLen; i++)
        {
            for (int j = 0; j < np; j++)
                for (int k = 1; gram[j][k] != ""; k++)
                    if (gram[j][k][0] == str[i-1]){
                        table[1][i][gram[j][0][0]]=true;
                    }
        }

        for(int l = 2; l <= strLen; ++l){
            #pragma omp parallel for schedule(dynamic)
            for(int s = 1; s <= (strLen - l + 1); ++s){
                for(int p = 1; p <= (l - 1); ++p){
                    for (int j = 0; j < np; j++)
                        for (int k = 1; gram[j][k] != ""; k++)
                            if(gram[j][k].length()>1){ // ie dual productions
                                if(table[p][s][gram[j][k][0]] and table[l - p][s + p][gram[j][k][1]]){
                                    #pragma omp critical
                                    table[l][s][gram[j][0][0]] = true;
                                }
                            }
                }
            }
        }

        double time = omp_get_wtime() - start_time;
        //printf("Total time for thread %d (in sec): %.8f\n", nthreads, time);
        printf("%d\t%d\t%.8f\t%.8f\t%.8f\n", strLen,nthreads, time,serialTime, serialTime/time);
        /*if (table[strLen][1][start])
            cout << "YES\n";
        else
            cout << "NO\n";
        */
    }


    if (table[strLen][1][start])
        cout << "YES\n";
    else
        cout << "NO\n";
}

int main(int argc, char** argv)
{
    int i, pt, j, l, k;
    string a, str, r, pr, start = "S";
    char buffer[150];
    np = 0;
    bool prod_found=false;

    if (argv[1] == nullptr) {
        cout << "Input grammar missing...EXITING";
        return 1;
    }       
    
    // Reading the Grammar -----------------------------------

    ifstream ifile(argv[1]);

    cout<<"Input Grammar:\n";
    while (ifile.getline(buffer, 99)) {
        a = buffer;
        a.erase(remove_if(a.begin(), a.end(), ::isspace), a.end());
        a.erase(remove(a.begin(), a.end(), ';'), a.end());
        cout<<a<<endl;
        pt = a.find("->");

        //left side of production
        gram[np][0] = a.substr(0, pt);
        a = a.substr(pt + 2, a.length());
        //right side of production
        for (j = 0; a.length()!=0; j++)
        {
            int i = a.find("|");
            if (i > a.length())
            {
                gram[np][j + 1] = a;
                a = "";
            }
            else
            {
                gram[np][j + 1] = a.substr(0, i);
                a = a.substr(i + 1, a.length());
            }
        }

        np++;
    }
    ifile.close();
    // END Reading the Grammar -----------------------------------

    

    // Reading the Input Strings -----------------------------------

    int nStrs;
    cin >> nStrs;
    vector<string> strings(nStrs);

    cout<<"\nInput Strings:"<<endl;

    for (int i = 0; i < nStrs; ++i)
    {
        cin>>strings[i];
        int strlen = strings[i].length();
        cout<<i+1<<": "<<strings[i].substr(0,min(5, strlen))+"..."<<" Len: "<<strlen<<endl;   
    }

    // END Reading the Input strings -----------------------------------
    
    cout<<"\nParsing:\n"<<endl;

    nStrs=0;
    for (auto str: strings){
        
        int strlen = str.length();
        cout<<(nStrs++) + 1<<": \n"<<str.substr(0,min(5, strlen))+"..."<<" Len: "<<strlen<<endl;
        
        cout<<"Serial:\n";
        cyk_parser(str,'S');
        
        cout<<"Parallel:\n";
        cyk_parser_parallel(str,'S');
        
        cout<<endl;
    }

    cout<<"\nDone...\n";
    return 0;
}