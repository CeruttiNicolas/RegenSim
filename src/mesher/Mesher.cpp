#include "mesher/Mesher.hpp"

std::vector<glm::dvec3> Mesher::run(SimulationInput& input){
    auto start = std::chrono::high_resolution_clock::now();
    std::cout << "Beginning meshing..." << std::endl;

    double stepSize = input.a;

    std::vector<glm::dvec3> resampledContour = resampleContour(input.contour, stepSize);
    std::vector<glm::dvec3> vertexNormals = computeVertexNormals(resampledContour);
    
    double ac = input.ac;
    double at = input.at;
    double ae = input.ae;
    double bc = input.bc;
    double bt = input.bt;
    double be = input.be;
    
    double chamberRadius = input.chamber.y;
    double throatRadius = input.throat.y;
    double exitRadius = input.exit.y;

    int numVertices = resampledContour.size() * (input.ni + input.nb + input.no + 1) * (input.nw * 2 + input.na + 1);
    std::vector<glm::dvec3> meshVertices;
    meshVertices.reserve(numVertices);
    
    int throatIndex = getThroatIndex(resampledContour);
    for (int i = 0; i < resampledContour.size(); i++) {
        glm::dvec3 point = resampledContour[i];
        double a, b;
        if (i < throatIndex) {
            double alpha = (point.y - throatRadius) / (chamberRadius - throatRadius);
            a = lerp(at, ac, alpha);
            b = lerp(bt, bc, alpha);
        } else {
            double alpha = (point.y - throatRadius) / (exitRadius - throatRadius);
            a = lerp(at, ae, alpha);
            b = lerp(bt, be, alpha);
        }
        std::vector<glm::dvec3> section = generateSection(input, point, a, b);
        glm::dvec3 normal = vertexNormals[i];
        section = placeSection(section, point, normal);
        meshVertices.insert(meshVertices.end(), section.begin(), section.end());
    }
	std::vector<double> volumes = computeVolumes(input, meshVertices);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Meshing took " << elapsed.count() << " seconds to generate " << meshVertices.size() << " vertices." << std::endl;
    return meshVertices;
}
