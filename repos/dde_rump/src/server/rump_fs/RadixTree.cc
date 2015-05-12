

#include "RadixTree.h"
#include <base/allocator_avl.h>
#include <os/config.h>
#include <os/path.h>
#include <util/string.h>

//
// RadixTree<void*> implementation.
//

const uint8_t nullKey[] = {0};

int toLower ( char c )
{
    if ( c >= 'A' && c <= 'Z' )
    {
        return c - 'A' + 'a';
    }

    else
    {
        return c;
    }
}

RadixTree<void *>::RadixTree() :
    m_nItems ( 0 ), m_pRoot ( 0 ), m_bCaseSensitive ( true )
{
}

RadixTree<void *>::RadixTree ( bool bCaseSensitive ) :
    m_nItems ( 0 ), m_pRoot ( 0 ), m_bCaseSensitive ( bCaseSensitive )
{
}

RadixTree<void *>::~RadixTree()
{
    clear();
    delete m_pRoot;
}

RadixTree<void *>::RadixTree ( const RadixTree & x ) :
    m_nItems ( 0 ), m_pRoot ( 0 ), m_bCaseSensitive ( x.m_bCaseSensitive )
{
    clear();
    delete m_pRoot;
    m_pRoot = cloneNode ( x.m_pRoot, 0 );
    m_nItems = x.m_nItems;
}

RadixTree<void *> & RadixTree<void *>::operator = ( const RadixTree & x )
{
    clear();
    delete m_pRoot;
    m_pRoot = cloneNode ( x.m_pRoot, 0 );
    m_nItems = x.m_nItems;
    m_bCaseSensitive = x.m_bCaseSensitive;
    return *this;
}

size_t RadixTree<void *>::count() const
{
    return m_nItems;
}

void RadixTree<void *>::insert ( char * key, void * value )
{
    if ( !m_pRoot )
    {
        // The root node always exists and is a lambda transition node (zero-length
        // key). This removes the need for most special cases.
        m_pRoot = new ( m_alloc ) Node ( m_bCaseSensitive );
        m_pRoot->setKey ( nullKey );
    }

    Node * pNode = m_pRoot;
    const uint8_t * cpKey = reinterpret_cast<const uint8_t *> ( static_cast<const char *> ( key ) );

    while ( true )
    {
        switch ( pNode->matchKey ( cpKey ) )
        {
            case Node::ExactMatch:
                {
                    pNode->setValue ( value );
                    return;
                }

            case Node::NoMatch:
                {
                    //FATAL("RadixTree: algorithmic error!");
                    break;
                }

            case Node::PartialMatch:
                {
                    // We need to create an intermediate node that contains the
                    // partial match, then adjust the key of this node.
                    // Find the common key prefix.
                    size_t i = 0;

                    if ( m_bCaseSensitive )
                        while ( cpKey[i] == pNode->m_pKey[i] )
                        {
                            i++;
                        }

                    else
                        while ( toLower ( cpKey[i] ) == toLower ( pNode->m_pKey[i] ) )
                        {
                            i++;
                        }

                    Node * pInter = new ( m_alloc ) Node ( m_bCaseSensitive );
                    // Intermediate node's key is the common prefix of both keys.
                    pInter->m_pKey = new ( m_alloc ) uint8_t[i + 1];
                    Genode::memcpy ( pInter->m_pKey, cpKey, i );
                    pInter->m_pKey[i] = 0;
                    // Must do this before pNode's key is changed.
                    pNode->getParent()->replaceChild ( pNode, pInter );
                    // pNode's new key is the uncommon postfix.
                    size_t len = 0;

                    while ( pNode->m_pKey[len] )
                    {
                        len++;
                    }

                    uint8_t * pTmpKey = new ( m_alloc ) uint8_t[ ( len - i ) + 1];
                    Genode::memcpy ( pTmpKey, &pNode->m_pKey[i], len - i );
                    pTmpKey[len - i] = 0;
                    pNode->m_pKey = pTmpKey;

                    // If the uncommon postfix of the key is non-zero length, we have
                    // to create another node, a child of pInter.
                    if ( cpKey[i] != 0 )
                    {
                        Node * pChild = new ( m_alloc ) Node ( m_bCaseSensitive );
                        pChild->setKey ( &cpKey[i] );
                        pChild->setValue ( value );
                        pChild->setParent ( pInter );
                        pInter->addChild ( pChild );
                    }

                    else
                    {
                        pInter->setValue ( value );
                    }

                    pInter->setParent ( pNode->getParent() );
                    pInter->addChild ( pNode );
                    pNode->setParent ( pInter );
                    m_nItems ++;
                    return;
                }

            case Node::OverMatch:
                {
                    size_t i = 0;

                    if ( pNode->m_pKey )
                        while ( pNode->m_pKey[i++] )
                        {
                            cpKey++;
                        }

                    Node * pChild = pNode->findChild ( cpKey );

                    if ( pChild )
                    {
                        pNode = pChild;
                        // Iterative case.
                        break;
                    }

                    else
                    {
                        // No child - create a new one.
                        pChild = new ( m_alloc ) Node ( m_bCaseSensitive );
                        pChild->setKey ( cpKey );
                        pChild->setValue ( value );
                        pChild->setParent ( pNode );
                        pNode->addChild ( pChild );
                        m_nItems ++;
                        return;
                    }
                }
        }
    }
}

void * RadixTree<void *>::lookup ( char * key ) const
{
    Node * pOldRoot = m_pRoot;

    if ( !m_pRoot )
    {
        return 0;
    }

    Node * pNode = m_pRoot;
    const uint8_t * cpKey = reinterpret_cast<const uint8_t *> ( static_cast<const char *> ( key ) );

    while ( true )
    {
        switch ( pNode->matchKey ( cpKey ) )
        {
            case Node::ExactMatch:
                return pNode->getValue();

            case Node::NoMatch:
            case Node::PartialMatch:
                return 0;

            case Node::OverMatch:
                {
                    size_t i = 0;

                    while ( pNode->m_pKey[i++] )
                    {
                        cpKey++;
                    }

                    Node * pChild = pNode->findChild ( cpKey );

                    if ( pChild )
                    {
                        pNode = pChild;
                        // Iterative case.
                        break;
                    }

                    else
                    {
                        return 0;
                    }
                }
        }
    }
}

void RadixTree<void *>::remove ( char * key )
{
    if ( !m_pRoot )
    {
        // The root node always exists and is a lambda transition node (zero-length
        // key). This removes the need for most special cases.
        m_pRoot = new ( m_alloc ) Node ( m_bCaseSensitive );
        m_pRoot->setKey ( nullKey );
    }

    Node * pNode = m_pRoot;
    const uint8_t * cpKey = reinterpret_cast<const uint8_t *> ( static_cast<const char *> ( key ) );

    // Our invariant is that the root node always exists. Therefore we must
    // special case here so it doesn't get deleted.
    if ( *cpKey == 0 )
    {
        m_pRoot->setValue ( 0 );
        return;
    }

    while ( true )
    {
        switch ( pNode->matchKey ( cpKey ) )
        {
            case Node::ExactMatch:
                {
                    // Delete this node. If we set the value to zero, it is effectively removed from the map.
                    // There are only certain cases in which we can delete the node completely, however.
                    pNode->setValue ( 0 );
                    m_nItems --;
                    // We have the invariant that the tree is always optimised. This means that when we delete a node
                    // we only have to optimise the local branch. There are two situations that need covering:
                    //   (a) No children. This means it's a leaf node and can be deleted. We then need to consider its parent,
                    //       which, if its value is zero and now has zero or one children can be optimised itself.
                    //   (b) One child. This is a linear progression and the child node's key can be changed to be concatenation of
                    //       pNode's and its. This doesn't affect anything farther up the tree and so no recursion is needed.
                    Node * pParent = 0;

                    if ( pNode->counter == 0 )
                    {
                        // Leaf node, can just delete.
                        pParent = pNode->getParent();
                        pParent->removeChild ( pNode );
                        delete pNode;
                        pNode = pParent;

                        // Optimise up the tree.
                        while ( true )
                        {
                            if ( pNode == m_pRoot )
                            {
                                return;
                            }

                            if ( pNode->counter == 1 && pNode->getValue() == 0 )
                                // Break out of this loop and get caught in the next
                                // if(pNode->m_nChildren == 1)
                            {
                                break;
                            }

                            if ( pNode->counter == 0 && pNode->getValue() == 0 )
                            {
                                // Leaf node, can just delete.
                                pParent = pNode->getParent();
                                pParent->removeChild ( pNode );
                                delete pNode;
                                pNode = pParent;
                                continue;
                            }

                            return;
                        }
                    }

                    if ( pNode->counter == 1 )
                    {
                        // Change the child's key to be the concatenation of ours and
                        // its.
                        Node * pChild = pNode->getFirstChild();
                        pParent = pNode->getParent();
                        // Must call this before delete, so pChild doesn't get deleted.
                        pNode->removeChild ( pChild );
                        pChild->prependKey ( pNode->getKey() );
                        pChild->setParent ( pParent );
                        pParent->removeChild ( pNode );
                        pParent->addChild ( pChild );
                        delete pNode;
                    }

                    return;
                }

            case Node::NoMatch:
            case Node::PartialMatch:
                // Can't happen unless the key didn't actually exist.
                return;

            case Node::OverMatch:
                {
                    size_t i = 0;

                    if ( pNode->m_pKey )
                        while ( pNode->m_pKey[i++] )
                        {
                            cpKey++;
                        }

                    Node * pChild = pNode->findChild ( cpKey );

                    if ( pChild )
                    {
                        pNode = pChild;
                        // Iterative case.
                        break;
                    }

                    else
                    {
                        return;
                    }
                }
        }
    }
}

RadixTree<void *>::Node * RadixTree<void *>::cloneNode ( Node * pNode, Node * pParent )
{
    // Deal with the easy case first.
    if ( !pNode )
    {
        return 0;
    }

    Node * n = new Node ( m_bCaseSensitive );
    n->setKey ( pNode->m_pKey );
    n->setValue ( pNode->value );
    n->setParent ( pParent );

    for ( int i = 0; i < pNode->counter; i++ )
    {
        n->addChild ( cloneNode ( pNode->m_Children[i], pParent ) );
    }

    return n;
}

void RadixTree<void *>::clear()
{
    //TODOdeleting m_pRoot;
    m_pRoot = new ( m_alloc ) Node ( m_bCaseSensitive );
    m_pRoot->setKey ( nullKey );
    m_nItems = 0;
}

//
// RadixTree::Node implementation.
//

RadixTree<void *>::Node::~Node()
{
    /* TODO


        for(size_t n = 0; n < counter; ++n)
        {
            delete m_Children[n];
        }
    */
}

RadixTree<void *>::Node * RadixTree<void *>::Node::findChild ( const uint8_t * cpKey )
{
    for ( size_t n = 0; n < counter; ++n )
    {
        if ( m_Children[n]->matchKey ( cpKey ) != NoMatch )
        {
            return m_Children[n];
        }
    }

    return 0;
}

void RadixTree<void *>::Node::addChild ( Node * pNode )
{
    if ( pNode )
    {
        m_Children[counter++] = pNode;
    }
}

void RadixTree<void *>::Node::replaceChild ( Node * pNodeOld, Node * pNodeNew )
{
    for ( size_t n = 0; n < counter; ++n )
    {
        if ( m_Children[n] == pNodeOld )
        {
            m_Children[n] = pNodeNew;
        }
    }
}

void RadixTree<void *>::Node::removeChild ( Node * pChild )
{
    for ( int i = 0;  i < counter; )
    {
        if ( m_Children[i] == pChild )
        {
            for ( int j = i + 1; j < counter; j++ )
            {
                m_Children[j - 1] = m_Children[j];
            }

            counter--;
        }

        else
        {
            ++i;
        }
    }
}

RadixTree<void *>::Node::MatchType RadixTree<void *>::Node::matchKey ( const uint8_t * cpKey )
{
    if ( !m_pKey )
    {
        return OverMatch;
    }

    size_t i = 0;

    while ( cpKey[i] && m_pKey[i] )
    {
        bool bMatch = false;

        if ( m_bCaseSensitive )
        {
            bMatch = cpKey[i] == m_pKey[i];
        }

        else
        {
            bMatch = toLower ( cpKey[i] ) == toLower ( m_pKey[i] );
        }

        if ( !bMatch )
        {
            return ( i == 0 ) ? NoMatch : PartialMatch;
        }

        i++;
    }

    // Why did the loop exit?
    if ( cpKey[i] == 0 && m_pKey[i] == 0 )
    {
        return ExactMatch;
    }

    else if ( cpKey[i] == 0 )
    {
        return PartialMatch;
    }

    else
    {
        return OverMatch;
    }
}

void RadixTree<void *>::Node::setKey ( const uint8_t * cpKey )
{
    if ( m_pKey );

    size_t len = 0;

    while ( cpKey[len] )
    {
        len++;
    }

    m_pKey = new ( m_alloc ) uint8_t[len + 1];
    size_t i = 0;

    while ( cpKey[i] )
    {
        m_pKey[i] = cpKey[i];
        i++;
    }

    m_pKey[i] = 0;
}

RadixTree<void *>::Node * RadixTree<void *>::Node::getFirstChild()
{
    if ( counter )
    {
        return m_Children[0];
    }

    return 0;
}

void RadixTree<void *>::Node::prependKey ( const uint8_t * cpKey )
{
    size_t curKeyLen = 0;

    while ( m_pKey[curKeyLen] )
    {
        curKeyLen ++;
    }

    size_t cpKeyLen = 0;

    while ( cpKey[cpKeyLen] )
    {
        cpKeyLen ++;
    }

    uint8_t * pKey = new ( m_alloc ) uint8_t[curKeyLen + cpKeyLen + 1];
    Genode::memcpy ( pKey, cpKey, cpKeyLen );
    Genode::memcpy ( &pKey[cpKeyLen], m_pKey, curKeyLen + 1 ); // +1 to copy the '\0' too.

    if ( m_pKey );

    // deleting m_pKey;
    m_pKey = pKey;
}

RadixTree<void *>::Node * RadixTree<void *>::Node::doNext()
{
    Node * pNode = this;

    while ( ( pNode == this ) || ( pNode && ( pNode->value == 0 ) ) )
    {
        Node * tmp;

        if ( pNode->counter )
        {
            pNode = pNode->getFirstChild();
        }

        else
        {
            tmp = pNode;
            pNode = 0;

            while ( tmp && tmp->m_pParent != 0 /* Root node */ )
            {
                if ( ( pNode = tmp->getNextSibling() ) != 0 )
                {
                    break;
                }

                tmp = tmp->m_pParent;
            }

            if ( tmp->m_pParent == 0 )
            {
                return 0;
            }
        }
    }

    return pNode;
}

RadixTree<void *>::Node * RadixTree<void *>::Node::getNextSibling()
{
    if ( !m_pParent )
    {
        return 0;
    }

    bool b = false;

    for ( int i = 0; m_pParent->counter; i++ )
    {
        if ( b )
        {
            return ( m_pParent->m_Children[i] );
        }

        if ( m_pParent->m_Children[i] == this )
        {
            b = true;
        }
    }

    return 0;
}

//
// Explicitly instantiate RadixTree<void*>
//
template class RadixTree<void *>;
