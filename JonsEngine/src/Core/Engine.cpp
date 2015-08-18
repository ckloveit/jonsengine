#include "include/Core/Engine.h"

#include "include/Core/Logging/Logger.h"
#include "include/Core/Memory/HeapAllocator.h"
#include "include/Core/EngineSettings.h"
#include "include/Core/DebugOptions.h"
#include "include/Scene/Scene.h"
#include "include/Window/WindowManager.h"
#include "include/Resources/ResourceManifest.h"
#include "include/Renderer/RenderQueue.h"

#include <exception>
#include <functional>


namespace JonsEngine
{
    Engine::Engine(const EngineSettings& settings) : mLog(Logger::GetCoreLogger()), 
                                                     mMemoryAllocator(HeapAllocator::GetDefaultHeapAllocator().AllocateObject<HeapAllocator>("DefaultHeapAllocator"), 
                                                                      [](HeapAllocator* allocator) { HeapAllocator::GetDefaultHeapAllocator().DeallocateObject(allocator); }),

                                                     mWindow(settings, mMemoryAllocator, mLog), 
                                                     mRenderer(settings, mMemoryAllocator, mLog),
                                                     mResourceManifest(mRenderer, mMemoryAllocator), 
                                                     mSceneManager(mResourceManifest),

                                                     mPrevMinDepth(0.0f),
                                                     mPrevMaxDepth(1.0f)
    {
        JONS_LOG_INFO(mLog, "-------- ENGINE INITIALIZED --------")
    }

    Engine::~Engine()
    {
        JONS_LOG_INFO(mLog, "-------- DESTROYING ENGINE --------")
    }

    void Engine::Tick(const DebugOptions& debugOptions)
    {
        // process input and window events
        mWindow.Poll();

		Scene& activeScene = mSceneManager.GetActiveScene();

        const uint32_t windowWidth = mWindow.GetScreenWidth();
        const uint32_t windowHeight = mWindow.GetScreenHeight();
        const float cameraFov = activeScene.GetSceneCamera().GetFOV();
        const float windowAspectRatio = windowWidth / static_cast<float>(windowHeight);
        const Mat4 cameraProjectionMatrix = PerspectiveMatrixFov(cameraFov, windowAspectRatio, mRenderer.GetZNear(), mRenderer.GetZFar());
        
        // get renderqueue from scene
        const RenderQueue& renderQueue = activeScene.GetRenderQueue(cameraProjectionMatrix, cameraFov, windowAspectRatio, mPrevMinDepth, mPrevMaxDepth);

        // render the scene
        mRenderer.Render(renderQueue, debugOptions.mRenderingFlags);

        // get min/max depth from frame, used in culling and rendering
        // TODO: move elsewhere?
        mRenderer.ReduceDepth(cameraProjectionMatrix, mPrevMinDepth, mPrevMaxDepth);
    }
}