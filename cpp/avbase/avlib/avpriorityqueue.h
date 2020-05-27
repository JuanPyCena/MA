///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author    Christian Muschick, c.muschick@avibit.com
    \brief     This class provides a priority queue template based on a QList
*/

#if !defined AVPRIORITYQUEUE_H_INCLUDED
#define AVPRIORITYQUEUE_H_INCLUDED

#include <qalgorithms.h>

///////////////////////////////////////////////////////////////////////////////

//! This class provides a priority queue template based on a QT merge request for QT4.8
/*  This class provides a priority queue template based on a QT merge request for QT4.8
 *
 *  AVPriorityQueue is a queue where you can only access to the
 *  element wich has the highest priority. Accessing this element
 *  is in O(1). Both inserting and removing are in O(log(n)), with
 *  n being the number of elements in the list.
 *
 *  Internally, AVPriorityQueue\<T, PriorityComparator\> is using an inlined binary heap.
 *  The binary heap is implemented using a QList\<T\>.
 *  To create a AVPriorityQueue\<T, PriorityComparator\>
 *  you will need a comparePriority function, returning true when the first argument has a
 *  smaller priority than the second.
 *
 *  AVPriorityQueue stores a queue of items. The default constructor creates an
 *  empty queue. To insert items into the list, you can use operator<<() or the function insert().
 *
 *  AVPriorityQueue provides these basic functions to add and remove
 *  items: insert() and pop() enqueue(), dequeue(). In addition, it provides the following
 *  convenience functions removeFirst(), append(), push(),
 *  pop()
 *
 *  To access the first element you can use head() or one of these
 *  convenience functions: top() and first().
 *  AVPriorityQueue's value type must be an assignable data type. This
 *  covers most data types that are commonly used, but the compiler
 *  won't let you, for example, store a QWidget as a value; instead,
 *  store a QWidget *.
 *
 *  AVPriorityQueue does not support inserting, prepending, appending or
 *  replacing with references to its own values. Doing so will cause
 *  your application to abort with an error message.
 *
 *  To make AVPriorityQueue as efficient as possible, its member functions don't
 *  validate their input before using it. Except for isEmpty(), member
 *  functions always assume the list is not empty. Member functions
 *  that take index values as parameters always assume their index
 *  value parameters are in the valid range. This means AVPriorityQueue member
 *  functions can fail. If you define QT_NO_DEBUG when you compile,
 *  failures will not be detected. If you don't define QT_NO_DEBUG,
 *  failures will be detected using Q_ASSERT() or Q_ASSERT_X() with an
 *  appropriate message.
 *
 *  To avoid failures when your list can be empty, call isEmpty()
 *  before calling other member functions. If you must pass an index
 *  value that might not be in the valid range, check that it is less
 *  than the value returned by size() but not less than 0.
 *
*/

template <class T> class QList;

template <class T, typename PriorityComparator = qLess < T > >
class AVPriorityQueuePrivate
{
public:
    inline explicit AVPriorityQueuePrivate(PriorityComparator l) : comparePriority(l), d() {}
    inline ~AVPriorityQueuePrivate() {}
    inline void clear() { return d.clear(); }
    inline int size() const { return d.size(); }
    inline bool empty() const { return d.empty(); }
    inline T &top() { return d.front(); }
    inline const T &top() const { return d.front(); }
    void pop();
    void push(const T &value);
    const QList<T> getList() const { return QList < T >(d); }
private:
    inline int parent(int i) {
        return (i - 1) / 2;
    }
    inline int leftChild(int i) {
        return 2 * i + 1;
    }
    inline int rightChild(int i) {
        return 2 * i + 2;
    }
    PriorityComparator comparePriority;
    QList < T > d;
};

template <class T, typename PriorityComparator = qLess < T > >
class AVPriorityQueue
{
public:
    //! Constructs an empty queue comparing priorities with the \a comparePriority function.
    inline explicit AVPriorityQueue(PriorityComparator l = qLess < T >()) :
            comparePriority(l), d(comparePriority) { }
    //! Constructs a copy of \a other.
    inline explicit AVPriorityQueue(const AVPriorityQueue<T> &q) :
            comparePriority(q.comparePriority), d(q.d) { }
    //! Destroys the queue. References to the values in the queue become invalid.
    inline ~AVPriorityQueue() { }
    //! Assigns other to this queue and returns a reference to this list.
    AVPriorityQueue<T> &operator=(const AVPriorityQueue<T> &q) { d = q.d; return *this; }

    //! Returns the number of items in the queue.
    inline int size() const { return d.size(); }
    //! Returns true if the queue contains no items; otherwise returns false.
    inline bool isEmpty() const { return d.empty(); }

    //! Removes all items from the queue.
    void clear();
    //! removes an item from the queue and resorts the queue; not the item T needs to have the
    //! operator==() implemented; returns number of elements that are removed
    int remove(const T & t);
    //! removes all items in the list from the queue and resorts the queue; not the item T needs
    //! to have the operator==() implemented; returns number of elements that are removed
    int remove(const QList <T> & list);

    //! triggers a resorting of the priority list
    //!(e.g. if priority parameter change within the list)
    void resort();

    //! Returns a copy of the underlying list (Note: this list is unsorted!)
    const QList<T> getList() const { return d.getList(); }

    //! Returns a reference to the queue's head item without removing it.
    //! This function assumes that the queue isn't empty.
    inline T &peek() { return d.top(); }
    //! overload of peek().
    inline const T &peek() const { return d.top(); }
    //! overload of peek().
    inline const T& top() const { return d.top(); }

    //! Adds value t to the queue.
    void insert(const T &t) { return push(t); }
    //! Adds List to the queue.
    void insert(const QList<T> &t);
    //! overload to insert
    inline void push(const T& t) { return d.push(t); }
    //! overload to insert
    inline AVPriorityQueue<T> &operator+=(const T &t) { push(t); return *this; }
    //! overload to insert
    inline AVPriorityQueue<T> &operator<< (const T &t) { push(t); return *this; }

    //! Returns a reference to the queue's head item and removes it.
    //! This function assumes that the queue isn't empty.
    T pop() { T t = d.top(); d.pop(); return t; }

    //! returns true if the head item equals the parameter
    inline bool startsWith(const T &t) const { return !isEmpty() && (peek() == t); }

private:
    //! PriorityComparator function which determines the ordering of the elements in the list
    PriorityComparator comparePriority;
    //! Container of the elements
    AVPriorityQueuePrivate <T> d;
};

template <typename T, typename PriorityComparator>
void AVPriorityQueue<T, PriorityComparator>::clear()
{
    d.clear();
}

template <typename T, typename PriorityComparator>
int AVPriorityQueue<T, PriorityComparator>::remove(const T& t)
{
    QList<T> copy = d.getList();
    int count = copy.removeAll(t);
    d.clear();
    this->insert(copy);
    return count;
}

template <typename T, typename PriorityComparator>
int AVPriorityQueue<T, PriorityComparator>::remove(const QList<T> & list)
{
    QList<T> copy = d.getList();
    int count = 0;
    foreach (T t , list)
        count += copy.removeAll(t);
    d.clear();
    foreach (T t , copy)
        this->insert(t);
    return count;
}

template <typename T, typename PriorityComparator>
void AVPriorityQueue<T, PriorityComparator>::resort()
{
    QList<T> copy = d.getList();
    d.clear();
    foreach (T t , copy)
        this->insert(t);
    return;
}

template <typename T, typename PriorityComparator>
void AVPriorityQueue<T, PriorityComparator>::insert(const QList<T> &t)
{
    foreach (T & e, t)
        push(e);
}

/*!
* Pop an element from the queue and reorder it using an
* inlined binary heap.
*/

template <typename T, typename PriorityComparator>
void AVPriorityQueuePrivate<T, PriorityComparator>::pop()
{
    int i = 0;
    int size = d.size();

    if(!size)
        return;
    if(size == 1)
        return d.clear();

    d[0] = d.takeLast();

    while(i < size - 1)
    {
        int left = leftChild(i);
        int right = rightChild(i);
        bool validLeft = left < size-1;
        bool validRight = right < size-1;

        if(validLeft && comparePriority(d.at(i), d.at(left)))
            if(validRight && !comparePriority(d.at(right), d.at(left)))
            {
                d.swap(i, right);
                i = right;
            } else {
                d.swap(i, left);
                i = left;
            }
        else if(validRight && comparePriority(d.at(i), d.at(right)))
        {
            d.swap(i, right);
            i = right;
        }
        else
            break;
    }
}

/*!
* Push an element with a given priority to the right place.
*/
template <typename T, typename PriorityComparator>
void AVPriorityQueuePrivate<T, PriorityComparator>::push(const T &value)
{
    int i = d.size();
    d.append(value);
    while(i != 0 && !comparePriority(d.at(i), d.at(parent(i))))
    {
        d.swap(i, parent(i));
        i = parent(i);
    }
}

#endif

// End of file
