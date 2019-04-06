// Implementation of the KMeans Algorithm
// reference: http://mnemstudio.org/clustering-k-means-example-1.htm

#include <iostream>
#include <vector>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <algorithm>
#include <chrono>


#include <cilk/cilk.h>
#include <cilk/reducer_opadd.h>
#include <cilk/reducer.h>
#include <cilk/cilk_api.h>

// #include "sum_count.h"
#include <limits>
#include <cstdlib>
#include <cstdio>
// #include "../common/kmeans.h"
// #include "reducer_cilk.h"



using namespace std;

class Point
{
private:
        int id_point, id_cluster;
        vector<double> values;
        int total_values;
        string name;

public:
        Point(int id_point, vector<double>& values, string name = "")
        {
                this->id_point = id_point;
                total_values = values.size();

                for(int i = 0; i < total_values; i++)
                        this->values.push_back(values[i]);

                this->name = name;
                id_cluster = -1;
        }

        int getID()
        {
                return id_point;
            }
        void setCluster(int id_cluster)
        {
                this->id_cluster = id_cluster;
        }

        int getCluster()
        {
                return id_cluster;
        }

        vector<double> getValues()
        {
            return values;
        }

        double getValue(int index)
        {
                return values[index];
        }

        int getTotalValues()
        {
                return total_values;
        }

        void addValue(double value)
        {
                values.push_back(value);
        }

        string getName()
        {
                return name;
        }
};



class Cluster
{
private:
        int id_cluster;
        vector<double> central_values;
        vector<Point> points;

public:
        Cluster(int id_cluster, Point point)
        {
                this->id_cluster = id_cluster;

                int total_values = point.getTotalValues();

                for(int i = 0; i < total_values; i++)
                        central_values.push_back(point.getValue(i));

                points.push_back(point);
        }

        void addPoint(Point point)
        {
                points.push_back(point);
        }

        bool removePoint(int id_point)
        {
                int total_points = points.size();

                for(int i = 0; i < total_points; i++)
                {
                        if(points[i].getID() == id_point)
                        {
                                points.erase(points.begin() + i);
                                return true;
                        }
                }
                return false;
        }

        double getCentralValue(int index)
        {
                return central_values[index];
        }

        void setCentralValue(double value, int index)
        {
                central_values[index] = value;

        }

        Point getPoint(int index)
        {
                return points[index];
        }

        int getTotalPoints()
        {
                return points.size();
        }

        int getID()
        {
                return id_cluster;
        }
};


struct sum_count {
    sum_count():sum(),count(0){}
    vector<double> sum;
    int count;
    void clear() {
        sum.clear();
        count = 0;
    }

   
    // vector<double> mean(){
    //     for(int i = 0; i<2; i++){
            
    //         sum[i] /= count;

            
    //     }
    //     cout<<"sum[0]"<<sum[0]<<endl;
    //     return sum;
    // }
    void operator+=(sum_count& other){
        for(int i = 0; i<4;i++)
        {
            sum[i] += other.sum[i];
        }
        count += other.count;
    }
};

template<typename T>
class reducer_cilk {
    struct View {
        T* array;
        View( size_t k ) : array( new T[k] ) {}
        ~View() {delete[] array;}
    };

    struct Monoid: cilk::monoid_base<View> {
        const size_t k;
        void identity(View* p) const {new(p) View(k);}
        void reduce(View* left, View* right) const {
            for(int x = 0;x<k;x++)
            {
                left->array[x] += right->array[x];
            }
            // left->array[0:k] += right->array[0:k];
        }
        Monoid( size_t k_ ) : k(k_) {}
    };
    cilk::reducer<Monoid> impl;

public:
    reducer_cilk( size_t k ) : impl(Monoid(k), k) {}
    void clear() {
        // Compute n in a separate statement to work around bug in icpc 14.0
        size_t n = impl.monoid().k;
        for(int x = 0; x<n;x++)
        {
            impl.view().array[x].clear();
        }

    }
    T* get_array() {return impl.view().array;}
    operator sum_count*() {return get_array();}
};


// #if HAVE_CILKPLUS

class KMeans
{
private:
        int K; // number of clusters
        int total_values, total_points, max_iterations;
        vector<Cluster> clusters;


        // return ID of nearest center (uses euclidean distance)
        int getIDNearestCenter(Point point)
        {
                double sum_ = 0.0, min_dist;
                int id_cluster_center = 0;
                // cout<<"total valuse"<<total_values<<endl;
                for(int i = 0; i < total_values; i++)
                {
                        sum_ += pow(clusters[0].getCentralValue(i) -
                                           point.getValue(i), 2.0);
                        // cout<<"cluster 0"<<clusters[0].getCentralValue(i)<<endl;
                        // cout<<point.getValue(i)<<endl;
                }
                
                min_dist = sqrt(sum_);
                // cout<<min_dist<<endl;

                for(int i = 1; i < K; i++)
                {
                        double dist;
                        sum_ = 0.0;
                        for(int j = 0; j < total_values; j++)
                        {
                                sum_ += pow(clusters[i].getCentralValue(j) -
                                                   point.getValue(j), 2.0);
                        }

                        dist = sqrt(sum_);
                        // cout<<dist<<endl;
                        if(dist < min_dist)
                        {
                            // cout<<"i == 1"<<endl;
                                min_dist = dist;
                                id_cluster_center = i;
                        }
                }

                return id_cluster_center;
        }

public:


        KMeans(int K, int total_points, int total_values, int max_iterations)
        {
                this->K = K;
                this->total_points = total_points;
                this->total_values = total_values;               
                this->max_iterations = max_iterations;
        }

        void run(vector<Point> & points){
                auto begin = chrono::high_resolution_clock::now();

                reducer_cilk<sum_count> Sum_1(K); 
                Sum_1.clear();

                if(K > total_points)
                        return;
                vector<int> prohibited_indexes;

                
                // choose K distinct values for the centers of the clusters
                // for(int i = 0; i < K; i++)
                // {
                //     while(true)
                //     {
                //             int index_point = rand() % total_points;

                //             if(find(prohibited_indexes.begin(), prohibited_indexes.end(),
                //                             index_point) == prohibited_indexes.end())
                //             {
                //                     prohibited_indexes.push_back(index_point);
                //                     points[index_point].setCluster(i);
                //                     Cluster cluster(i, points[index_point]);
                //                     clusters.push_back(cluster);    
                //                     cout<<"initial sum: "<<index_point<<endl;
                //                     // sum[i].tally(points[index_point]);
                //                     Sum_1[i].sum = points[index_point].getValues(); 
                //                     // sum[i].sum = points[index_point].getValues();
                //                     Sum_1[i].count = 1;
                //                     break;
                //             }

                //         }
                    
                // }
                __cilkrts_set_param("nworkers","1");
                cilk_for(int i = 0; i<K; i++)
                {
                    int index_point = rand() % (total_points/K);

                    index_point = index_point*(i+1);

                    prohibited_indexes.push_back(index_point);
                    points[index_point].setCluster(i);
                    Cluster cluster(i, points[index_point]);
                    clusters.push_back(cluster); 
                    // cout<<"initial sum: "<<index_point<<endl;

                    Sum_1[i].sum = points[index_point].getValues();
                    Sum_1[i].count = 1;

                    // cout<<"sum: "<<Sum_1[i].sum[0]<<endl;
                    

                }

                auto end_phase1 = chrono::high_resolution_clock::now();

                int iter = 1;

                
                cilk::reducer_opadd<size_t> change;
                do
                {   
                    // cout<<"start loop"<<endl;
                    // // cout<<sum.get_array()[1].sum[0]<<endl;
                    // cout<<"centroid1"<<clusters[0].getCentralValue(0)<<" "<<clusters[0].getCentralValue(1)<<endl;
                    // cout<<"centroid2"<<clusters[1].getCentralValue(0)<<" "<<clusters[1].getCentralValue(1)<<endl;
                        for(int i = 0; i<K; i++)
                        {
                            // cout<<"iiiiiiiiiii:  "<<i<<endl;
                            // cout<<"count: "<<Sum_1.get_array()[i].count<<endl;
                            for (int j = 0; j < total_values; j++)
                            {
                                double means;
                                
                                means = Sum_1.get_array()[i].sum[j]/Sum_1.get_array()[i].count;

                                clusters[i].setCentralValue(means,j);
                                cout<<means<<endl;
                                Sum_1.get_array()[i].sum[j]=0;

                            }
                            Sum_1.get_array()[i].count=0;
                            // Sum_1[i].sum.clear();

                            
                        
                        }
                        // cout<<"clear success? : "<<Sum_1[1].sum[0]<<endl;
                        Sum_1.clear();
                        // cout<<"sum size: "<<Sum_1[1].sum.size()<<endl;
                        // bool done = true;
                        change.set_value(0);
                        // associates each point to the nearest center
                        __cilkrts_set_param("nworkers","8");
                        cilk_for(int i = 0; i < total_points; i++)
                        {
                            // cout<<points[i].getID()<<endl;
                            int id_old_cluster = points[i].getCluster();
                            // cout<<"old: "<<id_old_cluster<<endl;
                            int id_nearest_center = getIDNearestCenter(points[i]);
                            // cout<<"new: "<<id_nearest_center<<endl;


                            for(int j = 0; j < total_values; j++)
                            {
                                Sum_1[id_nearest_center].sum[j] += points[i].getValue(j);
                                // cout<<"sum[new]"<<Sum_1[id_nearest_center].sum[j]<<endl;
                            }
                            Sum_1[id_nearest_center].count += 1;

                            // sum[id_nearest_center].tally(points[i]);
                            
                            if(id_old_cluster != id_nearest_center)
                            {
                                // if(id_old_cluster != -1)
                                        // clusters[id_old_cluster].removePoint(points[i].getID());

                                points[i].setCluster(id_nearest_center);
                                // clusters[id_nearest_center].addPoint(points[i]);
                                ++change;
                                // cout<<"change"<<endl;
                                    
                            }

                        }
                        // cout<<"end sum"<<change.get_value()<<endl;

                        // recalculating the center of each cluster
                        // for(int i = 0; i < K; i++)
                        // {
                        //         for(int j = 0; j < total_values; j++)
                        //         {
                        //                 int total_points_cluster = clusters[i].getTotalPoints();
                        //                 double sum = 0.0;

                        //                 if(total_points_cluster > 0)
                        //                 {
                        //                         for(int p = 0; p < total_points_cluster; p++)
                        //                                 sum += clusters[i].getPoint(p).getValue(j);
                        //                         clusters[i].setCentralValue(j, sum / total_points_cluster);
                        //                 }
                        //         }
                        // }

                        // if(done == true || iter >= max_iterations)
                        // {
                        //         cout << "Break in iteration " << iter << "\n\n";
                        //         break;
                        // }

                        iter++;
                } while(( change.get_value()!=0 ) && (iter < max_iterations));



                auto end = chrono::high_resolution_clock::now();

                // shows elements of clusters
                for(int i = 0; i < K; i++)
                {
                        // int total_points_cluster =  clusters[i].getTotalPoints();

                        // cout << "Cluster " << clusters[i].getID() + 1 << endl;
                        // for(int j = 0; j < total_points_cluster; j++)
                        // {
                        //         cout << "Point " << clusters[i].getPoint(j).getID() + 1 << ": ";
                        //         for(int p = 0; p < total_values; p++)
                        //                 cout << clusters[i].getPoint(j).getValue(p) << " ";

                        //         string point_name = clusters[i].getPoint(j).getName();

                        //         if(point_name != "")
                        //                 cout << "- " << point_name;

                        //         cout << endl;
                        // }

                        // cout << "Cluster values: ";

                        // for(int j = 0; j < total_values; j++)
                        //         cout << clusters[i].getCentralValue(j) << " ";


                        // cout << "\n\n";
                cout << "TOTAL EXECUTION TIME = "<<std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count()<<"\n";

                cout << "TIME PHASE 1 = "<<std::chrono::duration_cast<std::chrono::microseconds>(end_phase1-begin).count()<<"\n";

                cout << "TIME PHASE 2 = "<<std::chrono::duration_cast<std::chrono::microseconds>(end-end_phase1).count()<<"\n";
                }
        }

    
};

// #endif /* HAVE_CILKPLUS */

int main(int argc, char *argv[])
{
    // cout<<"start"<<endl;
        srand (time(NULL));

        int total_points, total_values, K, max_iterations, has_name;

        cin >> total_points >> total_values >> K >> max_iterations >> has_name;

        vector<Point> points;
        string point_name;

        for(int i = 0; i < total_points; i++)
        {
                vector<double> values;

                for(int j = 0; j < total_values; j++)
                {
                        double value;
                        cin >> value;
                        values.push_back(value);
                }

                if(has_name)
                {
                        cin >> point_name;
                        Point p(i, values, point_name);
                        points.push_back(p);
                }
                else
                {
                        Point p(i, values);
                        points.push_back(p);
                }
        }
        // cout<<"111111"<<endl;
        KMeans kmeans(K, total_points, total_values, max_iterations);
        // cout<<"222222"<<endl;
        kmeans.run(points);

        return 0; 
}

