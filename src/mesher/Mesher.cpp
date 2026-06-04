#include "mesher/Mesher.hpp"

Mesh Mesher::run(SimulationInput& input){
    auto start = std::chrono::high_resolution_clock::now();
    std::cout << "Beginning meshing..." << std::endl;
	
    Mesh mesh;
	generateVertices(input, mesh);
	generateIndices(input, mesh);
	computeVolumes(input, mesh);
	computeAreas(input, mesh);
	computeDistances(input, mesh);
	computeShortestEdgeLength(input, mesh);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Meshing took " << elapsed.count() * 1000 << " milliseconds" << std::endl;
	std::cout << "\n--- Meshing Report ----------" << std::endl;
	std::cout << "  Total Vertices   : " << mesh.vertices.size() << std::endl;
	std::cout << "  Total Triangles  : " << mesh.triangleIndices.size() / 3 << std::endl;
	std::cout << "  Total Cells      : " << mesh.volumes.size() - input.na * input.nb * mesh.Nx << std::endl;
	std::cout << "  Shortest Edge    : " << mesh.shortestEdgeLength << std::endl;
	std::cout << "-----------------------------\n" << std::endl;
    return mesh;
}
