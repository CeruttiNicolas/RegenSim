#include "mesher/Mesher.hpp"

void Mesher::generateVertices(const SimulationInput& input, Mesh& mesh) {
	auto start = std::chrono::high_resolution_clock::now();
	std::cout << "Generating point cloud..." << std::endl;

    double stepSize = input.a;
    std::vector<glm::dvec3> resampledContour = resampleContour(input.contour, stepSize);
    std::vector<glm::dvec3> vertexNormals = computeVertexNormals(resampledContour);

    double ac = input.ac, at = input.at, ae = input.ae;
    double bc = input.bc, bt = input.bt, be = input.be;
    double chamberRadius = input.chamber.y;
    double throatRadius = input.throat.y;
    double exitRadius = input.exit.y;

    int numVertices = resampledContour.size() * (input.ni + input.nb + input.no + 1) * (input.nw * 2 + input.na + 1);
    
    mesh.vertices.reserve(numVertices);

    int throatIndex = getThroatIndex(resampledContour);
    for (int i = 0; i < resampledContour.size(); i++) {
        glm::dvec3 point = resampledContour[i];
        double a, b;
        if (i < throatIndex) {
            double alpha = (point.y - throatRadius) / (chamberRadius - throatRadius);
            a = lerp(at, ac, alpha);
            b = lerp(bt, bc, alpha);
        }
        else {
            double alpha = (point.y - throatRadius) / (exitRadius - throatRadius);
            a = lerp(at, ae, alpha);
            b = lerp(bt, be, alpha);
        }

        std::vector<glm::dvec3> section = generateSection(input, point, a, b);
        glm::dvec3 normal = vertexNormals[i];
        section = placeSection(section, point, normal);

        mesh.vertices.insert(mesh.vertices.end(), section.begin(), section.end());
    }
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed = end - start;
	std::cout << "Generated " << mesh.vertices.size() << " vertices in " << elapsed.count() * 1000 << " milliseconds." << std::endl;
}