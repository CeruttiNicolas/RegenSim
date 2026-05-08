#include "VulkanRenderer.hpp"
#include <chrono>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdexcept>



void VulkanRenderer::createUniformBuffers() {
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);

        vkMapMemory(device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
    }
}

void VulkanRenderer::updateUniformBuffer(uint32_t currentImage) {
    UniformBufferObject ubo{};
	ubo.model = glm::mat4(1.0f);

    float maxPitch = glm::radians(89.0f);
    if (cameraPitch > maxPitch) cameraPitch = maxPitch;
    if (cameraPitch < -maxPitch) cameraPitch = -maxPitch;

    // Costruisci la matrice di vista (View Matrix)
    ubo.view = glm::mat4(1.0f);
    // Allontana la camera dal target lungo l'asse Z locale
    ubo.view = glm::translate(ubo.view, glm::vec3(0.0f, 0.0f, -cameraDistance));
    // Applica il pitch (rotazione su/giù attorno all'asse X locale)
    ubo.view = glm::rotate(ubo.view, cameraPitch, glm::vec3(1.0f, 0.0f, 0.0f));
    // Applica lo yaw (rotazione destra/sinistra attorno all'asse Y globale)
    ubo.view = glm::rotate(ubo.view, cameraYaw, glm::vec3(0.0f, 1.0f, 0.0f));
    // Trasla l'intero mondo in modo che il target diventi il centro di rotazione
    ubo.view = glm::translate(ubo.view, -cameraTarget);

    ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 1.0f, 100000.0f);
    ubo.proj[1][1] *= -1;

    memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}