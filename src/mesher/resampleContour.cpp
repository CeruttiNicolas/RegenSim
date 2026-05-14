#include "mesher/Mesher.hpp"

#define MAX_ATTEMPTS 100
#define ERROR_THRESHOLD 1e-5

std::vector<glm::dvec3> Mesher::resampleContour(const std::vector<glm::dvec3>& contour, double stepSize) {
    auto start = std::chrono::high_resolution_clock::now();
    std::cout << "Resampling contour..." << std::endl;
    double arcLenght = 0.0;
    for (int i = 0; i < contour.size() - 1; i++) {
        arcLenght += glm::distance(contour[i], contour[i+1]);
    }

    int numPoints = static_cast<int>(std::round(arcLenght / stepSize)) + 1;
    double upperBound = stepSize * 1.5;
    double lowerBound = stepSize * 0.5;

    std::vector<glm::dvec3> resampled;
    double error = std::numeric_limits<double>::infinity();
    int attempt = 0;
    while (error > ERROR_THRESHOLD) {
        double radius = (upperBound + lowerBound) / 2.0;
        resampled.clear();
        glm::dvec3 origin = contour[0];
        resampled.push_back(origin);
        bool done = false;
        for (int i = 0; i < contour.size() - 1; i++) {
            glm::dvec3 vector = contour[i+1] - contour[i];
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
        double lastDistance = glm::distance(contour.back(), resampled.back());
        if (lastDistance > radius) {
            lowerBound = radius;
        } else {
            upperBound = radius;
        }
        error = abs(lastDistance - radius);

        if (attempt > MAX_ATTEMPTS) {
            std::cout << "Resampling did not converge after " << MAX_ATTEMPTS << " attempts with final error being " << error << ", stopping." << std::endl;
            break;
        }
        attempt++;
    }
    resampled.push_back(contour.back());

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Resampling took " << elapsed.count() * 1000 << " milliseconds to generate " << resampled.size() << " points." << std::endl;
    return resampled;
}

