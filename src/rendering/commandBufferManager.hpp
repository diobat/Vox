#pragma once

class rendering_system;

// Third party includes
#include <entt/entt.hpp>
// GLFW
#include "wrapper/glfw.hpp"

struct FrameData
{
    VkCommandBuffer commandBuffer;
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
};

class command_buffer_system
{
public:
    command_buffer_system(rendering_system* core, VkQueue& graphicsQueue);

    void createCommandPools();
    void createCommandBuffers();

    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);

    void recordCommandBuffer(uint32_t frameIndex, uint32_t swapChainImageIndex);
    void resetCommandBuffer(uint32_t frameIndex);
    void submitCommandBuffer(uint32_t frameIndex, VkSemaphore imageAvailableSemaphore, VkSemaphore renderFinishedSemaphore, VkFence fence);

    VkCommandBuffer& getCommandBuffer(uint32_t index) { return _commandBuffers[index]; }

    void cleanup();

private:
    rendering_system* _core;

    const unsigned int& _framesInFlight;

    VkCommandPool _commandPool;                             // command pool
    VkCommandPool _transferCommandPool;                     // transfer command pool
    VkQueue& _graphicsQueue;

    std::vector<VkCommandBuffer> _commandBuffers;           // command buffers, one for each frame
};