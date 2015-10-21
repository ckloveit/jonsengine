#include "include/Resources/ModelAnimation.h"

namespace JonsEngine
{
    ModelAnimation::ModelAnimation(const PackageAnimation& pkgAnimation) : mName(pkgAnimation.mName), mAnimationDuration(Milliseconds(pkgAnimation.mDurationInMilliseconds))
    {
        auto nodeBeginIndex = mNodeTransforms.size();
        for (const PackageAnimatedNode& animNode : pkgAnimation.mAnimatedNodes)
        {
            for (const PackageAnimatedNodeTransform& nodeTransform : animNode.mAnimationTransforms)
                mNodeTransforms.emplace_back(nodeTransform.mTransform, nodeTransform.mTimestampMilliseconds);

            mNodeIDMapping.emplace_back(animNode.mNodeID, nodeBeginIndex);
            nodeBeginIndex = mNodeTransforms.size();
        }
    }

    ModelAnimation::~ModelAnimation()
    {
    }


    const Mat4& ModelAnimation::GetNodeTransform(const ModelNodeID nodeID, const Milliseconds elapsedTime) const
    {
        auto nodeMapIter = std::find_if(mNodeIDMapping.cbegin(), mNodeIDMapping.cend(), [nodeID](const NodeIDMap& nodeIDMap) { return nodeID == nodeIDMap.first; });
        assert(nodeMapIter != mNodeIDMapping.end());

        //auto nodeTransformIter = std::find_if(mNodeTransforms.cbegin(), mNodeTransforms.cend(), [elapsedTime](const NodeTransformTimestamp& nodeIDMap) { return nodeID == nodeIDMap.first; });
        //assert(nodeTransformIter != mNodeTransforms.cend());

        // TODO...
        return mNodeTransforms.front().first;
    }
    
    AnimationID ModelAnimation::GetAnimationID() const
    {
        return mAnimationID;
    }
    
    Milliseconds ModelAnimation::GetAnimationDuration() const
    {
        return mAnimationDuration;
    }

    const std::string& ModelAnimation::GetName() const
    {
        return mName;
    }
}