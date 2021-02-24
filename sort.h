#ifndef _SORTER_H_
#define _SORTER_H_
//for now default the len to 100, it will be changed later

// Begin/end the background thread which sorts random permutations.
void Sorter_startSorting(void);
void Sorter_stopSorting(void);

// Get the size of the array currently being sorted.
// Set the size the next array to sort (don’t change current array)
void Sorter_setArraySize(int newSize);
int Sorter_getArrayLength(void);

// Get a copy of the current (potentially partially sorted) array.
// Returns a newly allocated array and sets 'length' to be the 
// number of elements in the returned array (output-only parameter).
// The calling code must call free() on the returned pointer.
int* Sorter_getArrayData(int *length);
int Sorter_getArray_Value(int n);

// Get the number of arrays which have finished being sorted.
long long Sorter_getNumberArraysSorted(void);
#endif