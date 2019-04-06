
#include <limits>
#include <cstdlib>
#include <cstdio>
#include <cilk/cilk.h>
#include <cilk/reducer_opadd.h>

//#include "kmeans.h"


struct point {
    float x, y;
    void operator+=( const point& right ) {
        x += right.x;
        y += right.y;
    }
    point operator/( size_t count ) const {
        point p = *this;
        p.x /= count;
        p.y /= count;
        return p;
    }
};

inline float distance2( const point& a, const point& b ) {
    float dx = a.x-b.x;
    float dy = a.y-b.y;
    return dx*dx+dy*dy;
}

#include "sum_and_count.h"

typedef unsigned short cluster_id;

void repair_empty_clusters( size_t n, const point points[], cluster_id id[], size_t k, point centroid[], sum_and_count sum[] );

#include "elementwise_reducer.h"
#include <cstddef>

namespace cilk_example {
    void compute_k_means( size_t n, const point points[], size_t k, cluster_id id[], point centroid[] ) {
        
        // Create initial clusters and compute their sums.
        elementwise_reducer<sum_and_count> sum(k);
        sum.clear();
        cilk_for( size_t i=0; i<n; ++i ) {
            id[i] = i % k;
            // Peeled "Sum step"
            sum[id[i]].tally(points[i]);
        }
        
        // Loop until clusters do not change
        cilk::reducer_opadd<size_t> change;
        do {/*@\label{line:kmeans:dowhile:first}@*/
            // Repair any empty clusters
            repair_empty_clusters( n, points, id, k, centroid, sum );
            
            // "Divide step": Compute centroids from sums
            centroid[0:k] = sum.get_array()[0:k].mean();
            
            sum.clear();
            change.set_value(0);
            cilk_for( size_t i=0; i<n; ++i ) {/*@\label{line:kmeans:cilkfor:first}@*/
                // "Reassign step": Find index of centroid closest to points[i]
                cluster_id j = __sec_reduce_min_ind(distance2(centroid[0:k],points[i]));/*@\label{line:kmeans:reduce-min}@*/
                if( j!=id[i] ) {
                    // A different centroid is closer now.
                    id[i] = j;
                    ++change;
                }
                // "Sum step"
                sum[j].tally(points[i]);
            } /*@\label{line:kmeans:cilkfor:last}@*/
        } while( change.get_value()!=0 );/*@\label{line:kmeans:dowhile:last}@*/
    }

}

