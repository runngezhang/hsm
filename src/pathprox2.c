#include <stdlib.h>
#include <math.h>

void pathgraph_prox2(double *y, double *lam, double *w, int *assign, int *dim, int *depth, double *penalval) {
    /*
     Compute the proximal function under a path graph of depth d,
     where d is the number of nodes.
     Args:
        y: an array of dim elements.
        lam: a positive integer for tuning parameter.
        w: an array of positive weights for the depth nodes.
        assign: an array indexing each element in y with indices
                {0,1,2,...,depth}. Index 0 means the element is not in the graph.
        dim: dimension of *y = dimension of *assign.
        depth: dimension of *w.
        penalval: value of the penalty in prox function: 
                  lam * \min_{\sum(V's) = \beta} \sum_{i \in [1:depth]} w_i * ||V_i||_2
     */
    int i, loc_min;
    double g_min, b_cumsum, g_temp, multiplier;
    int track = 0;
    int p = *dim;
    int d = *depth;
    double lam2 = (*lam) * (*lam); // this is lambda^2, which is what we need below
    // we will only need w[i]^2, not w[i], so we redefine w to contain its square
    for (i = 0; i < d; i++) {
      w[i] = w[i] * w[i];
    }
    // allocate memory for an array which will contain ||y_{s_i}||^2 for each index i 
    double *b = malloc(d * sizeof(double));
    for (i = 0; i < d; i++) {
      b[i] = 0; // initialize each element to be 0.
    }
    for (i = 0; i < p; i++) {
        if (assign[i] == 0) {
            // the element is not in the graph, set it to 0.
            y[i] = 0;
        } else {
            // Compute sum of squares of values of variables assigned to each node
            b[assign[i]-1] += y[i] * y[i];
        }
    }
    // count the number of updates performed
    int num_update = 0;
    // allocate memory for an array which will contain f_update = g_min^(-0.5) at each update
    double *f_update = malloc(d * sizeof(double));
    // allocate memory for an array which will contain loc_min at each update
    int *loc_update = malloc(d * sizeof(int));
    // loop through the nodes in the graph
    while (track < d) {
        loc_min = track + 1;
        if (track == 0) {
          g_min = lam2 * w[track] / b[track];
        }
        else {
          g_min = lam2 * (w[track] - w[track-1]) / b[track];
        }
        // g = f^-2 for f in Algorithm 3
        if (track < d-1) {
            b_cumsum = b[track];
            for (i = track+2; i <= d; i++) {
                b_cumsum += b[i-1];
                if (track == 0) {
                  g_temp = lam2 * w[i-1] / b_cumsum;
                }
                else {
                  g_temp = lam2 * (w[i-1] - w[track-1]) / b_cumsum;  
                }
                if (g_min >= g_temp) {
                    g_min = g_temp;
                    loc_min = i;
                }
            }
        }
        // break the loop if the early-termination condition is met
        if (g_min >= 1) {
            for (i = 0; i < p; i++) {
                if (assign[i] > track)
                    y[i] = 0;
            }
            break;
        }
        /* do group-wise soft-thresholding for elements
         in the nodes track + 1 through loc_min */
        multiplier = 1 - sqrt(g_min);
        for (i = 0; i < p; i++) {
            if (assign[i] > track && assign[i] <= loc_min)
                y[i] = y[i] * multiplier;
        }
        track = loc_min;
        // update num_update, f_update and loc_update
        num_update += 1;
        f_update[num_update - 1] = 1 / sqrt(g_min);
        loc_update[num_update - 1] = loc_min;
    }
    // Compute the penalty value
    *penalval = 0;
    if (num_update > 0) {
        if (num_update > 1) {
            for (i = 0; i < num_update - 1; i++) {
                *penalval += lam2 * w[loc_update[i] - 1] * (f_update[i] - f_update[i + 1]);
            }
        }
        *penalval += lam2 * w[loc_update[num_update - 1] - 1] * (f_update[num_update - 1] - 1);
    }
    // free memory allocated by malloc
    free(b);
    free(f_update);
    free(loc_update);
}

