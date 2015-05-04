#pragma once

#include "include/Core/Types.h"

#include <string>
#include <memory>
#include <functional>

namespace JonsEngine
{
    class DirectionalLight
    {
    public:
        DirectionalLight(const std::string& name);

        bool operator==(const DirectionalLight& light);
        bool operator==(const std::string& lightName);


        const std::string mName;

        Vec4 mLightColor;
        Vec3 mLightDirection;


	private:
		const size_t mHashedID;
    };

	typedef std::unique_ptr<DirectionalLight, std::function<void(DirectionalLight*)>> DirectionalLightPtr;
}