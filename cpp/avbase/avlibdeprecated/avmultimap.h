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
    \author  Dietmar Goesseringer, d.goesseringer@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   AVMultiMap extending QMap to support multiple entries with the same key

    The AVMultiMap class is a value-based template class that provides a dictionary allowing
    multiple entries per key.
*/

///////////////////////////////////////////////////////////////////////////////////////////////////

#if !defined(AVMULTIMAP_H_INCLUDED)
#define AVMULTIMAP_H_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////

#include <QSharedData>
#include <QDataStream>
#include <QPair>
#include <QLinkedList>

#include "avlibdeprecated_export.h"
#include "avlog.h"
#include "avdeprecate.h"

#if defined(Q_OS_LINUX)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

struct AVLIBDEPRECATED_EXPORT AVMultiMapNodeBase
{
    enum Color { Red, Black };

    AVMultiMapNodeBase* left;
    AVMultiMapNodeBase* right;
    AVMultiMapNodeBase* parent;

    Color color;

    AVMultiMapNodeBase* minimum() {
        AVMultiMapNodeBase* x = this;
        while ( x->left )
            x = x->left;
        return x;
    }

    AVMultiMapNodeBase* maximum() {
        AVMultiMapNodeBase* x = this;
        while ( x->right )
            x = x->right;
        return x;
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////////

template <class K, class T>
struct AVMultiMapNode : public AVMultiMapNodeBase
{
    AVMultiMapNode( const K& _key, const T& _data ) { data = _data; key = _key; }
    AVMultiMapNode( const K& _key )      { key = _key; }
    AVMultiMapNode( const AVMultiMapNode<K,T>& _n ) { key = _n.key; data = _n.data; }
    AVMultiMapNode() { }
    T data;
    K key;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

template<class K, class T>
class AVMultiMapIterator
{
 public:
    /**
     * Typedefs
     */
    typedef AVMultiMapNode< K, T >* NodePtr;
    typedef T          value_type;
    typedef int        difference_type;
    typedef T*         pointer;
    typedef T&         reference;

    /**
     * Variables
     */
    AVMultiMapNode<K,T>* node;

    /**
     * Functions
     */
    AVMultiMapIterator() : node( 0 ) {}
    explicit AVMultiMapIterator( AVMultiMapNode<K,T>* p ) : node( p ) {}
    AVMultiMapIterator( const AVMultiMapIterator<K,T>& it ) : node( it.node ) {}

    bool operator==( const AVMultiMapIterator<K,T>& it ) const { return node == it.node; }
    bool operator!=( const AVMultiMapIterator<K,T>& it ) const { return node != it.node; }
    T& operator*() { return node->data; }
    const T& operator*() const { return node->data; }
    // UDT for T = x*
    // T* operator->() const { return &node->data; }

    const K& key() const { return node->key; }
    T& data() { return node->data; }
    const T& data() const { return node->data; }

private:
    int inc();
    int dec();

public:
    AVMultiMapIterator<K,T>& operator++() {
        inc();
        return *this;
    }

    AVMultiMapIterator<K,T> operator++(int) {
        AVMultiMapIterator<K,T> tmp = *this;
        inc();
        return tmp;
    }

    AVMultiMapIterator<K,T>& operator--() {
        dec();
        return *this;
    }

    AVMultiMapIterator<K,T> operator--(int) {
        AVMultiMapIterator<K,T> tmp = *this;
        dec();
        return tmp;
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////////

template <class K, class T>
int AVMultiMapIterator<K,T>::inc()
{
    AVMultiMapNodeBase* tmp = node;
    if ( tmp->right ) {
        tmp = tmp->right;
        while ( tmp->left )
            tmp = tmp->left;
    } else {
        AVMultiMapNodeBase* y = tmp->parent;
        while (tmp == y->right) {
            tmp = y;
            y = y->parent;
        }
        if (tmp->right != y)
            tmp = y;
    }
    node = (NodePtr)tmp;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

template <class K, class T>
int AVMultiMapIterator<K,T>::dec()
{
    AVMultiMapNodeBase* tmp = node;
    if (tmp->color == AVMultiMapNodeBase::Red &&
        tmp->parent->parent == tmp ) {
        tmp = tmp->right;
    } else if (tmp->left != 0) {
        AVMultiMapNodeBase* y = tmp->left;
        while ( y->right )
            y = y->right;
        tmp = y;
    } else {
        AVMultiMapNodeBase* y = tmp->parent;
        while (tmp == y->left) {
            tmp = y;
            y = y->parent;
        }
        tmp = y;
    }
    node = (NodePtr)tmp;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

template<class K, class T>
class AVMultiMapConstIterator
{
 public:
    /**
     * Typedefs
     */
    typedef AVMultiMapNode< K, T >* NodePtr;
    typedef T          value_type;
    typedef int difference_type;
    typedef const T*   pointer;
    typedef const T&   reference;


    /**
     * Variables
     */
    AVMultiMapNode<K,T>* node;

    /**
     * Functions
     */
    AVMultiMapConstIterator() : node( 0 ) {}
    explicit AVMultiMapConstIterator( AVMultiMapNode<K,T>* p ) : node( p ) {}
    AVMultiMapConstIterator( const AVMultiMapConstIterator<K,T>& it ) : node( it.node ) {}
    explicit AVMultiMapConstIterator( const AVMultiMapIterator<K,T>& it ) : node( it.node ) {}

    bool operator==( const AVMultiMapConstIterator<K,T>& it ) const { return node == it.node; }
    bool operator!=( const AVMultiMapConstIterator<K,T>& it ) const { return node != it.node; }
    const T& operator*()  const { return node->data; }
    // UDT for T = x*
    // const T* operator->() const { return &node->data; }

    const K& key() const { return node->key; }
    const T& data() const { return node->data; }

private:
    int inc();
    int dec();

public:
    AVMultiMapConstIterator<K,T>& operator++() {
        inc();
        return *this;
    }

    AVMultiMapConstIterator<K,T> operator++(int) {
        AVMultiMapConstIterator<K,T> tmp = *this;
        inc();
        return tmp;
    }

    AVMultiMapConstIterator<K,T>& operator--() {
        dec();
        return *this;
    }

    AVMultiMapConstIterator<K,T> operator--(int) {
        AVMultiMapConstIterator<K,T> tmp = *this;
        dec();
        return tmp;
    }
};

template <class K, class T>
int AVMultiMapConstIterator<K,T>::inc()
{
    AVMultiMapNodeBase* tmp = node;
    if ( tmp->right ) {
        tmp = tmp->right;
        while ( tmp->left )
            tmp = tmp->left;
    } else {
        AVMultiMapNodeBase* y = tmp->parent;
        while (tmp == y->right) {
            tmp = y;
            y = y->parent;
        }
        if (tmp->right != y)
            tmp = y;
    }
    node = (NodePtr)tmp;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

template <class K, class T>
int AVMultiMapConstIterator<K,T>::dec()
{
    AVMultiMapNodeBase* tmp = node;
    if (tmp->color == AVMultiMapNodeBase::Red &&
        tmp->parent->parent == tmp ) {
        tmp = tmp->right;
    } else if (tmp->left != 0) {
        AVMultiMapNodeBase* y = tmp->left;
        while ( y->right )
            y = y->right;
        tmp = y;
    } else {
        AVMultiMapNodeBase* y = tmp->parent;
        while (tmp == y->left) {
            tmp = y;
            y = y->parent;
        }
        tmp = y;
    }
    node = (NodePtr)tmp;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

class AVLIBDEPRECATED_EXPORT AVMultiMapPrivateBase : public QSharedData
{
public:
    AVMultiMapPrivateBase() {
        node_count = 0;
    }
    explicit AVMultiMapPrivateBase( const AVMultiMapPrivateBase* _map) {
        node_count = _map->node_count;
    }

    /**
     * Implementations of basic tree algorithms
     */
    void rotateLeft( AVMultiMapNodeBase* x, AVMultiMapNodeBase*& root);
    void rotateRight( AVMultiMapNodeBase* x, AVMultiMapNodeBase*& root );
    void rebalance( AVMultiMapNodeBase* x, AVMultiMapNodeBase*& root );
    AVMultiMapNodeBase* removeAndRebalance( AVMultiMapNodeBase* z, AVMultiMapNodeBase*& root,
                                   AVMultiMapNodeBase*& leftmost,
                                   AVMultiMapNodeBase*& rightmost );

    /**
     * Variables
     */
    int node_count;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

template <class Key, class T>
class AVMultiMapPrivate : public AVMultiMapPrivateBase
{
public:
    /**
     * Typedefs
     */
    typedef AVMultiMapIterator< Key, T > Iterator;
    typedef AVMultiMapConstIterator< Key, T > ConstIterator;
    typedef AVMultiMapNode< Key, T > Node;
    typedef AVMultiMapNode< Key, T >* NodePtr;

    /**
     * Functions
     */
    AVMultiMapPrivate();
    explicit AVMultiMapPrivate( const AVMultiMapPrivate< Key, T >* _map );
    ~AVMultiMapPrivate() { clear(); delete header; }

    NodePtr copy( NodePtr p );
    void clear();
    void clear( NodePtr p );

    Iterator begin()    { return Iterator( (NodePtr)(header->left ) ); }
    Iterator end()      { return Iterator( header ); }
    ConstIterator begin() const { return ConstIterator( (NodePtr)(header->left ) ); }
    ConstIterator end() const { return ConstIterator( header ); }

    ConstIterator find(const Key& k) const;

    void remove( const Iterator& it ) {
        NodePtr del =
            (NodePtr) removeAndRebalance( it.node, header->parent, header->left, header->right );
        delete del;
        --node_count;
    }

#ifdef QT_QMAP_DEBUG
    void inorder( AVMultiMapNodeBase* x = 0, int level = 0 ){
        if ( !x )
            x = header->parent;
        if ( x->left )
            inorder( x->left, level + 1 );
    //cout << level << " Key=" << key(x) << " Value=" << ((NodePtr)x)->data << endl;
        if ( x->right )
            inorder( x->right, level + 1 );
    }
#endif

    Iterator insertMulti(const Key& v){
        AVMultiMapNodeBase* y = header;
        AVMultiMapNodeBase* x = header->parent;
        while (x != 0){
            y = x;
            x = ( v < key(x) ) ? x->left : x->right;
        }
        return insert(x, y, v);
    }

    Iterator insertSingle( const Key& k );
    Iterator insert( AVMultiMapNodeBase* x, AVMultiMapNodeBase* y, const Key& k );

protected:
    /**
     * Helpers
     */
    const Key& key( AVMultiMapNodeBase* b ) const { return ((NodePtr)b)->key; }

    /**
     * Variables
     */
    NodePtr header;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

template <class Key, class T>
AVMultiMapPrivate<Key,T>::AVMultiMapPrivate() {
    header = new Node;
    AVASSERT(header != 0);
    header->color = AVMultiMapNodeBase::Red; // Mark the header
    header->parent = 0;
    header->left = header->right = header;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

template <class Key, class T>
AVMultiMapPrivate<Key,T>::AVMultiMapPrivate( const AVMultiMapPrivate< Key, T >* _map ) :
    AVMultiMapPrivateBase( _map )
{
    header = new Node;
    AVASSERT(header != 0);
    header->color = AVMultiMapNodeBase::Red; // Mark the header
    if ( _map->header->parent == 0 ) {
        header->parent = 0;
        header->left = header->right = header;
    } else {
        header->parent = copy( (NodePtr)(_map->header->parent) );
        header->parent->parent = header;
        header->left = header->parent->minimum();
        header->right = header->parent->maximum();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

template <class Key, class T>
typename AVMultiMapPrivate<Key,T>::NodePtr AVMultiMapPrivate<Key,T>::copy(
    typename AVMultiMapPrivate<Key,T>::NodePtr p )
{
    if ( !p )
        return 0;
    NodePtr n = new Node( *p );
    AVASSERT(n != 0);
    n->color = p->color;
    if ( p->left ) {
        n->left = copy( (NodePtr)(p->left) );
        n->left->parent = n;
    } else {
        n->left = 0;
    }
    if ( p->right ) {
        n->right = copy( (NodePtr)(p->right) );
        n->right->parent = n;
    } else {
        n->right = 0;
    }
    return n;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

template <class Key, class T>
void AVMultiMapPrivate<Key,T>::clear()
{
    clear( (NodePtr)(header->parent) );
    header->color = AVMultiMapNodeBase::Red;
    header->parent = 0;
    header->left = header->right = header;
    node_count = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

template <class Key, class T>
void AVMultiMapPrivate<Key,T>::clear(
    typename AVMultiMapPrivate<Key,T>::NodePtr p )
{
    while ( p != 0 ) {
        clear( (NodePtr)p->right );
        NodePtr y = (NodePtr)p->left;
        delete p;
        p = y;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

template <class Key, class T>
typename AVMultiMapPrivate<Key,T>::ConstIterator AVMultiMapPrivate<Key,T>::find(
    const Key& k) const
{
    AVMultiMapNodeBase* y = header;        // Last node
    AVMultiMapNodeBase* x = header->parent; // Root node.

    while ( x != 0 ) {
        // If as k <= key(x) go left
        if ( !( key(x) < k ) ) {
            y = x;
            x = x->left;
        } else {
            x = x->right;
        }
    }

    // Was k bigger/smaller then the biggest/smallest
    // element of the tree ? Return end()
    if ( y == header || k < key(y) )
        return ConstIterator( header );
    return ConstIterator( (NodePtr)y );
}

///////////////////////////////////////////////////////////////////////////////////////////////////

template <class Key, class T>
typename AVMultiMapPrivate<Key,T>::Iterator AVMultiMapPrivate<Key,T>::insertSingle(
    const Key& k )
{
    // Search correct position in the tree
    AVMultiMapNodeBase* y = header;
    AVMultiMapNodeBase* x = header->parent;
    bool result = true;
    while ( x != 0 ) {
        result = ( k < key(x) );
        y = x;
        x = result ? x->left : x->right;
    }
    // Get iterator on the last not empty one
    Iterator j( (NodePtr)y );
    if ( result ) {
        // Smaller then the leftmost one ?
        if ( j == begin() ) {
            return insert(x, y, k );
        } else {
            // Perhaps daddy is the right one ?
            --j;
        }
    }
    // Really bigger ?
    if ( (j.node->key) < k )
        return insert(x, y, k );
    // We are going to replace a node
    return j;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

template <class Key, class T>
typename AVMultiMapPrivate<Key,T>::Iterator AVMultiMapPrivate<Key,T>::insert(
    AVMultiMapNodeBase* x,
    AVMultiMapNodeBase* y,
    const Key& k )
{
    NodePtr z = new Node( k );
    AVASSERT(z != 0);
    if (y == header || x != 0 || k < key(y) ) {
        y->left = z;                // also makes leftmost = z when y == header
        if ( y == header ) {
            header->parent = z;
            header->right = z;
        } else if ( y == header->left )
            header->left = z;           // maintain leftmost pointing to min node
    } else {
        y->right = z;
        if ( y == header->right )
            header->right = z;          // maintain rightmost pointing to max node
    }
    z->parent = y;
    z->left = 0;
    z->right = 0;
    rebalance( z, header->parent );
    ++node_count;
    return Iterator(z);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef QT_CHECK_RANGE
# if !defined( QT_NO_DEBUG ) && defined( QT_CHECK_MAP_RANGE )
#  define QT_CHECK_INVALID_MAP_ELEMENT if ( empty() ) qWarning("AVMultiMap: Warning invalid elem")
#  define QT_CHECK_INVALID_MAP_ELEMENT_FATAL Q_ASSERT( !empty() );
# else
#  define QT_CHECK_INVALID_MAP_ELEMENT
#  define QT_CHECK_INVALID_MAP_ELEMENT_FATAL
# endif
#else
# define QT_CHECK_INVALID_MAP_ELEMENT
# define QT_CHECK_INVALID_MAP_ELEMENT_FATAL
#endif




/*! \brief
    This class is considered DEPRECATED. QMultiMap should be used.

    The AVMultiMap class is a value-based template class that provides a dictionary
    allowing multiple entries per key.

    For insertion of a new item, use the method insert( const Key& key, const T& value ),
    especially for the insertion of a new item with a key that already exists,
    rather than the operator[]( const Key& k ) which inserts a default value for a non-existing
    key and overwrites the value for an already existing key.

    For more, refer to the Qt documentation of QMap.
*/
template<class Key, class T>
class AVMultiMap
{
public:
    /**
     * Typedefs
     */
    typedef Key key_type;
    typedef T mapped_type;
    typedef QPair<const key_type, mapped_type> value_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef int difference_type;
    typedef size_t      size_type;
    typedef AVMultiMapIterator<Key,T> iterator;
    typedef AVMultiMapConstIterator<Key,T> const_iterator;
    typedef QPair<iterator,bool> insert_pair;

    typedef AVMultiMapIterator< Key, T > Iterator;
    typedef AVMultiMapConstIterator< Key, T > ConstIterator;
    typedef T ValueType;
    typedef AVMultiMapPrivate< Key, T > Priv;

    /**
     * API
     */
    AVMultiMap()
    {
        sh = new (LOG_HERE) AVMultiMapPrivate< Key, T >;
        AVDEPRECATE(AVMultiMap);
    }
    AVMultiMap( const AVMultiMap<Key,T>& m )
    {
        sh = m.sh; sh->ref.ref();
        AVDEPRECATE(AVMultiMap);
    }

    ~AVMultiMap()
    {
        if ( sh->ref.deref() )
            delete sh;
    }
    AVMultiMap<Key,T>& operator= ( const AVMultiMap<Key,T>& m );

    iterator begin() { detach(); return sh->begin(); }
    iterator end() { detach(); return sh->end(); }
    const_iterator begin() const { return ((const Priv*)sh)->begin(); }
    const_iterator end() const { return ((const Priv*)sh)->end(); }
    const_iterator constBegin() const { return begin(); }
    const_iterator constEnd() const { return end(); }

    iterator replace( const Key& k, const T& v )
    {
        remove( k );
        return insert( k, v );
    }

    size_type size() const
    {
        return sh->node_count;
    }
    bool empty() const
    {
        return sh->node_count == 0;
    }
    QPair<iterator,bool> insert( const value_type& x );

    void erase( const iterator& it )
    {
        detach();
        sh->remove( it );
    }
    void erase( const key_type& k );
    size_type count( const key_type& k ) const;
    T& operator[] ( const Key& k );
    void clear();

    iterator find ( const Key& k )
    {
        detach();
        return iterator( sh->find( k ).node );
    }
    const_iterator find ( const Key& k ) const {        return sh->find( k ); }

    const T& operator[] ( const Key& k ) const
        { QT_CHECK_INVALID_MAP_ELEMENT; return sh->find( k ).data(); }
    bool contains ( const Key& k ) const
        { return find( k ) != end(); }
        //{ return sh->find( k ) != ((const Priv*)sh)->end(); }

    size_type count() const { return sh->node_count; }

    QLinkedList<Key> keys() const {
        QLinkedList<Key> r;
        for (const_iterator i=begin(); i!=end(); ++i)
            r.append(i.key());
        return r;
    }

    QLinkedList<T> values() const {
        QLinkedList<T> r;
        for (const_iterator i=begin(); i!=end(); ++i)
            r.append(*i);
        return r;
    }

    bool isEmpty() const { return sh->node_count == 0; }

    iterator insert( const Key& key, const T& value );
    void remove( const iterator& it ) { detach(); sh->remove( it ); }
    void remove( const Key& k );

#if defined(Q_FULL_TEMPLATE_INSTANTIATION)
    bool operator==( const AVMultiMap<Key,T>& ) const { return false; }
#endif

protected:
    /**
     * Helpers
     */
    void detach() {  if ( sh->ref > 1 ) detachInternal(); }

    Priv* sh;
private:
    void detachInternal();

};

///////////////////////////////////////////////////////////////////////////////////////////////////

template<class Key, class T>
AVMultiMap<Key,T>& AVMultiMap<Key,T>::operator= ( const AVMultiMap<Key,T>& m )
{
    m.sh->ref.ref();
    if ( sh->ref.deref() )
        delete sh;
    sh = m.sh;
    return *this;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

template<class Key, class T>
typename AVMultiMap<Key,T>::insert_pair AVMultiMap<Key,T>::insert(
    const typename AVMultiMap<Key,T>::value_type& x )
{
    detach();
    size_type n = size();
    iterator it = sh->insertMulti( x.first );
    bool inserted = false;
    if ( n < size() ) {
        inserted = true;
        it.data() = x.second;
    }
    return QPair<iterator,bool>( it, inserted );
}

///////////////////////////////////////////////////////////////////////////////////////////////////

template<class Key, class T>
void AVMultiMap<Key,T>::erase( const Key& k )
{
    detach();
    iterator it( sh->find( k ).node );
    if ( it != end() )
        sh->remove( it );
}

///////////////////////////////////////////////////////////////////////////////////////////////////

template<class Key, class T>
typename AVMultiMap<Key,T>::size_type AVMultiMap<Key,T>::count(
    const Key& k ) const
{
    const_iterator it( sh->find( k ).node );
    if ( it != end() ) {
        size_type c = 0;
        while ( it != end() ) {
            ++it;
            ++c;
        }
        return c;
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

template<class Key, class T>
T& AVMultiMap<Key,T>::operator[] ( const Key& k )
{
    detach();
    AVMultiMapNode<Key,T>* p = sh->find( k ).node;
    if ( p != sh->end().node )
        return p->data;
    return insert( k, T() ).data();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

template<class Key, class T>
void AVMultiMap<Key,T>::clear()
{
    if ( sh->ref == 1 )
        sh->clear();
    else {
        sh->ref.deref();
        sh = new AVMultiMapPrivate<Key,T>;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

template<class Key, class T>
typename AVMultiMap<Key,T>::iterator AVMultiMap<Key,T>::insert(
    const Key& key,
    const T& value )
{
    detach();
    iterator it = sh->insertMulti( key );
    it.data() = value;
    return it;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

template<class Key, class T>
void AVMultiMap<Key,T>::remove( const Key& k )
{
    detach();
    iterator it( sh->find( k ).node );
    if ( it != end() )
        sh->remove( it );
}

///////////////////////////////////////////////////////////////////////////////////////////////////

template<class Key, class T>
void AVMultiMap<Key,T>::detachInternal()
{
    sh->ref.deref(); sh = new AVMultiMapPrivate<Key,T>( sh );
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef QT_NO_DATASTREAM
template<class Key, class T>
QDataStream& operator>>( QDataStream& s, AVMultiMap<Key,T>& m ) {
    m.clear();
    quint32 c;
    s >> c;
    for( quint32 i = 0; i < c; ++i ) {
        Key k; T t;
        s >> k >> t;
        m.insert( k, t );
        if ( s.atEnd() )
            break;
    }
    return s;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

template<class Key, class T>
QDataStream& operator<<( QDataStream& s, const AVMultiMap<Key,T>& m ) {
    s << (quint32)m.size();
    AVMultiMapConstIterator<Key,T> it = m.begin();
    for( ; it != m.end(); ++it )
        s << it.key() << it.data();
    return s;
}
#endif // #ifndef QT_NO_DATASTREAM

#if defined(Q_OS_LINUX)
#pragma GCC diagnostic pop
#endif


#endif // #if !defined(AVMULTIMAP_H_INCLUDED)

// End of file
