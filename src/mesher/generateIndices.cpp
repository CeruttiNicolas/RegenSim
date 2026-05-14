#include "mesher/Mesher.hpp"
    
void Mesher::generateIndices(const SimulationInput& input, Mesh& mesh) {
    auto start = std::chrono::high_resolution_clock::now();
    std::cout << "Generating indices..." << std::endl;

    int height = input.ni + input.nb + input.no + 1;
    int width = 2 * input.nw + input.na + 1;
    int depth = mesh.vertices.size() / (width * height);

    auto getIndex = [&](int x, int y, int z) {
        return y + (z * height) + (x * height * width);
    };

	int H = height - 1;
	int W = width - 1;
	int D = depth - 1;

	int outerPerimeter = 2 * (H + W);
	int innerPerimeter = 2 * (input.nb + input.na);
	int faceArea = (H * W) - (input.nb * input.na);

	int totalQuads = D * (outerPerimeter + innerPerimeter) + 2 * faceArea;
	int totalTriangles = totalQuads * 2;
    // Each quad is two tris, since each quad has 4 edges and
    // each edge is shared by 2 quads, we have that the number
	// of unique edges is twice the number of quads too.
    int totalEdges = totalTriangles;

    mesh.triangleIndices.reserve(totalTriangles * 3);
    mesh.lineIndices.reserve(totalEdges * 2);

    auto addTris = [&](int bl, int br, int tr, int tl, bool reverseWinding = false) {
        if (reverseWinding) {
            mesh.triangleIndices.push_back(bl); mesh.triangleIndices.push_back(tr); mesh.triangleIndices.push_back(br);
            mesh.triangleIndices.push_back(bl); mesh.triangleIndices.push_back(tl); mesh.triangleIndices.push_back(tr);
        }
        else {
            mesh.triangleIndices.push_back(bl); mesh.triangleIndices.push_back(br); mesh.triangleIndices.push_back(tr);
            mesh.triangleIndices.push_back(bl); mesh.triangleIndices.push_back(tr); mesh.triangleIndices.push_back(tl);
        }
    };

    auto addLine = [&](int v1, int v2) {
        mesh.lineIndices.push_back(v1);
        mesh.lineIndices.push_back(v2);
	};

    // --- Generate Tris ---
	// Front and back faces - X normal
    for (int x : {0, D}) {
		bool isReverseWinding = (x == D);
        for (int z = 0; z < W; z++) {
            for (int y = 0; y < H; y++) {
                if (z >= input.nw && z < input.nw + input.na && y >= input.ni && y < input.ni + input.nb) continue;
                addTris(getIndex(x, y, z), getIndex(x, y, z + 1), getIndex(x, y + 1, z + 1), getIndex(x, y + 1, z), isReverseWinding);
		    }
        }
	}

	// Side faces - Z normal
	int innerWallLeftZ = input.nw;
    int innerWallRightZ = input.nw + input.na;
    int outerWallRightZ = W;
    for (int z : {0, innerWallLeftZ, innerWallRightZ, outerWallRightZ}) {
		bool isInner = (z == innerWallLeftZ || z == innerWallRightZ);
		bool isReverseWinding = (z == innerWallLeftZ || z == outerWallRightZ);
        for (int x = 0; x < D; x++) {
            for (int y = 0; y < H; y++) {
                if (isInner && (y < input.ni || y >= input.ni + input.nb)) continue;
				addTris(getIndex(x, y, z), getIndex(x, y + 1, z), getIndex(x + 1, y + 1, z), getIndex(x + 1, y, z), isReverseWinding);
            }
		}
	}

	// Top and bottom faces - Y normal
    int innerFloorY = input.ni;
    int innerCeilY = input.ni + input.nb;
    int outerCeilY = H;
	for (int y : {0, innerFloorY, innerCeilY, outerCeilY }) {
		bool isInner = (y == innerFloorY || y == innerCeilY);
        bool isReverseWinding = (y == innerFloorY || y == outerCeilY);
        for (int x = 0; x < D; x++) {
            for (int z = 0; z < W; z++) {
				if (isInner && (z < input.nw || z >= input.nw + input.na)) continue;
                addTris(getIndex(x, y, z), getIndex(x + 1, y, z), getIndex(x + 1, y, z + 1), getIndex(x, y, z + 1), isReverseWinding);
			}
		}
	}

    // --- Generate edges ---
    // Front and back faces
    for (int x : {0, D}) {
        for (int z = 0; z < width; z++) {
            for (int y = 0; y < height; y++) {
                bool skipVertical = ((z > input.nw && z < input.nw + input.na) && (y >= input.ni && y < input.ni + input.nb)) || y == H;
                bool skipHorizontal = ((z >= input.nw && z < input.nw + input.na) && (y > input.ni && y < input.ni + input.nb)) || z == W;
                if (!skipVertical) {
                    addLine(getIndex(x, y, z), getIndex(x, y + 1, z));
                }
                if (!skipHorizontal) {
                    addLine(getIndex(x, y, z), getIndex(x, y, z + 1));
                }
            }
		}
    }

    // Side faces
    for (int x = 0; x < D; x++) {
        // Lines along y
        for (int z : {0, innerWallLeftZ, innerWallRightZ, outerWallRightZ}) {
            if (x == 0) break;
            bool isInner = (z == innerWallLeftZ || z == innerWallRightZ);
            for (int y = 0; y < H; y++) {
                if (isInner && (y < input.ni || y >= input.ni + input.nb)) continue;
                addLine(getIndex(x, y, z), getIndex(x, y + 1, z));
            }
        }
        // Lines along z
        for (int y : {0, innerFloorY, innerCeilY, outerCeilY }) {
            if (x == 0) break;
            bool isInner = (y == innerFloorY || y == innerCeilY);
            for (int z = 0; z < W; z++) {
                if (isInner && (z < input.nw || z >= input.nw + input.na)) continue;
                addLine(getIndex(x, y, z), getIndex(x, y, z + 1));
            }
        }
        // Lines along x
        for (int z = 0; z <= W; z++) {
            for (int y = 0; y <= H; y++) {
                bool isOuterSkin = (y == 0 || y == H) || (z == 0 || z == W);
                bool isInnerSkin = ((y == input.ni || y == input.ni + input.nb) && (z >= input.nw && z <= input.nw + input.na)) ||
                    ((z == input.nw || z == input.nw + input.na) && (y >= input.ni && y <= input.ni + input.nb));
                if (isOuterSkin || isInnerSkin) {
                    addLine(getIndex(x, y, z), getIndex(x + 1, y, z));
                }
            }
        }
    }
 
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Generated " << mesh.triangleIndices.size() / 3 << " triangles and " << mesh.lineIndices.size() / 2 << " unique edges in " << elapsed.count() * 1000 << " milliseconds." << std::endl;
}