#pragma once

#include "include/Scene/Scene.h"

#include <string>

namespace JonsEngine
{
    class IMemoryAllocator;
    class ResourceManifest;

    typedef uint32_t SceneID;

    class SceneManager
    {
    public:
        SceneManager(const ResourceManifest& resourceManifest);
        ~SceneManager();

        SceneID CreateScene(const std::string& sceneName);
        Scene& GetScene(const SceneID sceneID);
        // TODO - deferred because of fixing moveassignable Scenes...
        //void DeleteScene(SceneID& scene);

		SceneID GetActiveScene() const;
        Scene& GetActiveScene();
        void SetActiveScene(const SceneID scene);


    private:
        const ResourceManifest& mResourceManifest;

        IDMap<Scene> mScenes;
        SceneID mActiveScene;
    };
}