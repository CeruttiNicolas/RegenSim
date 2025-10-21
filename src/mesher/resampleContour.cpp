#include "Mesher.hpp"

#define MAX_ATTEMPTS 100
#define ERROR_THRESHOLD 1e-5

std::vector<glm::vec3> Mesher::resampleContour(const std::vector<glm::vec3>& contour, float stepSize) {
    auto start = std::chrono::high_resolution_clock::now();
    std::cout << "Resampling contour..." << std::endl;
    float arcLenght = 0.0f;
    for (int i = 0; i < contour.size() - 1; i++) {
        arcLenght += glm::distance(contour[i], contour[i+1]);
    }

    int numPoints = static_cast<int>(std::round(arcLenght / stepSize)) + 1;
    float upperBound = arcLenght / (numPoints - 1);
    float lowerBound = arcLenght / (numPoints + 1);

    std::vector<glm::vec3> resampled;
    float error = std::numeric_limits<float>::infinity();
    int attempt = 0;
    while (error > ERROR_THRESHOLD) {
        float radius = (upperBound + lowerBound) / 2.0f;
        resampled.clear();
        glm::vec3 origin = contour[0];
        resampled.push_back(origin);
        bool done = false;
        for (int i = 0; i < contour.size() - 1; i++) {
            glm::vec3 vector = contour[i+1] - contour[i];
            while (glm::distance(contour[i+1], origin) > radius) {
                origin = intersectRaySphere(contour[i], vector, origin, radius);
                resampled.push_back(origin);
                if (resampled.size() == numPoints - 1) {
                    done = true;
                    break;
                }
            }
            if (done) {
                break;  
            }
        }
        float lastDistance = glm::distance(contour.back(), resampled.back());
        if (lastDistance > radius) {
            lowerBound = radius;
        } else {
            upperBound = radius;
        }
        error = abs(lastDistance - radius);

        if (attempt > MAX_ATTEMPTS) {
            std::cout << "Resampling did not converge after " << MAX_ATTEMPTS << " attempts, stopping." << std::endl;
            break;
        }
        attempt++;
    }
    resampled.push_back(contour.back());

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Resampling took " << elapsed.count() << " seconds." << std::endl;
    return resampled;
}

