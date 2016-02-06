#pragma once

template<class T>
class BinarySearchTree
{
public:
	BinarySearchTree()
		: m_root(0)
	{
	}

	~BinarySearchTree()
	{
		// delete all the nodes, so find the leaf nodes, starting from m_root
		Node* n = m_root;

		// we use the m_next pointers to point back to parents so that we don't have to allocate memory for a stack.
		n->m_next = 0; // root has no parent

		while (n != 0)
		{
			if (n->m_left != 0)
			{
				// point the child back to the parent
				n->m_left->m_next = n;
				n = n->m_left;
			}
			else if (n->m_right != 0)
			{
				// point the child back to the parent
				n->m_right->m_next = n;
				n = n->m_right;
			}
			else
			{
				// we've reached the end of a branch, so delete the node
				Node* nodeToDelete = n;

				// go back to the parent
				n = n->m_next;

				if (n != 0)
				{
					// clear the parent's pointer to the deleted child
					if (n->m_left == nodeToDelete)
					{
						n->m_left = 0;
					}
					else if (n->m_right == nodeToDelete)
					{
						n->m_right = 0;
					}
				}

				delete nodeToDelete;
			}
		}
	}

	struct Node
	{
		Node(const T& a_value)
			: m_left(0)
			, m_right(0)
			, m_next(0)
			, m_value(a_value)
		{
		}
		Node* m_left;
		Node* m_right;
		Node* m_next;
		T m_value;
	};


	void Insert(const T& a_value)
	{
		Node** n = &m_root;
		while (*n != 0)
		{
			n = ((*n)->m_value > a_value) ? &((*n)->m_left) : &((*n)->m_right);
		}
		*n = new Node(a_value);
	}

	Node* GetRoot() const { return m_root; }



private:
	
	Node* m_root;
};

