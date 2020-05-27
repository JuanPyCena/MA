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

// local includes
#include "avmultimap.h"

///////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////

typedef AVMultiMapNodeBase* NodePtr;
typedef AVMultiMapNodeBase Node;

///////////////////////////////////////////////////////////////////////////////////////////////////

void AVMultiMapPrivateBase::rotateLeft( NodePtr x, NodePtr& root)
{
    NodePtr y = x->right;
    x->right = y->left;
    if (y->left !=0)
    y->left->parent = x;
    y->parent = x->parent;
    if (x == root)
    root = y;
    else if (x == x->parent->left)
    x->parent->left = y;
    else
    x->parent->right = y;
    y->left = x;
    x->parent = y;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void AVMultiMapPrivateBase::rotateRight( NodePtr x, NodePtr& root )
{
    NodePtr y = x->left;
    x->left = y->right;
    if (y->right != 0)
    y->right->parent = x;
    y->parent = x->parent;
    if (x == root)
    root = y;
    else if (x == x->parent->right)
    x->parent->right = y;
    else
    x->parent->left = y;
    y->right = x;
    x->parent = y;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void AVMultiMapPrivateBase::rebalance( NodePtr x, NodePtr& root)
{
    x->color = Node::Red;
    while ( x != root && x->parent->color == Node::Red ) {
    if ( x->parent == x->parent->parent->left ) {
        NodePtr y = x->parent->parent->right;
        if (y && y->color == Node::Red) {
        x->parent->color = Node::Black;
        y->color = Node::Black;
        x->parent->parent->color = Node::Red;
        x = x->parent->parent;
        } else {
        if (x == x->parent->right) {
            x = x->parent;
            rotateLeft( x, root );
        }
        x->parent->color = Node::Black;
        x->parent->parent->color = Node::Red;
        rotateRight (x->parent->parent, root );
        }
    } else {
        NodePtr y = x->parent->parent->left;
        if ( y && y->color == Node::Red ) {
        x->parent->color = Node::Black;
        y->color = Node::Black;
        x->parent->parent->color = Node::Red;
        x = x->parent->parent;
        } else {
        if (x == x->parent->left) {
            x = x->parent;
            rotateRight( x, root );
        }
        x->parent->color = Node::Black;
        x->parent->parent->color = Node::Red;
        rotateLeft( x->parent->parent, root );
        }
    }
    }
    root->color = Node::Black;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

NodePtr AVMultiMapPrivateBase::removeAndRebalance( NodePtr z, NodePtr& root,
                         NodePtr& leftmost,
                         NodePtr& rightmost )
{
    NodePtr y = z;
    NodePtr x;
    NodePtr x_parent;
    if (y->left == 0) {
    x = y->right;
    } else {
    if (y->right == 0)
        x = y->left;
    else
        {
        y = y->right;
        while (y->left != 0)
            y = y->left;
        x = y->right;
        }
    }
    if (y != z) {
    z->left->parent = y;
    y->left = z->left;
    if (y != z->right) {
        x_parent = y->parent;
        if (x)
        x->parent = y->parent;
        y->parent->left = x;
        y->right = z->right;
        z->right->parent = y;
    } else {
        x_parent = y;
    }
    if (root == z)
        root = y;
    else if (z->parent->left == z)
        z->parent->left = y;
    else
        z->parent->right = y;
    y->parent = z->parent;
    // Swap the colors
    Node::Color c = y->color;
    y->color = z->color;
    z->color = c;
    y = z;
    } else {
    x_parent = y->parent;
    if (x)
        x->parent = y->parent;
    if (root == z)
        root = x;
    else if (z->parent->left == z)
        z->parent->left = x;
    else
        z->parent->right = x;
    if ( leftmost == z ) {
        if (z->right == 0)
        leftmost = z->parent;
        else
        leftmost = x->minimum();
    }
    if (rightmost == z) {
        if (z->left == 0)
        rightmost = z->parent;
        else
        rightmost = x->maximum();
    }
    }
    if (y->color != Node::Red) {
    while (x != root && (x == 0 || x->color == Node::Black)) {
        if (x == x_parent->left) {
        NodePtr w = x_parent->right;
        if (w->color == Node::Red) {
            w->color = Node::Black;
            x_parent->color = Node::Red;
            rotateLeft(x_parent, root);
            w = x_parent->right;
        }
        if ((w->left == 0 || w->left->color == Node::Black) &&
            (w->right == 0 || w->right->color == Node::Black)) {
            w->color = Node::Red;
            x = x_parent;
            x_parent = x_parent->parent;
        } else {
            if (w->right == 0 || w->right->color == Node::Black) {
            if (w->left)
                w->left->color = Node::Black;
            w->color = Node::Red;
            rotateRight(w, root);
            w = x_parent->right;
            }
            w->color = x_parent->color;
            x_parent->color = Node::Black;
            if (w->right)
            w->right->color = Node::Black;
            rotateLeft(x_parent, root);
            break;
        }
        } else {
        NodePtr w = x_parent->left;
        if (w->color == Node::Red) {
            w->color = Node::Black;
            x_parent->color = Node::Red;
            rotateRight(x_parent, root);
            w = x_parent->left;
        }
        if ((w->right == 0 || w->right->color == Node::Black) &&
            (w->left == 0 || w->left->color == Node::Black)) {
            w->color = Node::Red;
            x = x_parent;
            x_parent = x_parent->parent;
        } else {
            if (w->left == 0 || w->left->color == Node::Black) {
            if (w->right)
                w->right->color = Node::Black;
            w->color = Node::Red;
            rotateLeft(w, root);
            w = x_parent->left;
            }
            w->color = x_parent->color;
            x_parent->color = Node::Black;
            if (w->left)
            w->left->color = Node::Black;
            rotateRight(x_parent, root);
            break;
        }
        }
    }
    if (x)
        x->color = Node::Black;
    }
    return y;
}

// End of file
