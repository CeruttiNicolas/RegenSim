#include "graphics/VulkanRenderer.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


void VulkanRenderer::initWindow(const char* windowName) { // TODO check if std::string can be used instead
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    //glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(WIDTH, HEIGHT, windowName, nullptr, nullptr);

    glfwSetWindowUserPointer(window, this);

    // Callback per la rotella del mouse (Zoom)
    glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset) {
        auto app = reinterpret_cast<VulkanRenderer*>(glfwGetWindowUserPointer(window));
        app->cameraDistance *= static_cast<float>(yoffset) * 0.025f + 1;
        if (app->cameraDistance < 1.0f) app->cameraDistance = 1.0f;
		if (app->cameraDistance > 10000.0f) app->cameraDistance = 10000.0f;
        });

    // Callback per i bottoni del mouse
    glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {
        auto app = reinterpret_cast<VulkanRenderer*>(glfwGetWindowUserPointer(window));

        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            if (action == GLFW_PRESS) app->isLeftMouseDown = true;
            else if (action == GLFW_RELEASE) app->isLeftMouseDown = false;
        }
        else if (button == GLFW_MOUSE_BUTTON_MIDDLE) { // Intercept the mouse wheel click
            if (action == GLFW_PRESS) app->isMiddleMouseDown = true;
            else if (action == GLFW_RELEASE) app->isMiddleMouseDown = false;
        }
        });

    // Callback per il movimento del mouse
    glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos) {
        auto app = reinterpret_cast<VulkanRenderer*>(glfwGetWindowUserPointer(window));

        float deltaX = static_cast<float>(xpos - app->lastMouseX);
        float deltaY = static_cast<float>(ypos - app->lastMouseY);

        if (app->isLeftMouseDown) {
            // --- Orbit (Rotation) ---
            float rotSensitivity = 0.005f;
            app->cameraYaw += deltaX * rotSensitivity;
            app->cameraPitch += deltaY * rotSensitivity;
        }
        else if (app->isMiddleMouseDown) {
            // --- Panning Standard (Screen-Space Translation) ---
            float panSensitivity = 0.001f * app->cameraDistance;

            // 1. Ricrea la ESATTA sequenza di rotazione usata nel tuo updateUniformBuffer
            glm::mat4 viewRot = glm::mat4(1.0f);
            viewRot = glm::rotate(viewRot, app->cameraPitch, glm::vec3(1.0f, 0.0f, 0.0f));
            viewRot = glm::rotate(viewRot, app->cameraYaw, glm::vec3(0.0f, 1.0f, 0.0f));

            // 2. Inverti la matrice. 
            // La matrice di vista va dal "Mondo" alla "Camera". La sua inversa va dalla "Camera" al "Mondo".
            glm::mat4 cameraTransform = glm::inverse(viewRot);

            // 3. Ora le colonne 0 e 1 sono ESATTAMENTE l'asse X (Destra) e l'asse Y (Alto) della telecamera
            glm::vec3 right = glm::vec3(cameraTransform[0]);
            glm::vec3 up = glm::vec3(cameraTransform[1]);

            // Applica la traslazione sottraendo la x (per andare a destra) e aggiungendo la y (per andare in alto)
            app->cameraTarget -= right * deltaX * panSensitivity;
            app->cameraTarget += up * deltaY * panSensitivity;
        }

        app->lastMouseX = xpos;
        app->lastMouseY = ypos;
        });
}

void VulkanRenderer::initVulkan() {
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createImageViews();
    createRenderPass();
    createDescriptorSetLayout();
    createGraphicsPipeline();
    createDepthResources();
    createFrameBuffers();
    createCommandPool();
    createVertexBuffer();
    createIndexBuffer();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffers();
    createSyncObjects();
}

void VulkanRenderer::mainLoop() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        drawFrame();
    }

    vkDeviceWaitIdle(device);
}

void VulkanRenderer::cleanupSwapChain() {
    for (auto framebuffer : swapChainFramebuffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }

    for (auto imageView : swapChainImageViews) {
        vkDestroyImageView(device, imageView, nullptr);
    }
    vkDestroyImageView(device, depthImageView, nullptr);
    vkDestroyImage(device, depthImage, nullptr);
    vkFreeMemory(device, depthImageMemory, nullptr);
    vkDestroySwapchainKHR(device, swapChain, nullptr);
}

void VulkanRenderer::cleanup() {
    cleanupSwapChain();
   
    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyRenderPass(device, renderPass, nullptr);

    vkDestroyBuffer(device, indexBuffer, nullptr);
    vkFreeMemory(device, indexBufferMemory, nullptr);

    vkDestroyBuffer(device, vertexBuffer, nullptr);
    vkFreeMemory(device, vertexBufferMemory, nullptr);

    for (size_t i = 0; i < swapChainImages.size(); i++) {
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(device, inFlightFences[i], nullptr);
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyBuffer(device, uniformBuffers[i], nullptr);
        vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
    }

    vkDestroyDescriptorPool(device, descriptorPool, nullptr);

    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

    vkDestroyCommandPool(device, commandPool, nullptr);

    vkDestroyDevice(device, nullptr);

    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }

    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(window);
    
    glfwTerminate();
}

