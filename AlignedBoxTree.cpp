#include "AlignedBoxTree.hpp"
#include "Logger.hpp"

#include <stack>

AlignedBoxNode::AlignedBoxNode() :Box(AlignedBox()), ParentNodeIndex(0xFFFFFFFF), LeftNodeIndex(0xFFFFFFFF), RightNodeIndex(0xFFFFFFFF), NextNodeIndex(0xFFFFFFFF)
{
}

AlignedBoxNode::~AlignedBoxNode()
{
	this->Reset();
}

AlignedBoxNode::AlignedBoxNode(const AlignedBoxNode& node) : Box(node.Box), ParentNodeIndex(node.ParentNodeIndex), LeftNodeIndex(node.LeftNodeIndex), RightNodeIndex(node.RightNodeIndex), NextNodeIndex(node.NextNodeIndex)
{
}

AlignedBoxNode::AlignedBoxNode(AlignedBoxNode&& node) noexcept : Box(std::move(node.Box)), ParentNodeIndex(node.ParentNodeIndex), LeftNodeIndex(node.LeftNodeIndex), RightNodeIndex(node.RightNodeIndex), NextNodeIndex(node.NextNodeIndex)
{
	node.Reset();
}

AlignedBoxNode& AlignedBoxNode::operator=(const AlignedBoxNode& node)
{
	this->Box = node.Box;
	this->ParentNodeIndex = node.ParentNodeIndex;
	this->LeftNodeIndex = node.LeftNodeIndex;
	this->RightNodeIndex = node.RightNodeIndex;
	this->NextNodeIndex = node.NextNodeIndex;

	return *this;
}

AlignedBoxNode& AlignedBoxNode::operator=(AlignedBoxNode&& node) noexcept
{
	this->Box = std::move(node.Box);
	this->ParentNodeIndex = node.ParentNodeIndex;
	this->LeftNodeIndex = node.LeftNodeIndex;
	this->RightNodeIndex = node.RightNodeIndex;
	this->NextNodeIndex = node.NextNodeIndex;
	node.Reset();

	return *this;
}

bool AlignedBoxNode::IsLeaf() const
{
	return this->LeftNodeIndex == 0xFFFFFFFF;
}

bool AlignedBoxNode::Reset()
{
	bool result = true;

	if (!this->Box.Reset())
		result = false;

	this->ParentNodeIndex = 0xFFFFFFFF;
	this->LeftNodeIndex = 0xFFFFFFFF;
	this->RightNodeIndex = 0xFFFFFFFF;
	this->NextNodeIndex = 0xFFFFFFFF;

	if (!result)
		return Logger::SaveMessage("Error: AlignedBoxNode::Reset() - 1.");

	return result;
}

AlignedBoxTree::AlignedBoxTree() : ObjectNodeIndexMap(std::map<size_t, size_t>()), Nodes(std::vector<AlignedBoxNode>()), RootNodeIndex(0xFFFFFFFF), AllocatedNodeCount(0xFFFFFFFF), NextFreeNodeIndex(0xFFFFFFFF), NodeCapacity(0xFFFFFFFF), GrowthSize(0xFFFFFFFF)
{
}

AlignedBoxTree::AlignedBoxTree(const size_t initialSize) : ObjectNodeIndexMap(std::map<size_t, size_t>()), Nodes(std::vector<AlignedBoxNode>()), RootNodeIndex(0xFFFFFFFF), AllocatedNodeCount(0), NextFreeNodeIndex(0), NodeCapacity(initialSize), GrowthSize(initialSize)
{
	this->Nodes.resize(initialSize);

	for (size_t nodeIndex = 0; nodeIndex < initialSize; ++nodeIndex)
	{
		AlignedBoxNode& node = this->Nodes[nodeIndex];
		node.NextNodeIndex = nodeIndex + 1;
	}

	this->Nodes[initialSize - 1].NextNodeIndex = 0xFFFFFFFF;
}

AlignedBoxTree::~AlignedBoxTree()
{
	this->Reset();
}

AlignedBoxTree::AlignedBoxTree(const AlignedBoxTree& tree) : ObjectNodeIndexMap(tree.ObjectNodeIndexMap), Nodes(tree.Nodes), RootNodeIndex(tree.RootNodeIndex), AllocatedNodeCount(tree.AllocatedNodeCount), NextFreeNodeIndex(tree.NextFreeNodeIndex), NodeCapacity(tree.NodeCapacity), GrowthSize(tree.GrowthSize)
{
}

AlignedBoxTree::AlignedBoxTree(AlignedBoxTree&& tree) noexcept : ObjectNodeIndexMap(std::move(tree.ObjectNodeIndexMap)), Nodes(std::move(tree.Nodes)), RootNodeIndex(tree.RootNodeIndex), AllocatedNodeCount(tree.AllocatedNodeCount), NextFreeNodeIndex(tree.NextFreeNodeIndex), NodeCapacity(tree.NodeCapacity), GrowthSize(tree.GrowthSize)
{
	tree.Reset();
}

AlignedBoxTree& AlignedBoxTree::operator=(const AlignedBoxTree& tree)
{
	this->ObjectNodeIndexMap = tree.ObjectNodeIndexMap;
	this->Nodes = tree.Nodes;
	this->RootNodeIndex = tree.RootNodeIndex;
	this->AllocatedNodeCount = tree.AllocatedNodeCount;
	this->NextFreeNodeIndex = tree.NextFreeNodeIndex;
	this->NodeCapacity = tree.NodeCapacity;
	this->GrowthSize = tree.GrowthSize;

	return *this;
}

AlignedBoxTree& AlignedBoxTree::operator=(AlignedBoxTree&& tree) noexcept
{
	this->ObjectNodeIndexMap = std::move(tree.ObjectNodeIndexMap);
	this->Nodes = std::move(tree.Nodes);
	this->RootNodeIndex = tree.RootNodeIndex;
	this->AllocatedNodeCount = tree.AllocatedNodeCount;
	this->NextFreeNodeIndex = tree.NextFreeNodeIndex;
	this->NodeCapacity = tree.NodeCapacity;
	this->GrowthSize = tree.GrowthSize;
	tree.Reset();

	return *this;
}

size_t AlignedBoxTree::AllocateNode()
{
	size_t nodeIndex = 0;

	if (this->NextFreeNodeIndex == 0xFFFFFFFF)
	{
		this->NodeCapacity += this->GrowthSize;
		this->Nodes.resize(this->NodeCapacity);

		for (nodeIndex = this->AllocatedNodeCount; nodeIndex < this->NodeCapacity; ++nodeIndex)
		{
			AlignedBoxNode& node = this->Nodes[nodeIndex];
			node.NextNodeIndex = nodeIndex + 1;
		}

		this->Nodes[this->NodeCapacity - 1].NextNodeIndex = 0xFFFFFFFF;
		this->NextFreeNodeIndex = this->AllocatedNodeCount;
	}

	nodeIndex = this->NextFreeNodeIndex;

	AlignedBoxNode& allocatedNode = this->Nodes[nodeIndex];

	allocatedNode.ParentNodeIndex = 0xFFFFFFFF;
	allocatedNode.LeftNodeIndex = 0xFFFFFFFF;
	allocatedNode.RightNodeIndex = 0xFFFFFFFF;
	this->NextFreeNodeIndex = allocatedNode.NextNodeIndex;
	++this->AllocatedNodeCount;

	return nodeIndex;
}

void AlignedBoxTree::DeallocateNode(const size_t nodeIndex)
{
	AlignedBoxNode& deallocatedNode = this->Nodes[nodeIndex];

	deallocatedNode.NextNodeIndex = this->NextFreeNodeIndex;
	this->NextFreeNodeIndex = nodeIndex;
	--this->AllocatedNodeCount;
}

void AlignedBoxTree::InsertLeaf(const size_t leafNodeIndex)
{
	if (this->RootNodeIndex == 0xFFFFFFFF)
	{
		this->RootNodeIndex = leafNodeIndex;
		return;
	}

	size_t treeNodeIndex = this->RootNodeIndex;
	AlignedBoxNode& leafNode = this->Nodes[leafNodeIndex];

	while (!this->Nodes[treeNodeIndex].IsLeaf())
	{
		const AlignedBoxNode& treeNode = this->Nodes[treeNodeIndex];
		size_t leftNodeIndex = treeNode.LeftNodeIndex, rightNodeIndex = treeNode.RightNodeIndex;
		const AlignedBoxNode& leftNode = this->Nodes[leftNodeIndex];
		const AlignedBoxNode& rightNode = this->Nodes[rightNodeIndex];
		AlignedBox combinedAlignedBox = treeNode.Box.Merge(leafNode.Box);
		float newParentNodeCost = 2.0f * combinedAlignedBox.SurfaceArea(), minimumPushDownCost = 2.0f * (combinedAlignedBox.SurfaceArea() - treeNode.Box.SurfaceArea()), costLeft, costRight;

		if (leftNode.IsLeaf())
			costLeft = leafNode.Box.Merge(leftNode.Box).SurfaceArea() + minimumPushDownCost;

		else
		{
			AlignedBox newLeftAlignedBox = leafNode.Box.Merge(leftNode.Box);
			costLeft = (newLeftAlignedBox.SurfaceArea() - leftNode.Box.SurfaceArea()) + minimumPushDownCost;
		}

		if (rightNode.IsLeaf())
			costRight = leafNode.Box.Merge(rightNode.Box).SurfaceArea() + minimumPushDownCost;

		else
		{
			AlignedBox newRightAlignedBox = leafNode.Box.Merge(rightNode.Box);
			costRight = (newRightAlignedBox.SurfaceArea() - rightNode.Box.SurfaceArea()) + minimumPushDownCost;
		}

		if (newParentNodeCost < costLeft && newParentNodeCost < costRight)
			break;

		if (costLeft < costRight)
			treeNodeIndex = leftNodeIndex;

		else
			treeNodeIndex = rightNodeIndex;
	}

	size_t leafSiblingIndex = treeNodeIndex;
	AlignedBoxNode& leafSibling = this->Nodes[leafSiblingIndex];
	size_t oldParentIndex = leafSibling.ParentNodeIndex, newParentIndex = this->AllocateNode();
	AlignedBoxNode& newParent = this->Nodes[newParentIndex];

	newParent.ParentNodeIndex = oldParentIndex;
	newParent.Box = leafNode.Box.Merge(leafSibling.Box);
	newParent.LeftNodeIndex = leafSiblingIndex;
	newParent.RightNodeIndex = leafNodeIndex;
	leafNode.ParentNodeIndex = newParentIndex;
	leafSibling.ParentNodeIndex = newParentIndex;

	if (oldParentIndex == 0xFFFFFFFF)
		this->RootNodeIndex = newParentIndex;

	else
	{
		AlignedBoxNode& oldParent = this->Nodes[oldParentIndex];

		if (oldParent.LeftNodeIndex == leafSiblingIndex)
			oldParent.LeftNodeIndex = newParentIndex;

		else
			oldParent.RightNodeIndex = newParentIndex;
	}

	treeNodeIndex = leafNode.ParentNodeIndex;
	this->FixUpwardsTree(treeNodeIndex);
}

void AlignedBoxTree::RemoveLeaf(const size_t leafNodeIndex)
{
	if (leafNodeIndex == this->RootNodeIndex)
	{
		this->RootNodeIndex = 0xFFFFFFFF;
		return;
	}

	AlignedBoxNode& leafNode = this->Nodes[leafNodeIndex];
	size_t parentNodeIndex = leafNode.ParentNodeIndex;
	const AlignedBoxNode& parentNode = this->Nodes[parentNodeIndex];
	size_t grandParentNodeIndex = parentNode.ParentNodeIndex, siblingNodeIndex = parentNode.LeftNodeIndex == leafNodeIndex ? parentNode.RightNodeIndex : parentNode.LeftNodeIndex;
	AlignedBoxNode& siblingNode = this->Nodes[siblingNodeIndex];

	if (grandParentNodeIndex != 0xFFFFFFFF)
	{
		AlignedBoxNode& grandParentNode = this->Nodes[grandParentNodeIndex];

		if (grandParentNode.LeftNodeIndex == parentNodeIndex)
			grandParentNode.LeftNodeIndex = siblingNodeIndex;

		else
			grandParentNode.RightNodeIndex = siblingNodeIndex;

		siblingNode.ParentNodeIndex = grandParentNodeIndex;
		this->DeallocateNode(parentNodeIndex);
		this->FixUpwardsTree(grandParentNodeIndex);
	}

	else
	{
		this->RootNodeIndex = siblingNodeIndex;
		siblingNode.ParentNodeIndex = 0xFFFFFFFF;
		this->DeallocateNode(parentNodeIndex);
	}

	leafNode.ParentNodeIndex = 0xFFFFFFFF;
}

void AlignedBoxTree::UpdateLeaf(const size_t leafNodeIndex, const AlignedBox& newAaab)
{
	AlignedBoxNode& node = this->Nodes[leafNodeIndex];

	if (node.Box.Contains(newAaab))
		return;

	this->RemoveLeaf(leafNodeIndex);
	node.Box = newAaab;
	this->InsertLeaf(leafNodeIndex);
}

void AlignedBoxTree::FixUpwardsTree(size_t treeNodeIndex)
{
	while (treeNodeIndex != 0xFFFFFFFF)
	{
		AlignedBoxNode& treeNode = this->Nodes[treeNodeIndex];
		const AlignedBoxNode& leftNode = this->Nodes[treeNode.LeftNodeIndex];
		const AlignedBoxNode& rightNode = this->Nodes[treeNode.RightNodeIndex];

		treeNode.Box = leftNode.Box.Merge(rightNode.Box);
		treeNodeIndex = treeNode.ParentNodeIndex;
	}
}

void AlignedBoxTree::InsertObject(const AlignedBox& object)
{
	size_t nodeIndex = this->AllocateNode();
	AlignedBoxNode& node = this->Nodes[nodeIndex];

	node.Box = object;
	this->InsertLeaf(nodeIndex);
	this->ObjectNodeIndexMap[object.Id] = nodeIndex;
}

void AlignedBoxTree::RemoveObject(const AlignedBox& object)
{
	size_t nodeIndex = this->ObjectNodeIndexMap[object.Id];

	this->RemoveLeaf(nodeIndex);
	this->DeallocateNode(nodeIndex);
	this->ObjectNodeIndexMap.erase(object.Id);
}

void AlignedBoxTree::UpdateObject(const AlignedBox& object)
{
	size_t nodeIndex = this->ObjectNodeIndexMap[object.Id];
	this->UpdateLeaf(nodeIndex, object);
}

std::vector<size_t> AlignedBoxTree::QueryOverlaps(const Frustum& object) const
{
	std::vector<size_t> overlaps;
	std::stack<size_t> stack;

	stack.push(this->RootNodeIndex);

	while (!stack.empty())
	{
		size_t nodeIndex = stack.top();
		stack.pop();

		if (nodeIndex == 0xFFFFFFFF)
			continue;

		const AlignedBoxNode& node = this->Nodes[nodeIndex];

		if (!object.Cull(node.Box))
		{
			if (node.IsLeaf())
				overlaps.push_back(node.Box.Id);

			else
			{
				stack.push(node.LeftNodeIndex);
				stack.push(node.RightNodeIndex);
			}
		}
	}

	return overlaps;
}

bool AlignedBoxTree::Reset()
{
	this->ObjectNodeIndexMap.clear();
	this->Nodes.clear();
	RootNodeIndex = 0xFFFFFFFF;
	AllocatedNodeCount = 0xFFFFFFFF;
	NextFreeNodeIndex = 0xFFFFFFFF;
	NodeCapacity = 0xFFFFFFFF;
	GrowthSize = 0xFFFFFFFF;

	return true;
}