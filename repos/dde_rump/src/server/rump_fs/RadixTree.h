#ifndef RADIX_H
#define RADIX_H


extern "C" {
#include <sys/cdefs.h>
#include <sys/types.h>
}
#include <base/allocator_avl.h>

template<class E>
class RadixTree;

/** Tree specialisation for void* */
template<>
class RadixTree<void *> {
    private:
        /** Tree node. */
        class Node {
            public:
                typedef Node * childlist_t[50];
                enum MatchType
                {
                    ExactMatch,   ///< Key matched node key exactly.
                    NoMatch,      ///< Key didn't match node key at all.
                    PartialMatch, ///< A subset of key matched the node key.
                    OverMatch     ///< Key matched node key, and had extra characters.
                };

                Node ( bool bCaseSensitive ) :
                    m_pKey ( 0 ), value ( 0 ), m_pParent ( 0 ),
                    m_bCaseSensitive ( bCaseSensitive ), counter ( 0 ) {
                }

                ~Node ();

                /** Get the next data structure in the list
                 *\return pointer to the next data structure in the list */
                Node * next() {
                    return doNext();
                }
                /** Get the previous data structure in the list
                 *\return pointer to the previous data structure in the list
                 *\note Not implemented! */
                Node * previous() {
                    return 0;
                }

                /** Locates a child of this node, given the key portion of key
                    (lookahead on the first token) */
                Node * findChild ( const uint8_t * cpKey );

                /** Adds a new child. */
                void addChild ( Node * pNode );

                /** Replaces a currently existing child. */
                void replaceChild ( Node * pNodeOld, Node * pNodeNew );

                /** Removes a child (doesn't delete it) */
                void removeChild ( Node * pChild );

                /** Compares cpKey and this node's key, returning the type of match
                    found. */
                MatchType matchKey ( const uint8_t * cpKey );

                /** Returns the first found child of the node. */
                Node * getFirstChild();

                /** Sets the node's key to the concatenation of \p cpKey and the
                 *  current key.
                 *\param cpKey Key to prepend to the current key. */
                void prependKey ( const uint8_t * cpKey );

                void setKey ( const uint8_t * cpKey );
                inline uint8_t * getKey() {
                    return m_pKey;
                }
                inline void setValue ( void * pV ) {
                    value = pV;
                }
                inline void * getValue() {
                    return value;
                }
                inline void setParent ( Node * pP ) {
                    m_pParent = pP;
                }
                inline Node * getParent() {
                    return m_pParent;
                }

                /** Node key, zero terminated. */
                uint8_t * m_pKey;
                /** Node value.
                    \note Parting from coding standard because Iterator requires the
                          member be called 'value'. */
                void * value;
                /** Array of 16 pointers to 16 nodes (256 total). */
                childlist_t m_Children;
                /** Parent node. */
                Node * m_pParent;
                /** Controls case-sensitive matching. */
                bool m_bCaseSensitive;
                uint8_t counter;

            private:
                Node ( const Node & );
                Node & operator = ( const Node & );

                /** Returns the next Node to look at during an in-order iteration. */
                Node * doNext();

                /** Returns the node's next sibling, by looking at its parent's children. */
                Node * getNextSibling();
        };

    public:

        /** The default constructor, does nothing */
        RadixTree();
        /** The copy-constructor
         *\param[in] x the reference object to copy */
        RadixTree ( const RadixTree<void *> & x );
        /** Constructor that offers case sensitivity adjustment. */
        RadixTree ( bool bCaseSensitive );
        /** The destructor, deallocates memory */
        ~RadixTree();

        /** The assignment operator
         *\param[in] x the object that should be copied */
        RadixTree & operator = ( const RadixTree & x );

        /** Get the number of elements in the Tree
         *\return the number of elements in the Tree */
        size_t count() const;
        /** Add an element to the Tree.
         *\param[in] key the key
         *\param[in] value the element */
        void insert ( char * key, void * value );
        /** Attempts to find an element with the given key.
         *\return the element found, or NULL if not found. */
        void * lookup ( char * key ) const;
        /** Attempts to remove an element with the given key. */
        void remove ( char * key );

        /** Clear the tree. */
        void clear();




        static Genode::Allocator * m_alloc;

    private:
        /** Internal function to create a copy of a subtree. */
        Node * cloneNode ( Node * node, Node * parent );

        /** Number of items in the tree. */
        size_t m_nItems;
        /** The tree's root. */
        Node * m_pRoot;
        /** Whether matches are case-sensitive or not. */
        bool m_bCaseSensitive;
};

/** RadixTree template specialisation for pointers. Just forwards to the
 * void* template specialisation of RadixTree.
 *\brief RadixTree template specialisation for pointers */
template<class T>
class RadixTree<T *> {
    public:
        /** Default constructor, does nothing */
        inline RadixTree()
            : m_VoidRadixTree() {}
        /** Copy-constructor
         *\param[in] x reference object */
        inline RadixTree ( const RadixTree & x )
            : m_VoidRadixTree ( x.m_VoidRadixTree ) {}
        /** Constructor with case-sensitivity. */
        inline RadixTree ( bool bCaseSensitive )
            : m_VoidRadixTree ( bCaseSensitive ) {}
        /** Destructor, deallocates memory */
        inline ~RadixTree() {
        }

        /** Assignment operator
         *\param[in] x the object that should be copied */
        inline RadixTree & operator = ( const RadixTree & x ) {
            m_VoidRadixTree = x.m_VoidRadixTree;
            return *this;
        }

        /** Get the number of elements in the RadixTree */
        inline size_t count() const {
            return m_VoidRadixTree.count();
        }

        inline void insert ( char * key, T * value ) {
            m_VoidRadixTree.insert ( key, reinterpret_cast<void *> ( value ) );
        }
        inline T * lookup ( char * key ) const {
            return reinterpret_cast<T *> ( m_VoidRadixTree.lookup ( key ) );
        }
        inline void remove ( char * key ) {
            m_VoidRadixTree.remove ( key );
        }


        /** Remove all elements from the RadixTree */
        inline void clear() {
            m_VoidRadixTree.clear();
        }


    private:
        /** The actual container */
        RadixTree<void *> m_VoidRadixTree;
};

/** @} */

#endif

