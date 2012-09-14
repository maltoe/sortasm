# sortasm

* Playing with sorting algorithms and x86 assembly.
* Most of the C standard implementations are copied from or heavily inspired by their correspondent Wikipedia articles.
* Almost all assembly implementations (except for Gnomesort and Heapsort) perform worse than their C counterparts.

Sample output (`-O2`, 200000 elements, Core i5):

<code>
insertionsort : 15.312573 s.

insertionsort_asm : 18.261358 s.

bubblesort : 64.524628 s.

bubblesort_asm : 64.991386 s.

gnomesort : 27.534470 s.

gnomesort_rewrite : 32.100063 s.

gnomesort_asm : 20.111547 s.

combsort : 0.026741 s.

combsort_asm : 0.028166 s.

quicksort_recursive : 0.015209 s.

quicksort_iterative : 0.016078 s.

quicksort_iterative_asm : 0.016154 s.

heapsort : 0.026895 s.

heapsort_asm : 0.024864 s.
</code>



