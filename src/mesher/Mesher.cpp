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
    return mesh;
}
