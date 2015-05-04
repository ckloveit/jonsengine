#pragma once

#include "include/Scene/Scene.h"

#include <vector>
#include <memory>

namespace JonsEngine
{
    class IMemoryAllocator;
    struct EngineSettings;

    class SceneManager
    {
    public:
        SceneManager();
        ~SceneManager();

        Scene* CreateScene(const std::string& sceneName);
        void DeleteScene(Scene* scene);
        const std::vector<ScenePtr>& GetAllScenes() const;

		Scene* GetActiveScene() const;
        void SetActiveScene(Scene* scene);
        void SetActiveScene(const std::string& sceneName);


    private:
        IMemoryAllocator& mMemoryAllocator;
        std::vector<ScenePtr> mScenes;

		Scene* mActiveScene;
    };
}