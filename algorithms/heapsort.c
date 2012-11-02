#include "algorithms.h"

#ifndef DEBUG
inline
#endif
void heapsort_siftdown_helper(int start, int end, uint32_t *out)
{
    int root = start;
    
    // While the root has at least one child.
    while(root * 2 + 1 <= end) {
        int child = root * 2 + 1;
        int s = root;
        
        // Check if root is smaller than child.
        if(out[s] < out[child])
            s = child;
        
        // Check if right child exists, and if it's bigger than what we're currently swapping with.
        if(child + 1 <= end && out[s] < out[child + 1])
            s = child + 1;
        
        // Check if we need to swap at all.
        if(s != root) {
            swap(out, root, s);
            root = s;
        } else
            // No swap? Done sifting down.
            break;
    }
}

#ifndef DEBUG
inline
#endif
void heapsort_heapify_helper(uint32_t n, uint32_t *out)
{
    // Start is assigned the index in a of the last parent node.   
    for(int start = (int) (n - 2) / 2; start >= 0; start--) {
        
        // Sift down the node at index start to the proper place such that all nodes below
        // the start index are in heap order.
        heapsort_siftdown_helper(start, (int) n - 1, out);

    }
}

void heapsort(uint32_t n, uint32_t *in, uint32_t *out)
{
    (void) in;

    // First place out in max-heap order.
    heapsort_heapify_helper(n, out);
    
    int end = (int) n - 1;
    while(end > 0) {
        // Swap the root(maximum value) of the heap with the last element of the heap.
        swap(out, 0, end);
        end--;
        heapsort_siftdown_helper(0, end, out);
    }
}
