/*
 * Implements the MTS-LS1 indicated in MTS
 * http://sci2s.ugr.es/EAMHCO/pdfs/contributionsCEC08/tseng08mts.pdf
 * Lin-Yu Tseng; Chun Chen, "Multiple trajectory search for Large Scale Global Optimization," Evolutionary Computation, 2008. CEC 2008. (IEEE World Congress on Computational Intelligence). IEEE Congress on , vol., no., pp.3052,3059, 1-6 June 2008
 * doi: 10.1109/CEC.2008.4631210
 */

#include <iostream>
#include <algorithm>
#include <vector>
#include <random>

#include "Header.h"
#include <sys/time.h>
#include <cstdio>
#include <unistd.h>

using namespace std;

typedef struct{
  vector<double> solution;
  double fitness;
  unsigned evals;
} LSresult;

unsigned dim = 1000;

LSresult mts_ls1_improve_dim(Benchmarks * bench, vector<double>& sol, double best_fitness, unsigned i, vector<double> & SR ){
  vector<double> newsol = sol;
  newsol[i] -= SR[i];

  double initial_fit = best_fitness;

  newsol[i] = min(newsol[i], (double)bench->getMaxX());
  newsol[i] = max(newsol[i], (double)bench->getMinX());

  double fitness_newsol = bench->compute(newsol.data());

  unsigned evals = 1;

  if( fitness_newsol < best_fitness ){
    best_fitness = fitness_newsol;
    sol = newsol;
  } else if( fitness_newsol > best_fitness ){
    newsol[i] = sol[i];
    newsol[i] += 0.5 * SR[i];

    newsol[i] = min(newsol[i], (double)bench->getMaxX());
    newsol[i] = max(newsol[i], (double)bench->getMinX());

    fitness_newsol = bench->compute(newsol.data());
    evals++;

    if( fitness_newsol < best_fitness ){
      best_fitness = fitness_newsol;
      sol = newsol;
    }
  }
  //printf("%1.20E -> %1.20E\n", initial_fit, best_fitness);
  return LSresult{sol, best_fitness, evals};
}

void mts_ls1(Benchmarks * bench, unsigned maxevals, vector<double>& sol){
  //unsigned int n_dim = ndim;
  unsigned totalevals = 0;
  double best_fitness = bench->compute(sol.data());

  vector<double> SR(sol.size());
  for( unsigned i = 0; i < SR.size(); i++ ){
    SR[i] = (bench->getMaxX() - bench->getMinX()) * 0.4;
  }

  LSresult result;
  LSresult current_best = {sol, best_fitness, 0};

  vector<double> improvement(dim, 0.0);

  vector<double> dim_sorted(dim);
  iota(dim_sorted.begin(), dim_sorted.end(), 0);

  double improve;
  //warm-up
  if( totalevals < maxevals ){
    next_permutation(dim_sorted.begin(), dim_sorted.end());
    for( auto it = dim_sorted.begin(); it != dim_sorted.end(); it++ ){
      result = mts_ls1_improve_dim(bench, sol, best_fitness, *it, SR);
      totalevals += result.evals;
      improve = max(current_best.fitness - result.fitness, 0.0);
      improvement[*it] = improve;

      if( improve > 0.0 ){
        printf("[1] %.10lf > %.10lf ~ improve: %.20lf\n", current_best.fitness, result.fitness, improve);
        current_best = result;
      } else {
        SR[*it] /= 2.0f;
      }
    }
  }

  iota(dim_sorted.begin(), dim_sorted.end(), 0);
  sort(dim_sorted.begin(), dim_sorted.end(), [&](unsigned i1, unsigned i2) { return improvement[i1] > improvement[i2]; });

  int i, d = 0, next_d, next_i;
  while( totalevals < maxevals ){
    i = dim_sorted[d];
    result = mts_ls1_improve_dim(bench, current_best.solution, current_best.fitness, i, SR);
    totalevals += result.evals;
    improve = max(current_best.fitness - result.fitness, 0.0);
    improvement[i] = improve;
    next_d = (d+1)%dim;
    next_i = dim_sorted[next_d];

    if( improve > 0.0 ){
      printf("[2] %.10lf > %.10lf ~ improve: %.20lf\n", current_best.fitness, result.fitness, improve);
      current_best = result;

      if( improvement[i] < improvement[next_i] ){
        iota(dim_sorted.begin(), dim_sorted.end(), 0);
        sort(dim_sorted.begin(), dim_sorted.end(), [&](unsigned i1, unsigned i2) { return improvement[i1] > improvement[i2]; });
      }
    } else {
      SR[i] /= 2.0;
      d = next_d;
      if( SR[i] < 1e-15 ){
        SR[i] = (bench->getMaxX() - bench->getMinX()) * 0.4;
      }
    }
  }
  sol = current_best.solution;
}

int main(){
  /*  Test the basic benchmark function */
  Benchmarks * fp = NULL;
  vector<double> runTimeVec;
  struct timeval start, end;
  long seconds, useconds;
  double mtime;

  fp = new F12(); //generateFuncObj(12);

  std::default_random_engine generator;
  std::uniform_real_distribution<double> distribution(fp->getMinX(), fp->getMaxX());

  vector<double> sol(dim);
  for( unsigned i = 0; i < dim; i++ ){
    sol[i] = distribution(generator);
  }

  printf("F %d value = %1.20E\n", fp->getID(), fp->compute(sol.data()));

  gettimeofday(&start, NULL);
  fp->compute(sol.data());
  gettimeofday(&end, NULL);

  seconds  = end.tv_sec  - start.tv_sec;
  useconds = end.tv_usec - start.tv_usec;

  mtime = (((seconds) * 1000 + useconds/1000.0) + 0.5)/1000;

  runTimeVec.push_back(mtime);
  printf ( "F %d, Running Time = %f s\n\n", fp->getID(), mtime);

  mts_ls1(fp, 1e6, sol);

  printf("F %d value = %1.20E\n", fp->getID(), fp->compute(sol.data()));

  delete fp;
  return 0;
}

// create new object of class with default setting
Benchmarks* generateFuncObj(int funcID){
  Benchmarks *fp;
  // run each of specified function in "configure.ini"
  if (funcID==1){
    fp = new F1();
  }else if (funcID==2){
    fp = new F2();
  }else if (funcID==3){
    fp = new F3();
  }else if (funcID==4){
    fp = new F4();
  }else if (funcID==5){
    fp = new F5();
  }else if (funcID==6){
    fp = new F6();
  }else if (funcID==7){
    fp = new F7();
  }else if (funcID==8){
    fp = new F8();
  }else if (funcID==9){
    fp = new F9();
  }else if (funcID==10){
    fp = new F10();
  }else if (funcID==11){
    fp = new F11();
  }else if (funcID==12){
    fp = new F12();
  }else if (funcID==13){
    fp = new F13();
  }else if (funcID==14){
    fp = new F14();
  }else if (funcID==15){
    fp = new F15();
  }else{
    cerr<<"Fail to locate Specified Function Index"<<endl;
    exit(-1);
  }
  return fp;
}
