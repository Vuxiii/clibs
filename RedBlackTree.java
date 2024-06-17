public class RedBlackTree {
    public static void main( String[] args ) {

        // Node node5 = new Node( 5, new Node( 2, true ), new Node( 7, true ), false );
        // Node node20 = new Node( 20, null, new Node( 23, true ), false );
        // Node node18 = new Node( 18, node5, node20, true );
        // Node node26 = new Node( 26, null, new Node( 27, true ), false );
        
        // Node root = new Node( 24, node18, node26, false );

        // Tree tree = new Tree( root );

        // tree.setParents();
        // tree.inorder();
        // System.out.println();
        // tree.insertNode( new Node( 21, true ) );
        // tree.inorder();

        Tree tree = new Tree( new Node( 24 ) );
        tree.insertNode( new Node( 18 ) );
        tree.insertNode( new Node( 26 ) );
        tree.insertNode( new Node( 5 ) );
        tree.insertNode( new Node( 20 ) );
        tree.insertNode( new Node( 27 ) );
        tree.insertNode( new Node( 2 ) );
        tree.insertNode( new Node( 7 ) );
        tree.insertNode( new Node( 23 ) );
        tree.insertNode( new Node( 21 ) );

        tree.inorder();

        // Node node10 = new Node( 10, new Node( 8, true ), new Node( 11, true ), false );
        // Node node22 = new Node( 22, null, new Node( 26, true ), false );
        // Node node18 = new Node( 18, node10, node22, true );
        
        // Node root = new Node( 7, new Node( 3, false ), node18, false  );

        // Tree tree = new Tree( root );
        
        // tree.setParents();
        // tree.insertNode( new Node( 29, false ) );
        // tree.inorder();

        // tree.delete( tree.find( 29 ) );
        // tree.inorder();
    }

    public static class Node {
        public int key;

        public Node parent = Tree.nil;
        public Node left = Tree.nil;
        public Node right = Tree.nil;

        public boolean isRed;
        
        Node( int key, Node left, Node right, boolean isRed ) {
            this.key = key;
            this.left = left == null ? Tree.nil : left;
            this.right = right == null ? Tree.nil : right;
            this.isRed = isRed;
            
        }

        Node( int key, boolean isRed ) {
            this.key = key;
            this.isRed = isRed;
        }

        Node( int key ) {
            this.key = key;
            this.isRed = false;
        }

        public String toString() {
            return (isRed ? "Red" : "Blk") + "-Node " + key + ", left " + left.key + ", right " + right.key + ", parent " + parent.key;
        }
    }

    public static class Tree {

        private static Node nil = new Node( -1, null, null, false );

        Node root;

        Tree( Node root ) {
            this.root = root;
            this.root.parent = Tree.nil;
        }

        /**
         * Sets each childs parent to the node above it (It's parent).
         */
        public void setParents() {
            setParents( root );
        }

        private void setParents( Node current ) {
            if ( !current.equals( Tree.nil ) ) {
                if ( !current.left.equals( Tree.nil ) ) current.left.parent = current;
                if ( !current.right.equals( Tree.nil ) ) current.right.parent = current;
                setParents( current.left );
                setParents( current.right );
            }
        }

        public Node find( int key ) {
            Node current = root;
            while( !current.equals( Tree.nil) && current.key != key )
                current = key < current.key ? current.left : current.right;
            return current;
        }

        public void insertNode( Node newNode ) {
            Node current = root;
            Node parent = Tree.nil;
            // Find the spot to insert the newNode
            while ( !current.equals( Tree.nil ) ) {
                parent = current;
                if ( newNode.key < current.key ) 
                    current = current.left;
                else
                    current = current.right;
            }
            // Sets the new node's parent.
            newNode.parent = parent;
            // Place the node in the tree
            if ( parent.equals( Tree.nil ) )
                root = newNode;
            else if ( newNode.key < parent.key ) 
                parent.left = newNode;
            else
                parent.right = newNode;
            
            newNode.isRed = true;
            insertFixUp( newNode );
        }

        private void insertFixUp( Node node ) {
            while ( node.parent.isRed ) {
                if ( node.parent.equals( node.parent.parent.left ) ) {
                    Node y = node.parent.parent.right;
                    if ( y.isRed ) {
                        node.parent.isRed = false;
                        y.isRed = false;
                        node.parent.parent.isRed = true;
                        node = node.parent.parent;
                    } else {
                        if ( node.equals( node.parent.right ) ) {
                            node = node.parent;
                            leftRotate( node );
                        }
                        node.parent.isRed = false;
                        node.parent.parent.isRed = true;
                        rightRotate( node.parent.parent );
                    }
                } else {
                    Node y = node.parent.parent.left;
                    if ( y.isRed ) {
                        node.parent.isRed = false;
                        y.isRed = false;
                        node.parent.parent.isRed = true;
                        node = node.parent.parent;
                    } else {
                        if ( node.equals( node.parent.left ) ) {
                            node = node.parent;
                            rightRotate( node );
                        }
                        node.parent.isRed = false;
                        node.parent.parent.isRed = true;
                        leftRotate( node.parent.parent );
                    }
                }
            }
            root.isRed = false;
        }

        private void leftRotate( Node x ) {
            Node y = x.right;
            x.right = y.left;
            if ( !y.left.equals( Tree.nil )) 
                y.left.parent = x;
            y.parent = x.parent;
            if ( x.parent.equals( Tree.nil ) )
                root = y;
            else if ( x.equals( x.parent.left ) ) 
                x.parent.left = y;
            else
                x.parent.right = y;
            y.left = x;
            x.parent = y;
        }

        private void rightRotate( Node x ) {
            Node y = x.left;
            x.left = y.right;
            if ( !y.right.equals( Tree.nil )) 
                y.right.parent = x;
            y.parent = x.parent;
            if ( x.parent.equals( Tree.nil ) )
                root = y;
            else if ( x.equals( x.parent.right ) ) 
                x.parent.right = y;
            else
                x.parent.left = y;
            y.right = x;
            x.parent = y;
        }

        private void transplant( Node u, Node v ) {
            if ( u.parent.equals( Tree.nil ) ) 
                root = v;
            else if ( u.equals( u.parent.left ) )
                u.parent.left = v;
            else
                u.parent.right = v;
            v.parent = u.parent;
        }

        public Node treeMin( Node x ) {
            while ( !x.left.equals( Tree.nil ) )
                x = x.left;
            return x;
        }

        private void deleteFixup( Node x ) {
            while( !x.equals( Tree.nil ) && !x.isRed ) {
                if ( x.equals( x.parent.left ) ) {
                    Node w = x.parent.right;
                    if ( w.isRed ) {
                        w.isRed = false;
                        x.parent.isRed = true;
                        leftRotate( x.parent );
                        w = x.parent.right;
                    }
                    if ( w.left.isRed == false && w.right.isRed == false ) {
                        w.isRed = true;
                        x = x.parent;
                    } else if ( w.right.isRed == false ) {
                        w.left.isRed = false;
                        w.isRed = true;
                        rightRotate( w );
                        w = x.parent.right;
                    }
                    w.isRed = x.parent.isRed;
                    x.parent.isRed = false;
                    w.right.isRed = false;
                    leftRotate( x.parent );
                    x = root;
                } else {
                    Node w = x.parent.left;
                    if ( w.isRed ) {
                        w.isRed = false;
                        x.parent.isRed = true;
                        rightRotate( x.parent );
                        w = x.parent.left;
                    }
                    if ( w.right.isRed == false && w.left.isRed == false ) {
                        w.isRed = true;
                        x = x.parent;
                    } else if ( w.left.isRed == false ) {
                        w.right.isRed = false;
                        w.isRed = true;
                        leftRotate( w );
                        w = x.parent.left;
                    }
                    w.isRed = x.parent.isRed;
                    x.parent.isRed = false;
                    w.left.isRed = false;
                    rightRotate( x.parent );
                    x = root;
                }
            }
            x.isRed = false;
        }

        public void delete( Node z ) {
            Node x;
            Node y = z;
            boolean yOriColor = y.isRed;

            if ( z.left.equals( Tree.nil ) ) {
                x = z.right;
                transplant( z, z.right );
            } else if ( z.right.equals( Tree.nil ) ) {
                x = z.left;
                transplant( z, z.left );
            } else {
                y = treeMin( z.right );
                yOriColor = y.isRed;
                x = y.right;
                if ( y.parent.equals( z ) )
                    x.parent = y;
                else {
                    transplant( y, y.right );
                    y.right = z.right;
                    y.right.parent = y;
                }
                transplant( z, y );
                y.left = z.left;
                y.left.parent = y;
                y.isRed = z.isRed;
            }
            if ( !yOriColor ) {
                deleteFixup( x );
            }
        }

        public void inorder() {
            inorder( root );
        }

        private void inorder( Node current ) {
            if ( !current.equals( Tree.nil ) ) {
                inorder( current.left );
                // DO CODE
                System.out.println( current );
                
                inorder( current.right );
            }
        }
    }
}
