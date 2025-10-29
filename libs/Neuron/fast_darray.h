//===============================================================//
//                        F D A R R A Y                          //
//                                                               //
//                   By Christopher Delay                        //
//                           V1.3                                //
//===============================================================//

#ifndef _included_fdarray_h
#define _included_fdarray_h


#include "darray.h"


//=================================================================
// Fast Dynamic array object
// Use : A dynamically sized list of data
// Which can be indexed into - an entry's index never changes
// Same as DArray, but has new advantages: Insert is MUCH faster

template <class T>
class FastDArray: public DArray <T>
{
protected:
	int numused;
	int	*freelist;
	int firstfree;

	void RebuildFreeList();									// SLOW
	void RebuildNumUsed	();									// SLOW
	void Grow			();									// SLOW

public:
    FastDArray ();
    FastDArray ( int newstepsize );
	~FastDArray();

    void SetSize		( int newsize );					// SLOW

    inline int PutData	( const T &newdata );				// FAST Returns index used
    void PutData		( const T &newdata, int index );	// SLOW

	void MarkUsed		( int index );						// SLOW
	inline void MarkNotUsed	( int index );					// FAST

    inline T *GetPointer();                                 // FAST Returns next free element, sets to 'used'
    inline T *GetPointer(int index);                        // FAST Returns next free element, sets to 'used'
    inline int GetNextFree();								// FAST Sets the returned index to 'used'

    inline int NumUsed() const;								// FAST Returns the number of used entries

    void Empty();											// FAST Resets the array to empty
    void EmptyAndDelete();                                  // FAST
};

#include "pch.h"


#include <stdlib.h>


#include "fast_darray.h"


template <class T>
FastDArray <T>::FastDArray ()
	: DArray <T> ()
{
	numused = 0;
	freelist = NULL;
	firstfree = -1;
}


template <class T>
FastDArray <T>::FastDArray ( int newstepsize )
	: DArray<T> (newstepsize)
{
	numused = 0;
	freelist = NULL;
	firstfree = -1;
}

template <class T>
FastDArray <T>::~FastDArray ()
{
	Empty();
	// user must call EmptyAndDelete() by hand when T is pointer,
	// otherwise memory leaks
}

template <class T>
void FastDArray <T>::RebuildFreeList ()
{
	//  Reset free list

	delete freelist;
	freelist = new int[m_arraySize];
	firstfree = -1;
	int lastknownfree = -1;

	// Step through, rebuilding

	for ( int i = 0; i < m_arraySize; ++i ) {
		if ( shadow[i] == 0 ) {
			if ( firstfree == -1 ) {
				firstfree = i;
			}
			else {
				if ( lastknownfree != -1 ) {
					freelist[lastknownfree] = i;
				}
			}

			freelist[i] = -1;
			lastknownfree = i;
		}
		else {
			freelist[i] = -2;
		}
	}
}


template <class T>
void FastDArray <T>::RebuildNumUsed()
{
	numused = 0 ;
	for ( int i = 0; i < m_arraySize; ++i )
	{
		if ( shadow[i] == 1 ) ++numused;
	}
}


template <class T>
void FastDArray <T>::SetSize ( int newsize )
{
	if ( newsize > m_arraySize )
	{
		int oldarraysize = m_arraySize;

		m_arraySize = newsize;
		T *temparray = new T[ m_arraySize ];
		char *tempshadow = new char[ m_arraySize ];
		int *tempfreelist = new int[ m_arraySize ];

		int a;

		for ( a = 0; a < oldarraysize; ++a )
		{
			temparray[a] = array[a];
			tempshadow[a] = shadow[a];
			tempfreelist[a] = freelist[a];
		}

		for ( a = oldarraysize; a < m_arraySize; ++a )
			tempshadow[a] = 0;

		int oldfirstfree = firstfree;
		firstfree = oldarraysize;
		for ( a = oldarraysize; a < m_arraySize-1; ++a )
			tempfreelist[a] = a + 1;
		tempfreelist[m_arraySize-1] = oldfirstfree;

		delete[] array;
		delete[] shadow;
		delete[] freelist;

		array = temparray;
		shadow = tempshadow;
		freelist = tempfreelist;
	}
	else if ( newsize < m_arraySize )
	{
		m_arraySize = newsize;
		T *temparray = new T[m_arraySize];
		char *tempshadow = new char[m_arraySize];
		int *tempfreelist = new int[m_arraySize];

		for ( int a = 0; a < m_arraySize; ++a )
		{
			temparray[a] = array[a];
			tempshadow[a] = shadow[a];
			tempfreelist[a] = freelist[a];
		}

		delete[] array;
		delete[] shadow;
		delete[] freelist;

		array = temparray;
		shadow = tempshadow;
		freelist = tempfreelist;

		RebuildNumUsed();
		RebuildFreeList();
	}
	else if ( newsize == m_arraySize )
	{
		// Do nothing
	}
}


template <class T>
void FastDArray <T>::Grow()
{
	if (m_stepSize == -1)
	{
		if (m_arraySize == 0)
		{
			SetSize( 1 );
		}
		else
		{
			// Double array size
			SetSize( m_arraySize * 2 );
		}
	}
	else
	{
		// Increase array size by fixed amount
		SetSize( m_arraySize + m_stepSize );
	}
}


template <class T>
int FastDArray <T>::PutData( const T &newdata )
{
    if ( firstfree == -1 )
	{
		// Must resize the array
		Grow();
	}

DEBUG_ASSERT(firstfree != -1);
    if ( firstfree == -1 )
	{
		// Must resize the array
		Grow();
	}

	int freeslot = firstfree;
	int nextfree = freelist[freeslot];

    array[freeslot] = newdata;
	if ( shadow[freeslot] == 0 ) ++numused;
    shadow[freeslot] = 1;
	freelist[freeslot] = -2;
	firstfree = nextfree;

    return freeslot;
}


template <class T>
void FastDArray <T>::PutData( const T &newdata, int index )
{
    DEBUG_ASSERT ( index < m_arraySize && index >= 0 );

    array[index] = newdata;

    if ( shadow[index] == 0 )
	{
		shadow[index] = 1;
		++numused;
		RebuildFreeList();
	}
}


template <class T>
void FastDArray <T>::EmptyAndDelete()
{
	delete[] freelist;
	freelist = NULL;

	firstfree = -1;
	numused = 0;

    DArray<T>::EmptyAndDelete();
}

template <class T>
void FastDArray <T>::Empty()
{
	delete[] freelist;
	freelist = NULL;

	firstfree = -1;
	numused = 0;

	DArray<T>::Empty();
}


template <class T>
void FastDArray <T>::MarkUsed( int index )
{
    DEBUG_ASSERT ( index < m_arraySize && index >= 0 );
    DEBUG_ASSERT ( shadow[index] == 0 );

    shadow[index] = 1;
	++numused;
    RebuildFreeList();
}


template <class T>
void FastDArray <T>::MarkNotUsed( int index )
{
    DEBUG_ASSERT ( index < m_arraySize && index >= 0 );
    DEBUG_ASSERT ( shadow[index] != 0 );

	--numused;
    shadow[index] = 0;
    freelist[index] = firstfree;
	firstfree = index;
}


template <class T>
T *FastDArray<T>::GetPointer ()
{
    return GetPointer( GetNextFree() );
}


template <class T>
T *FastDArray<T>::GetPointer(int index)
{
	return DArray<T>::GetPointer(index);
}


template <class T>
int FastDArray<T>::GetNextFree()
{
    if ( firstfree == -1 )
	{
		// Must resize the array
		Grow();
	}

	int freeslot = firstfree;
	int nextfree = freelist[freeslot];

	if ( shadow[freeslot] == 0 )
	{
		++numused;
	}

    shadow[freeslot] = 1;
	freelist[freeslot] = -2;
	firstfree = nextfree;

    return freeslot;
}


template <class T>
int FastDArray <T>::NumUsed() const
{
	return numused;
}

#endif
