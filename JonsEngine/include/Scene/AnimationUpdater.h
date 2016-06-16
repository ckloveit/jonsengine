#pragma once

#include "include/Resources/Bone.h"
#include "include/Resources/Animation.h"
#include "include/Resources/AnimationInstance.h"
#include "include/Core/Containers/IDMap.hpp"
#include "include/Core/Types.h"
#include "include/Core/Utils/Time.h"
#include "include/Core/Utils/IDGenerator.hpp"

#include <vector>
#include <unordered_map>

namespace JonsEngine
{
    class ResourceManifest;

    class AnimationUpdater
    {
    public:
		AnimationUpdater(const ResourceManifest& resourceManifest);
        ~AnimationUpdater();

        void Update(const Milliseconds elapsedTime);

        AnimationInstanceID PlayAnimation(const AnimationID animationID);
        void StopAnimation(AnimationInstanceID& animationInstance);

		const BoneTransforms& GetBonedata() const;
		const BoneIndexRange& GetBoneRange(const AnimationInstanceID instanceID) const;


    private:
        typedef std::vector<AnimationInstance> AnimationInstances;
        typedef std::unordered_map<AnimationInstanceID, AnimationInstance&> AnimationInstanceMap;

        Mat4 InterpolateTransform(const BoneIndex index, const Milliseconds elapsedTime);


        const ResourceManifest& mResourceManifest;
		IDGenerator32 mAnimInstanceIDGen;

        AnimationInstances mActiveAnimations;
        AnimationInstanceMap mAnimationInstanceMap;
		BoneTransforms mBoneTransforms;
    };
}