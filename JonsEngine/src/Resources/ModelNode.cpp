#include "include/Resources/ModelNode.h"

#include <algorithm>

namespace JonsEngine
{
    //
    // ModelNode::ImmediateChildrenIter
    //

    ModelNode::ImmediateChildrenIter::ImmediateChildrenIter(const NodeIterator& iter) : NodeIterator(iter)
    {
    }

    ModelNode::ImmediateChildrenIter& ModelNode::ImmediateChildrenIter::operator++()
    {
        this->_Ptr = this->_Ptr->mNext._Ptr;

        return *this;
    }



    //
    // ModelNode
    //

    ModelNode::ModelNode(const PackageNode& pkgNode, const Mat4& parentTransform, const ImmediateChildrenIterator& immChildIter, const AllChildrenIterator& childIter, const MeshIterator& meshIter, const NodeIterator& next) :
        mName(pkgNode.mName), mLocalAABB(pkgNode.mAABB.mMinBounds, pkgNode.mAABB.mMaxBounds), mLocalTransform(parentTransform * pkgNode.mTransform), mImmediateChildNodes(immChildIter), mAllChildNodes(childIter),
        mMeshes(meshIter), mNext(next)
    {
    }

    ModelNode::ModelNode(const std::string& name, const Vec3& minBounds, const Vec3& maxBounds, const Mat4& initialTransform, const ImmediateChildrenIterator& immChildIter, const AllChildrenIterator& allChildIter,
        const MeshIterator& meshIter, const NodeIterator& next) :
        mName(name), mLocalAABB(minBounds, maxBounds), mLocalTransform(initialTransform), mImmediateChildNodes(immChildIter), mAllChildNodes(allChildIter), mMeshes(meshIter), mNext(next)
    {
    }

    ModelNode::~ModelNode()
    {
    }
}