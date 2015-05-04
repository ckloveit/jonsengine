#include "include/Scene/SceneNode.h"
#include "include/Core/Utils/Math.h"
#include "include/Core/Memory/HeapAllocator.h"

#include "boost/functional/hash.hpp"
#include <algorithm>

namespace JonsEngine
{
    SceneNode::SceneNode(const std::string& nodeName, const OnSceneNodeDirtyFunc& onDirty) :
        mName(nodeName), mHashedID(boost::hash_value(nodeName)), mMemoryAllocator(HeapAllocator::GetDefaultHeapAllocator()), mWorldMatrix(1.0f), mOrientation(1.0f, 0.0f, 0.0f, 0.0f),
        mScale(1.0f, 1.0f, 1.0f), mTranslation(0.0f, 0.0f, 0.0f), mOnDirtyFunc(onDirty)
    {
    }
        
    SceneNode::~SceneNode()
    {
    }


	bool SceneNode::operator==(const SceneNode& s1)
    {
        return mHashedID == s1.mHashedID;
    }
        
    bool SceneNode::operator==(const std::string& nodeName)
    {
        return mHashedID == boost::hash_value(nodeName);
    }


    SceneNodePtr SceneNode::CreateChildNode(const std::string& nodeName)
    {
        mChildNodes.emplace_back(SceneNodePtr(HeapAllocator::GetDefaultHeapAllocator().AllocateObject<SceneNode, const std::string&>(nodeName, mOnDirtyFunc), std::bind(&HeapAllocator::DeallocateObject<SceneNode>, &HeapAllocator::GetDefaultHeapAllocator(), std::placeholders::_1)));

        return mChildNodes.back();
    }
        
    SceneNodePtr SceneNode::FindChildNode(const std::string& nodeName)
    {
        SceneNodePtr ret;
        auto iter = std::find_if(mChildNodes.begin(), mChildNodes.end(), [nodeName](const SceneNodePtr node) { return node->GetNodeName() == nodeName; });

        if (iter == mChildNodes.end())
        {
            for(SceneNodePtr node : mChildNodes)
            {
                ret = node->FindChildNode(nodeName);
                if (ret && *ret == nodeName)
                    break;
            }
        }
        else
            ret = *iter;

        return ret;
    }
        
    bool SceneNode::RemoveChildNode(const std::string& nodeName)
    {
        bool isFound = false;
        auto iter = std::find_if(mChildNodes.begin(), mChildNodes.end(), [nodeName](const SceneNodePtr node) { return node->GetNodeName() == nodeName; });

        if (iter != mChildNodes.end())
        {
            mChildNodes.erase(iter);
            isFound = true;
        }
        else
        {
            for (SceneNodePtr childNode : mChildNodes)
            {
                isFound = childNode->RemoveChildNode(nodeName);
                if (isFound)
                    break;
            }
        }

        return isFound;
    }
        
    bool SceneNode::RemoveChildNode(SceneNodePtr node)
    {
        bool isFound = false;
        auto iter = std::find(mChildNodes.begin(), mChildNodes.end(), node);

        if (iter != mChildNodes.end())
        {
            mChildNodes.erase(iter);
            isFound = true;
        }
        else
        {
            for (SceneNodePtr childNode : mChildNodes)
            {
                isFound = childNode->RemoveChildNode(node);
                if (isFound)
                    break;
            }
        }

        return isFound;
    }

        
    void SceneNode::ScaleNode(const Vec3& scaleVec)                                
    { 
        mScale *= scaleVec;

        mOnDirtyFunc(this);
    }

    void SceneNode::TranslateNode(const Vec3& translateVec)
    { 
        mTranslation += translateVec;

        mOnDirtyFunc(this);
    }

    void SceneNode::RotateNode(const float angle, const Vec3& rotateVec)       
    { 
        Quaternion rotation = glm::angleAxis(glm::radians(angle), rotateVec);
        mOrientation = mOrientation * rotation;

        mOnDirtyFunc(this);
    }


    void SceneNode::UpdateWorldMatrix()
    {
        UpdateTransform();

        for (SceneNodePtr childNode : mChildNodes)
            childNode->UpdateChildren(mWorldMatrix);
    }


    Vec3 SceneNode::Position() const
    {
        Vec3 position(0.0f);
        position += mTranslation;

        return position;
    }


    void SceneNode::UpdateTransform()
    {
        // TODO: necessary?
        mWorldMatrix = Mat4(1.0f);

        mWorldMatrix = glm::translate(mWorldMatrix, mTranslation);
        mWorldMatrix *= glm::toMat4(mOrientation);
        mWorldMatrix = glm::scale(mWorldMatrix, mScale);
    }

    void SceneNode::UpdateChildren(const Mat4& parentModelMatrix)
    {
        UpdateTransform();

        mWorldMatrix = parentModelMatrix * mWorldMatrix;

        for (SceneNodePtr childNode : mChildNodes)
            childNode->UpdateChildren(mWorldMatrix);
    }
}