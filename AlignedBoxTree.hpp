#ifndef AlignedBoxTree_H
#define AlignedBoxTree_H

#include "AlignedBox.hpp"
#include "Frustum.hpp"

#include <map>
#include <vector>

class AlignedBoxNode
{
public:
	AlignedBox Box;
	size_t ParentNodeIndex, LeftNodeIndex, RightNodeIndex, NextNodeIndex;

	AlignedBoxNode();
	~AlignedBoxNode();
	AlignedBoxNode(const AlignedBoxNode&);
	AlignedBoxNode(AlignedBoxNode&&) noexcept;
	AlignedBoxNode& operator=(const AlignedBoxNode&);
	AlignedBoxNode& operator=(AlignedBoxNode&&) noexcept;

	bool IsLeaf() const;
	bool Reset();
};

class AlignedBoxTree
{
public:
	std::map<size_t, size_t> ObjectNodeIndexMap;
	std::vector<AlignedBoxNode> Nodes;
	size_t RootNodeIndex, AllocatedNodeCount, NextFreeNodeIndex, NodeCapacity, GrowthSize;

	AlignedBoxTree();
	AlignedBoxTree(const size_t);
	~AlignedBoxTree();
	AlignedBoxTree(const AlignedBoxTree&);
	AlignedBoxTree(AlignedBoxTree&&) noexcept;
	AlignedBoxTree& operator=(const AlignedBoxTree&);
	AlignedBoxTree& operator=(AlignedBoxTree&&) noexcept;

	size_t AllocateNode();
	void DeallocateNode(const size_t);
	void InsertLeaf(const size_t);
	void RemoveLeaf(const size_t);
	void UpdateLeaf(const size_t, const AlignedBox&);
	void FixUpwardsTree(size_t);

	void InsertObject(const AlignedBox&);
	void RemoveObject(const AlignedBox&);
	void UpdateObject(const AlignedBox&);
	std::vector<size_t> QueryOverlaps(const Frustum&) const;

	bool Reset();
};

#endif