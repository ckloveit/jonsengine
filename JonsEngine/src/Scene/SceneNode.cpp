#include "include/Scene/SceneNode.h"
#include "include/Core/Math/Math.h"
#include "include/Core/Memory/HeapAllocator.h"

namespace JonsEngine
{
    SceneNode::SceneNode(const std::string& nodeName, const SceneNodeID parent, const OnSceneNodeDirtyFunc& onDirty) :
        mName(nodeName), mWorldTransform(gIdentityMatrix), mOrientation(1.0f, 0.0f, 0.0f, 0.0f), mScale(1.0f), mTranslation(0.0f), mParent(parent), mOnDirtyFunc(onDirty)
    {
    }
        
    SceneNode::~SceneNode()
    {
    }

        
    void SceneNode::ScaleNode(const float scale)
    { 
        mScale *= scale;

        mOnDirtyFunc(this);
    }

    void SceneNode::TranslateNode(const Vec3& translateVec)
    { 
        mTranslation += translateVec;

        mOnDirtyFunc(this);
    }

    void SceneNode::RotateNode(const float angle, const Vec3& rotateVec)       
    { 
        mOrientation = glm::rotate(mOrientation, glm::radians(angle), rotateVec);

        mOnDirtyFunc(this);
    }


    void SceneNode::UpdateWorldMatrix(const Mat4& parentMatrix)
    {
        mWorldTransform = glm::translate(gIdentityMatrix, mTranslation);
        mWorldTransform *= glm::toMat4(mOrientation);
        mWorldTransform = glm::scale(mWorldTransform, mScale);
        mWorldTransform = parentMatrix * mWorldTransform;


        //UpdateTransform();

        //for (SceneNodePtr childNode : mChildNodes)
        //    childNode->UpdateChildren(mTransform.Get());
    }


    const Vec3& SceneNode::Position() const
    {
        return mTranslation;
    }

    const Vec3& SceneNode::Scale() const
    {
        return mScale;
    }
}
