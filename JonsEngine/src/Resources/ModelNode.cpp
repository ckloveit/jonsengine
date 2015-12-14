#include "include/Resources/ModelNode.h"

#include <algorithm>

namespace JonsEngine
{
    uint32_t CountNumMeshes(const ModelNode::MeshIterator& meshIter);


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
        mName(pkgNode.mName), mNodeIndex(pkgNode.mNodeIndex), mLocalAABB(pkgNode.mAABB.mMinBounds, pkgNode.mAABB.mMaxBounds), mLocalTransform(parentTransform * Mat4(1.0f)), mNumMeshes(CountNumMeshes(meshIter)), mImmediateChildNodes(immChildIter), mAllChildNodes(childIter),
        mMeshes(meshIter), mNext(next)
    {
    }

    ModelNode::ModelNode(const std::string& name, const ModelNodeIndex nodeIndex, const Vec3& minBounds, const Vec3& maxBounds, const Mat4& initialTransform, const ImmediateChildrenIterator& immChildIter, const AllChildrenIterator& allChildIter,
        const MeshIterator& meshIter, const NodeIterator& next) :
        mName(name), mNodeIndex(nodeIndex), mLocalAABB(minBounds, maxBounds), mLocalTransform(Mat4(1.0f)), mNumMeshes(CountNumMeshes(meshIter)), mImmediateChildNodes(immChildIter), mAllChildNodes(allChildIter), mMeshes(meshIter), mNext(next)
    {
    }

    ModelNode::~ModelNode()
    {
    }

    uint32_t ModelNode::GetNumMeshes() const
    {
        return mNumMeshes;
    }

    const std::string& ModelNode::GetName() const
    {
        return mName;
    }

    ModelNodeIndex ModelNode::GetModelNodeIndex() const
    {
        return mNodeIndex;
    }

    const AABB& ModelNode::GetLocalAABB() const
    {
        return mLocalAABB;
    }

    const Mat4& ModelNode::GetLocalTransform() const
    {
        return mLocalTransform;
    }


    ModelNode::MeshIterator ModelNode::GetMeshes() const
    {
        return mMeshes;
    }

    ModelNode::AllChildrenIterator ModelNode::GetAllChildren() const
    {
        return mAllChildNodes;
    }

    ModelNode::ImmediateChildrenIterator ModelNode::GetImmediateChildren() const
    {
        return mImmediateChildNodes;
    }


    uint32_t CountNumMeshes(const ModelNode::MeshIterator& meshIter)
    {
        uint32_t ret = 0;

        for (const Mesh& mesh : meshIter)
            ++ret;

        return ret;
    }
}