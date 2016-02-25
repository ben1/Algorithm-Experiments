#pragma once

// Reusing parts of Tree Definitions
// If two trees are based on 90% the same nodes, but a few in the middle are different, can we reuse the trees, or can we only reuse the leaf nodes.
// Is it even worth re-using the leaf nodes? Probably not, unless you had hundreds or thousands of different trees, or the trees had thousands of nodes.
// You could reuse parts of definitions by using a special selection for the node that has been customized, which selects the node to go to based on the version number of the definition being used.
// That node then can eventually point back to nodes in the original tree.

class ThinkTreeNode {
public:
	ThinkTreeNode ();
	virtual ~ThinkTreeNode ();

private:
};


class ThinkTreeBuilder {
public:
	ThinkTreeBuilder ();
	~ThinkTreeBuilder ();

private:
	ThinkTreeNode* m_root;
};


class ThinkTreeDefinition {
public:
	ThinkTreeDefinition (const ThinkTreeBuilder& builder);

	size_t GetRequiredStateSize() const { return m_requiredStateSize; }

private:
	size_t m_requiredStateSize;
};


class ThinkTree
{
public:
	ThinkTree ();
	~ThinkTree ();

	void Reset (const ThinkTreeDefinition* definition);
	void Update();


private:
	const ThinkTreeDefinition* m_definition;

	// size of memory needed is determined by the tree definition 
	uint8_t* m_state;
};

